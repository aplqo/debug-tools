#include "system.h"

#include <sys/resource.h>
#include <time.h>

namespace apdebug::System
{
    const TimeUsage unit {
        .real = 1000,
        .user = 1,
        .sys = 1
    };
    namespace Usage
    {
        unsigned long long getRealTime()
        {
            timespec tm;
            clock_gettime(CLOCK_MONOTONIC, &tm);
            return tm.tv_sec * 1000000000ull + tm.tv_nsec;
        }
        std::pair<TimeUsage, MemoryUsage> getUsage()
        {
            rusage ru;
            getrusage(RUSAGE_SELF, &ru);
            return {
                TimeUsage {
                    .real = getRealTime(),
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
}