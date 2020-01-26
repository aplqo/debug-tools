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
class point : public tpoint
{
public:
    point(int i)
        : id(i)
    {
    }
    void init();
    void exec();
    void print();
    int id;

    static result exe, tes;

private:
    void getOut();
};
result point::exe;
result point::tes;
void point::init()
{
    this->rres = exe;
    this->tres = tes;
    getOut();
    this->tpoint::init();
}
void point::exec()
{
    cout << endl;
    cout << col::BLUE << "[Info] Start program for test #" << id << col::CYAN << endl;
    PrintRun(*this, cout);
    cout << col::NONE << endl;
    this->run();
    this->parse();
    cout << s->verbose();
    if (success() && !tres.cmd.empty() && !ans.empty())
    {
        cout << col::BLUE << "[Info] Start testing for test #" << id << col::CYAN << endl;
        PrintTest(*this, cout);
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
    hash<string> hs;
    {

        directory_iterator init(indir);
        for (auto& i : init)
        {
            if (!i.is_regular_file())
                continue;
            string s = i.path().filename().string();
            smatch m, m2;
            if (!(regex_search(s, m, regex(testreg), regex_constants::match_any) && regex_search(s, m2, regex(inreg), regex_constants::match_any)))
                continue;
            s = regex_replace(s, regex(testreg), "");
            s = regex_replace(s, regex(inreg), "");
            tests.push_back(point(cur));
            table[hs(s)] = cur;
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
            smatch m, m2;
            if (!(regex_search(s, m, regex(testreg), regex_constants::match_any) && regex_search(s, m2, regex(ansreg), regex_constants::match_any)))
                continue;
            s = regex_replace(s, regex(testreg), "");
            s = regex_replace(s, regex(ansreg), "");
            auto it = table.find(hs(s));
            if (it == table.end())
                continue;
            tests[it->second].ans = i.path().string();
        }
    }
}

int main(int argc, char* argv[])
{
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
        if (!strcmp(argv[i], "-time"))
        {
            point::lim = atoi(argv[++i]) * timType(1000);
            point::hardlim = point::lim * 10;
        }
        if (!strcmp(argv[i], "-hlimit"))
            point::hardlim = atoi(argv[++i]) * timType(1000);
        if (!strcmp(argv[i], "-args"))
        {
            int ccmd = atoi(argv[++i]);
            ++i;
            cout << col::CYAN << "[Info] Arguments: ";
            for (int j = 1; j <= ccmd; ++j, ++i)
            {
                point::exe.args += string(" \"") + argv[i] + "\"";
                cout << argv[i] << " ";
            }
            cout << endl;
        }
        if (!strcmp(argv[i], "-testargs"))
        {
            int num = atoi(argv[++i]);
            ++i;
            cout << col::CYAN << "[Info] Test command: ";
            for (int j = 0; j < num; ++j, ++i)
                point::tes.args = point::tes.args + " \"" + argv[i] + "\"";
            cout << point::tes.cmd << " " << point::tes.args << endl;
        }
    }
    getfiles(indir, ansdir, inreg, ansreg);
    PrintLimit<point>(cout);
    cout << col::NONE << endl;
    for (auto& i : tests)
    {
        i.init();
        i.exec();
        i.release();
        tab.update(In, i.in.length() + 2);
        tab.update(Out, i.out.length() + 2);
        tab.update(Ans, i.ans.length() + 2);
        tab.update(Det, i.s->details().length() + 2);
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
    return 0;
}