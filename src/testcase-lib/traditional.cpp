#include "include/logfile.h"
#include "include/output.h"
#include "include/testcase.h"
#include "include/utility.h"
#include "process.h"
#include <cstring>
#include <filesystem>
#include <initializer_list>
#include <regex>
#include <string>

#include <fmt/compile.h>
#include <fmt/format.h>

using namespace apdebug::Output;
using namespace std::string_literals;
using apdebug::TestTools::TemporaryFile;
using std::regex;
using std::strcmp;
using std::string;

namespace apdebug
{
    namespace Testcase
    {
        void TraditionalTemplate::init()
        {
            using namespace fmt::literals;
            platform->memoryProtect.setLimit(hardMemoryLimit);
            platform->timeProtect.setExpire(hardTimeLimit);
            multiReplace2(fmt::make_format_args("thread"_a = platform->threadId), program, tester);
        }
        bool TraditionalTemplate::parseArgument(int& argc, const char* const argv[])
        {
            if (LimitInfo::parseArgument(argc, argv))
                return true;
            else if (!strcmp(argv[argc], "-program"))
                program.path = argv[++argc];
            else if (!strcmp(argv[argc], "-args"))
                program.parseArgument(++argc, argv);
            else if (!strcmp(argv[argc], "-test"))
                tester.path = argv[++argc];
            else if (!strcmp(argv[argc], "-test-args"))
                tester.parseArgument(++argc, argv);
            else if (!strcmp(argv[argc], "-autodiff"))
                autodiff.parseArgument(argc, argv);
            else if (!strcmp(argv[argc], "-tmpfile"))
                tmpfiles.parseArgument(argc, argv);
            else
                return false;
            return true;
        }

        TraditionalTest::TraditionalTest(std::string&& input, std::string&& answer, const TraditionalTemplate& te)
            : input(input)
            , answer(answer)
            , runPass(true)
            , testPass(true)
            , accept(true)
            , runResult {}
            , diff(te.autodiff)
            , program(te.program)
            , tester(te.tester)
            , tmpfile(te.tmpfiles)
            , platform(*te.platform)
            , limit(te)
            , cur(mem)
        {
            using namespace fmt::literals;
            output = input + "-" + platform.threadId + ".out";
            multiReplace4(
                fmt::make_format_args("input"_a = input, "output"_a = output, "answer"_a = answer, "thread"_a = te.platform->threadId),
                diff, tmpfile, program, tester);
            program
                .appendArgument(platform.sharedMemory.name)
                .setRedirect(Process::RedirectType::StdOut, output.c_str())
                .finalizeForExec();
            if (!tester.path.empty())
                tester.finalizeForExec();
        }
        void TraditionalTest::run()
        {
            platform.memoryProtect.clear();
            const auto p = program.execute();
            platform.memoryProtect.addProcess(p);
            const auto [exp, ret] = platform.timeProtect.waitFor(p);
            if (exp)
            {
                accept = runPass = false;
                runResult[0] = &hardTLE;
                runMemory = 0;
                runTime.real = runTime.sys = runTime.user = limit.hardTimeLimit;
            }
            else if (platform.memoryProtect.isExceed())
            {
                accept = runPass = false;
                runResult[0] = &hardMLE;
                runMemory = limit.hardMemoryLimit;
                runTime = Process::TimeUsage {};
            }
            else
                exitStatus = ret;
        }
        void TraditionalTest::parse()
        {
            if (!runPass)
            {
                tmpfile.release(TemporaryFile::Run, false, false);
                return;
            }
            Process::MemoryStream ms { reinterpret_cast<char*>(platform.sharedMemory.ptr) };
            using Logfile::RStatus;
            RStatus stat;
            ms.read(stat);
            unsigned int runPtr = 0;
            switch (stat)
            {
            case RStatus::Return:
                ms.read(runTime);
                ms.read(runMemory);
                if (runTime.real > limit.timeLimit)
                {
                    runResult[runPtr++] = &TLE;
                    accept = false;
                }
                if (runMemory > limit.memoryLimit)
                {
                    runResult[runPtr++] = &MLE;
                    accept = false;
                }
                *cur = Result {
                    .type = Result::Type::Pass,
                    .name = "Pass",
                    .color = SGR::TextGreen,
                    .verbose = fmt::format(FMT_COMPILE("[Pass] Program finished. \n\tTime(real/user/sys): {}/{}/{} ms ({}/{}/{} us) \n\tMemory: {} MiB ({} KiB)"),
                        runTime.real / 1000.0, runTime.user / 1000.0, runTime.sys / 1000.0,
                        runTime.real, runTime.user, runTime.sys,
                        runMemory / 1024.0, runMemory)
                };
                runResult[runPtr++] = cur++;
                break;
            case RStatus::RuntimeError:
                runResult[0] = Logfile::parseRE(ms, cur);
                goto err;
                break;
            case RStatus::Warn:
                runResult[0] = Logfile::parseWarn(ms, cur);
                goto err;
                break;
            default:
                *cur = Result {
                    .type = Result::Type::Unknown,
                    .name = "UKE",
                    .color = SGR::None,
                    .verbose = fmt::format(FMT_COMPILE("[UKE] Program return code {}"), exitStatus)
                };
                runResult[0] = cur++;
            err:
                runMemory = 0;
                runTime = Process::TimeUsage {};
                accept = runPass = false;
                break;
            }
            tmpfile.release(TemporaryFile::Run, runPass, accept);
        }
        void TraditionalTest::test()
        {
            if (!runPass || tester.path.empty())
            {
                testResult = &Skip;
                return;
            }
            diff.check(tester);
            if (const int ret = tester.execute().wait(); ret)
            {
                *cur = Result {
                    .type = Result::Type::WA,
                    .name = "WA",
                    .color = SGR::TextRed,
                    .verbose = fmt::format(FMT_COMPILE("[WA] Test program return {}"), ret)
                };
                testResult = cur++;
                testPass = accept = false;
            }
            else
                testResult = &Accept;
            tmpfile.release(TemporaryFile::Test, testPass, accept);
        }
        void TraditionalTest::release()
        {
            using namespace std::filesystem;
            if (accept)
            {
                Utility::removeFile(output);
                diff.release();
            }
        }
        void TraditionalTest::printRunInfo(std::ostream& os)
        {
            os << SGR::None << SGR::TextCyan << "[info] Input file: " << input << "\n";
            os << "[info] Output file: " << output << "\n";
            os << SGR::TextBlue << "[info] Run command line: " << program << SGR::None << "\n";
        }
        void TraditionalTest::printTestInfo(std::ostream& os)
        {
            os << SGR::None;
            if (!answer.empty())
                os << SGR::TextCyan << "[info] Answer file: " << answer << "\n";
            os << SGR::TextBlue << "[info] Test command line: " << tester << SGR::None << "\n";
        }
    }
}
