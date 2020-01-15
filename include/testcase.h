#ifndef TESTCASE_H
#define TESTCASE_H

#include "include/define.h"
#include "include/exception.h"
#include <string>

namespace apdebug
{
    namespace testcase
    {
        struct result
        {
            void exec();
            int ret;
            string cmd, args;
        };

        class tpoint
        {
        public:
            void run();
            void parse();
            void test();
            ~tpoint();

            std::string in, out, ans, log;
            result rres, tres;
            /*--run result---*/
            exception::state* s = nullptr;
            string des, what;
            timer::timType tim;

            /*---static config---*/
            static timer::timType lim, hardlim;

        private:
            void concat(string& s);
        };
    }
}
#endif