#include "include/memory.h"

#ifdef __linux__

#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <utility>

namespace apdebug::memory
{
    using std::ifstream;
    using std::move;
    using std::ofstream;
    using std::string;
    using std::filesystem::create_directory;
    using std::filesystem::exists;
    using std::filesystem::path;
    using std::filesystem::remove_all;

    bool swapaccount = false;

    template <class T = size_t>
    static T read(const path& p)
    {
        ifstream f(p);
        T ret;
        f >> ret;
        return ret;
    }
    template <class T = size_t>
    static T readProperty(const path& p, const string key)
    {
        ifstream f(p);
        while (f)
        {
            string k;
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
    template <class T = size_t>
    static void write(const path& p, T val)
    {
        ofstream f(p);
        f << val;
    }

    ProcessMem::ProcessMem(ProcessMem&& o)
    {
        if (this == &o)
            return;
        this->group = move(o.group);
        this->enable = o.enable;
        o.enable = false;
    }
    ProcessMem& ProcessMem::operator=(ProcessMem&& o)
    {
        if (this == &o)
            return *this;
        this->group = move(o.group);
        this->enable = o.enable;
        o.enable = false;
        return *this;
    }
    size_t ProcessMem::getUsage()
    {
        if (!enable)
            return 0;
        return read(group / (swapaccount ? "memory.memsw.max_usage_in_bytes" : "memory.max_usage_in_bytes"))
            - readProperty(group / "memory.stat", "cache");
    }
    bool ProcessMem::isExceed()
    {
        if (!enable)
            return false;
        return swapaccount ? read(group / "memory.memsw.failcnt") : read(group / "memory.failcnt");
    }
    void ProcessMem::release()
    {
        if (enable)
        {
            remove(group);
            enable = false;
        }
    }
    string ProcessMem::getArg()
    {
        return enable ? group / "cgroup.procs" : "";
    }
    ProcessMem::~ProcessMem()
    {
        release();
    }

    static void writeLimit(const path& p, const size_t lim)
    {
        write(p / "memory.limit_in_bytes", lim);
        if (swapaccount)
            write(p / "memory.memsw.limit_in_bytes", lim);
        else
            write(p / "memory.swappiness", 0);
    }

    void MemoryLimit::init(const string& id)
    {
        group = id;
        enable = create_directory(group);
    }
    ProcessMem MemoryLimit::addProcess(const string& id)
    {
        if (!enable)
            return ProcessMem("", false);
        const path& p = group / id;
        const bool en = create_directory(p);
        if (en && lim)
            writeLimit(p, lim);
        return ProcessMem(p, en);
    }
    void MemoryLimit::setLimit(const size_t lim)
    {
        if (!enable)
            return;
        this->lim = lim;
    }
    MemoryLimit::~MemoryLimit()
    {
        if (enable)
            remove(group);
    }
}

#else

namespace apdebug::memory
{
    ProcessMem::ProcessMem(ProcessMem&& o) {}
    ProcessMem& ProcessMem::operator=(ProcessMem&& o) {}
    bool ProcessMem::isExceed() { return false; }
    size_t ProcessMem::getUsage() { return 0; }
    std::string ProcessMem::getArg() { return ""; }
    void ProcessMem::release() {}
    ProcessMem::~ProcessMem() {}

    void MemoryLimit::init(const std::string&) {};
    ProcessMem MemoryLimit::addProcess(const std::string&) { return ProcessMem(); };
    void MemoryLimit::setLimit(const size_t) {}
    MemoryLimit::~MemoryLimit() {}
}

#endif