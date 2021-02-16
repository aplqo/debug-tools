#ifndef PROCESS_H
#define PROCESS_H

#include <fmt/format.h>
#include <signal.h>
#include <sys/types.h>
#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <initializer_list>
#include <ostream>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "include/dynArray.h"
#include "include/system_common.h"

namespace apdebug::System {
const unsigned int shmNameLength = 8;

struct Process {
  int wait() const;
  void terminate() const;

  typedef pid_t Pid;
  Pid pid;
};

class SharedMemory {
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

class Command {
 public:
  Command() = default;
  Command(const Command&) = default;
  Command(Command&&);
  Command& appendArgument(const std::string_view val);
  Command& instantiate(fmt::format_args args);
  Command& instantiate();
  Process execute() const;
  Command& setRedirect(const RedirectType which, const int fd);
  Command& setRedirect(const RedirectType which,
                       const std::filesystem::path& file);
  void release();
  void parseArgument(const YAML::Node& nod);
  friend std::ostream& operator<<(std::ostream& os, const Command& c);
  ~Command();

  std::string_view path;

 private:
  int fd[3]{-1, -1, -1};
  bool created[3]{}, instantiated = false, replaced = false;
  std::vector<const char*> args;

  DynArray::DynArray<const char*> templateArgs;
};
struct killParam {
  pid_t pid;
  bool killed;
};
class TimeLimit {
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
class MemoryLimit {
 public:
  MemoryLimit();
  MemoryLimit(const MemoryLimit&) = delete;
  MemoryLimit& operator=(const MemoryLimit&) = delete;

  void setLimit(const MemoryUsage m);
  void addProcess(const Process& p);
  bool isExceed();
  void clear(){};
  ~MemoryLimit();

 private:
  std::filesystem::path cgroup;
#if cgroupVersion == 1
  bool swapAccount;
#endif
  unsigned int count;
};
class Pipe {
 public:
  Pipe();
  ~Pipe();

  int read, write;
};
namespace Usage {
unsigned long long getRealTime();
TimeUsage getTimeUsage();
std::pair<TimeUsage, MemoryUsage> getUsage();
}  // namespace Usage
inline void consoleInit() {}

void protectPage(void* const address, const size_t size, const bool write);

static const unsigned int interactArgsSize = sizeof(Process::Pid);
}  // namespace apdebug::System

#endif
