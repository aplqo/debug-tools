#include "system.h"

namespace apdebug::System
{
    Pipe::Pipe()
    {
        SECURITY_ATTRIBUTES sec {
            .nLength = sizeof(SECURITY_ATTRIBUTES),
            .lpSecurityDescriptor = nullptr,
            .bInheritHandle = true
        };
        CreatePipe(&read, &write, &sec, 0);
    }
    Pipe::~Pipe()
    {
        CloseHandle(read);
        CloseHandle(write);
    }
}