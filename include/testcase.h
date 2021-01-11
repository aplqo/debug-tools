#ifndef TESTCASE_H
#define TESTCASE_H

#include "include/define.h"
#include "include/output.h"
#include "include/testtools.h"
#include "process.h"
#include <memory>
#include <regex>
#include <string>
#include <string_view>
#include <type_traits>

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
                AC,
                WA,
                Pass,
                TLE,
                RE,
                MLE,
                Warn,
                Unknown,
                HardMLE,
                HardTLE,
                Skip,
                Protocol,
                Other
            } type;
            const char *name, *color;
            std::string verbose, details;
        };
        /*-----Result constants-----*/
        namespace ResultConstant
        {
            inline const unsigned int TypeNumber = 13;
            extern const char *name[TypeNumber], *color[TypeNumber];
            extern const Result hardTLE;
            extern const Result hardMLE;
            extern const Result TLE;
            extern const Result MLE;
            extern const Result Skip;
            extern const Result Accept;
        }

        struct LimitInfo
        {
            unsigned long long timeLimit = defaultTime, hardTimeLimit = defaultTime * 10;
            Process::MemoryUsage memoryLimit = defaultMemory, hardMemoryLimit = 1024 * 1024 * 3;

            bool parseArgument(int& argc, const char* const argv[]);
            friend std::ostream& operator<<(std::ostream& os, const LimitInfo& lim);
        };
        struct TestResult
        {
            bool runPass = true, testPass = true, accept = true;
            const Result *runResult[3] {}, *testResult = nullptr, *finalResult;
            Process::TimeUsage runTime;
            Process::MemoryUsage runMemory;

        protected:
            Result mem[4], *cur = mem;
        };
        struct Platform
        {
            std::string threadId;
            Process::TimeLimit timeProtect;
            Process::MemoryLimit memoryProtect;
            Process::SharedMemory sharedMemory;
#ifdef Interact
            Process::SharedMemory interArgs;
#endif

            void init();
        };

        struct BasicTemplate : public LimitInfo
        {
            Process::Command program, tester;
#ifdef Interact
            Process::Command interactor;
#endif
            TestTools::AutoDiff diff;
            TestTools::TemporaryFile tmpfiles;
            Platform* platform;

            void init();
            bool parseArgument(int& argc, const char* const argv[]);
        };
        class BasicTest : public TestResult, private BasicTemplate
        {
        public:
            BasicTest(std::string&& input, std::string&& answer, const BasicTemplate& tmpl);
            BasicTest(BasicTest&&) = default;
            BasicTest(const BasicTest&) = delete;
            BasicTest& operator=(const BasicTest&) = delete;
            void run();
            void test();
            void release();
            void printRunInfo(std::ostream& os);
            void printTestInfo(std::ostream& os);

            std::string input, output, answer;
            using BasicTemplate::diff;

        protected:
            using BasicTemplate::program, BasicTemplate::tester;

        private:
            void parse();
            int exitStatus;
        };
        class Summary
        {
        public:
            void mergeData(const Summary& other);
            void insert(std::string&& s, const TestResult& tst);
            void print(std::ostream& os);

        private:
            template <class T>
            struct ValSummary
            {
                struct ValFrom
                {
                    T val;
                    std::shared_ptr<std::string> from;
                } min { .val = std::numeric_limits<T>::max() }, max { .val = 0 };
                T sum = 0;
                void update(T val, std::shared_ptr<std::string>& from);
                void mergeData(const ValSummary& other);
                template <unsigned long long unit>
                std::string format(const double cnt) const;
            };
            struct SummaryEntry
            {
                unsigned int count;
                ValSummary<decltype(Process::TimeUsage::real)> timeReal, timeUser, timeSys;
                ValSummary<Process::MemoryUsage> memory;

                void update(std::shared_ptr<std::string>& s, const TestResult& tst);
            } entries[ResultConstant::TypeNumber + 1];
        };
    }
}
#endif
