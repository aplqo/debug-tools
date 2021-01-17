#include "include/system_common.h"
#include <iomanip>
#include <sstream>
#include <thread>

namespace apdebug::System
{
    std::string GetThreadId()
    {
        std::ostringstream os;
        os << std::hex << std::this_thread::get_id();
        return os.str();
    }
    void* roundToPage(void* p)
    {
        static constexpr unsigned long long modMsk = pageSize - 1, divMsk = ~modMsk;
        const unsigned long long addr = reinterpret_cast<unsigned long long>(p);
        return reinterpret_cast<void*>((addr & divMsk) + ((addr & modMsk) ? pageSize : 0));
    }
}