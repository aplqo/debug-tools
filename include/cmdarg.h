#ifndef CMDARG
#define CMDARG

#include "include/define.h"
#include "include/testcase.h"
#include <cstring>
#include <type_traits>

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
    }
}

#endif