#include <iomanip>
#include <sstream>
#include <thread>

#include "include/system_common.h"

namespace apdebug::System {
std::string GetThreadId()
{
  std::ostringstream os;
  os << std::hex << std::this_thread::get_id();
  return os.str();
}
}  // namespace apdebug::System