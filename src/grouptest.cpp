#include "include/cmdarg.h"
#include "include/define.h"
#include "include/output.h"
#include "include/regexseq.h"
#include "include/testcase.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
using apdebug::regex_seq::RegexSeq;
using apdebug::testcase::result;
using apdebug::testcase::tpoint;
using apdebug::timer::timType;
using namespace std;
using namespace std::filesystem;
using namespace apdebug::out;
using namespace apdebug::args;

using resultTab = table<11>;
enum cols
{
    Id = 0,
    RState = 1,
    TState = 2,
    In = 3,
    Out = 4,
    Ans = 5,
    MsTim = 6,
    UsTim = 7,
    MbMem = 8,
    KbMem = 9,
    Det = 10
};
class point : tpoint
{
public:
    point(int i)
        : id(i)
    {
    }
    void init(const result& exe, const result& tes);
    void exec();
    void print(resultTab& tab);
    int id;

    using tpoint::initMemLimit;
    using tpoint::lim;
    using tpoint::memLimit;
    using tpoint::release;

    static string cmd;
    static unsigned int grpId;
    static bool verbose;

    friend class group;

private:
    void getOut();
};
bool point::verbose = false;
unsigned int point::grpId = 0;
string point::cmd;
void point::init(const result& exe, const result& tes)
{
    this->rres = exe;
    this->rres.cmd = cmd;
    this->tres = tes;
    getOut();
    this->tpoint::init();
}
void point::exec()
{
    cout << endl;
    cout << col::Underline << col::BLUE << "[Info] Start program for test #" << grpId << "." << id << col::NONE;
    if (verbose)
    {
        cout << col::CYAN << endl;
        PrintRun(*this, cout, false);
    }
    cout << col::NONE << endl;
    this->run();
    this->parse();
    cout << s->verbose();
    if (success() && !tres.cmd.empty() && !ans.empty())
    {
        cout << col::BLUE << "[Info] Start testing for test #" << grpId << "." << id << col::NONE;
        if (verbose)
        {
            cout << col::CYAN << endl;
            PrintTest(*this, cout, false);
        }
        cout << col::NONE << endl;
        this->test();
        cout << ts->verbose();
    }
    cout << col::Underline << col::BLUE << "[Info] Test #" << grpId << "." << id << " finished." << col::NONE << endl;
}
void point::print(resultTab& tab)
{
    if (rres.ret || fail)
        tab.newColumn(s->color());
    else if (ts != nullptr)
        tab.newColumn(ts->color());
    else
        tab.newColumn(s->color());
    tab.writeColumn(cols::Id, id);
    tab.writeColumn(cols::RState, s->name());
    if (ts != nullptr)
        tab.writeColumn(cols::TState, ts->name());
    else
        tab.writeColumn(cols::TState, "skip");
    tab.writeColumn(cols::In, in);
    tab.writeColumn(cols::Out, out);
    tab.writeColumn(Ans, ans);
    tab.writeColumn(MsTim, tim / 1000);
    tab.writeColumn(UsTim, tim);
    tab.writeColumn(KbMem, mem / 1024.0);
    tab.writeColumn(MbMem, mem / 1024.0 / 1024.0);
    tab.writeColumn(Det, s->details());
}
void point::getOut()
{
    path p(rres.cmd);
    p.replace_extension("");
    p.replace_filename(p.filename().string() + "-" + path(in).stem().string());
    p.replace_extension(".out");
    this->out = p.string();
}

struct config
{
    shared_ptr<path> indir, ansdir;
    bool inrec = false, ansrec = false, verbose = false;
    shared_ptr<RegexSeq> rin, rans, rtest;
    shared_ptr<result> exe, tes;
    apdebug::testcase::limits lim;

protected:
    void read(int& pos, char* argv[]);

private:
    template <class T>
    inline static shared_ptr<T>& create(shared_ptr<T>& p)
    {
        if (p.use_count() > 1)
            p = make_shared<T>(*p);
        else if (!p)
            p = make_shared<T>();
        return p;
    }
};
void config::read(int& pos, char* argv[])
{
    for (; true; ++pos)
    {
        if (!strcmp(argv[pos], "-test-regex"))
            rtest = make_shared<RegexSeq>(++pos, argv);
        else if (!strcmp(argv[pos], "-indir"))
        {
            if (!strcmp(argv[pos + 1], "-R"))
            {
                ++pos;
                inrec = true;
            }
            indir = make_shared<path>(argv[++pos]);
        }
        else if (!strcmp(argv[pos], "-in-regex"))
            rin = make_shared<RegexSeq>(++pos, argv);
        else if (!strcmp(argv[pos], "-ansdir"))
        {
            if (!strcmp(argv[pos + 1], "-R"))
            {
                ++pos;
                ansrec = true;
            }
            ansdir = make_shared<path>(argv[++pos]);
        }
        else if (!strcmp(argv[pos], "-ans-regex"))
            rans = make_shared<RegexSeq>(++pos, argv);
        else if (ReadLimit(lim, pos, argv))
            continue;
        else if (!strcmp(argv[pos], "-args"))
            ReadArgument(*create(exe), ++pos, argv);
        else if (!strcmp(argv[pos], "-test"))
            create(tes)->cmd = argv[++pos];
        else if (!strcmp(argv[pos], "-testargs"))
            ReadArgument(*create(tes), ++pos, argv);
        else if (!strcmp(argv[pos], "-verbose"))
            verbose = true;
        else if (!strcmp(argv[pos], ";"))
            break;
    }
}

table confTable(std::array<const char*, 10> {
                    "Id", "Input directory", "Answer directory", "Argument",
                    "Test command", "Time limit", "Hard time limit",
                    "Memory limit", "Hard memory limit", "Verbose" },
    col::CYAN);
enum class confCol
{
    Id = 0,
    Input = 1,
    Answer = 2,
    Arg = 3,
    Test = 4,
    Time = 5,
    HardLim = 6,
    Memory = 7,
    HardMem = 8,
    Verbose = 9
};
using confTab = decltype(confTable);
class group : config
{
public:
    group(unsigned int i, int& pos, char* argv[]);
    group(const unsigned int c, unsigned int i, int& pos, char* argv[]);
    group(group&&) = default;
    void exec();
    void findFile();
    void printResult();
    void printConfig(confTab& t);

private:
    template <bool rec>
    void findInput();
    template <bool rec>
    void findAnswer();
    template <bool rec>
    pair<bool, size_t> isInclude(const path& p, const RegexSeq& r);

    static const array<const char*, 11> colName;

    resultTab tab;
    unsigned int id = 0;
    vector<point> tests;
    map<size_t, int> table;
};
const array<const char*, 11> group::colName { "Id", "State(Run)", "State(Test)", "Input",
    "Output", "Answer", "Time(ms)", "Time(us)", "Memory(MiB)", "Memory(KiB)",
    "Details" };
vector<group> grp;
group::group(unsigned int i, int& pos, char* argv[])
    : id(i)
    , tab(colName, col::NONE)
{
    read(pos, argv);
}
group::group(const unsigned int c, unsigned int i, int& pos, char* argv[])
    : config::config(grp[c])
    , id(i)
    , tab(colName, col::NONE)
{
    read(pos, argv);
}
void group::exec()
{
    const auto deref = [](shared_ptr<result>& s) { return s ? *s : result(); };
    point::lim = this->lim;
    point::grpId = id;
    point::verbose = verbose;
    point::initMemLimit();
    cout << col::BLUE << col::Underline << "[Info] Start testing for group #" << id << col::NONE;
    for (auto& i : tests)
    {
        i.init(deref(exe), deref(tes));
        i.exec();
        i.release();
    }
    cout << col::BLUE << col::Underline << "[Info] Group #" << id << " finished." << col::NONE << endl;
    cout << endl;
}
void group::printResult()
{
    cout << col::NONE << "Test result for group #" << id << endl;
    if (!tests.size())
    {
        cout << col::RED << "[Err] Can't find any test data." << col::NONE << endl;
        return;
    }
    for (auto& i : tests)
        i.print(tab);
    tab.printHeader(cout);
    tab.printAll(cout);
}
void group::printConfig(confTab& t)
{
    const static auto col = [](confCol a) { return static_cast<int>(a); };
    t.newColumn(col::CYAN);
    t.writeColumn(col(confCol::Id), id);
    t.writeColumn(col(confCol::Input), *indir, inrec ? "(recursive)" : "");
    if (ansdir)
        t.writeColumn(col(confCol::Answer), *ansdir, ansrec ? "(recursive)" : "");
    if (exe)
        t.writeColumn(col(confCol::Arg), exe ? exe->args : "");
    if (tes)
        t.writeColumn(col(confCol::Test), tes->cmd + " " + tes->args);
    t.writeColumn(col(confCol::Time), lim.lim / 1000.0, "ms (", lim.lim / 1e6, "s)");
    t.writeColumn(col(confCol::HardLim), lim.hardlim / 1e3, "ms (", lim.hardlim / 1e6, "s)");
    {
        const auto write = [&t](confCol c, const size_t m) {
            if (m)
                t.writeColumn(col(c), m / 1024.0, " KiB (", m / 1024.0 / 1024.0, " MiB)");
            else
                t.writeColumn(col(c), "unlimited");
        };
        write(confCol::Memory, lim.memLimByte);
        write(confCol::HardMem, lim.hardMemByte);
    }
    t.writeColumn(col(confCol::Verbose), boolalpha, verbose);
}
void group::findFile()
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
void group::findInput()
{
    unsigned int cur = 0;
    conditional_t<rec, recursive_directory_iterator, directory_iterator> init(*indir);
    for (auto& i : init)
    {
        if (!i.is_regular_file())
            continue;
        const auto [suc, hsh] = isInclude<rec>(i.path(), *rin);
        if (!suc)
            continue;
        tests.emplace_back(cur);
        table[hsh] = cur;
        tests[cur].in = i.path().string();
        ++cur;
    }
}
template <bool rec>
void group::findAnswer()
{
    if (!ansdir || !exists(*ansdir))
        return;
    conditional_t<rec, recursive_directory_iterator, directory_iterator> ansit(*ansdir);
    for (auto& i : ansit)
    {
        if (!i.is_regular_file())
            continue;
        const auto [suc, hsh] = isInclude<rec>(i.path(), *rans);
        if (!suc)
            continue;
        auto it = table.find(hsh);
        if (it == table.end())
            continue;
        tests[it->second].ans = i.path().string();
        table.erase(it);
    }
}
template <bool rec>
pair<bool, size_t> group::isInclude(const path& p, const RegexSeq& r)
{
    string s;
    if constexpr (rec)
        s = p.string();
    else
        s = p.filename().string();
    const auto [suc1, val1] = rtest->eval(s);
    if (!suc1)
        return make_pair(false, 0);
    const auto [suc2, val2] = r.eval(s);
    if (!suc2)
        return make_pair(false, 0);
    return make_pair(true, hash<string>()(val1 + val2));
}

int main(int argc, char* argv[])
{
    if (strcmp(argv[2], "-no-version"))
        PrintVersion("group test runner", cout);
    point::cmd = argv[1];
    {
        int p = 2;
        for (unsigned int i = 0; i < 2; ++i)
            if (readMemoryConf<point>(p, argv))
                ++p;
        for (int id = 0; p < argc; ++p)
        {
            if (strcmp(argv[p], "-use"))
                grp.emplace_back(id++, p, argv);
            else
            {
                const unsigned int u = atoi(argv[++p]);
                grp.emplace_back(u, id++, ++p, argv);
            }
        }
    }
    printMemConf<point>(cout, true);
    cout << col::CYAN << "[Info] Group config: " << endl;
    for (auto& i : grp)
        i.printConfig(confTable);
    confTable.printHeader(cout);
    confTable.printAll(cout);
    cout << endl;

    for (auto& i : grp)
    {
        i.findFile();
        i.exec();
    }
    for (auto& i : grp)
        i.printResult();
    return 0;
}
