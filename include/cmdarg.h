#ifndef CMDARG
#define CMDARG

#include "include/define.h"
#include "include/memory.h"
#include "include/testcase.h"
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

namespace apdebug
{
    namespace args
    {
        using apdebug::timer::timType;
        void ReadArgument(testcase::result&, int&, char*[]);
        bool ReadLimit(testcase::limits&, int& pos, char* argv[]);
        template <class T>
        bool readMemoryConf(int& pos, char* argv[])
        {
            if (!strcmp(argv[pos], "-cgroup"))
            {
                T::memLimit.init(argv[++pos]);
                return true;
            }
            if (!strcmp(argv[pos], "-swapaccount"))
            {
                apdebug::memory::swapaccount = true;
                return true;
            }
            return false;
        }
        template <class T, class... Args>
        std::vector<T> readArray(int& pos, char* argv[], Args... args)
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
    }
}

#endif