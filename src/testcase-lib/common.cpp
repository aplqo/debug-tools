#include <cstring>
#include <ostream>
#include <unordered_map>

#include "include/color.h"
#include "include/testcase.h"

namespace Escape = apdebug::Output::Escape;
using std::strcmp;

namespace apdebug::Testcase {
/*----- Result constants-----*/
namespace ResultConstant {
const char* name[14] = {
    "TLE",  "MLE",      "AC",       "WA", "Pass",  "Killed(WA)", "RE",
    "Warn", "Hard MLE", "Hard TLE", "PV", "Other", "Unknown",    "Skip"};
const char* color[14] = {
    Escape::TextYellow, Escape::TextYellow, Escape::TextGreen,
    Escape::TextRed,    Escape::TextGreen,  Escape::TextRed,
    Escape::TextPurple, Escape::TextYellow, Escape::TextRed,
    Escape::TextRed,    Escape::TextRed,    Escape::None,
    Escape::None,       Escape::None};
const Result hardTLE{.type = Result::Type::HardTLE,
                     .name = "Hard TLE",
                     .color = Output::Escape::TextRed,
                     .verbose = "[Hard TLE] Killed: Hard time limit exceed."};
const Result hardMLE{.type = Result::Type::HardMLE,
                     .name = "Hard MLE",
                     .color = Output::Escape::TextRed,
                     .verbose = "[Hard MLE] Killed: Hard memory limit exceed."};
const Result TLE{.type = Result::Type::TLE,
                 .name = "TLE",
                 .color = Output::Escape::TextYellow,
                 .verbose = "[TLE] Time limit exceed"};
const Result MLE{.type = Result::Type::MLE,
                 .name = "MLE",
                 .color = Output::Escape::TextYellow,
                 .verbose = "[MLE] Memory limit exceed"};
const Result Skip{.type = Result::Type::Skip, .name = "skip", .color = ""};
const Result TestAccept{.type = Result::Type::AC,
                        .name = "AC",
                        .color = Output::Escape::TextGreen,
                        .verbose = "[AC] Test program return code 0"};
const Result InteractRunWA{
    .type = Result::Type::WAKilled,
    .name = "Killed",
    .color = Output::Escape::TextRed,
    .verbose = "[WA] Killed: Grader report wrong answer"};
}  // namespace ResultConstant
namespace {
enum class Param { Memory, HardMemory, Time, HardTime };
static const std::unordered_map<std::string, Param> par{
    {"memory", Param::Memory},
    {"hmemory", Param::HardMemory},
    {"time", Param::Time},
    {"htime", Param::HardTime}};
}  // namespace
void LimitInfo::parseArgument(const YAML::Node& nod)
{
  for (const auto& it : nod)
    switch (par.at(it.first.Scalar())) {
      case Param::Memory:
        memoryLimit = it.second.as<decltype(memoryLimit)>() * 1024;
        break;
      case Param::HardMemory:
        hardMemoryLimit = it.second.as<decltype(hardMemoryLimit)>() * 1024;
        break;
      case Param::Time:
        timeLimit = it.second.as<decltype(timeLimit)>() * 1000;
        break;
      case Param::HardTime:
        hardTimeLimit = it.second.as<decltype(hardTimeLimit)>() * 1000;
        break;
    }
}
std::ostream& operator<<(std::ostream& os, const LimitInfo& lim)
{
  os << "[info] Time limit: " << lim.timeLimit / 1000.0 << " ms ("
     << lim.timeLimit / 1e6 << " s)\n";
  os << "[info] Hard time limit: " << lim.hardTimeLimit / 1000.0 << " ms ("
     << lim.hardTimeLimit / 1e6 << " s)\n";
  os << "[info] Memory limit: " << lim.memoryLimit / 1024.0 << " MiB ("
     << lim.memoryLimit / (1024 * 1024.0) << " GiB)\n";
  os << "[info] Hard memory limit: " << lim.hardMemoryLimit / 1024.0 << " MiB ("
     << lim.hardMemoryLimit / (1024.0 * 1024) << " GiB)";
  return os;
}

void Platform::init()
{
  threadId = System::GetThreadId();
  timeProtect.create();
}
}  // namespace apdebug::Testcase
