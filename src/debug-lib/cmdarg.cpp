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