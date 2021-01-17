#ifndef PROCESS_H
#define PROCESS_H

#include "include/system_common.h"

#define NOMINMAX
#include <windows.h>

#include <filesystem>
#include <ostream>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include <fmt/format.h>

namespace apdebug::System
{
    const unsigned int shmNameLength = 30;

    struct Process
    {
    public:
        typedef HANDLE NativeHandle;
        Process() = default;
        Process(const Process&) = delete;
        Process(Process&&);
        Process(NativeHandle v);
        ~Process();

        int wait() const;
        void terminate() const;

        NativeHandle nativeHandle;
        bool owns;
    };
    class SharedMemory
    {
    public:
        SharedMemory();
        SharedMemory(const char* name);
        SharedMemory(const SharedMemory&) = delete;
        ~SharedMemory();

        char name[shmNameLength + 1], *ptr;

    private:
        HANDLE fd;
    };
    class Command
    {
    public:
        Command& appendArgument(const std::string_view arg);
        Command& instantiate(fmt::format_args args);
        Command& instantiate();
        Process execute();
        Command& setRedirect(RedirectType typ, const std::filesystem::path& file);
        Command& setRedirect(RedirectType typ, HANDLE had);
        void parseArgument(int&, const char* const argv[]);
        friend std::ostream& operator<<(std::ostream& os, const Command& cmd);
        ~Command();

        std::string_view path;

    private:
        bool instantiated = false;
        std::string cmdline;
        const std::string* templateCmdline = nullptr;
        STARTUPINFOA info;
        HANDLE openFile[3] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
    };
    class TimeLimit
    {
    public:
        inline void create() { }
        inline void setExpire(const unsigned long long us) { time = us / 1000; }
        std::pair<bool, int> waitFor(const Process& p);
        inline bool isExceed() const { return exceed; }

    private:
        bool exceed;
        unsigned long long time; // ms!
    };

    struct KillParam
    {
        HANDLE job, iocp;
        unsigned int exceed;
    };
    class MemoryLimit
    {
    public:
        MemoryLimit();
        MemoryLimit(const MemoryLimit&) = delete;
        MemoryLimit& operator=(const MemoryLimit&) = delete;

        void setLimit(const MemoryUsage kb);
        void addProcess(const Process& p);
        bool isExceed();
        inline void clear() { }
        ~MemoryLimit();

    private:
        unsigned int exceed;
        HANDLE job, iocp;
        KillParam cntrl;
        std::thread watch;
    };
    class Pipe
    {
    public:
        Pipe();
        ~Pipe();

        HANDLE read, write;
    };
    std::pair<TimeUsage, MemoryUsage> getUsage();
    TimeUsage getTimeUsage();
    void systemInit();
    void consoleInit();

    void protectPage(void* const address, const size_t size, const bool write);

    constexpr static unsigned int interactArgsSize = sizeof(Process::NativeHandle);
}

#endif
