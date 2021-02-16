#include <Psapi.h>

#include "system.h"

namespace apdebug::System {
static inline unsigned long long getRealUnit()
{
  LARGE_INTEGER ret;
  QueryPerformanceFrequency(&ret);
  return ret.QuadPart / 1000000;
}
const TimeUsage unit{.real = getRealUnit(), .user = 10, .sys = 10};

namespace Usage {
static constexpr unsigned long long toInterger(const FILETIME f)
{
  return ((static_cast<unsigned long long>(f.dwHighDateTime)
           << std::numeric_limits<DWORD>::digits) |
          f.dwLowDateTime);
}
unsigned long long getRealTime()
{
  LARGE_INTEGER ret;
  QueryPerformanceCounter(&ret);
  return ret.QuadPart;
}
TimeUsage getTimeUsage()
{
  FILETIME create, exit, user, sys;
  GetProcessTimes(GetCurrentProcess(), &create, &exit, &sys, &user);
  return TimeUsage{
      .real = getRealTime(), .user = toInterger(user), .sys = toInterger(sys)};
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
}  // namespace Usage
}  // namespace apdebug::System