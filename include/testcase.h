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
            int ret = 0;
            std::string cmd, args;
        };

        class tpoint
        {
        public:
            void init();
            void run();
            void parse();
            void test();
            void release();
            bool success();
            ~tpoint();

            std::string in, out, ans, log;
            result rres, tres;
            /*--run result---*/
            exception::state* s = nullptr; //run state
            exception::state* ts = nullptr;
            timer::timType tim = 0;
            bool fail = true;

            /*---static config---*/
            static timer::timType lim, hardlim;

        private:
            void getLog();
            void getArgs(result& r);
            void concat(std::string& s);
        };
    }
}
#endif