#include "system.h"

namespace apdebug::System
{
    void protectPage(void* const address, const size_t size, const bool write)
    {
        DWORD old;
        VirtualProtect(address, size, write ? PAGE_READWRITE : PAGE_READONLY, &old);
    }
}