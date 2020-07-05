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

        static void writeStr(const char* str, const size_t len, std::ofstream& os)
        {
            os.write(reinterpret_cast<const char*>(&len), sizeof(len));
            os.write(str, len);
            os.flush();
        }
        void WriteString(const char* str, std::ofstream& os = logf)
        {
            writeStr(str, strlen(str), os);
        }
        void WriteString(const std::string& str, std::ofstream& os = logf)
        {
            writeStr(str.c_str(), str.length(), os);
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