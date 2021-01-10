#include "include/utility.h"
#include "process.h"

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

#include <fmt/format.h>

namespace fs = std::filesystem;

namespace apdebug::Process
{
    static const size_t SharedMemorySize = 1 * 1024 * 1024; // 1 MiB
    static const size_t cgroupNamelength = 20;

    int Process::wait() const
    {
        int status;
        waitpid(nativeHandle, &status, 0);
        return WEXITSTATUS(status);
    }
    inline void Process::terminate() const
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
        ptr = mmap(nullptr, SharedMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    }
    SharedMemory::SharedMemory(const char* name)
        : created(false)
    {
        fd = shm_open(name, O_RDWR, S_IWGRP | S_IWOTH);
        ptr = mmap(nullptr, SharedMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
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
        , pointers(other.pointers)
    {
        if (&other == this)
            return;
        for (unsigned int i = 0; i < 3; ++i)
            fd[i] = other.fd[i];
        other.pointers = nullptr;
    }
    Command& Command::appendArgument(const std::string_view val)
    {
        args.emplace_back(val);
        return *this;
    }
    Command& Command::replace(fmt::format_args args)
    {
        for (auto& i : this->args)
            i = fmt::vformat(i, args);
        return *this;
    }
    Command& Command::finalizeForExec()
    {
        pointers = new const char*[args.size() + 2];
        pointers[0] = path.data();
        std::transform(args.cbegin(), args.cend(), pointers + 1, std::mem_fn(&std::string::c_str));
        pointers[args.size() + 1] = nullptr;
        return *this;
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
            execvp(pointers[0], const_cast<char** const>(pointers));
        }
        Process ret;
        ret.nativeHandle = p;
        return ret;
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
    Command& Command::setRedirect(const RedirectType which, const char* file)
    {
        setRedirect(which, open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR));
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
        delete[] pointers;
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
        cntrl = killParam { .pid = p.nativeHandle, .killed = false };
        timer_settime(timer, 0, &spec, nullptr);
        int stat;
        waitpid(p.nativeHandle, &stat, 0);
        return std::make_pair(cntrl.killed, WEXITSTATUS(stat));
    }
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
        swapaccount = getenv("swapaccount");
#endif
    }
    void MemoryLimit::setLimit(const MemoryUsage kb)
    {
#if cgroupVersion == 1
        if (swapAccount)
            Utility::writeFile(cgroup / "memory.memsw.limit_in_bytes", kb * 1024);
        else
        {
            Utility::writeFile(cgroup / "memory.limit_in_bytes", kb * 1024);
            Utility::writeFile(cgroup / "memory.swappiness", 0);
        }
#else
        Utility::writeFile(cgroup / "memory.max", kb * 1024);
        Utility::writeFile(cgroup / "memory.swap.max", 0);
#endif
    }
    void MemoryLimit::addProcess(const Process& p)
    {
        Utility::writeFile(cgroup / "cgroup.procs", p.nativeHandle);
    }
    bool MemoryLimit::isExceed()
    {
        const unsigned int old = count;
#if cgroupVersion == 1
        count = readProperty(cgroup / (swapAccount ? "memory.memsw.failcnt" : "memory.failcnt"));
#else
        count = readProperty(cgroup / "memory.events", "oom_kill");
#endif
        return count > old;
    }
    MemoryLimit::~MemoryLimit()
    {
        fs::remove(cgroup);
    }

    MemoryUsage getMemoryUsage()
    {
        rusage ru;
        getrusage(RUSAGE_SELF, &ru);
        return ru.ru_maxrss;
    }
    TimeUsage getTimeUsage()
    {
        rusage ru;
        getrusage(RUSAGE_SELF, &ru);
        return TimeUsage {
            .real = ru.ru_utime.tv_usec + ru.ru_stime.tv_usec,
            .user = ru.ru_utime.tv_usec,
            .sys = ru.ru_stime.tv_usec
        };
    }
}
