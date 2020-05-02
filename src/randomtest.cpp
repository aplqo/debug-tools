#include "include/cmdarg.h"
#include "include/define.h"
#include "include/exception.h"
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
#include <regex>
#include <string>
#include <thread>
#include <vector>
using apdebug::testcase::result;
using apdebug::testcase::tpoint;
using apdebug::timer::timType;
using namespace std;
using namespace std::filesystem;
using namespace apdebug::args;
using namespace apdebug::out;
using namespace apdebug::utility;

const chrono::milliseconds print_duration(100);
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
    void update_table(table& t);

    int id;
    using tpoint::hardlim;
    using tpoint::lim;

    static path tmpdir;
    static result gen, std, exe, tes;

private:
    string dif;
    result generator, standard;
    inline void getArgs(result& r);
    void getFiles();
};
result tests::gen, tests::exe, tests::tes, tests::std;
path tests::tmpdir;
void tests::init()
{
    this->generator = gen;
    this->rres = exe;
    this->tres = tes;
    this->standard = std;
    getFiles();
    getArgs(this->generator);
    getArgs(this->standard);
    getArgs(this->rres);
    getArgs(this->tres);
    this->tpoint::getArgs(this->generator);
    this->tpoint::getArgs(this->standard);
    this->tpoint::init();
}
void tests::generate()
{
    this->generator.exec();
    if (!standard.cmd.empty())
        this->standard.exec();
}
bool tests::exec()
{
    this->run();
    this->parse();
    if (success() && !tres.cmd.empty())
        this->test();
    this->tpoint::release();
    return success() && !this->fail;
}
void tests::print()
{
    if (rres.ret || fail)
        cout << s->color();
    else if (ts != nullptr)
        cout << ts->color();
    else
        cout << s->color();
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
void tests::update_table(table& t)
{
    t.update(In, in.length() + 2);
    t.update(Out, out.length() + 2);
    t.update(Ans, ans.length() + 2);
    t.update(Dif, dif.length() + 2);
    t.update(Det, s->details().length() + 2);
}
void tests::getArgs(result& r)
{
    r.args = regex_replace(r.args, regex(R"(\[differ\])"), dif);
}
void tests::getFiles()
{
    string i = to_string(id);
    path p = tmpdir / (path(rres.cmd).stem().concat("-" + GetThreadId() + "-" + to_string(id)));
    this->in = p.replace_extension(".in").string();
    this->out = p.replace_extension(".out").string();
    this->ans = p.replace_extension(".ans").string();
    this->dif = p.replace_extension(".diff").string();
}

mutex mqueue; // mutex for queue, tab
unsigned long id = 0; // id for print
atomic_ulong cur = 0, wait = 0; // test count
atomic_bool fail = false;
unsigned long times;
unsigned int parallel = thread::hardware_concurrency();
bool stop = false, create = false, showall = true;
queue<tests*> pqueue; // print queue
vector<tests*> fqueue; //failed tests

inline bool isRun()
{
    return cur.load() < times && !(stop && fail.load());
}
void PrintThrdFail()
{
    unsigned long lst = 0;
    while (isRun())
    {
        if (cur.load() != lst)
        {
            cout << "\r" << col::BLUE << "[Info] Testing " << cur.load() << " data...";
            cout.flush();
        }
        lst = cur.load();
        std::this_thread::sleep_for(print_duration);
    }
}
void PrintThrdAll()
{
    while (wait.load() || isRun())
    {
        if (wait.load())
        {
            while (wait.load())
            {
                unique_lock lk(mqueue);
                tests* i = pqueue.front();
                pqueue.pop();
                lk.unlock();

                i->id = ++id;
                i->print();
                delete i;
                --wait;
            }
        }
        std::this_thread::sleep_for(print_duration);
    }
}
inline void PrintFail(tests* s)
{
    fail = true;
    if (showall)
    {
        ++wait;
        lock_guard lk(mqueue);
        pqueue.push(s);
    }
    else
    {
        lock_guard lk(mqueue);
        fqueue.push_back(s);
    }
}
inline void PrintPass(tests* s)
{
    if (showall)
    {
        ++wait;
        lock_guard lk(mqueue);
        pqueue.push(s);
    }
    else
        delete s;
}
void thrd()
{
    for (unsigned long i = 0; isRun() && (!(stop && fail.load())); ++i, ++cur)
    {
        tests* tp = new tests(i);
        bool s;
        tp->init();
        tp->generate();
        s = tp->exec();
        if (!s)
        {
            PrintFail(tp);
            if (stop)
                return;
        }
        else
        {
            tp->release();
            PrintPass(tp);
        }
    }
}
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
        if (!strcmp(argv[i], "-parallelism"))
            parallel = stoi(argv[++i]);
        if (!strcmp(argv[i], "-fail-only"))
            showall = false;
        if (!strcmp(argv[i], "-standard"))
            tests::std.cmd = argv[++i];
        if (!strcmp(argv[i], "-std-args"))
        {
            cout << col::CYAN << "[Info] Answer command: ";
            ReadArgument(tests::std, ++i, argv);
            cout << tests::std.cmd << " " << tests::std.args << endl;
        }
    }
    cout << col::CYAN << "[Info] Test time: " << times << endl;
    cout << col::CYAN << "[Info] Parallelism: " << parallel << endl;
    cout << col::CYAN << "[Info] Stop on error: " << boolalpha << stop << endl;
    cout << col::CYAN << "[Info] Show all test: " << boolalpha << showall << endl;
    PrintLimit<tests>(cout, false);
    cout << col::NONE << endl;
    //Set table width
    {
        unsigned int len = tests::exe.cmd.length() + 1 + log10(times) + 1 + tests::tmpdir.string().length() + 2;
        unsigned int rlen = strlen("(Released)");
        tab.update(Id, log10(times) + 2);
        tab.update(MsTim, log10(tpoint::hardlim / 1000) + 2);
        tab.update(UsTim, log10(tpoint::hardlim) + 2);
        if (showall)
        {
            tab.update(In, max(len + 3 + 12 + 1, rlen));
            tab.update(Out, max(len + 4 + 12 + 1, rlen));
            tab.update(Ans, max(len + 4 + 12 + 1, rlen));
            tab.update(Dif, max(len + 5 + 12 + 1, rlen));
            cout << "Test Results:" << endl;
            tab.header(cout);
        }
    }

    {
        vector<thread> th;
        for (unsigned int i = 1; i < parallel; ++i)
            th.push_back(thread(thrd));
        if (showall)
            th.push_back(thread(PrintThrdAll));
        else
            th.push_back(thread(PrintThrdFail));
        thrd();
        for (auto& i : th)
            i.join();
    }
    if (!showall)
    {
        cout << col::NONE << endl;
        cout << "Test results:" << endl;
        cout.flush();
        for (auto i : fqueue)
        {
            i->update_table(tab);
            i->id = id++;
        }
        tab.header(cout);
        for (auto i : fqueue)
        {
            i->print();
            delete i;
        }
    }
    cout << col::NONE << endl;

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
