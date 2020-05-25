#ifndef TESTCASE_H
#define TESTCASE_H

#include "include/define.h"
#include "include/exception.h"
#include <regex>
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

            std::string in, out, ans;
            result rres, tres;
            /*--run result---*/
            exception::state* s = nullptr; //run state
            exception::state* ts = nullptr;
            timer::timType tim = 0;
            bool fail = true;

            /*---static config---*/
            static timer::timType lim, hardlim;

        protected:
            void getArgs(result& r);
            static const thread_local std::string thrdId;

        private:
            std::string log;
            void getLog();
            void concat(std::string& s);

            // regex for argument placeholders
            static const std::regex rin, rout, rans, rthr, rind;
        };
    }
}
#endif