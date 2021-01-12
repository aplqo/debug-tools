#include "include/define.h"
#include "include/output.h"
#include "include/regexseq.h"
#include "include/testcase.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <fmt/format.h>
using apdebug::regex_seq::RegexSeq;
using namespace apdebug;
namespace fs = std::filesystem;
namespace SGR = Output::SGR;
using Output::Table;

typedef Testcase::BasicTemplate TestTemplate;
typedef Testcase::BasicTest TestcaseType;

Testcase::Platform platform;

typedef Table<12> ResultTable;
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
const std::array<const char*, 12> ResultHeader {
    "Id", "State(Run)", "State(Test)", "Input",
    "Output", "Answer", "Differ", "Real", "User", "System",
    "Memory(MiB)", "Details"
};

enum class GroupColumn
{
    id,
    inDir,
    ansDir,
    arg,
#ifdef Interact
    interact,
#endif
    test,
    timeLimit,
    hardTimeLimit,
    memoryLimit,
    hardMemoryLimit,
    verbose
};
#ifdef Interact
const std::array<const char*, 11> GroupHeader {
    "Id", "InDir", "AnsDir", "Argument", "Interactor",
    "Test command", "Time", "Hard time",
    "Memory", "Hard memory", "Verbose"
};
typedef Table<11> GroupTable;
#else
const std::array<const char*, 10> GroupHeader {
    "Id", "Input", "Answer", "Argument",
    "Test command", "Time", "Hard time",
    "Mmory", "Hard memory", "Verbose"
};
typedef Table<10> GroupTable;
#endif

class TestPoint : public TestcaseType
{
public:
    TestPoint(const unsigned int id, const unsigned int gid, std::string&& input, std::string&& answer, TestTemplate& te)
        : TestcaseType(std::move(input), std::move(answer), te)
        , id(id)
        , gid(gid)
    {
    }
    void execute(const bool verbose);
    void writeToTable(ResultTable& dest);

private:
    unsigned int id, gid;
};
void TestPoint::execute(const bool verbose)
{
    std::cout << SGR::Underline << SGR::TextBlue << "[Info] Start program for test #" << gid << "." << id << SGR::None << "\n";
    if (verbose)
        printRunInfo(std::cout);
    std::cout.flush();
    run();
    for (unsigned int i = 0; runResult[i]; ++i)
        std::cout << runResult[i]->color << runResult[i]->verbose << SGR::None << "\n";
    if (runPass && !tester.path.empty())
    {
        std::cout << SGR::TextBlue << "[Info] Start testing for test #" << gid << "." << id << SGR::None << "\n";
        if (verbose)
            printTestInfo(std::cout);
        std::cout.flush();
        test();
        std::cout << testResult->color << testResult->verbose << SGR::None << "\n";
    }
    else
        testResult = &Testcase::ResultConstant::Skip;
    release();
    std::cout << SGR::Underline << SGR::TextBlue << "[Info] Test #" << gid << "." << id << " finished.\n";
}
void TestPoint::writeToTable(ResultTable& dest)
{
    dest.newColumn(finalResult->color);
    dest.writeColumnList<ResultColumn, std::string&&>({ { ResultColumn::id, std::to_string(id) },
        { ResultColumn::runState, std::string(runResult[0]->name) + (runResult[1] ? runResult[1]->name : "") },
        { ResultColumn::testState, std::string(testResult->name) },
        { ResultColumn::input, std::move(input) },
        { ResultColumn::output, std::move(output) },
        { ResultColumn::answer, std::move(answer) },
        { ResultColumn::differ, std::move(diff.differ) },
        { ResultColumn::realTime, fmt::format("{}", runTime.real / 1000.0) },
        { ResultColumn::userTime, fmt::format("{}", runTime.user / 1000.0) },
        { ResultColumn::sysTime, fmt::format("{}", runTime.sys / 1000.0) },
        { ResultColumn::mbMemory, fmt::format("{}", runMemory / 1024.0) },
        { ResultColumn::detail, std::string(runResult[0]->details) } });
}

class TestGroup
{
public:
    TestGroup(const unsigned int id, int& argc, const char* const argv[]);
    TestGroup(const TestGroup& p, const unsigned int id, int& argc, const char* const argv[]);
    void findFile();
    void execute();
    void printConfig(GroupTable& t);
    void printResult(Testcase::Summary& totalSummary);

private:
    static const std::array<const char*, 12> colName;

    void parseArgument(int& argc, const char* const argv[]);
    template <bool rec>
    void findInput();
    template <bool rec>
    void findAnswer();
    template <bool rec>
    std::pair<bool, std::string> isInclude(const fs::path& p, const RegexSeq* r);

    unsigned int gid;
    fs::path indir, ansdir;
    bool inrec = false, ansrec = false, verbose = false;
    RegexSeq *inputPattern, *answerPattern, *testPattern;
    TestTemplate tmpl;
    ResultTable results;
    std::vector<std::pair<fs::path, fs::path>> tests;
    std::unordered_map<std::string, int> table;
    Testcase::Summary summary;
};

TestGroup::TestGroup(const unsigned int id, int& argc, const char* const argv[])
    : gid(id)
    , results(ResultHeader, SGR::None)
{
    parseArgument(argc, argv);
}
TestGroup::TestGroup(const TestGroup& p, const unsigned int id, int& argc, const char* const argv[])
    : TestGroup(p)
{
    gid = id;
    parseArgument(argc, argv);
}
void TestGroup::parseArgument(int& argc, const char* const argv[])
{
    for (; strcmp(argv[argc], ";"); ++argc)
    {
        if (!strcmp(argv[argc], "-test-regex"))
            testPattern = new RegexSeq(++argc, argv);
        else if (!strcmp(argv[argc], "-indir"))
        {
            if (!strcmp(argv[argc + 1], "-R"))
            {
                ++argc;
                inrec = true;
            }
            else
                inrec = false;
            indir = argv[++argc];
        }
        else if (!strcmp(argv[argc], "-in-regex"))
            inputPattern = new RegexSeq(++argc, argv);
        else if (!strcmp(argv[argc], "-ansdir"))
        {
            if (!strcmp(argv[argc + 1], "-R"))
            {
                ++argc;
                ansrec = true;
            }
            else
                ansrec = false;
            ansdir = argv[++argc];
        }
        else if (!strcmp(argv[argc], "-ans-regex"))
            answerPattern = new RegexSeq(++argc, argv);
        else if (!strcmp(argv[argc], "-verbose"))
            verbose = true;
        else if (!strcmp(argv[argc], "-silent"))
            verbose = false;
        else if (tmpl.parseArgument(argc, argv))
            continue;
    }
}
void TestGroup::execute()
{
    tmpl.platform = &platform;
    tmpl.init();
    std::cout << SGR::TextBlue << SGR::Underline << "[Info] Start testing for group #" << gid;
    for (unsigned int i = 0; i < tests.size(); ++i)
    {
        std::cout.put('\n');
        TestPoint tst(i, gid, tests[i].first.string(), tests[i].second.string(), tmpl);
        tst.execute(verbose);
        tst.writeToTable(results);
        summary.insert(fmt::format(FMT_STRING("{}.{}"), gid, i), tst);
    }
    std::cout << SGR::TextBlue << SGR::Underline << "[Info] Group #" << gid << " finished." << SGR::None << "\n\n";
}
void TestGroup::printResult(Testcase::Summary& totalSummary)
{
    std::cout << SGR::None << "Test result for group #" << gid << "\n";
    if (!tests.size())
    {
        std::cout << SGR::TextRed << "[Err] Can't find any test data." << SGR::None << "\n";
        return;
    }
    results.printHeader(std::cout);
    results.printAll(std::cout);
    std::cout << "Summary: \n";
    summary.print(std::cout);
    std::cout.put('\n');
    totalSummary.mergeData(summary);
}
void TestGroup::findFile()
{
    if (inrec)
        findInput<true>();
    else
        findInput<false>();
    if (ansrec)
        findAnswer<true>();
    else
        findAnswer<false>();
    table.clear();
}
template <bool rec>
void TestGroup::findInput()
{
    unsigned int cur = 0;
    std::conditional_t<rec, fs::recursive_directory_iterator, fs::directory_iterator> init(indir);
    for (auto& i : init)
        if (i.is_regular_file())
        {
            if (const auto [suc, hsh] = isInclude<rec>(i.path(), inputPattern); suc)
            {
                tests.emplace_back(i.path(), "");
                table[hsh] = cur;
                ++cur;
            }
        }
}
template <bool rec>
void TestGroup::findAnswer()
{
    if (!exists(ansdir))
        return;
    std::conditional_t<rec, fs::recursive_directory_iterator, fs::directory_iterator> ansit(ansdir);
    for (auto& i : ansit)
        if (i.is_regular_file())
            if (const auto [suc, hsh] = isInclude<rec>(i.path(), answerPattern); suc)
                if (auto it = table.find(hsh); it != table.end())
                    tests[it->second].second = i.path();
}
template <bool rec>
std::pair<bool, std::string> TestGroup::isInclude(const fs::path& p, const RegexSeq* r)
{
    std::string s;
    if constexpr (rec)
        s = p.string();
    else
        s = p.filename().string();
    const auto [suc1, val1] = testPattern->eval(s);
    if (!suc1)
        return std::make_pair(false, "");
    const auto [suc2, val2] = r->eval(s);
    if (!suc2)
        return std::make_pair(false, "");
    return std::make_pair(true, val1 + val2);
}
void TestGroup::printConfig(GroupTable& dest)
{
    static constexpr double ms = 1e3, sec = 1e6, mb = 1024.0, gb = mb * 1024;
    using namespace std::string_literals;
    dest.newColumn(SGR::TextCyan);
    dest.writeColumnList<GroupColumn, std::string&&>({ { GroupColumn::id, std::to_string(gid) },
        { GroupColumn::inDir, inrec ? indir.string() + "(recursive)" : indir.string() },
        { GroupColumn::ansDir, ansrec ? ansdir.string() + "(recursive)" : ansdir.string() },
        { GroupColumn::arg, Output::writeToString(tmpl.program) },
#ifdef Interact
        { GroupColumn::interact, Output::writeToString(tmpl.interactor) },
#endif
        { GroupColumn::test, Output::writeToString(tmpl.tester) },
        { GroupColumn::timeLimit, fmt::format("{} ms ({} s)", tmpl.timeLimit / ms, tmpl.timeLimit / sec) },
        { GroupColumn::hardTimeLimit, fmt::format("{} ms ({} s)", tmpl.hardTimeLimit / ms, tmpl.hardTimeLimit / sec) },
        { GroupColumn::memoryLimit, fmt::format("{} MiB ({} GiB)", tmpl.memoryLimit / mb, tmpl.memoryLimit / gb) },
        { GroupColumn::hardMemoryLimit, fmt::format("{} MiB ({} GiB)", tmpl.hardMemoryLimit / mb, tmpl.hardMemoryLimit / gb) },
        { GroupColumn::verbose, verbose ? "True"s : "False"s } });
}
std::vector<TestGroup> grp;

int main(int argc, char* argv[])
{
    if (strcmp(argv[1], "-no-version"))
        Output::PrintVersion("group test runner", std::cout);
    for (int p = 1, id = 0; p < argc; ++p)
    {
        if (strcmp(argv[p], "-use"))
            grp.emplace_back(id++, p, argv);
        else
        {
            const unsigned int u = atoi(argv[++p]);
            grp.emplace_back(grp[u], id++, ++p, argv);
        }
    }
    std::cout << SGR::TextCyan << "[Info] Group config: \n";
    {
        GroupTable gtable(GroupHeader, SGR::TextCyan);
        for (auto& i : grp)
            i.printConfig(gtable);
        gtable.printHeader(std::cout);
        gtable.printAll(std::cout);
        std::cout.put('\n');
    }
    System::systemInit();
    platform.init();
    for (auto& i : grp)
    {
        i.findFile();
        i.execute();
    }
    Testcase::Summary total;
    for (auto& i : grp)
        i.printResult(total);
    std::cout << "Summary for all group: \n";
    total.print(std::cout);
    return 0;
}
