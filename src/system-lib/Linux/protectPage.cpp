#include "system.h"

#include <sys/mman.h>

namespace apdebug::System
{
    void protectPage(void* const address, const size_t len, const bool write)
    {
        mprotect(address, len, write ? PROT_READ | PROT_WRITE : PROT_READ);
    }
}