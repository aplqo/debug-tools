#include "system.h"

namespace apdebug::System {
static const size_t sharedMemorySize = 1 * 1024 * 1024;  // 1 MiB

SharedMemory::SharedMemory()
{
  randomName(name, shmNameLength);
  name[shmNameLength] = '\0';
  fd = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
                          sharedMemorySize, name);
  ptr = reinterpret_cast<char*>(
      MapViewOfFile(fd, FILE_MAP_ALL_ACCESS, 0, 0, sharedMemorySize));
}
SharedMemory::SharedMemory(const char* name)
{
  fd = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, name);
  ptr = reinterpret_cast<char*>(
      MapViewOfFile(fd, FILE_MAP_ALL_ACCESS, 0, 0, sharedMemorySize));
}
SharedMemory::~SharedMemory()
{
  UnmapViewOfFile(ptr);
  CloseHandle(fd);
}
}  // namespace apdebug::System