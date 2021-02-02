#include "include/color.h"
#include "include/define.h"
#include "include/io.h"
#include "include/regexseq.h"
#include "include/table.h"
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
namespace Escape = Output::Escape;

typedef Testcase::BasicTemplate TestTemplate;
typedef Testcase::BasicTest TestcaseType;

Testcase::Platform platform;

typedef Table::Table<12> ResultTable;
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
    timeLimit,
    hardTimeLimit,
    memoryLimit,
    hardMemoryLimit,
    verbose
};
const std::array<const char*, 8> GroupHeader {
    "Id", "InDir", "AnsDir", "Time", "Hard time",
    "Memory", "Hard memory", "Verbose"
};
typedef Table::Table<8> GroupTable;

class TestPoint : public TestcaseType
{
public:
    TestPoint(const unsigned int id, const unsigned int gid, fs::path&& input, fs::path&& answer, TestTemplate& te)
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
    std::cout << Escape::Underline << Escape::TextBlue << "[Info] Start program for test #" << gid << "." << id << Escape::None << "\n";
    if (verbose)
        printRunInfo(std::cout);
    std::cout.flush();
    run();
    for (unsigned int i = 0; runResult[i]; ++i)
        std::cout << runResult[i]->color << runResult[i]->verbose << Escape::None << "\n";
    if (runPass && !tester.path.empty())
    {
        std::cout << Escape::TextBlue << "[Info] Start testing for test #" << gid << "." << id << Escape::None << "\n";
        if (verbose)
            printTestInfo(std::cout);
        std::cout.flush();
        test();
    }
    if (testResult)
        std::cout << testResult->color << testResult->verbose << Escape::None << "\n";
    else
        testResult = &Testcase::ResultConstant::Skip;
    release();
    std::cout << Escape::Underline << Escape::TextBlue << "[Info] Test #" << gid << "." << id << " finished.\n";
}
void TestPoint::writeToTable(ResultTable& dest)
{
    using namespace std::string_literals;
    dest.newColumn(finalResult->color);
    dest.writeColumnList<ResultColumn, std::string&&>({ { ResultColumn::id, std::to_string(id) },
        { ResultColumn::runState, std::string(runResult[0]->name) + (runResult[1] ? " "s + runResult[1]->name : ""s) },
        { ResultColumn::testState, std::string(testResult->name) },
        { ResultColumn::input, input.string() },
        { ResultColumn::output, output.string() },
        { ResultColumn::answer, answer.string() },
        { ResultColumn::differ, diff.differ.string() },
        { ResultColumn::realTime, fmt::format("{}", runTime.real / 1000.0) },
        { ResultColumn::userTime, fmt::format("{}", runTime.user / 1000.0) },
        { ResultColumn::sysTime, fmt::format("{}", runTime.sys / 1000.0) },
        { ResultColumn::mbMemory, fmt::format("{}", runMemory / 1024.0) },
        { ResultColumn::detail, std::string(runResult[0]->details) } });
}

class TestGroup
{
public:
    TestGroup(const unsigned int id, const YAML::Node& node);
    TestGroup(const TestGroup& p, const unsigned int id, const YAML::Node& node);
    void findFile();
    void execute();
    void printConfig(GroupTable& t);
    void printResult(Testcase::Summary& totalSummary);

private:
    static const std::array<const char*, 12> colName;

    void parseArgument(const YAML::Node& node);
    template <bool rec>
    void findInput();
    template <bool rec>
    void findAnswer();
    template <bool rec>
    std::pair<bool, std::string> isInclude(const fs::path& p, const RegexSeq* r);

    unsigned int gid;
    struct DataPath
    {
        fs::path path;
        bool rec = false;
        RegexSeq* pattern;

        void parseArgument(const YAML::Node& node);
    } input, answer;
    bool verbose = false;
    RegexSeq* testPattern;
    TestTemplate tmpl;
    ResultTable results;
    std::vector<std::pair<fs::path, fs::path>> tests;
    std::unordered_map<std::string, int> table;
    Testcase::Summary summary;
};

namespace
{
    enum class DataParam
    {
        TestRegex,
        Input,
        Answer
    };
    static const std::unordered_map<std::string, DataParam> dataPar {
        { "regex", DataParam::TestRegex },
        { "input", DataParam::Input },
        { "answer", DataParam::Answer }
    };
    enum class GroupParam
    {
        Verbose,
        Data,
        Basic,
        Use
    };
    static const std::unordered_map<std::string, GroupParam> groupPar {
        { "verbose", GroupParam::Verbose },
        { "data", GroupParam::Data },
        { "basic", GroupParam::Basic },
        { "use", GroupParam::Use }
    };
    enum class DirParam
    {
        Path,
        Recursive,
        Regex
    };
    static const std::unordered_map<std::string, DirParam> dirPar {
        { "path", DirParam::Path },
        { "recursive", DirParam::Recursive },
        { "regex", DirParam::Regex }
    };
}

void TestGroup::DataPath::parseArgument(const YAML::Node& node)
{
    for (const auto& it : node)
        switch (dirPar.at(it.first.Scalar()))
        {
        case DirParam::Path:
            path = it.second.as<std::string>();
            break;
        case DirParam::Recursive:
            rec = it.second.as<bool>();
            break;
        case DirParam::Regex:
            pattern = new RegexSeq(it.second);
            break;
        }
}
TestGroup::TestGroup(const unsigned int id, const YAML::Node& node)
    : gid(id)
    , results(ResultHeader, Escape::None)
{
    parseArgument(node);
}
TestGroup::TestGroup(const TestGroup& p, const unsigned int id, const YAML::Node& node)
    : TestGroup(p)
{
    gid = id;
    parseArgument(node);
}
void TestGroup::parseArgument(const YAML::Node& node)
{
    for (const auto& it : node)
        switch (groupPar.at(it.first.Scalar()))
        {
        case GroupParam::Verbose:
            verbose = it.second.as<bool>();
            break;
        case GroupParam::Data:
        {
            for (const auto& dit : it.second)
                switch (dataPar.at(dit.first.Scalar()))
                {
                case DataParam::TestRegex:
                    testPattern = new RegexSeq(dit.second);
                    break;
                case DataParam::Input:
                    input.parseArgument(dit.second);
                    break;
                case DataParam::Answer:
                    answer.parseArgument(dit.second);
                    break;
                }
            break;
        }
        case GroupParam::Basic:
            tmpl.parseArgument(it.second);
            break;
        default:
            break;
        }
}
void TestGroup::execute()
{
    tmpl.platform = &platform;
    tmpl.init();
    std::cout << Escape::TextBlue << Escape::Underline << "[Info] Start testing for group #" << gid;
    for (unsigned int i = 0; i < tests.size(); ++i)
    {
        std::cout.put('\n');
        TestPoint tst(i, gid, std::move(tests[i].first), std::move(tests[i].second), tmpl);
        tst.execute(verbose);
        tst.writeToTable(results);
        summary.insert(fmt::format(FMT_STRING("{}.{}"), gid, i), tst);
    }
    std::cout << Escape::TextBlue << Escape::Underline << "[Info] Group #" << gid << " finished." << Escape::None << "\n\n";
}
void TestGroup::printResult(Testcase::Summary& totalSummary)
{
    std::cout << Escape::None << "Test result for group #" << gid << "\n";
    if (!tests.size())
    {
        std::cout << Escape::TextRed << "[Err] Can't find any test data." << Escape::None << "\n";
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
    if (input.rec)
        findInput<true>();
    else
        findInput<false>();
    if (answer.rec)
        findAnswer<true>();
    else
        findAnswer<false>();
    table.clear();
}
template <bool rec>
void TestGroup::findInput()
{
    unsigned int cur = 0;
    std::conditional_t<rec, fs::recursive_directory_iterator, fs::directory_iterator> init(input.path);
    for (auto& i : init)
        if (i.is_regular_file())
        {
            if (const auto [suc, hsh] = isInclude<rec>(i.path(), input.pattern); suc)
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
    if (!exists(answer.path))
        return;
    std::conditional_t<rec, fs::recursive_directory_iterator, fs::directory_iterator> ansit(answer.path);
    for (auto& i : ansit)
        if (i.is_regular_file())
            if (const auto [suc, hsh] = isInclude<rec>(i.path(), answer.pattern); suc)
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
    dest.newColumn(Escape::TextCyan);
    dest.writeColumnList<GroupColumn, std::string&&>({ { GroupColumn::id, std::to_string(gid) },
        { GroupColumn::inDir, input.rec ? input.path.string() + "(recursive)" : input.path.string() },
        { GroupColumn::ansDir, answer.rec ? answer.path.string() + "(recursive)" : answer.path.string() },
        { GroupColumn::timeLimit, fmt::format("{} ms ({} s)", tmpl.timeLimit / ms, tmpl.timeLimit / sec) },
        { GroupColumn::hardTimeLimit, fmt::format("{} ms ({} s)", tmpl.hardTimeLimit / ms, tmpl.hardTimeLimit / sec) },
        { GroupColumn::memoryLimit, fmt::format("{} MiB ({} GiB)", tmpl.memoryLimit / mb, tmpl.memoryLimit / gb) },
        { GroupColumn::hardMemoryLimit, fmt::format("{} MiB ({} GiB)", tmpl.hardMemoryLimit / mb, tmpl.hardMemoryLimit / gb) },
        { GroupColumn::verbose, verbose ? "True"s : "False"s } });
}
DynArray::DynArray<TestGroup> grp;
YAML::Node file, config;
bool version = true;

void parseArgument(int argc, const char* argv[])
{
    file = YAML::LoadFile(argv[1]);
    config = file["group"];
    for (const auto& it : config)
        if (it.first.Scalar() == "version")
            version = it.second.as<bool>();
        else if (it.first.Scalar() == "groups")
        {
            grp.allocate(it.second.size());
            TestGroup* ptr = grp.data;
            unsigned int gid = 0;
            for (const auto& git : it.second)
                if (const YAML::Node& i = git["use"]; i)
                    new (ptr++) TestGroup(grp.data[i.as<unsigned int>()], gid++, git);
                else
                    new (ptr++) TestGroup(gid++, git);
        }
}

int main(int argc, const char* argv[])
{
    System::consoleInit();
    parseArgument(argc, argv);
    if (version)
        Output::PrintVersion("group test runner", std::cout);
    std::cout << Escape::TextCyan << "[Info] Group config: \n";
    {
        GroupTable gtable(GroupHeader, Escape::TextCyan);
        for (auto& i : grp)
            i.printConfig(gtable);
        gtable.printHeader(std::cout);
        gtable.printAll(std::cout);
        std::cout.put('\n');
    }
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
