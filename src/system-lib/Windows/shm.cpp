#include "system.h"

namespace apdebug::System
{
    static const size_t sharedMemorySize = 1 * 1024 * 1024; // 1 MiB

    SharedMemory::SharedMemory()
    {
        SECURITY_ATTRIBUTES sec {
            .nLength = sizeof(SECURITY_ATTRIBUTES),
            .lpSecurityDescriptor = nullptr,
            .bInheritHandle = true
        };
        fd = CreateFileMapping(INVALID_HANDLE_VALUE, &sec, PAGE_READWRITE, 0, sharedMemorySize, NULL);
        ptr = reinterpret_cast<char*>(MapViewOfFile(fd, FILE_MAP_ALL_ACCESS, 0, 0, sharedMemorySize));
        *fmt::format_to(name, "{}", reinterpret_cast<unsigned long long>(fd)) = '\0';
    }
    SharedMemory::SharedMemory(const char* name)
    {
        fd = reinterpret_cast<HANDLE>(std::stoull(name));
        ptr = reinterpret_cast<char*>(MapViewOfFile(fd, FILE_MAP_ALL_ACCESS, 0, 0, sharedMemorySize));
    }
    SharedMemory::~SharedMemory()
    {
        UnmapViewOfFile(ptr);
        CloseHandle(fd);
    }
}