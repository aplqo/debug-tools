#include "include/cmdarg.h"
#include "include/testcase.h"
#include <cstring>
#include <string>

namespace apdebug
{
    namespace args
    {
        using std::stoul;
        using std::strcmp;

        bool ReadLimit(testcase::limits& o, int& pos, char* argv[])
        {
            if (!strcmp(argv[pos], "-time"))
            {
                o.lim = atoi(argv[++pos]) * timType(1000);
                return true;
            }
            if (!strcmp(argv[pos], "-hlimit"))
            {
                o.hardlim = atoi(argv[++pos]) * timType(1000);
                return true;
            }
            if (!strcmp(argv[pos], "-memory"))
            {
                o.memLimByte = std::stoull(argv[++pos]) * 1024;
                return true;
            }
            if (!strcmp(argv[pos], "-hmem"))
            {
                o.hardMemByte = std::stoull(argv[++pos]) * 1024;
                if (!o.memLimByte)
                    o.memLimByte = o.hardMemByte;
                return true;
            }
            return false;
        }
        void ReadArgument(testcase::result& o, int& pos, char* argv[])
        {
            if (strcmp(argv[pos], "["))
            {
                o.args = std::string(" \"") + argv[pos] + "\"";
                return;
            }
            unsigned int stk = 1;
            ++pos;
            for (; stk; ++pos)
            {
                if (!strcmp(argv[pos], "]"))
                {
                    if (stk == 1)
                        break;
                    else
                        --stk;
                }
                else if (!strcmp(argv[pos], "["))
                    ++stk;
                o.args = o.args + " \"" + argv[pos] + "\"";
            }
        }
    }
}