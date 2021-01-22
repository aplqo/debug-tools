#include "system.h"

#include<Psapi.h>

namespace apdebug::System
{
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
}