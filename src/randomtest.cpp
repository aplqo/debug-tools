#include "include/define.h"
#include "include/output.h"
#include "include/testcase.h"
#include "include/utility.h"

#include <atomic>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include <fmt/compile.h>
#include <fmt/format.h>

#include <boost/thread/latch.hpp>

using namespace apdebug;
namespace fs = std::filesystem;
namespace SGR = Output::SGR;

typedef Testcase::TraditionalTemplate TestTemplateBase;
typedef Testcase::TraditionalTest TestcaseType;

const std::chrono::milliseconds print_duration(100);
const unsigned int maxStdRetries = 20, maxVaRetries = 20;
typedef Output::Table<12> ResultTable;
ResultTable results(std::array<const char*, 12> {
                        "Id", "State(Run)", "State(Test)",
                        "Input", "Output", "Answer", "Diff",
                        "Time(ms)", "Time(us)", "Memory(MiB)", "Memory(KiB)", "Details" },
    SGR::None);
enum class ResultColumn
{
    id,
    runState,
    testState,
    input,
    output,
    answer,
    differ,
    msTime,
    usTime,
    mbMemory,
    kbMemory,
    detail
};
struct TestPointTemplate : public TestTemplateBase
{
    Process::Command generator, validor, standard;
    fs::path tmpdir;
    bool created;

    void globalInit();
    void threadInit();
    bool parseArgument(int& argc, const char* const argv[]);
    void print(std::ostream& os) const;
} global;
class TestPoint : private TestcaseType
{
public:
    TestPoint(const unsigned int tid, const unsigned int id, TestPointTemplate& tp);
    bool generate();
    bool run();
    void printTable(ResultTable& dest);

    using TestcaseType::accept;

protected:
    void release();
    unsigned int tid, id;
    Process::Command generator, validor, standard;
};
static const Testcase::Result judgeFail {
    .name = "Fail",
    .color = SGR::None,
    .details = "Generate data failed"
};

void TestPointTemplate::globalInit()
{
    if (!fs::exists(tmpdir))
    {
        fs::create_directory(tmpdir);
        created = true;
    }
    else
        created = false;
    autodiff.enable = true;
    autodiff.size = 0;
    autodiff.differ = "{input}.diff";
}
void TestPointTemplate::threadInit()
{
    using namespace fmt::literals;
    platform->init();
    TestTemplateBase::init();
}
bool TestPointTemplate::parseArgument(int& argc, const char* const argv[])
{
    if (TestTemplateBase::parseArgument(argc, argv))
        return true;
    else if (!strcmp(argv[argc], "-tmpdir"))
        tmpdir = argv[++argc];
    else if (!strcmp(argv[argc], "-generator"))
        generator.path = argv[++argc];
    else if (!strcmp(argv[argc], "-gen-args"))
        generator.parseArgument(++argc, argv);
    else if (!strcmp(argv[argc], "-standard"))
        standard.path = argv[++argc];
    else if (!strcmp(argv[argc], "-std-args"))
        standard.parseArgument(++argc, argv);
    else if (!strcmp(argv[argc], "-validator"))
        validor.path = argv[++argc];
    else if (!strcmp(argv[argc], "-valargs"))
        validor.parseArgument(++argc, argv);
    else
        return false;
    return true;
}
void TestPointTemplate::print(std::ostream& os) const
{
    os << SGR::None << SGR::TextCyan;
    os << "[Info] Temporary directory: " << tmpdir;
    if (created)
        os << "(created)";
    os.put('\n');
    os << "[info] Generator command line: " << generator << "\n";
    if (!validor.path.empty())
        os << "[info] Validator command line: " << validor << "\n";
    if (!standard.path.empty())
        os << "[info] Standard command line: " << standard << "\n";
    os << "[info] Program command line: " << program << "\n";
    if (!tester.path.empty())
        os << "[info] Test comman line: " << tester << "\n";
    os << static_cast<const Testcase::LimitInfo&>(*this) << "\n";
    os << SGR::None;
}

TestPoint::TestPoint(const unsigned int tid, const unsigned int id, TestPointTemplate& tp)
    : TestcaseType(
        tp.tmpdir / fmt::format(FMT_COMPILE("{}-{}.in"), tp.platform->threadId, id),
        tp.tmpdir / fmt::format(FMT_COMPILE("{}-{}.ans"), tp.platform->threadId, id),
        tp)
    , tid(tid)
    , id(id)
    , generator(tp.generator)
    , validor(tp.validor)
    , standard(tp.standard)
{
    using namespace fmt::literals;
    this->id = id;
    multiReplace3(
        fmt::make_format_args("input"_a = input, "output"_a = output, "answer"_a = answer, "thread"_a = tp.platform->threadId),
        generator, validor, standard);
    generator.finalizeForExec();
    validor.finalizeForExec();
    standard.finalizeForExec();
}
bool TestPoint::generate()
{
    if (standard.path.empty() && validor.path.empty())
    {
        generator.execute().wait();
        return true;
    }
    unsigned int fv = maxVaRetries, fs = maxStdRetries;
    static auto tryRun = [](Process::Command& r, unsigned int& i) -> bool {
        if (r.path.empty())
            return true;
        if (r.execute().wait())
        {
            --i;
            return false;
        }
        return true;
    };
    do
    {
        generator.execute().wait();
        if (!tryRun(validor, fv))
            continue;
        if (!tryRun(standard, fs))
            continue;
        return true;
    } while (fv && fs);
    runResult[0] = &judgeFail;
    testResult = &Testcase::Skip;
    runPass = accept = false;
    output = "<null>";
    return false;
}
bool TestPoint::run()
{
    TestcaseType::run();
    parse();
    test();
    release();
    return accept;
}
void TestPoint::release()
{
    TestcaseType::release();
    if (accept)
    {
        Utility::removeFile(input);
        Utility::removeFile(answer);
    }
}
void TestPoint::printTable(ResultTable& dest)
{
    if (!runPass || (testPass && !accept) || tester.path.empty())
        dest.newColumn(runResult[0]->color);
    else
        dest.newColumn(testResult->color);
    dest.writeColumnList<ResultColumn, std::string&&>({ { ResultColumn::id, fmt::format("{}.{}", tid, id) },
        { ResultColumn::runState, std::string(runResult[0]->name) + (runResult[1] ? runResult[1]->name : "") },
        { ResultColumn::testState, std::string(testResult->name) },
        { ResultColumn::input, std::move(const_cast<std::string&>(input)) },
        { ResultColumn::output, std::move(output) },
        { ResultColumn::answer, std::move(const_cast<std::string&>(answer)) },
        { ResultColumn::differ, std::move(diff.differ) },
        { ResultColumn::mbMemory, std::to_string(runMemory / 1024.0) },
        { ResultColumn::kbMemory, std::to_string(runMemory) },
        { ResultColumn::msTime, fmt::format("{} / {} / {}", runTime.real / 1e3, runTime.user / 1e3, runTime.sys / 1e3) },
        { ResultColumn::usTime, fmt::format("{} / {} / {}", runTime.real, runTime.user, runTime.sys) },
        { ResultColumn::detail, std::string(runResult[0]->details) } });
}
std::mutex tableLock;
std::atomic_flag empty;

boost::latch* lat;
std::atomic_ulong testedCount; // test count
std::atomic_bool fail = false;
unsigned long times;
bool stop = false, showall = true, realTime = false;

inline bool isRun()
{
    return testedCount.load() < times && !(stop && fail.load());
}
template <bool realTime>
void PrintThread()
{
    {
        lat->wait();
        {
            std::cout << "Test Results:\n";
            std::lock_guard lk(tableLock);
            results.printHeader(std::cout);
        }
        std::cout.flush();
    }
    ResultTable local;
    while (isRun())
    {
        if (!empty.test_and_set())
        {
            std::lock_guard lk(tableLock);
            local.mergeData(std::move(results));
        }
        local.printAll(std::cout);
        std::cout.flush();
        std::this_thread::sleep_for(print_duration);
    }
}
template <>
void PrintThread<false>()
{
    unsigned long lst = 0;
    while (isRun())
    {
        if (testedCount != lst)
        {
            lst = testedCount;
            std::cout << SGR::TextBlue << "\rTesting " << lst << " data..." << std::flush;
        }
        std::this_thread::sleep_for(print_duration);
    }
    std::cout.put('\n');
}
template <bool realTime>
void threadMain(const unsigned int tid)
{
    Testcase::Platform plat;
    TestPointTemplate tmpl = global;
    tmpl.platform = &plat;
    tmpl.threadInit();
    if constexpr (realTime)
    {
        const unsigned int tp = log10(times + 1) + 2, len = log10(tid + 1), idl = plat.threadId.length();
        {
            std::lock_guard lk(tableLock);
            results.update(ResultColumn::id, tp + len + 1);
            results.update(ResultColumn::input, idl + tp + 1 + 3);
            results.update(ResultColumn::output, idl + tp + 1 + 4);
            results.update(ResultColumn::answer, idl + tp + 1 + 4);
        }
        lat->count_down_and_wait();
    }
    ResultTable local;
    for (unsigned int i = 0; isRun(); ++i)
    {
        ++testedCount;
        TestPoint tst(tid, i, tmpl);
        if (tst.generate())
            tst.run();
        if (!tst.accept)
            fail = true;
        if (!tst.accept || showall)
            tst.printTable(local);
        if constexpr (realTime)
        {
            if (tableLock.try_lock())
            {
                results.mergeData(std::move(local));
                tableLock.unlock();
                empty.clear();
            }
        }
    }
    {
        std::lock_guard lk(tableLock);
        results.mergeData(std::move(local));
    }
}
void setMinWidth()
{
    constexpr unsigned int rlen = sizeof("(Released)");
    results.update(ResultColumn::msTime, log10(global.hardTimeLimit / 1000 + 1));
    results.update(ResultColumn::usTime, log10(global.hardTimeLimit));
    results.update(ResultColumn::mbMemory, log10(global.hardMemoryLimit / 1024.0 + 1));
    results.update(ResultColumn::kbMemory, log10(global.hardMemoryLimit + 1));
    results.update(ResultColumn::input, rlen);
    results.update(ResultColumn::output, rlen);
    results.update(ResultColumn::answer, rlen);
    results.update(ResultColumn::differ, rlen);
}
template <bool realTime>
void runTest(const unsigned int parallel)
{
    std::thread* th = new std::thread[parallel];
    if constexpr (realTime)
    {
        setMinWidth();
        lat = new boost::latch(parallel);
    }
    for (unsigned int i = 0; i < parallel; ++i)
        th[i] = std::thread(threadMain<realTime>, i);
    PrintThread<realTime>();
    for (unsigned int i = 0; i < parallel; ++i)
        th[i].join();
    delete[] th;
    if constexpr (!realTime)
    {
        std::cout << "Test results:\n";
        results.printHeader(std::cout);
    }
    results.printAll(std::cout);
}

int main(int argc, char* argv[])
{
    if (strcmp(argv[1], "-no-version"))
        Output::PrintVersion("random test runner", std::cout);
    unsigned int parallel = std::thread::hardware_concurrency();
    for (int i = 1; i < argc; ++i)
    {
        if (global.parseArgument(i, argv))
            continue;
        else if (!strcmp(argv[i], "-times"))
            times = atoll(argv[++i]);
        else if (!strcmp(argv[i], "-stop-on-error"))
            stop = true;
        else if (!strcmp(argv[i], "-parallelism"))
            parallel = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-fail-only"))
            showall = false;
        else if (!strcmp(argv[i], "-real-time"))
            realTime = true;
    }
    std::cout << SGR::TextCyan;
    std::cout << "[Info] Test time: " << times << "\n";
    std::cout << "[Info] Parallelism: " << parallel << "\n";
    std::cout << "[Info] Stop on error: " << std::boolalpha << stop << "\n";
    std::cout << "[Info] Show all test: " << std::boolalpha << showall << "\n";
    std::cout << "[info] Real time update table: " << std::boolalpha << realTime << "\n";
    global.globalInit();
    global.print(std::cout);
    std::cout.flush();
    if (realTime)
        runTest<true>(parallel);
    else
        runTest<false>(parallel);
    if (global.created && !fail)
    {
        std::cout << SGR::TextCyan << "\n[Info] Test passed removed temporary directory.";
        remove_all(global.tmpdir);
    }
    std::cout << SGR::None << "\n";
    return 0;
}
