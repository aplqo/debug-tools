#include <sstream>
#include <string>
#include <thread>

namespace apdebug
{
    namespace utility
    {
        using std::string;
        using std::stringstream;
        string GetThreadId()
        {
            stringstream s;
            s << std::hex << std::this_thread::get_id();
            return s.str();
        }
    } // namespace utility
} // namespace apdebug
