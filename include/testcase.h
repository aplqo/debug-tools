#ifndef TESTCASE_H
#define TESTCASE_H

#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <memory>
#include <regex>
#include <string>
#include <string_view>
#include <type_traits>

#include "include/define.h"
#include "include/testtools.h"
#include "system.h"

namespace apdebug {
namespace Testcase {
static const inline System::MemoryUsage defaultMemory =
    System::MemoryUsage(1024) * 1024 * 3;
static const inline unsigned long long defaultTime = 1000000;

struct Result {
  enum class Type {
    TLE,
    MLE,
    AC,
    WA,
    Pass,
    WAKilled,
    RE,
    Warn,
    HardMLE,
    HardTLE,
    Protocol,
    Other,
    Unknown,
    Skip
  } type;
  const char *name, *color;
  std::string verbose, details;
};
/*-----Result constants-----*/
namespace ResultConstant {
inline const unsigned int TypeNumber = 14;
extern const char *name[TypeNumber], *color[TypeNumber];
extern const Result hardTLE;
extern const Result hardMLE;
extern const Result TLE;
extern const Result MLE;
extern const Result Skip;
extern const Result TestAccept;
extern const Result InteractRunWA;
}  // namespace ResultConstant

struct LimitInfo {
  unsigned long long timeLimit = defaultTime, hardTimeLimit = defaultTime * 10;
  System::MemoryUsage memoryLimit = defaultMemory,
                      hardMemoryLimit = 1024 * 1024 * 3;

  void parseArgument(const YAML::Node& nod);
  friend std::ostream& operator<<(std::ostream& os, const LimitInfo& lim);
};
struct TestResult {
  bool runPass = true, testPass = true, accept = true;
  const Result *runResult[3]{}, *testResult = nullptr, *finalResult;
  System::TimeUsage runTime;
  System::MemoryUsage runMemory;

 protected:
  Result mem[4], *cur = mem;
};
struct Platform {
  std::string threadId;
  System::TimeLimit timeProtect;
  System::MemoryLimit memoryProtect;
  System::SharedMemory sharedMemory;
  unsigned int count = 0;

  void init();
};

struct BasicTemplate : public LimitInfo {
  System::Command program, tester;
#ifdef Interact
  System::Command interactor;
#endif
  TestTools::AutoDiff diff;
  TestTools::TemporaryFile tmpfiles;
  Platform* platform;

  void init();
  void parseArgument(const YAML::Node& node);
};
class BasicTest : public TestResult, private BasicTemplate {
 public:
  BasicTest(std::filesystem::path&& input, std::filesystem::path&& answer,
            const BasicTemplate& tmpl);
  BasicTest(const BasicTest&) = delete;
  BasicTest& operator=(const BasicTest&) = delete;
  void run();
  void test();
  void release();
  void printRunInfo(std::ostream& os);
  void printTestInfo(std::ostream& os);

  std::filesystem::path input, output, answer;
  using BasicTemplate::diff, BasicTemplate::program, BasicTemplate::tester;

 private:
  void parse();
  int exitStatus;
};
class Summary {
 public:
  void mergeData(const Summary& other);
  void insert(std::string&& s, const TestResult& tst);
  void print(std::ostream& os);

 private:
  template <class T>
  struct ValSummary {
    struct ValFrom {
      T val;
      std::shared_ptr<std::string> from;
    } min{.val = std::numeric_limits<T>::max()}, max{.val = 0};
    T sum = 0;
    void update(T val, std::shared_ptr<std::string>& from);
    void mergeData(const ValSummary& other);
    template <unsigned long long unit>
    std::string format(const double cnt) const;
  };
  struct SummaryEntry {
    unsigned int count = 0;
    ValSummary<decltype(System::TimeUsage::real)> timeReal, timeUser, timeSys;
    ValSummary<System::MemoryUsage> memory;

    void update(std::shared_ptr<std::string>& s, const TestResult& tst);
  } entries[ResultConstant::TypeNumber + 1];
};
}  // namespace Testcase
}  // namespace apdebug
#endif
