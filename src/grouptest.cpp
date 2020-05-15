#include "include/cmdarg.h"
#include "include/define.h"
#include "include/output.h"
#include "include/testcase.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>
using apdebug::testcase::result;
using apdebug::testcase::tpoint;
using apdebug::timer::timType;
using namespace std;
using namespace std::filesystem;
using namespace apdebug::out;
using namespace apdebug::args;

table tab {
    "Id", "State(Run)", "State(Test)", "Input",
    "Output", "Answer", "Time(ms)", "Time(us)",
    "Details"
};
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
    void init();
    void exec();
    void print();
    void update_table(table& t);
    int id;

    using tpoint::hardlim;
    using tpoint::lim;
    using tpoint::release;

    static result exe, tes;
    static bool verbose;

    friend void getfiles(path indir, path ansdir, regex inreg, regex ansreg);

private:
    bool testen = true;
    void getOut();
};
result point::exe;
result point::tes;
bool point::verbose = false;
void point::init()
{
    this->rres = exe;
    this->tres = tes;
    getOut();
    if (regex_search(tres.args, regex(R"(\[answer\])")) && ans.empty())
        testen = false;
    this->tpoint::init();
}
void point::exec()
{
    cout << endl;
    cout << col::BLUE << "[Info] Start program for test #" << id;
    if (verbose)
    {
        cout << col::CYAN << endl;
        PrintRun(*this, cout, false);
    }
    cout << col::NONE << endl;
    this->run();
    this->parse();
    cout << s->verbose();
    if (success() && !tres.cmd.empty() && testen)
    {
        cout << col::BLUE << "[Info] Start testing for test #" << id;
        if (verbose)
        {
            cout << col::CYAN << endl;
            PrintTest(*this, cout, false);
        }
        cout << col::NONE << endl;
        this->test();
        cout << ts->verbose();
    }
    cout << col::BLUE << "[Info] Test #" << id << " finished." << endl;
}
void point::print()
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

vector<point> tests;
regex testreg;

void getfiles(path indir, path ansdir, regex inreg, regex ansreg)
{
    static map<size_t, int> table;
    static int cur = 0;
    auto getHash = [](string s, regex reg) -> size_t {
        s = regex_replace(s, regex(testreg), "");
        s = regex_replace(s, reg, "");
        return hash<string>()(s);
    };
    auto isInclude = [](string s, regex reg) -> bool {
        smatch m1, m2;
        return regex_search(s, m1, testreg) && regex_search(s, m2, reg);
    };
    {

        directory_iterator init(indir);
        for (auto& i : init)
        {
            if (!i.is_regular_file())
                continue;
            string s = i.path().filename().string();
            if (!isInclude(s, inreg))
                continue;
            tests.push_back(point(cur));
            table[getHash(s, inreg)] = cur;
            tests[cur].in = i.path().string();
            ++cur;
        }
    }
    {
        if (!exists(ansdir))
            return;
        directory_iterator ansit(ansdir);
        for (auto& i : ansit)
        {
            if (!i.is_regular_file())
                continue;
            string s = i.path().filename().string();
            if (!isInclude(s, ansreg))
                continue;
            auto it = table.find(getHash(s, ansreg));
            if (it == table.end())
                continue;
            tests[it->second].ans = i.path().string();
        }
    }
}

int main(int argc, char* argv[])
{
    if (strcmp(argv[2], "-no-version"))
        PrintVersion("group test runner", cout);

    path indir, ansdir;
    regex inreg, ansreg;

    point::exe.cmd = argv[1];
    for (int i = 2; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-test-regex"))
        {
            cout << col::CYAN << "[Info] Test file regex: \'" << argv[++i] << "\'" << endl;
            testreg = regex(argv[i]);
            continue;
        }
        if (!strcmp(argv[i], "-indir"))
        {
            cout << col::CYAN << "[Info] Input directory: " << argv[++i] << endl;
            indir = argv[i];
            continue;
        }
        if (!strcmp(argv[i], "-in-regex"))
        {
            cout << col::CYAN << "[Info] Input regex: \'" << argv[++i] << "\'" << endl;
            inreg = regex(argv[i]);
            continue;
        }
        if (!strcmp(argv[i], "-ansdir"))
        {
            cout << col::CYAN << "[Info] Answer directory: " << argv[++i] << endl;
            ansdir = argv[i];
            continue;
        }
        if (!strcmp(argv[i], "-ans-regex"))
        {
            cout << col::CYAN << "[Info] Answer regex: \'" << argv[++i] << "\'" << endl;
            ansreg = regex(argv[i]);
            continue;
        }
        if (!strcmp(argv[i], "-test"))
            point::tes.cmd = argv[++i];
        if (ReadLimit<point>(i, argv))
            continue;
        if (!strcmp(argv[i], "-args"))
        {
            cout << col::CYAN << "[Info] Arguments: ";
            ReadArgument(point::exe, ++i, argv);
            cout << endl;
        }
        if (!strcmp(argv[i], "-testargs"))
        {
            cout << col::CYAN << "[Info] Test command: ";
            ReadArgument(point::tes, ++i, argv);
            cout << point::tes.cmd << " " << point::tes.args << endl;
        }
        if (!strcmp(argv[i], "-verbose"))
            point::verbose = true;
    }
    cout << col::CYAN << "[Info] Verbose output: " << boolalpha << point::verbose << endl;
    PrintLimit<point>(cout, false);
    cout << col::NONE << endl;
    getfiles(indir, ansdir, inreg, ansreg);
    if (!tests.size())
    {
        cout << col::RED << "[Err] Can't find any test data." << col::NONE << endl;
        return 1;
    }
    for (auto& i : tests)
    {
        i.init();
        i.exec();
        i.release();
        i.update_table(tab);
    }
    //print table
    {
        tab.update(Id, log10(tests.size()) + 2);
        tab.update(MsTim, log10(point::hardlim / 1000) + 2);
        tab.update(UsTim, log10(point::hardlim) + 2);
        cout << col::NONE << endl
             << "Test results:" << endl;
        tab.header(cout);
    }
    for (auto& i : tests)
        i.print();
    cout << col::NONE << endl;
    return 0;
}
