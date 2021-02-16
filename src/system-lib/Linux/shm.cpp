#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "system.h"

namespace apdebug::System {
static const size_t SharedMemorySize = 1 * 1024 * 1024;  // 1 MiB

SharedMemory::SharedMemory() : created(true)
{
  name[0] = '/';
  randomName(name + 1, shmNameLength - 1);
  name[shmNameLength] = '\0';
  fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
  ftruncate(fd, SharedMemorySize);
  ptr = reinterpret_cast<char*>(mmap(
      nullptr, SharedMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
}
SharedMemory::SharedMemory(const char* name) : created(false)
{
  fd = shm_open(name, O_RDWR, S_IWGRP | S_IWOTH);
  ptr = reinterpret_cast<char*>(mmap(
      nullptr, SharedMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
}
SharedMemory::~SharedMemory()
{
  munmap(ptr, SharedMemorySize);
  close(fd);
  if (created) shm_unlink(name);
}
}  // namespace apdebug::System