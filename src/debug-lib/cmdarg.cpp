#include "include/cmdarg.h"
#include "include/testcase.h"
#include <cstring>

namespace apdebug
{
    namespace args
    {
        using std::stoul;

        void ReadArgument(testcase::result& o, int& pos, char* argv[])
        {
            unsigned long num=stoul(argv[pos]);
            for (unsigned long i = 0; i < num; ++i)
                o.args = o.args + " \"" + argv[++pos] + "\"";
        }
    }
}