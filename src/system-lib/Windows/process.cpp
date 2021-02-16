#include "system.h"

namespace apdebug::System {

Process::Process(Pid v)
    : pid(v), handle(OpenProcess(PROCESS_ALL_ACCESS, false, v)), owns(true){};
Process::Process(HANDLE h, Pid p) : pid(p), handle(h), owns(true){};
Process::Process(Process&& other)
    : pid(other.pid), handle(other.handle), owns(other.owns)
{
  other.owns = false;
}
Process& Process::operator=(Process&& r)
{
  pid = r.pid;
  handle = r.handle;
  owns = r.owns;
  r.owns = false;
  return *this;
}
Process::~Process()
{
  if (owns) CloseHandle(handle);
}
int Process::wait() const
{
  WaitForSingleObject(handle, INFINITE);
  DWORD ret;
  GetExitCodeProcess(handle, &ret);
  return ret;
}
void Process::terminate() const
{
  TerminateProcess(handle, 9);
  WaitForSingleObject(handle, INFINITE);
}

}  // namespace apdebug::System