#include "include/system_common.h"
#include <iomanip>
#include <sstream>
#include <thread>

namespace apdebug::Process
{
    std::string GetThreadId()
    {
        std::ostringstream os;
        os << std::hex << std::this_thread::get_id();
        return os.str();
    }
}