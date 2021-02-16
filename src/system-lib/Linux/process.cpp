#include <signal.h>
#include <sys/wait.h>

#include "system.h"

namespace fs = std::filesystem;

namespace apdebug::System {

int Process::wait() const
{
  int status;
  waitpid(pid, &status, 0);
  return status;
}
void Process::terminate() const { kill(pid, SIGKILL); }

}  // namespace apdebug::System