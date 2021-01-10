#ifndef UTILITY_H
#define UTILITY_H
#include "process.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <fmt/format.h>

namespace apdebug
{
    namespace Utility
    {
        template <class T>
        void writeFile(const std::filesystem::path& p, const T& dat)
        {
            std::ofstream f(p);
            f << dat;
            f.close();
        }
        template <class T>
        T readFileVal(const std::filesystem::path& p)
        {
            std::ifstream f(p);
            T ret;
            f >> ret;
            return ret;
        };
        template <class T, class... Args>
        std::vector<T> parseCmdArray(int& pos, const char* const argv[], Args... args)
        {
            std::vector<T> ret;
            if (strcmp(argv[pos], "["))
            {
                ret.emplace_back(argv[pos], args...);
                return ret;
            }
            for (++pos; strcmp(argv[pos], "]"); ++pos)
                ret.emplace_back(argv[pos], args...);
            return ret;
        }
        inline bool removeFile(std::string& s)
        {
            namespace fs = std::filesystem;
            if (fs::path tmp(s); fs::exists(tmp))
            {
                fs::remove(tmp);
                s = "<released>";
                return true;
            }
            else
                s = "<unused>";
            return false;
        }

#define multiReplace1(fmt, dat1) (dat1).replace(fmt)
#define multiReplace2(fmt, dat1, dat2) \
    {                                  \
        multiReplace1(fmt, dat1);      \
        multiReplace1(fmt, dat2);      \
    }
#define multiReplace3(fmt, dat1, ...)    \
    {                                    \
        multiReplace1(fmt, dat1);        \
        multiReplace2(fmt, __VA_ARGS__); \
    }
#define multiReplace4(fmt, dat1, ...)    \
    {                                    \
        multiReplace1(fmt, dat1);        \
        multiReplace3(fmt, __VA_ARGS__); \
    }
    } // namespace utility
} // namespace apdebug

#endif
