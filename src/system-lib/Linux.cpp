#include "include/io.h"
#include "system.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <iterator>
#include <regex>
#include <utility>

#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <err.h>

#include <fmt/format.h>

namespace fs = std::filesystem;

namespace apdebug::System
{
    static const size_t SharedMemorySize = 1 * 1024 * 1024; // 1 MiB
    static const size_t cgroupNamelength = 20;
    static const size_t maxArgsSize = 500;

    Process::Process(NativeHandle v)
        : nativeHandle(v) {};
    int Process::wait() const
    {
        int status;
        waitpid(nativeHandle, &status, 0);
        return WEXITSTATUS(status);
    }
    void Process::terminate() const
    {
        kill(nativeHandle, SIGKILL);
    }

    SharedMemory::SharedMemory()
        : created(true)
    {
        name[0] = '/';
        randomName(name + 1, shmNameLength - 1);
        name[shmNameLength] = '\0';
        fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
        ftruncate(fd, SharedMemorySize);
        ptr = reinterpret_cast<char*>(mmap(nullptr, SharedMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    }
    SharedMemory::SharedMemory(const char* name)
        : created(false)
    {
        fd = shm_open(name, O_RDWR, S_IWGRP | S_IWOTH);
        ptr = reinterpret_cast<char*>(mmap(nullptr, SharedMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    }
    SharedMemory::~SharedMemory()
    {
        munmap(ptr, SharedMemorySize);
        close(fd);
        if (created)
            shm_unlink(name);
    }

    Command::Command(Command&& other)
        : path(other.path)
        , args(std::move(other.args))
    {
        if (&other == this)
            return;
        for (unsigned int i = 0; i < 3; ++i)
        {
            fd[i] = other.fd[i];
            other.fd[i] = -1;
        }
        other.args.clear();
    }
    Command& Command::appendArgument(const std::string_view val)
    {
        args.emplace_back(val.data());
        return *this;
    }
    Command& Command::instantiate(fmt::format_args args)
    {
        const size_t cnt = templateArgs->size();
        this->args.reserve(cnt + 1);
        this->args.push_back(path.data());
        for (size_t i = 1; i < cnt; ++i)
        {
            char* buf = new char[maxArgsSize + 1];
            fmt::vformat_to(buf, templateArgs->at(i), args);
            this->args.push_back(buf);
        }
        return *this;
    }
    Command& Command::instantiate()
    {
        args.push_back(path.data());
        args.reserve(1 + templateArgs->size());
        std::copy(templateArgs->cbegin(), templateArgs->cend(), std::back_inserter(args));
    }
    Process Command::execute() const
    {
        const pid_t p = fork();
        if (!p)
        {
            if (fd[0] != -1)
                dup2(fd[0], STDIN_FILENO);
            if (fd[1] != -1)
                dup2(fd[1], STDOUT_FILENO);
            if (fd[2] != -1)
                dup2(fd[2], STDERR_FILENO);
            execvp(args.front(), const_cast<char**>(args.data()));
            exit(1);
        }
        return Process(p);
    }
    Command& Command::setRedirect(const RedirectType which, const int fd)
    {
        switch (which)
        {
        case RedirectType::StdIn:
            this->fd[0] = fd;
            break;
        case RedirectType::StdOut:
            this->fd[1] = fd;
            break;
        case RedirectType::StdErr:
            this->fd[2] = fd;
            break;
        }
        return *this;
    }
    Command& Command::setRedirect(const RedirectType which, const fs::path& file)
    {
        setRedirect(which, open(file.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR));
        created[static_cast<unsigned int>(which)] = true;
        return *this;
    }
    void Command::parseArgument(int& argc, const char* const argv[])
    {
        if (std::strcmp(argv[argc], "["))
        {
            args.push_back(argv[argc]);
            return;
        }
        unsigned int stk = 1;
        ++argc;
        for (; stk; ++argc)
        {
            if (!std::strcmp(argv[argc], "]") && !(--stk))
                break;
            else if (!std::strcmp(argv[argc], "["))
                ++stk;
            args.push_back(argv[argc]);
        }
    }
    std::ostream& operator<<(std::ostream& os, const Command& c)
    {
        os << std::quoted(c.path);
        for (const auto& i : c.args)
            os << " " << std::quoted(i);
        return os;
    }
    Command::~Command()
    {
        for (unsigned int i = 0; i < 3; ++i)
            if (created[i])
                close(fd[i]);
        for (unsigned int i = 1; i < args.size(); ++i)
            delete[] args[i];
    }

    static void killer(union ::sigval v)
    {
        killParam* t = reinterpret_cast<killParam*>(v.sival_ptr);
        t->killed = true;
        kill(t->pid, SIGKILL);
    }
    void TimeLimit::create()
    {
        sigev.sigev_notify = SIGEV_THREAD;
        sigev.sigev_value.sival_ptr = &cntrl;
        sigev.sigev_notify_function = killer;
        sigev.sigev_notify_attributes = nullptr;
        id = timer_create(CLOCK_MONOTONIC, &sigev, &timer);
    }
    void TimeLimit::setExpire(const unsigned long long us)
    {
        spec = itimerspec {
            .it_interval = {},
            .it_value {
                .tv_sec = us / 1000000,
                .tv_nsec = (us % 1000000) * 1000 }
        };
    }
    std::pair<bool, int> TimeLimit::waitFor(const Process& p)
    {
        static const itimerspec stop {};
        cntrl = killParam { .pid = p.nativeHandle, .killed = false };
        timer_settime(timer, 0, &spec, nullptr);
        int stat;
        waitpid(p.nativeHandle, &stat, 0);
        if (!cntrl.killed)
            timer_settime(timer, 0, &stop, nullptr);
        return std::make_pair(cntrl.killed, WEXITSTATUS(stat));
    }
    bool TimeLimit::isExceed() { return cntrl.killed; }
    TimeLimit::~TimeLimit()
    {
        timer_delete(timer);
    }

    template <class T = size_t>
    static T readProperty(const fs::path& p, const std::string key)
    {
        std::ifstream f(p);
        while (f)
        {
            std::string k;
            f >> k;
            if (k == key)
            {
                T val;
                f >> val;
                return val;
            }
            f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        return 0;
    }
    MemoryLimit::MemoryLimit()
        : count(0)
    {
        std::string tmp;
        tmp.reserve(cgroupNamelength);
        randomName(std::back_inserter(tmp), cgroupNamelength);
        cgroup = getenv("cgroup") / fs::path(tmp);
        fs::create_directory(cgroup);

#if cgroupVersion == 1
        swapAccount = getenv("swapaccount");
#endif
    }
    void MemoryLimit::setLimit(const MemoryUsage kb)
    {
#if cgroupVersion == 1
        if (swapAccount)
            Output::writeFile(cgroup / "memory.memsw.limit_in_bytes", kb * 1024);
        else
        {
            Output::writeFile(cgroup / "memory.limit_in_bytes", kb * 1024);
            Output::writeFile(cgroup / "memory.swappiness", 0);
        }
#else
        Output::writeFile(cgroup / "memory.max", kb * 1024);
        Output::writeFile(cgroup / "memory.swap.max", 0);
#endif
    }
    void MemoryLimit::addProcess(const Process& p)
    {
        Output::writeFile(cgroup / "cgroup.procs", p.nativeHandle);
    }
    bool MemoryLimit::isExceed()
    {
        const unsigned int old = count;
#if cgroupVersion == 1
        count = Input::readFileVal<unsigned int>(cgroup / (swapAccount ? "memory.memsw.failcnt" : "memory.failcnt"));
#else
        count = readProperty(cgroup / "memory.events", "oom_kill");
#endif
        return count > old;
    }
    MemoryLimit::~MemoryLimit()
    {
        fs::remove(cgroup);
    }

    std::pair<TimeUsage, MemoryUsage> getUsage()
    {
        timespec tm;
        rusage ru;
        getrusage(RUSAGE_SELF, &ru);
        clock_gettime(CLOCK_MONOTONIC, &tm);
        return {
            TimeUsage {
                .real = tm.tv_sec * 1000000ull + tm.tv_nsec / 1000,
                .user = ru.ru_utime.tv_sec * 1000000ull + ru.ru_utime.tv_usec,
                .sys = ru.ru_stime.tv_sec * 1000000ull + ru.ru_stime.tv_usec },
            MemoryUsage { ru.ru_maxrss }
        };
    }
    TimeUsage getTimeUsage()
    {
        return getUsage().first;
    }

    Pipe::Pipe()
    {
        int fd[2];
        pipe(fd);
        read = fd[0];
        write = fd[1];
    }
    Pipe::~Pipe()
    {
        close(read);
        close(write);
    }

    bool createPageAt(void* const address, const size_t len)
    {
        return mmap(address, len, PROT_READ | PROT_WRITE, MAP_ANON | MAP_FIXED | MAP_PRIVATE, -1, 0) != MAP_FAILED;
    }
    void protectPage(void* const address, const size_t len, const bool write)
    {
        mprotect(address, len, write ? PROT_READ | PROT_WRITE : PROT_READ);
    }
}
