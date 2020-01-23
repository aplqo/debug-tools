#include "include/define.h"
#include "include/output.h"
#include "include/testcase.h"
#include <algorithm>
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

    static string::size_type inlen, outlen, anslen;
    static result exe, tes;

private:
    void getArgs(result& r);
    void getOut();
};
string::size_type point::inlen = 5;
string::size_type point::outlen = 6;
string::size_type point::anslen = 6;
result point::exe;
result point::tes;
void point::init()
{
    this->rres = exe;
    this->tres = tes;
    getOut();
    getArgs(rres);
    getArgs(tres);
}
void point::exec()
{
    cout << endl;
    cout << col::BLUE << "[Info] Start program for test #" << id << col::CYAN << endl;
    cout << "[Info] Input file: " << in << endl;
    cout << "[Info] Output file: " << out << endl;
    cout << "[Info] Run args: " << rres.args << col::NONE << endl;
    this->run();
    this->parse();
    s->verbose();
    if (success() && !tres.cmd.empty() && !ans.empty())
    {
        cout << col::BLUE << "[Info] Start testing for test #" << id << col::CYAN << endl;
        cout << "[Info] Answer file: " << ans << endl;
        cout << "[Info] Test command: " << tres.cmd << " " << tres.args << col::NONE << endl;
        this->test();
        ts->verbose();
    }
    cout << col::BLUE << "[Info] Test #" << id << " finished." << endl;
}
void point::print()
{
    if (rres.ret)
        s->color();
    else if (ts != nullptr)
        ts->color();
    else
        s->color();
    cout << endl;
    cout << setw(5) << id << "  ";
    cerr.width(11);
    s->name();
    cout << "  ";
    cerr.width(12);
    if (ts != nullptr)
        ts->name();
    else
        cerr << "skip";
    cout << "  ";
    cout << setw(inlen + 1) << in << "  " << setw(outlen + 1) << out << "  " << setw(anslen + 1) << ans << "  ";
    cout << setw(9) << tim / 1000 << "  " << setw(12) << tim << "  ";
    s->details();
}
void point::getArgs(result& r)
{
    r.args = regex_replace(r.args, regex(R"(\[input\])"), "\"" + in + "\"");
    r.args = regex_replace(r.args, regex(R"(\[output\])"), "\"" + out + "\"");
    r.args = regex_replace(r.args, regex(R"(\[answer\])"), "\"" + ans + "\"");
}
void point::getOut()
{
    path p(rres.cmd);
    p.replace_extension("");
    p.replace_filename(p.filename().string() + "-" + path(in).stem().string());
    p.replace_extension(".out");
    this->out = p.string();
    outlen = max(outlen, this->out.size());
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
            string s = i.path().string();
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
            string s = i.path().string();
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
    cout << endl;
    cout << "Aplqo debug tool: group test runner" << endl;
    cout << "Version git@" << apdebug::info::hash << " " << apdebug::info::version << endl;
    cout << "Build branch: " << apdebug::info::branch << endl;
    cout << "Build on " << __TIME__ << " " << __DATE__ << " by " << apdebug::info::builder << endl;
    cout << endl;

    path indir, ansdir;
    regex inreg, ansreg;

    point::exe.cmd = argv[1];
    cout << col::CYAN;
    for (int i = 2; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-test-regex"))
        {
            cout << "[Info] Test file regex: \'" << argv[++i] << "\'" << endl;
            testreg = regex(argv[i]);
            continue;
        }
        if (!strcmp(argv[i], "-indir"))
        {
            cout << "[Info] Input directory: " << argv[++i] << endl;
            indir = argv[i];
            continue;
        }
        if (!strcmp(argv[i], "-in-regex"))
        {
            cout << "[Info] Input regex: \'" << argv[++i] << "\'" << endl;
            inreg = regex(argv[i]);
            continue;
        }
        if (!strcmp(argv[i], "-ansdir"))
        {
            cout << "[Info] Answer directory: " << argv[++i] << endl;
            ansdir = argv[i];
            continue;
        }
        if (!strcmp(argv[i], "-ans-regex"))
        {
            cout << "[Info] Answer regex: \'" << argv[++i] << "\'" << endl;
            ansreg = regex(argv[i]);
            continue;
        }
        if (!strcmp(argv[i], "-test"))
            point::tes.cmd = argv[++i];
        if (!strcmp(argv[i], "-time"))
        {
            point::lim = atoi(argv[++i]) * timType(1000) * 1000;
            point::hardlim = point::lim * 10;
        }
        if (!strcmp(argv[i], "-hlimit"))
            point::hardlim = atoi(argv[++i]) * timType(1000) * 1000;
        if (!strcmp(argv[i], "-args"))
        {
            int ccmd = atoi(argv[++i]);
            ++i;
            cout << "[Info] Arguments: ";
            for (int j = 1; j <= ccmd; ++j, ++i)
            {
                point::exe.args += argv[i];
                cout << argv[i] << " ";
            }
            cout << endl;
        }
        if (!strcmp(argv[i], "-testargs"))
        {
            int num = atoi(argv[++i]);
            ++i;
            cout << "[Info] Test command: ";
            for (int j = 0; j < num; ++j, ++i)
                point::tes.args = point::tes.args + " " + argv[i];
            cout << point::tes.cmd << " " << point::tes.args << endl;
        }
    }
    getfiles(indir, ansdir, inreg, ansreg);
    printT(tpoint::lim, "Time limit", cout);
    cout << endl;
    printT(tpoint::hardlim, "Hard time limit", cout);
    cout << col::NONE << endl;
    for (auto& i : tests)
    {
        i.init();
        i.exec();
        point::inlen = max(point::inlen, i.in.size());
        point::anslen = max(point::anslen, i.ans.size());
        point::outlen = max(point::outlen, i.out.size());
        i.release();
    }
    //print table
    {
        cout << col::NONE << endl
             << "Test results:" << endl;
        cout << setw(5) << "Id"
             << "  " << setw(11) << "State(Run)"
             << "  " << setw(12) << "State(Test)"
             << "  ";
        cout << setw(point::inlen + 1) << "Input"
             << "  " << setw(point::outlen + 1) << "Output"
             << "  " << setw(point::anslen + 1) << "Answer"
             << "  ";
        cout << setw(9) << "Time(ms)"
             << "  " << setw(12) << "Time(us)"
             << "  " << setw(30) << "Details" << endl;
        const static auto fil = [](int num) -> void {
            for (int i = 0; i < num; ++i)
                cout << "-";
            cout << "  ";
        };
        fil(5);
        fil(11);
        fil(12);
        fil(point::inlen + 1);
        fil(point::outlen + 1);
        fil(point::anslen + 1);
        fil(9);
        fil(12);
        fil(30);
    }
    for (auto& i : tests)
        i.print();
    return 0;
}