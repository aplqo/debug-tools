#include "include/color.h"
#include "include/define.h"
#include "include/io.h"
#include "include/pathFormat.h"
#include "include/replace.h"
#include "include/table.h"
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
namespace Escape = Output::Escape;

typedef Testcase::BasicTemplate TestTemplateBase;
typedef Testcase::BasicTest TestcaseType;

const std::chrono::milliseconds print_duration(500);
const unsigned int maxStdRetries = 20, maxVaRetries = 20;
typedef Table::Table<12> ResultTable;
ResultTable results(std::array<const char*, 12> {
                        "Id", "State(Run)", "State(Test)",
                        "Input", "Output", "Answer", "Diff",
                        "Real", "User", "System", "Memory", "Details" },
    Escape::None);
Testcase::Summary summary;

enum class ResultColumn
{
    id,
    runState,
    testState,
    input,
    output,
    answer,
    differ,
    realTime,
    userTime,
    sysTime,
    mbMemory,
    detail
};
struct TestPointTemplate : public TestTemplateBase
{
    System::Command generator, validor, standard;
    fs::path tmpdir;
    bool created;

    void globalInit();
    void threadInit();
    bool parseArgument(int& argc, const char* const argv[]);
    void print(std::ostream& os) const;
} global;
class TestPoint : public TestcaseType
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
    System::Command generator, validor, standard;
};
static const Testcase::Result judgeFail {
    .type = Testcase::Result::Type::Other,
    .name = "Fail",
    .color = Escape::None,
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
    diff.enable = true;
    diff.size = 0;
    diff.differTemplate = "{input}.diff";
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
    os << Escape::None << Escape::TextCyan;
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
    os << Escape::None;
}

TestPoint::TestPoint(const unsigned int tid, const unsigned int id, TestPointTemplate& tp)
    : TestcaseType(
        (tp.tmpdir / fmt::format(FMT_COMPILE("{}-{}.in"), tp.platform->threadId, id)).string(),
        (tp.tmpdir / fmt::format(FMT_COMPILE("{}-{}.ans"), tp.platform->threadId, id)).string(),
        tp)
    , tid(tid)
    , id(id)
    , generator(tp.generator)
    , validor(tp.validor)
    , standard(tp.standard)
{
    using namespace fmt::literals;
    this->id = id;
    Replace::ReplaceStrict(
        fmt::make_format_args(
            "input"_a = input,
            "output"_a = output,
            "answer"_a = answer,
            "thread"_a = tp.platform->threadId),
        generator, validor, standard);
}
bool TestPoint::generate()
{
    if (standard.path.empty() && validor.path.empty())
    {
        generator.execute().wait();
        return true;
    }
    unsigned int fv = maxVaRetries, fs = maxStdRetries;
    static auto tryRun = [](System::Command& r, unsigned int& i) -> bool {
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
    runResult[0] = finalResult = &judgeFail;
    testResult = &Testcase::ResultConstant::Skip;
    runPass = accept = false;
    runTime = System::TimeUsage {};
    runMemory = 0;
    output = "<null>";
    return false;
}
bool TestPoint::run()
{
    TestcaseType::run();
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
    dest.newColumn(finalResult->color);
    dest.writeColumnList<ResultColumn, std::string&&>({ { ResultColumn::id, fmt::format("{}.{}", tid, id) },
        { ResultColumn::runState, std::string(runResult[0]->name) + (runResult[1] ? std::string(" ") + runResult[1]->name : "") },
        { ResultColumn::input, input.string() },
        { ResultColumn::output, output.string() },
        { ResultColumn::answer, answer.string() },
        { ResultColumn::differ, diff.differ.string() },
        { ResultColumn::testState, std::string(testResult->name) },
        { ResultColumn::realTime, fmt::format(FMT_COMPILE("{}"), runTime.real / 1000.0) },
        { ResultColumn::userTime, fmt::format(FMT_COMPILE("{}"), runTime.user / 1000.0) },
        { ResultColumn::sysTime, fmt::format(FMT_COMPILE("{}"), runTime.sys / 1000.0) },
        { ResultColumn::mbMemory, fmt::format(FMT_COMPILE("{}"), runMemory / 1024.0) },
        { ResultColumn::detail, std::string(runResult[0]->details) } });
}
std::mutex tableLock;
std::atomic_flag empty;

boost::latch* lat;
std::atomic_ulong testedCount; // test count
std::atomic_bool fail = false;
unsigned long times;
bool stop = false, showall = true, realTime = false;
unsigned long long begin, spentTime;

inline bool isRun()
{
    return testedCount.load(std::memory_order_consume) < times && !(stop && fail.load(std::memory_order_consume));
}
struct Status
{
    unsigned long lst;
    unsigned long long lastTime;

    bool printStatus()
    {
        const unsigned long curCount = testedCount.load(std::memory_order_consume);
        if (curCount == lst)
            return false;
        const unsigned long long curTime = System::Usage::getRealTime();
        const double period = (curTime - lastTime) / 1e6 / System::unit.real, total = (curTime - begin) / 1e6 / System::unit.real;
        std::cout << Escape::ClearLine << Escape::TextBlue << "Testing " << lst << " data. Spent " << total << "s."
                  << " Average " << curCount / total << "/sec Current " << (curCount - lst) / period << "/sec";
        lastTime = curTime;
        lst = curCount;
        return true;
    }
};
template <bool realTime>
void PrintThread()
{
    ResultTable local;
    {
        lat->wait();
        {
            std::cout << "Test Results:\n";
            std::lock_guard lk(tableLock);
            results.printHeader(std::cout);
            local.mergeData(std::move(results));
        }
        std::cout.flush();
    }
    Status stat { .lst = 0, .lastTime = begin };
    while (isRun())
    {
        bool flush = false;
        if (!empty.test_and_set(std::memory_order_consume))
        {
            {
                std::lock_guard lk(tableLock);
                local.mergeData(std::move(results));
            }
            std::cout << Escape::ClearLine << Escape::None;
            local.printAll(std::cout);
            flush = true;
        }
        flush = stat.printStatus() || flush;
        if (flush)
            std::cout.flush();
        std::this_thread::sleep_for(print_duration);
    }
    spentTime = System::Usage::getRealTime() - begin;
    std::cout << Escape::None << "\n";
}
template <>
void PrintThread<false>()
{
    Status stat { .lst = 0, .lastTime = begin };
    while (isRun())
    {
        if (stat.printStatus())
            std::cout.flush();
        std::this_thread::sleep_for(print_duration);
    }
    spentTime = System::Usage::getRealTime() - begin;
    std::cout << Escape::None << "\n";
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
    Testcase::Summary localSummary;
    for (unsigned int i = 0; isRun(); ++i)
    {
        testedCount.fetch_add(1, std::memory_order_relaxed);
        TestPoint tst(tid, i, tmpl);
        if (tst.generate())
            tst.run();
        if (!tst.accept)
            fail = true;
        if (!tst.accept || showall)
            tst.printTable(local);
        localSummary.insert(fmt::format(FMT_COMPILE("{}.{}"), tid, i), tst);
        if constexpr (realTime)
        {
            if (tableLock.try_lock())
            {
                results.mergeData(std::move(local));
                tableLock.unlock();
                empty.clear(std::memory_order_relaxed);
            }
        }
    }
    {
        std::lock_guard lk(tableLock);
        results.mergeData(std::move(local));
        summary.mergeData(localSummary);
    }
}
void setMinWidth()
{
    constexpr unsigned int rlen = sizeof("(Released)");
    const unsigned int tlen = log10(global.hardTimeLimit / 1000.0 + 1);
    results.update(ResultColumn::realTime, tlen + 5);
    results.update(ResultColumn::userTime, tlen + 5);
    results.update(ResultColumn::sysTime, tlen + 5);
    results.update(ResultColumn::mbMemory, log10(global.hardMemoryLimit / 1024.0 + 1) + 5);
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
    begin = System::Usage::getRealTime();
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
    std::cout << "Summary: \n";
    summary.print(std::cout);
}

int main(int argc, char* argv[])
{
    System::consoleInit();
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
    std::cout << Escape::TextCyan;
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
        std::cout << Escape::TextCyan << "\n[Info] Test passed removed temporary directory.";
        remove_all(global.tmpdir);
    }
    {
        const double time = spentTime / 1e6 / System::unit.real;
        std::cout << Escape::TextBlue << "\n[Info] Test finished in " << time << "s. Average " << testedCount.load() / time << "/sec";
    }
    std::cout << Escape::None << "\n";
    return 0;
}
