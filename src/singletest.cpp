#include <yaml-cpp/yaml.h>

#include <cstring>
#include <filesystem>
#include <iostream>
#include <unordered_map>

#include "include/color.h"
#include "include/define.h"
#include "include/io.h"
#include "include/testcase.h"
using namespace apdebug;
using std::cout;
using std::strcmp;
namespace Escape = apdebug::Output::Escape;

typedef Testcase::BasicTemplate TestTemplate;
typedef Testcase::BasicTest TestcaseType;

namespace {
enum class Param { Version, Data, Testcase };
static const std::unordered_map<std::string, Param> par{
    {"version", Param::Version},
    {"data", Param::Data},
    {"basic", Param::Testcase}};
}  // namespace

YAML::Node file, config;
bool version;
Testcase::Platform plat;
std::string input, answer;
TestTemplate tmpl;

void parseArgument(int argc, const char* argv[])
{
  file = YAML::LoadFile(argv[1]);
  config = file["single"];
  for (const auto& i : config)
    switch (par.at(i.first.Scalar())) {
      case Param::Version:
        version = i.second.as<bool>();
        break;
      case Param::Data: {
        for (const auto& it : i.second) {
          if (it.first.Scalar() == "input")
            input = it.second.as<std::string>();
          else if (it.first.Scalar() == "answer")
            answer = it.second.as<std::string>();
        }
        break;
      }
      case Param::Testcase:
        tmpl.parseArgument(i.second);
        break;
    }
  if (argc > 2) {
    input = argv[2];
    if (argc > 3) answer = argv[3];
  }
}

int main(int argc, const char* argv[])
{
  System::consoleInit();
  parseArgument(argc, argv);
  if (version) Output::PrintVersion("Single test runner", cout);
  tmpl.platform = &plat;
  plat.init();
  tmpl.init();
  TestcaseType test(std::move(input), std::move(answer), tmpl);
  std::cout << Escape::TextCyan << "[info] Config file: " << argv[1] << "\n";
  test.printRunInfo(cout);
  cout << Escape::TextCyan << static_cast<Testcase::LimitInfo&>(tmpl)
       << Escape::TextBlue << "\n";
  cout << "[info] Start program" << Escape::None << std::endl;
  test.run();
  for (unsigned int i = 0; test.runResult[i]; ++i)
    std::cout << test.runResult[i]->color << test.runResult[i]->verbose << "\n";
  if (test.runPass && !test.tester.path.empty()) {
    cout << Escape::TextBlue << "[info] Start testing" << Escape::None
         << std::endl;
    test.printTestInfo(cout);
    test.test();
  }
  if (test.testResult)
    std::cout << test.testResult->color << test.testResult->verbose << "\n";
  test.release();
  std::cout << Escape::None << "\n";
  return 0;
}