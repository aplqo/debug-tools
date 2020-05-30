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
    Det = 8
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
    void print(table& tab);
    void update_table(table& t);
    int id;

    using tpoint::hardlim;
    using tpoint::lim;
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
        cout << col::BLUE << "[Info] Start testing for test #" << grpId << "." << id;
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
void point::print(table& tab)
{
    if (rres.ret || fail)
        cout << s->color();
    else if (ts != nullptr)
        cout << ts->color();
    else
        cout << s->color();
    cout << endl;
    tab.print(Id, id, cout);
    tab.setw(RState, cerr);
    tab.print(RState, s->name(), cout);
    if (ts != nullptr)
        tab.print(TState, ts->name(), cout);
    else
    {
        tab.setw(TState, cout);
        cout << "skip"
             << "  ";
    }
    tab.print(In, in, cout);
    tab.print(Out, out, cout);
    tab.print(Ans, ans, cout);
    tab.print(MsTim, tim / 1000, cout);
    tab.print(UsTim, tim, cout);
    tab.print(Det, s->details(), cout);
    cout << col::NONE;
}
void point::update_table(table& t)
{
    t.update(In, in.length() + 2);
    t.update(Out, out.length() + 2);
    t.update(Ans, ans.length() + 2);
    t.update(Det, s->details().length() + 2);
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
    timType hlimit = 1000 * 1000, lim = 100 * 10 * 1000;

protected:
    void read(int& pos, char* argv[]);
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
        else if (ReadLimit<point>(pos, argv))
        {
            hlimit = point::hardlim;
            lim = point::lim;
        }
        else if (!strcmp(argv[pos], "-args"))
        {
            exe = make_shared<result>(*exe);
            ReadArgument(*exe, ++pos, argv);
        }
        else if (!strcmp(argv[pos], "-test"))
        {
            if (tes.use_count() > 1)
                tes = make_shared<result>(*tes);
            else if (!tes)
                tes = make_shared<result>();
            tes->cmd = argv[++pos];
        }
        else if (!strcmp(argv[pos], "-testargs"))
        {
            if (tes.use_count() > 1)
                tes = make_shared<result>(*tes);
            else if (!tes)
                tes = make_shared<result>();
            ReadArgument(*tes, ++pos, argv);
        }
        else if (!strcmp(argv[pos], "-verbose"))
            verbose = true;
        else if (!strcmp(argv[pos], ";"))
            break;
    }
}
class group : config
{
public:
    group(unsigned int i, int& pos, char* argv[]);
    group(const unsigned int c, unsigned int i, int& pos, char* argv[]);
    void exec();
    void findFile();
    void print();

private:
    template <bool rec>
    void findInput();
    template <bool rec>
    void findAnswer();
    template <bool rec>
    pair<bool, size_t> isInclude(const path& p, const RegexSeq& r);

    table tab { "Id", "State(Run)", "State(Test)", "Input",
        "Output", "Answer", "Time(ms)", "Time(us)",
        "Details" };
    unsigned int id = 0;
    vector<point> tests;
    map<size_t, int> table;
};
vector<group> grp;
group::group(unsigned int i, int& pos, char* argv[])
    : id(i)
{
    read(pos, argv);
}
group::group(const unsigned int c, unsigned int i, int& pos, char* argv[])
    : config::config(grp[c])
    , id(i)
{
    read(pos, argv);
}
void group::exec()
{
    const auto deref = [](shared_ptr<result>& s) { return s ? *s : result(); };
    point::lim = lim;
    point::hardlim = hlimit;
    point::grpId = id;
    cout << col::BLUE << col::Underline << "[Info] Start testing for group #" << id << col::NONE;
    for (auto& i : tests)
    {
        i.init(deref(exe), deref(tes));
        i.exec();
        i.release();
        i.update_table(tab);
    }
    cout << col::BLUE << col::Underline << "[Info] Group #" << id << " finished." << col::NONE << endl;
    cout << endl;
}
void group::print()
{
    cout << col::NONE << "Test result for group #" << id << endl;
    if (!tests.size())
    {
        cout << col::RED << "[Err] Can't find any test data." << col::NONE << endl;
        return;
    }
    //print table
    {
        tab.update(Id, log10(tests.size()) + 2);
        tab.update(MsTim, log10(hlimit / 1000) + 2);
        tab.update(UsTim, log10(hlimit) + 2);
        tab.header(cout);
    }
    for (auto& i : tests)
        i.print(tab);
    cout << col::NONE << endl;
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
        tests.push_back(point(cur));
        table[hsh] = cur;
        tests[cur].in = i.path().string();
        ++cur;
    }
}
template <bool rec>
void group::findAnswer()
{
    if (!exists(*ansdir))
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
    for (int i = 2, id = 0; i < argc; ++i)
    {
        if (strcmp(argv[i], "-use"))
            grp.emplace_back(id++, i, argv);
        else
        {
            const unsigned int u = atoi(argv[++i]);
            grp.emplace_back(u, id++, ++i, argv);
        }
    }
    for (auto& i : grp)
    {
        i.findFile();
        i.exec();
    }
    for (auto& i : grp)
        i.print();
    cout << col::NONE << endl;
    return 0;
}
