#ifndef TESTCASE_H
#define TESTCASE_H

#include "include/define.h"
#include "include/output.h"
#include "include/testtools.h"
#include "process.h"
#include <regex>
#include <string>
#include <string_view>

namespace apdebug
{
    namespace Testcase
    {
        static const inline Process::MemoryUsage defaultMemory = Process::MemoryUsage(1024) * 1024 * 3;
        static const inline unsigned long long defaultTime = 1000000;

        struct Result
        {
            enum class Type
            {
                Pass,
                HardTLE,
                HardMLE,
                TLE,
                MLE,
                Unknown,
                Skip,
                AC,
                WA,
                Warn,
                RE
            } type;
            const char *name, *color;
            std::string verbose, details;
        };
        /*-----Result constants-----*/
        extern const Result hardTLE;
        extern const Result hardMLE;
        extern const Result TLE;
        extern const Result MLE;
        extern const Result Skip;
        extern const Result Accept;

        struct LimitInfo
        {
            unsigned long long timeLimit = defaultTime, hardTimeLimit = defaultTime * 10;
            Process::MemoryUsage memoryLimit = defaultMemory, hardMemoryLimit = 1024 * 1024 * 3;

            bool parseArgument(int& argc, const char* const argv[]);
            friend std::ostream& operator<<(std::ostream& os, const LimitInfo& lim);
        };
        struct Platform
        {
            std::string threadId;
            Process::TimeLimit timeProtect;
            Process::MemoryLimit memoryProtect;
            Process::SharedMemory sharedMemory;

            void init();
        };

        struct TraditionalTemplate : public LimitInfo
        {
            Process::Command program, tester;
            TestTools::AutoDiff autodiff;
            TestTools::TemporaryFile tmpfiles;
            Platform* platform;

            void init();
            bool parseArgument(int& argc, const char* const argv[]);
        };
        class TraditionalTest
        {
        public:
            TraditionalTest(std::string&& input, std::string&& answer, const TraditionalTemplate& tmpl);
            TraditionalTest(TraditionalTest&&) = default;
            TraditionalTest(const TraditionalTest&) = delete;
            TraditionalTest& operator=(const TraditionalTest&) = delete;
            void run();
            void parse();
            void test();
            void release();
            void printRunInfo(std::ostream& os);
            void printTestInfo(std::ostream& os);

            std::string input, output, answer;
            bool runPass, testPass, accept;
            const Result *runResult[3], *testResult;
            Process::TimeUsage runTime;
            Process::MemoryUsage runMemory;
            TestTools::AutoDiff diff;

        protected:
            Process::Command program, tester;

        private:
            TestTools::TemporaryFile tmpfile;
            int exitStatus;
            Platform& platform;
            LimitInfo limit;
            Result mem[4], *cur;
        };
    }
}
#endif
