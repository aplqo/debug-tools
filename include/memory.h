#ifndef APMEMORY_H
#define APMEMORY_H

#include <filesystem>
#include <string>

namespace apdebug::memory
{
    extern bool swapaccount;

    class ProcessMem
    {
    public:
        ProcessMem() = default;
        ProcessMem(const std::filesystem::path& p, bool en)
            : group(p)
            , enable(en)
        {
        }
        ProcessMem(const ProcessMem&) = delete;
        ProcessMem(ProcessMem&& o);
        ProcessMem& operator=(const ProcessMem&) = delete;
        ProcessMem& operator=(ProcessMem&& o);
        bool isExceed();
        size_t getUsage();
        std::string getArg();
        void release();
        ~ProcessMem();

    private:
        std::filesystem::path group;
        bool enable = false;
    };
    class MemoryLimit
    {
    public:
        MemoryLimit() = default;
        MemoryLimit(const MemoryLimit&) = delete;
        MemoryLimit& operator=(const MemoryLimit&) = delete;

        void init(const std::string& id);
        ProcessMem addProcess(const std::string& id);
        void setLimit(const size_t lim);
        ~MemoryLimit();

        std::filesystem::path group;
        bool enable = false;

    private:
        size_t lim;
    };
}

#endif