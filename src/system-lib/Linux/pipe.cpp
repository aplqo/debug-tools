#include <unistd.h>

#include "system.h"

namespace apdebug::System {
Pipe::Pipe()
{
  int fd[2];
  pipe(fd);
  read = fd[0];
  write = fd[1];
}
Pipe::~Pipe()
{
  close(read);
  close(write);
}
}  // namespace apdebug::System