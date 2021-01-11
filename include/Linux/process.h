#ifndef PROCESS_H
#define PROCESS_H

#include "include/process_common.h"
#include <filesystem>
#include <initializer_list>
#include <ostream>
#include <ranges>
#include <regex>
#include <signal.h>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <utility>
#include <vector>

#include <fmt/format.h>

namespace apdebug::Process
{
    const unsigned int shmNameLength = 8;

    struct Process
    {
        typedef pid_t NativeHandle;
        Process() = default;
        Process(const NativeHandle v);
        Process(const Process&) = delete;
        Process(Process&&) = default;
        Process& operator=(const Process&) = delete;

        int wait() const;
        void terminate() const;

        NativeHandle nativeHandle;

    private:
    };

    class SharedMemory
    {
    public:
        SharedMemory();
        SharedMemory(const char* name);
        SharedMemory(const SharedMemory&) = delete;
        SharedMemory& operator=(const SharedMemory&) = delete;
        ~SharedMemory();

        char name[shmNameLength + 1];

        char* ptr;

    private:
        bool created;
        int fd;
    };

    class Command
    {
    public:
        Command() = default;
        Command(const Command&) = default;
        Command(Command&&);
        void init();
        Command& appendArgument(const std::string_view val);
        Command& replace(fmt::format_args args);
        Command& finalizeForExec();
        Process execute() const;
        Command& setRedirect(const RedirectType which, const int fd);
        Command& setRedirect(const RedirectType which, const char* file);
        void parseArgument(int& argc, const char* const argv[]);
        friend std::ostream& operator<<(std::ostream& os, const Command& c);
        ~Command();

        std::string_view path;

    private:
        int fd[3] { -1, -1, -1 };
        std::vector<std::string> args;
        const char** pointers = nullptr; // pointer to args
    };
    struct killParam
    {
        pid_t pid;
        bool killed;
    };
    class TimeLimit
    {
    public:
        void create();
        void setExpire(const unsigned long long us);
        std::pair<bool, int> waitFor(const Process& p);
        bool isExceed();
        ~TimeLimit();

    private:
        int id = -1;
        timer_t timer;
        sigevent sigev;
        itimerspec spec;
        killParam cntrl;
    };
    class MemoryLimit
    {
    public:
        MemoryLimit();
        MemoryLimit(const MemoryLimit&) = delete;
        MemoryLimit& operator=(const MemoryLimit&) = delete;

        void setLimit(const MemoryUsage m);
        void addProcess(const Process& p);
        bool isExceed();
        void clear() {};
        ~MemoryLimit();

    private:
        std::filesystem::path cgroup;
#if cgroupVersion == 1
        bool swapAccount;
#endif
        unsigned int count;
    };
    class Pipe
    {
    public:
        Pipe();

        int read, write;
    };
    TimeUsage getTimeUsage();
    MemoryUsage getMemoryUsage();
}

#endif
