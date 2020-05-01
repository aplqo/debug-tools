#include <iostream>
#include <sstream>
#include <string>
#include <thread>

namespace apdebug
{
    namespace utility
    {
        using std::string;
        string GetThreadId()
        {
            std::stringstream s;
            s << std::hex << std::this_thread::get_id();
            return s.str();
        }
        string readString(std::istream& is)
        {
            uint32_t siz;
            is.read(reinterpret_cast<char*>(&siz), sizeof(siz));
            char* buf = new char[siz+5];
            is.read(buf, siz);
            buf[siz] = '\0';
            string ret(buf);
            delete[] buf;
            return ret;
        }
    } // namespace utility
} // namespace apdebug
