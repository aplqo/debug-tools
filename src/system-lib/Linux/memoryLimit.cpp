#include <cstdlib>
#include <filesystem>
#include <limits>

#include "include/io.h"
#include "system.h"

namespace fs = std::filesystem;

namespace apdebug::System {
static const size_t cgroupNamelength = 20;

template <class T = size_t>
static T readProperty(const fs::path& p, const std::string key)
{
  std::ifstream f(p);
  while (f) {
    std::string k;
    f >> k;
    if (k == key) {
      T val;
      f >> val;
      return val;
    }
    f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }
  return 0;
}
MemoryLimit::MemoryLimit() : count(0)
{
  std::string tmp;
  tmp.reserve(cgroupNamelength);
  randomName(std::back_inserter(tmp), cgroupNamelength);
  cgroup = getenv("cgroup") / fs::path(tmp);
  fs::create_directory(cgroup);

#if cgroupVersion == 1
  swapAccount = getenv("swapaccount");
#endif
}
void MemoryLimit::setLimit(const MemoryUsage kb)
{
#if cgroupVersion == 1
  if (swapAccount)
    Output::writeFile(cgroup / "memory.memsw.limit_in_bytes", kb * 1024);
  else {
    Output::writeFile(cgroup / "memory.limit_in_bytes", kb * 1024);
    Output::writeFile(cgroup / "memory.swappiness", 0);
  }
#else
  Output::writeFile(cgroup / "memory.max", kb * 1024);
  Output::writeFile(cgroup / "memory.swap.max", 0);
#endif
}
void MemoryLimit::addProcess(const Process& p)
{
  Output::writeFile(cgroup / "cgroup.procs", p.pid);
}
bool MemoryLimit::isExceed()
{
  const unsigned int old = count;
#if cgroupVersion == 1
  count = Input::readFileVal<unsigned int>(
      cgroup / (swapAccount ? "memory.memsw.failcnt" : "memory.failcnt"));
#else
  count = readProperty(cgroup / "memory.events", "oom_kill");
#endif
  return count > old;
}
MemoryLimit::~MemoryLimit() { fs::remove(cgroup); }
}  // namespace apdebug::System