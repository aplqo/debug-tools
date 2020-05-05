#ifndef LOG_H
#define LOG_H

#include <cstring>
#include <fstream>
#include <string>

namespace apdebug
{
    namespace log
    {
        std::ofstream logf;

        void WriteString(const char* str, std::ofstream& os = logf)
        {
            uint32_t len = strlen(str);
            os.write(reinterpret_cast<char*>(&len), sizeof(len));
            os.write(str, len);
            os.flush();
        }
        void WriteString(const std::string& str, std::ofstream& os = logf)
        {
            uint32_t len = str.length();
            os.write(reinterpret_cast<char*>(&len), sizeof(len));
            os.write(str.c_str(), len);
            os.flush();
        }
        template <class T>
        void WriteObj(const T val, std::ofstream& os = logf)
        {
            os.write(reinterpret_cast<const char*>(&val), sizeof(val));
            os.flush();
        }
    }
}

#endif