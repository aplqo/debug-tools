#include "include/system_common.h"

namespace apdebug::System
{
    void* roundToPage(void* p)
    {
        static constexpr unsigned long long modMsk = pageSize - 1, divMsk = ~modMsk;
        const unsigned long long addr = reinterpret_cast<unsigned long long>(p);
        return reinterpret_cast<void*>((addr & divMsk) + ((addr & modMsk) ? pageSize : 0));
    }
}