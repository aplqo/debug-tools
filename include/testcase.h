#ifndef TESTCASE_H
#define TESTCASE_H

#include "include/define.h"
#include "include/exception.h"
#include "include/memory.h"
#include <regex>
#include <string>

namespace apdebug
{
    namespace testcase
    {
        struct result
        {
            void exec();
            result& concat(const std::string& s);
            int ret = 0;
            std::string cmd, args;
        };
        struct limits
        {
            timer::timType lim = 1000 * 1000, hardlim = 1000 * 10 * 1000;
            size_t memLimByte, hardMemByte;
        };

        class tpoint
        {
        public:
            tpoint() = default;
            tpoint(tpoint&& r) = default;
            void init();
            void run();
            void parse();
            void test();
            void release();
            bool success();
            ~tpoint();

            static void initMemLimit();

            std::string in, out, ans;
            result rres, tres;
            /*--run result---*/
            exception::state* s = nullptr; //run state
            exception::state* ts = nullptr;
            timer::timType tim = 0;
            size_t mem = 0;
            bool fail = true;

            /*---static config---*/
            static limits lim;
            static apdebug::memory::MemoryLimit memLimit;

        protected:
            void getArgs(result& r);
            static const thread_local std::string thrdId;

        private:
            apdebug::memory::ProcessMem m;
            std::string log;
            void getLog();

            // regex for argument placeholders
            static const std::regex rin, rout, rans, rthr;
        };
    }
}
#endif