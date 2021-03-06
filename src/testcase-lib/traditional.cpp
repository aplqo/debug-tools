#include <fmt/compile.h>
#include <fmt/format.h>

#include <cstring>
#include <filesystem>
#include <initializer_list>
#include <regex>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "include/color.h"
#include "include/logfile.h"
#include "include/pathFormat.h"
#include "include/replace.h"
#include "include/testcase.h"
#include "include/utility.h"
#include "system.h"

namespace fs = std::filesystem;
namespace Escape = apdebug::Output::Escape;
using namespace std::string_literals;
using apdebug::TestTools::TemporaryFile;
using std::regex;
using std::strcmp;
using std::string;

namespace apdebug {
namespace Testcase {
namespace {
enum class Param { Limit, Test, Program, Autodiff, Tmpfile, Interactor };
static const std::unordered_map<std::string, Param> par{
    {"limit", Param::Limit},     {"test", Param::Test},
    {"program", Param::Program}, {"autodiff", Param::Autodiff},
    {"tmpfile", Param::Tmpfile}, {"interact", Param::Interactor}};
}  // namespace
void BasicTemplate::init()
{
  using namespace fmt::literals;
  platform->memoryProtect.setLimit(hardMemoryLimit);
  platform->timeProtect.setExpire(hardTimeLimit);
}
void BasicTemplate::parseArgument(const YAML::Node& node)
{
  for (const auto& it : node)
    switch (par.at(it.first.Scalar())) {
      case Param::Limit:
        LimitInfo::parseArgument(it.second);
        break;
      case Param::Test:
        tester.parseArgument(it.second);
        break;
      case Param::Program:
        program.parseArgument(it.second);
        break;
      case Param::Autodiff:
        diff.parseArgument(it.second);
        break;
      case Param::Tmpfile:
        tmpfiles.parseArgument(it.second);
        break;
#ifdef Interact
      case Param::Interactor:
        interactor.parseArgument(it.second);
        break;
#endif
    }
}

BasicTest::BasicTest(fs::path&& input, fs::path&& answer,
                     const BasicTemplate& te)
    : BasicTemplate(te),
      input(input),
      output(this->input.parent_path() / fmt::format(FMT_COMPILE("{}-{}.out"),
                                                     platform->threadId,
                                                     platform->count)),
      answer(answer)
{
  using namespace fmt::literals;
  Replace::ReplaceStrict(
      fmt::make_format_args("input"_a = this->input, "output"_a = output,
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
  *reinterpret_cast<Logfile::RStatus*>(platform->sharedMemory.ptr +
                                       System::interactArgsSize) =
      Logfile::RStatus::Unknown;
}
#ifdef Interact
void BasicTest::run()
{
  using System::RedirectType;
  System::Pipe pigo, pogi;  // program in grader out, program out grader in
  program.setRedirect(RedirectType::StdIn, pigo.read)
      .setRedirect(RedirectType::StdOut, pogi.write);
  interactor.setRedirect(RedirectType::StdIn, pogi.read)
      .setRedirect(RedirectType::StdOut, pigo.write);
  const auto p = program.execute();
  std::memcpy(platform->sharedMemory.ptr, &p.pid, sizeof(p.pid));
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
  program.setRedirect(System::RedirectType::StdIn, input)
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
  if (platform->timeProtect.isExceed()) {
    accept = runPass = false;
    finalResult = runResult[0] = &ResultConstant::hardTLE;
    runMemory = 0;
    runTime.real = runTime.sys = runTime.user = hardTimeLimit;
    return;
  }
  else if (platform->memoryProtect.isExceed()) {
    accept = runPass = false;
    finalResult = runResult[0] = &ResultConstant::hardMLE;
    runMemory = hardMemoryLimit;
    runTime = System::TimeUsage{};
    return;
  }
  System::MemoryStream ms{platform->sharedMemory.ptr +
                          System::interactArgsSize};
  using Logfile::RStatus;
  RStatus stat;
  ms.read(stat);
  ms.read(runTime);
  ms.read(runMemory);
  runTime.convert();
  unsigned int runPtr = 0;
  switch (stat) {
    case RStatus::Accept: {
      std::string msg = "Message: " + Logfile::readString(ms);
      *cur = Result{.type = Result::Type::AC,
                    .name = "AC",
                    .color = Escape::TextGreen,
                    .verbose = "[AC] Grader report accepted. " + msg,
                    .details = std::move(msg)};
      testResult = cur++;
      [[fallthrough]];
    }
    case RStatus::Return:
      if (runTime.real > timeLimit) {
        runResult[runPtr++] = &ResultConstant::TLE;
        accept = false;
      }
      if (runMemory > memoryLimit) {
        runResult[runPtr++] = &ResultConstant::MLE;
        accept = false;
      }
      *cur = Result{
          .type = Result::Type::Pass,
          .name = "Pass",
          .color = Escape::TextGreen,
          .verbose = fmt::format(
              FMT_COMPILE(
                  "[Pass] Program finished. \n\tTime(real/user/sys): {} / {} "
                  "/ "
                  "{} ms ({} / {} / {} us) \n\tMemory: {} MiB ({} KiB)"),
              runTime.real / 1000.0, runTime.user / 1000.0,
              runTime.sys / 1000.0, runTime.real, runTime.user, runTime.sys,
              runMemory / 1024.0, runMemory)};
      runResult[runPtr++] = cur++;
      break;
    case RStatus::RuntimeError:
      runResult[0] = Logfile::parseRE(ms, cur);
      goto err;
    case RStatus::Warn:
      runResult[0] = Logfile::parseWarn(ms, cur);
      goto err;
    case RStatus::Protocol: {
      std::string msg = "Message: " + Logfile::readString(ms);
      *cur = Result{.type = Result::Type::Protocol,
                    .name = "PV",
                    .color = Escape::TextRed,
                    .verbose = "[PV] Protocol Violation. " + msg,
                    .details = std::move(msg)};
      runResult[0] = cur++;
    }
      goto err;
    case RStatus::WrongAnswer: {
      runResult[0] = &ResultConstant::InteractRunWA;
      std::string msg = "Message: " + Logfile::readString(ms);
      *cur = Result{.type = Result::Type::WA,
                    .name = "WA",
                    .color = Escape::TextRed,
                    .verbose = "[WA] Wrong answer. " + msg,
                    .details = std::move(msg)};
      testPass = false;
      testResult = cur++;
    }
      goto err;
    default:
      *cur =
          Result{.type = Result::Type::Unknown,
                 .name = "UKE",
                 .color = Escape::None,
                 .verbose = fmt::format(
                     FMT_COMPILE("[UKE] Program return code {}"), exitStatus),
                 .details = fmt::format(FMT_COMPILE("Program return code {}"),
                                        exitStatus)};
      runMemory = 0;
      runTime = System::TimeUsage{};
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
  if (testResult) return;
  if (!runPass || tester.path.empty()) {
    testResult = &ResultConstant::Skip;
    return;
  }
  diff.check(tester);
  if (const int ret = tester.execute().wait(); ret) {
    *cur = Result{.type = Result::Type::WA,
                  .name = "WA",
                  .color = Escape::TextRed,
                  .verbose = fmt::format(
                      FMT_COMPILE("[WA] Test program return {}"), ret)};
    finalResult = testResult = cur++;
    testPass = accept = false;
  }
  else {
    testResult = &ResultConstant::TestAccept;
    if (accept) finalResult = testResult;
  }
  tmpfiles.release(TemporaryFile::Test, testPass, accept);
}
void BasicTest::release()
{
  using namespace std::filesystem;
  if (testPass) {
    Utility::removeFile(output);
    diff.release();
  }
}
void BasicTest::printRunInfo(std::ostream& os)
{
  os << Escape::None << Escape::TextCyan << "[info] Input file: " << input
     << "\n";
  os << "[info] Output file: " << output << "\n";
  os << "[info] Program command line: " << program;
#ifdef Interact
  os << "\n[info] Interactor command line: " << interactor;
#endif
  os << Escape::None << "\n";
}
void BasicTest::printTestInfo(std::ostream& os)
{
  os << Escape::None << Escape::TextCyan;
  if (!answer.empty()) os << "[info] Answer file: " << answer << "\n";
  os << "[info] Test command line: " << tester << Escape::None << "\n";
}
}  // namespace Testcase
}  // namespace apdebug
