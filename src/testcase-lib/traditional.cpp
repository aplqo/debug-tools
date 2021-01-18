#include "include/color.h"
#include "include/logfile.h"
#include "include/testcase.h"
#include "include/utility.h"
#include "system.h"
#include <cstring>
#include <filesystem>
#include <initializer_list>
#include <regex>
#include <string>
#include <type_traits>

#include <fmt/compile.h>
#include <fmt/format.h>

namespace fs = std::filesystem;
namespace SGR = apdebug::Output::SGR;
using namespace std::string_literals;
using apdebug::TestTools::TemporaryFile;
using std::regex;
using std::strcmp;
using std::string;

namespace apdebug
{
    namespace Testcase
    {
        void BasicTemplate::init()
        {
            using namespace fmt::literals;
            platform->memoryProtect.setLimit(hardMemoryLimit);
            platform->timeProtect.setExpire(hardTimeLimit);
        }
        bool BasicTemplate::parseArgument(int& argc, const char* const argv[])
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
                diff.parseArgument(++argc, argv);
            else if (!strcmp(argv[argc], "-tmpfile"))
                tmpfiles.parseArgument(++argc, argv);
#ifdef Interact
            else if (!strcmp(argv[argc], "-interact"))
                interactor.path = argv[++argc];
            else if (!strcmp(argv[argc], "-interact-args"))
                interactor.parseArgument(++argc, argv);
#endif
            else
                return false;
            return true;
        }

        BasicTest::BasicTest(fs::path&& input, fs::path&& answer, const BasicTemplate& te)
            : BasicTemplate(te)
            , input(input)
            , output(this->input.parent_path() / fmt::format(FMT_COMPILE("{}-{}.out"), platform->threadId, platform->count))
            , answer(answer)
        {
            using namespace fmt::literals;
            System::ReplaceStrict(
                fmt::make_format_args(
                    "input"_a = this->input,
                    "output"_a = output,
                    "answer"_a = this->answer,
                    "thread"_a = platform->threadId),
#ifdef Interact
                diff, tmpfiles, program, tester, interactor
#else
                diff, tmpfiles, program, tester
#endif
            );
            ++(platform->count);
            program.appendArgument(platform->sharedMemory.name);
#ifdef Interact
            interactor.appendArgument(platform->sharedMemory.name);
#endif
            platform->memoryProtect.clear();
            *reinterpret_cast<Logfile::RStatus*>(platform->sharedMemory.ptr + System::interactArgsSize)
                = Logfile::RStatus::Unknown;
        }
#ifdef Interact
        void BasicTest::run()
        {
            using System::RedirectType;
            System::Pipe pigo, pogi; // program in grader out, program out grader in
            program
                .setRedirect(RedirectType::StdIn, pigo.read)
                .setRedirect(RedirectType::StdOut, pogi.write);
            interactor
                .setRedirect(RedirectType::StdIn, pogi.read)
                .setRedirect(RedirectType::StdOut, pigo.write);
            const auto p = program.execute();
            *reinterpret_cast<System::Process::NativeHandle*>(platform->sharedMemory.ptr) = p.nativeHandle;
            const auto i = interactor.execute();
            platform->memoryProtect.addProcess(p);
            exitStatus = platform->timeProtect.waitFor(p).second;
            if (platform->timeProtect.isExceed() || exitStatus)
                i.terminate();
            else
                platform->timeProtect.waitFor(i);
            parse();
        }
#else
        void BasicTest::run()
        {
            program
                .setRedirect(System::RedirectType::StdIn, input)
                .setRedirect(System::RedirectType::StdOut, output);
            const auto p = program.execute();
            platform->memoryProtect.addProcess(p);
            exitStatus = platform->timeProtect.waitFor(p).second;
            parse();
            program.release();
        }
#endif
        void BasicTest::parse()
        {
            if (platform->timeProtect.isExceed())
            {
                accept = runPass = false;
                finalResult = runResult[0] = &ResultConstant::hardTLE;
                runMemory = 0;
                runTime.real = runTime.sys = runTime.user = hardTimeLimit;
                return;
            }
            else if (platform->memoryProtect.isExceed())
            {
                accept = runPass = false;
                finalResult = runResult[0] = &ResultConstant::hardMLE;
                runMemory = hardMemoryLimit;
                runTime = System::TimeUsage {};
                return;
            }
            System::MemoryStream ms { platform->sharedMemory.ptr + System::interactArgsSize };
            using Logfile::RStatus;
            RStatus stat;
            ms.read(stat);
            ms.read(runTime);
            ms.read(runMemory);
            unsigned int runPtr = 0;
            switch (stat)
            {
            case RStatus::Accept:
                testResult = &ResultConstant::InteractAccept;
                [[fallthrough]];
            case RStatus::Return:
                if (runTime.real > timeLimit)
                {
                    runResult[runPtr++] = &ResultConstant::TLE;
                    accept = false;
                }
                if (runMemory > memoryLimit)
                {
                    runResult[runPtr++] = &ResultConstant::MLE;
                    accept = false;
                }
                *cur = Result {
                    .type = Result::Type::Pass,
                    .name = "Pass",
                    .color = SGR::TextGreen,
                    .verbose = fmt::format(FMT_COMPILE("[Pass] Program finished. \n\tTime(real/user/sys): {} / {} / {} ms ({} / {} / {} us) \n\tMemory: {} MiB ({} KiB)"),
                        runTime.real / 1000.0, runTime.user / 1000.0, runTime.sys / 1000.0,
                        runTime.real, runTime.user, runTime.sys,
                        runMemory / 1024.0, runMemory)
                };
                runResult[runPtr++] = cur++;
                break;
            case RStatus::RuntimeError:
                runResult[0] = Logfile::parseRE(ms, cur);
                goto err;
            case RStatus::Warn:
                runResult[0] = Logfile::parseWarn(ms, cur);
                goto err;
            case RStatus::Protocol:
                *cur = Result {
                    .type = Result::Type::Protocol,
                    .name = "PV",
                    .color = SGR::TextRed,
                    .verbose = "[PV] Protocol Violation. Mssage: " + Logfile::readString(ms)
                };
                runResult[0] = cur++;
                goto err;
            case RStatus::WrongAnswer:
                runResult[0] = &ResultConstant::InteractRunWA;
                *cur = Result {
                    .type = Result::Type::WA,
                    .name = "WA",
                    .color = SGR::TextRed,
                    .verbose = "[WA] Wrong answer. Message: " + Logfile::readString(ms)
                };
                testPass = false;
                testResult = cur++;
                goto err;
            default:
                *cur = Result {
                    .type = Result::Type::Unknown,
                    .name = "UKE",
                    .color = SGR::None,
                    .verbose = fmt::format(FMT_COMPILE("[UKE] Program return code {}"), exitStatus),
                    .details = fmt::format(FMT_COMPILE("Program return code {}"), exitStatus)
                };
                runMemory = 0;
                runTime = System::TimeUsage {};
                runResult[0] = cur++;
            err:
                accept = runPass = false;
                break;
            }
            tmpfiles.release(TemporaryFile::Run, runPass, accept);
            if (!testResult || (testPass && !accept) || !runPass)
                finalResult = runResult[0];
            else
                finalResult = testResult;
        }
        void BasicTest::test()
        {
            if (testResult)
                return;
            if (!runPass || tester.path.empty())
            {
                testResult = &ResultConstant::Skip;
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
                finalResult = testResult = cur++;
                testPass = accept = false;
            }
            else
            {
                testResult = &ResultConstant::TestAccept;
                if (accept)
                    finalResult = testResult;
            }
            tmpfiles.release(TemporaryFile::Test, testPass, accept);
        }
        void BasicTest::release()
        {
            using namespace std::filesystem;
            if (testPass)
            {
                Utility::removeFile(output);
                diff.release();
            }
        }
        void BasicTest::printRunInfo(std::ostream& os)
        {
            os << SGR::None << SGR::TextCyan << "[info] Input file: " << input << "\n";
            os << "[info] Output file: " << output << "\n";
            os << "[info] Program command line: " << program;
#ifdef Interact
            os << "\n[info] Interactor command line: " << interactor;
#endif
            os << SGR::None << "\n";
        }
        void BasicTest::printTestInfo(std::ostream& os)
        {
            os << SGR::None << SGR::TextCyan;
            if (!answer.empty())
                os << "[info] Answer file: " << answer << "\n";
            os << "[info] Test command line: " << tester << SGR::None << "\n";
        }
    }
}
