#ifndef UTILITY_H
#define UTILITY_H
#include <iostream>
#include <string>

namespace apdebug
{

    namespace utility
    {
        std::string GetThreadId();
        std::string readString(std::istream& is);
    } // namespace utility
} // namespace apdebug

#endif