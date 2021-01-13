#ifndef OUTPUT_H
#define OUTPUT_H

#if __cplusplus < 201103L
#error ISO c++11 is required to use test output!
#endif

#include "include/define.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <list>
#include <ostream>
#include <sstream>

namespace apdebug
{
    namespace Output
    {
        template <class... Args>
        std::string writeToString(Args&&... args)
        {
            std::ostringstream os;
            (os << ... << args);
            return os.str();
        }
        template <class T>
        void writeFile(const std::filesystem::path& p, const T& dat)
        {
            std::ofstream f(p);
            f << dat;
            f.close();
        }

        /*-----Print version info-----*/
        static void PrintVersion(const char* str, std::ostream& os)
        {
            namespace info = apdebug::info;
            os << "\nDebug tool: " << str;
#if defined Interact
            os << " (interactive)\n";
#elif defined Tradition
            os << " (traditional)\n";
#else
            os << "\n";
#endif
            os << "Version " << info::branch << "@" << info::hash << " " << info::version << "\n";
            os << "Build compiler: " << info::compier << "\n";
            os << "Build on " << __TIME__ << " " << __DATE__ << " by " << info::builder << "\n";
            os << std::endl;
        }
    }
    namespace Input
    {
        template <class T>
        T readFileVal(const std::filesystem::path& p)
        {
            std::ifstream f(p);
            T ret;
            f >> ret;
            return ret;
        }
    }
}
#endif
