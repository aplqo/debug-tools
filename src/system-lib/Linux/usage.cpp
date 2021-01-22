#include "system.h"

#include <sys/resource.h>
#include <time.h>

namespace apdebug::System
{
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
}