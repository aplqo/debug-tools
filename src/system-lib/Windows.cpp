#include "include/io.h"
#include "system.h"

#include <Psapi.h>
#include <windows.h>

#include <charconv>
#include <cstring>
#include <iomanip>
#include <thread>

namespace apdebug::System
{
    static const size_t sharedMemorySize = 1 * 1024 * 1024; // 1 MiB
    static const SECURITY_ATTRIBUTES attrInherit {
        .nLength = sizeof(SECURITY_ATTRIBUTES),
        .lpSecurityDescriptor = nullptr,
        .bInheritHandle = true
    };
    static HANDLE stdIO[3];

    Process::Process(NativeHandle v)
        : nativeHandle(v)
        , owns(false) {};
    Process::Process(Process&& other)
        : nativeHandle(other.nativeHandle)
        , owns(other.owns)
    {
        other.owns = false;
    }
    Process::~Process()
    {
        if (owns)
            CloseHandle(nativeHandle);
    }
    int Process::wait() const
    {
        WaitForSingleObject(nativeHandle, INFINITE);
        DWORD ret;
        GetExitCodeProcess(nativeHandle, &ret);
        return ret;
    }
    void Process::terminate() const
    {
        TerminateProcess(nativeHandle, 9);
        WaitForSingleObject(nativeHandle, INFINITE);
    }

    SharedMemory::SharedMemory()
    {
        SECURITY_ATTRIBUTES sec = attrInherit;
        fd = CreateFileMapping(INVALID_HANDLE_VALUE, &sec, PAGE_READWRITE, 0, sharedMemorySize, NULL);
        ptr = reinterpret_cast<char*>(MapViewOfFile(fd, FILE_MAP_ALL_ACCESS, 0, 0, sharedMemorySize));
        *fmt::format_to(name, "{}", reinterpret_cast<unsigned long long>(fd)) = '\0';
    }
    SharedMemory::SharedMemory(const char* name)
    {
        fd = reinterpret_cast<HANDLE>(std::stoull(name));
        ptr = reinterpret_cast<char*>(MapViewOfFile(fd, FILE_MAP_ALL_ACCESS, 0, 0, sharedMemorySize));
    }
    SharedMemory::~SharedMemory()
    {
        UnmapViewOfFile(ptr);
        CloseHandle(fd);
    }

    Command::Command()
    {
        ZeroMemory(&info, sizeof(info));
        info.cb = sizeof(info);
        info.hStdInput = stdIO[0];
        info.hStdOutput = stdIO[1];
        info.hStdError = stdIO[2];
    }
    Command& Command::appendArgument(const std::string_view arg)
    {
        instantiated = true;
        cmdline += Output::writeToString(" ", std::quoted<char, std::char_traits<char>>(arg));
        return *this;
    }
    Command& Command::instantiate(fmt::format_args args)
    {
        instantiated = true;
        if (templateCmdline)
            cmdline = fmt::vformat(*templateCmdline, args);
        return *this;
    }
    Command& Command::instantiate()
    {
        if (templateCmdline)
            cmdline = *templateCmdline;
        return *this;
    }
    Process Command::execute()
    {
        PROCESS_INFORMATION pinfo;
        cmdline = path.data() + (" " + cmdline);
        CreateProcessA(NULL, const_cast<char*>(cmdline.data()), NULL, NULL, true, 0, NULL, NULL, &info, &pinfo);
        CloseHandle(pinfo.hThread);
        Process ret(pinfo.hProcess);
        ret.owns = true;
        return ret;
    }
    static inline HANDLE open(const wchar_t* file)
    {
        SECURITY_ATTRIBUTES sec = attrInherit;
        return CreateFileW(file,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
            &sec,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, NULL);
    }
    Command& Command::setRedirect(RedirectType typ, const std::filesystem::path& file)
    {
        info.dwFlags = STARTF_USESTDHANDLES;
        switch (typ)
        {
        case RedirectType::StdIn:
            info.hStdInput = openFile[0] = open(file.c_str());
            break;
        case RedirectType::StdOut:
            info.hStdOutput = openFile[1] = open(file.c_str());
            break;
        case RedirectType::StdErr:
            info.hStdError = openFile[2] = open(file.c_str());
            break;
        }
        return *this;
    }
    Command& Command::setRedirect(RedirectType typ, HANDLE had)
    {
        info.dwFlags = STARTF_USESTDHANDLES;
        switch (typ)
        {
        case RedirectType::StdIn:
            info.hStdInput = had;
            break;
        case RedirectType::StdOut:
            info.hStdOutput = had;
            break;
        case RedirectType::StdErr:
            info.hStdError = had;
            break;
        }
        return *this;
    }
    void Command::parseArgument(int& argc, const char* const argv[])
    {
        std::string* tmpl = new std::string;
        templateCmdline = tmpl;
        if (std::strcmp(argv[argc], "["))
        {
            tmpl->append(Output::writeToString(std::quoted<char>(argv[argc])));
            return;
        }
        unsigned int dep = 1;
        ++argc;
        for (; dep; ++argc)
        {
            if (!std::strcmp(argv[argc], "]") && !--dep)
                break;
            else if (!std::strcmp(argv[argc], "["))
                ++dep;
            *tmpl += Output::writeToString(" ", std::quoted<char>(argv[argc]));
        }
    }
    std::ostream& operator<<(std::ostream& os, const Command& c)
    {
        return os << (c.instantiated ? c.cmdline : *c.templateCmdline);
    }
    Command::~Command()
    {
        for (auto i : openFile)
            if (i != INVALID_HANDLE_VALUE)
                CloseHandle(i);
    }

    std::pair<bool, int> TimeLimit::waitFor(const Process& p)
    {
        if (WaitForSingleObject(p.nativeHandle, time) == WAIT_TIMEOUT)
        {
            p.terminate();
            exceed = true;
            return std::make_pair(true, 9);
        }
        exceed = false;
        DWORD ret;
        GetExitCodeProcess(p.nativeHandle, &ret);
        return std::make_pair(false, ret);
    }

    void watchThread(KillParam* const kp)
    {
        ULONG_PTR key;
        LPOVERLAPPED over;
        DWORD msg;
        while (GetQueuedCompletionStatus(kp->iocp, &msg, &key, &over, INFINITE))
        {
            if (msg == JOB_OBJECT_MSG_JOB_MEMORY_LIMIT)
            {
                TerminateJobObject(kp->job, 9);
                ++(kp->exceed);
            }
            else if (key)
                return;
        }
    }
    MemoryLimit::MemoryLimit()
    {
        job = CreateJobObject(NULL, NULL);
        iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
        cntrl = KillParam {
            .job = job,
            .iocp = iocp,
            .exceed = 0
        };
        {
            JOBOBJECT_ASSOCIATE_COMPLETION_PORT ass {
                .CompletionKey = 0, .CompletionPort = iocp
            };
            SetInformationJobObject(job, JobObjectAssociateCompletionPortInformation, &ass, sizeof(ass));
        }
        exceed = 0;
        watch = std::thread(watchThread, &cntrl);
    }
    void MemoryLimit::setLimit(const MemoryUsage kb)
    {

        JOBOBJECT_EXTENDED_LIMIT_INFORMATION lim {
            .BasicLimitInformation = JOBOBJECT_BASIC_LIMIT_INFORMATION {
                .LimitFlags = JOB_OBJECT_LIMIT_JOB_MEMORY },
            .JobMemoryLimit = kb * 1024
        };
        SetInformationJobObject(job, JobObjectBasicLimitInformation, &lim, sizeof(lim));
    }
    void MemoryLimit::addProcess(const Process& p)
    {
        AssignProcessToJobObject(job, p.nativeHandle);
    }
    bool MemoryLimit::isExceed()
    {
        const unsigned int old = exceed;
        exceed = cntrl.exceed;
        return exceed > old;
    }
    MemoryLimit::~MemoryLimit()
    {
        PostQueuedCompletionStatus(iocp, 0, 1, 0);
        watch.join();
        CloseHandle(iocp);
        CloseHandle(job);
    }

    Pipe::Pipe()
    {
        SECURITY_ATTRIBUTES sec = attrInherit;
        CreatePipe(&read, &write, &sec, 0);
    }
    Pipe::~Pipe()
    {
        CloseHandle(read);
        CloseHandle(write);
    }
    TimeUsage getTimeUsage()
    {
        static LARGE_INTEGER dur {};
        if (!dur.QuadPart)
            QueryPerformanceFrequency(&dur);
        FILETIME create, exit, user, sys;
        LARGE_INTEGER cur;
        QueryPerformanceCounter(&cur);
        GetProcessTimes(GetCurrentProcess(), &create, &exit, &sys, &user);
        return TimeUsage {
            .real = static_cast<unsigned long long>(cur.QuadPart / (dur.QuadPart / 1000000)),
            .user = ((static_cast<unsigned long long>(user.dwHighDateTime) << std::numeric_limits<DWORD>::digits) | user.dwLowDateTime) / 10,
            .sys = ((static_cast<unsigned long long>(sys.dwHighDateTime) << std::numeric_limits<DWORD>::digits) | sys.dwLowDateTime) / 10
        };
    }
    static MemoryUsage getMemoryUsage()
    {
        PROCESS_MEMORY_COUNTERS val;
        GetProcessMemoryInfo(GetCurrentProcess(), &val, sizeof(val));
        return val.PeakWorkingSetSize / 1024;
    }
    std::pair<TimeUsage, MemoryUsage> getUsage()
    {
        const TimeUsage tu = getTimeUsage();
        return std::pair(tu, getMemoryUsage());
    }

    void systemInit()
    {
        stdIO[0] = GetStdHandle(STD_INPUT_HANDLE);
        stdIO[1] = GetStdHandle(STD_OUTPUT_HANDLE);
        stdIO[2] = GetStdHandle(STD_ERROR_HANDLE);
    }

    bool createPageAt(void* const address, const size_t size)
    {
        return !VirtualAlloc(address, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    }
    void protectPage(void* const address, const size_t size)
    {
        DWORD old;
        VirtualProtect(address, size, write ? PAGE_READWRITE : PAGE_READONLY, &old);
    }
}