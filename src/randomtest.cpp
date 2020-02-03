#include "include/cmdarg.h"
#include "include/define.h"
#include "include/exception.h"
#include "include/output.h"
#include "include/testcase.h"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
using apdebug::testcase::result;
using apdebug::testcase::tpoint;
using apdebug::timer::timType;
using namespace std;
using namespace std::filesystem;
using namespace apdebug::args;
using namespace apdebug::out;

table tab {
    "Id", "State(Run)", "State(Test)",
    "Input", "Output", "Answer", "Diff",
    "Time(ms)", "Time(us)", "Details"
};
enum cols
{
    Id = 0,
    RState = 1,
    TState = 2,
    In = 3,
    Out = 4,
    Ans = 5,
    Dif = 6,
    MsTim = 7,
    UsTim = 8,
    Det = 9
};
class tests : tpoint
{
public:
    tests(int id)
        : id(id)
    {
    }
    void init();
    void generate();
    bool exec();
    void print();
    void release();

    using tpoint::hardlim;
    using tpoint::lim;
    int id;
    result generator;

    static path tmpdir;
    static result gen, exe, tes;

private:
    string dif;
    inline void getArgs(result& r);
    void getFiles();
};
result tests::gen, tests::exe, tests::tes;
path tests::tmpdir;
void tests::init()
{
    this->generator = gen;
    this->rres = exe;
    this->tres = tes;
    getFiles();
    getArgs(this->generator);
    getArgs(this->rres);
    getArgs(this->tres);
    this->tpoint::getArgs(this->generator);
    this->tpoint::init();
}
void tests::generate()
{
    this->generator.exec();
}
bool tests::exec()
{
    this->run();
    this->parse();
    if (success() && !tres.cmd.empty())
        this->test();
    return success() && !this->fail;
}
void tests::print()
{
    if (rres.ret || fail)
        cout << s->color();
    else if (ts != nullptr)
        cout << ts->color();
    cout << endl;
    tab.print(Id, id, cout);
    tab.print(RState, s->name(), cout);
    if (ts != nullptr)
        tab.print(TState, ts->name(), cout);
    else
    {
        tab.setw(TState, cout);
        cout << "skipped"
             << "  ";
    }
    tab.print(In, in, cout);
    tab.print(Out, out, cout);
    tab.print(Ans, ans, cout);
    tab.print(Dif, dif, cout);
    tab.print(MsTim, tim / 1000, cout);
    tab.print(UsTim, tim, cout);
    tab.print(Det, s->details(), cout);
}
void tests::release()
{
    static const auto rm = [](string& p) {
        if (exists(p))
            remove(p);
        p = "(Released)";
    };
    rm(in);
    rm(out);
    rm(ans);
    rm(dif);
}
void tests::getArgs(result& r)
{
    r.args = regex_replace(r.args, regex(R"(\[differ\])"), dif);
}
void tests::getFiles()
{
    string i = to_string(id);
    path p = tmpdir / (path(rres.cmd).stem().concat("-" + to_string(id)));
    this->in = p.replace_extension(".in").string();
    this->out = p.replace_extension(".out").string();
    this->ans = p.replace_extension(".ans").string();
    this->dif = p.replace_extension(".diff").string();
}

unsigned long times;
bool stop = false, create = false, fail = false;

int main(int argc, char* argv[])
{
    PrintVersion("random test runner", cout);

    tests::exe.cmd = argv[1];
    for (int i = 2; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-tmpdir"))
        {
            tests::tmpdir = argv[++i];
            cout << col::CYAN << "[Info] Temporary directory: " << tests::tmpdir;
            if (!exists(tests::tmpdir))
            {
                create_directory(tests::tmpdir);
                create = true;
                cout << " ( created )";
            }
            cout << endl;
            continue;
        }
        if (!strcmp(argv[i], "-test"))
            tests::tes.cmd = argv[++i];
        if (ReadLimit<tests>(i, argv))
            continue;
        if (!strcmp(argv[i], "-args"))
        {
            cout << col::CYAN << "[Info] Arguments: ";
            ReadArgument(tests::exe, ++i, argv);
            cout << tests::exe.args << endl;
        }
        if (!strcmp(argv[i], "-testargs"))
        {
            cout << col::CYAN << "[Info] Test command: ";
            ReadArgument(tests::tes, ++i, argv);
            cout << tests::tes.cmd << " " << tests::tes.args << endl;
        }
        if (!strcmp(argv[i], "-generator"))
            tests::gen.cmd = argv[++i];
        if (!strcmp(argv[i], "-genargs"))
        {
            cout << col::CYAN << "[Info] Generator command: ";
            ReadArgument(tests::gen, ++i, argv);
            cout << tests::gen.cmd << " " << tests::gen.args << endl;
        }
        if (!strcmp(argv[i], "-times"))
            times = stoul(argv[++i]);
        if (!strcmp(argv[i], "-stop-on-error"))
            stop = true;
    }
    cout << col::CYAN << "[Info] Test time: " << times << endl;
    cout << col::CYAN << "[Info] Stop on error: " << boolalpha << stop << endl;
    PrintLimit<tests>(cout, false);
    cout << col::NONE << endl;
    cout << "Test Results:" << endl;
    //Set table width
    {
        unsigned int len = tests::exe.cmd.length() + 1 + log10(times) + 2 + tests::tmpdir.string().length() + 1;
        unsigned int rlen = strlen("(Released)");
        tab.update(Id, log10(times) + 2);
        tab.update(In, max(len, rlen));
        tab.update(Out, max(len + 1, rlen));
        tab.update(Ans, max(len + 1, rlen));
        tab.update(Dif, max(len + 2, rlen));
        tab.update(MsTim, log10(tpoint::hardlim / 1000) + 2);
        tab.update(UsTim, log10(tpoint::hardlim) + 2);
    }
    tab.header(cout);
    for (unsigned long i = 0; i < times; ++i)
    {
        tests tp(i);
        bool s;
        tp.init();
        tp.generate();
        s = tp.exec();
        if ((!s))
        {
            if (stop)
            {
                tp.print();
                cout << col::NONE << endl;
                return 0;
            }
            fail = true;
        }
        if (s)
            tp.release();
        tp.print();
    }
    if (create && !fail)
    {
        cout << col::CYAN << endl
             << "[Info] Test passed removed temporary directory." << endl;
        remove_all(tests::tmpdir);
    }
    cout << col::NONE;
    cout.flush();
    return 0;
}