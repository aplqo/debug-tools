#ifndef CMDARG
#define CMDARG

#include "include/define.h"
#include "include/testcase.h"
#include <cstring>
#include <type_traits>
#include <vector>

namespace apdebug
{
    namespace args
    {
        using apdebug::timer::timType;
        void ReadArgument(testcase::result&, int&, char*[]);
        template <class T>
        bool ReadLimit(int& pos, char* argv[])
        {
            static_assert(std::is_base_of_v<testcase::tpoint, T>);
            if (!strcmp(argv[pos], "-time"))
            {
                T::lim = atoi(argv[++pos]) * timType(1000);
                T::hardlim = T::lim * 10;
                return true;
            }
            if (!strcmp(argv[pos], "-hlimit"))
            {
                T::hardlim = atoi(argv[++pos]) * timType(1000);
                return true;
            }
            return false;
        }
        template <class T, class... Args>
        std::vector<T> readArray(int& pos, const char* argv[], Args... args)
        {
            std::vector<T> ret;
            if (strcmp(argv[pos], "["))
            {
                ret.emplace_back(argv[pos++], args...);
                return ret;
            }
            for (++pos; strcmp(argv[pos], "]"); ++pos)
                ret.emplace_back(argv[pos], args...);
            ++pos;
            return ret;
        }
    }
}

#endif