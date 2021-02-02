#ifndef CONVERT_H
#define CONVERT_H

#include <string>

namespace apdebug::Convert
{
    inline const char* toChar(const std::string& s)
    {
        char* ch = new char[s.size() + 1];
        s.copy(ch, s.length());
        ch[s.length()] = '\0';
        return ch;
    }
}

#endif