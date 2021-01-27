#include "include/system_common.h"

namespace apdebug::System
{
    TimeUsage TimeUsage::operator-(const TimeUsage& r) const
    {
        return TimeUsage {
            .real = real - r.real,
            .user = user - r.user,
            .sys = sys - r.sys
        };
    }
    void TimeUsage::convert()
    {
        real /= unit.real;
    }
}