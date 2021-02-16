#ifndef PROCESS_H
#define PROCESS_H

#include "include/system_common.h"

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOMINMAX
#include <Windows.h>
#include <fmt/format.h>
#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <ostream>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

namespace apdebug::System {
const unsigned int shmNameLength = 10;

struct Process {
 public:
  typedef DWORD Pid;

  Process() = default;
  Process(Pid p);
  Process(HANDLE hand, Pid p);
  Process(const Process&) = delete;
  Process(Process&&);
  Process& operator=(Process&&);
  ~Process();

  int wait() const;
  void terminate() const;

  Pid pid;
  HANDLE handle;

 private:
  bool owns = false;
};
class SharedMemory {
 public:
  SharedMemory();
  SharedMemory(const char* name);
  SharedMemory(const SharedMemory&) = delete;
  ~SharedMemory();

  char name[shmNameLength + 1], *ptr;

 private:
  HANDLE fd;
};
class Command {
 public:
  Command& appendArgument(const std::string_view arg);
  Command& instantiate(fmt::format_args args);
  Command& instantiate();
  Process execute();
  Command& setRedirect(RedirectType typ, const std::filesystem::path& file);
  Command& setRedirect(RedirectType typ, HANDLE had);
  void parseArgument(const YAML::Node& node);
  void release();
  friend std::ostream& operator<<(std::ostream& os, const Command& cmd);
  ~Command();

  std::string_view path;

 private:
  bool instantiated = false;
  std::string cmdline;
  const std::string* templateCmdline = nullptr;
  HANDLE openFile[3] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
                        INVALID_HANDLE_VALUE},
         stdioHandle[3];
};
class TimeLimit {
 public:
  inline void create() {}
  inline void setExpire(const unsigned long long us) { time = us / 1000; }
  std::pair<bool, int> waitFor(const Process& p);
  inline bool isExceed() const { return exceed; }

 private:
  bool exceed;
  unsigned long long time;  // ms!
};

struct KillParam {
  HANDLE job, iocp;
  unsigned int exceed;
};
class MemoryLimit {
 public:
  MemoryLimit();
  MemoryLimit(const MemoryLimit&) = delete;
  MemoryLimit& operator=(const MemoryLimit&) = delete;

  void setLimit(const MemoryUsage kb);
  void addProcess(const Process& p);
  bool isExceed();
  inline void clear() {}
  ~MemoryLimit();

 private:
  unsigned int exceed;
  HANDLE job, iocp;
  KillParam cntrl;
  std::thread watch;
};
class Pipe {
 public:
  Pipe();
  ~Pipe();

  HANDLE read, write;
};
namespace Usage {
unsigned long long getRealTime();
std::pair<TimeUsage, MemoryUsage> getUsage();
TimeUsage getTimeUsage();
}  // namespace Usage
void consoleInit();

void protectPage(void* const address, const size_t size, const bool write);

constexpr static unsigned int interactArgsSize = sizeof(Process::Pid);
}  // namespace apdebug::System

#endif
