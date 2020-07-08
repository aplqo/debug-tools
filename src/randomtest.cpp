#include "include/cmdarg.h"
#include "include/define.h"
#include "include/exception.h"
#include "include/memory.h"
#include "include/output.h"
#include "include/testcase.h"
#include "include/utility.h"
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
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
const unsigned int maxStdRetries = 20, maxVaRetries = 20;
table tab(std::array<const char*, 12> {
              "Id", "State(Run)", "State(Test)",
              "Input", "Output", "Answer", "Diff",
              "Time(ms)", "Time(us)", "Memory(MiB)", "Memory(KiB)", "Details" },
    col::NONE);
using resultTab = decltype(tab);
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
    MbMem = 9,
    KbMem = 10,
    Det = 11
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

    int id;
    using tpoint::initMemLimit;
    using tpoint::lim;
    using tpoint::memLimit;

    static path tmpdir;
    static result gen, std, exe, tes, va;

private:
    bool genFail = false;
    string dif;
    result generator, standard, valid;
    inline void getArgs(result& r);
    void getFiles();

    static const regex rdiff;
};
const regex tests::rdiff(R"(<differ>)", regex_constants::ECMAScript | regex_constants::optimize | regex_constants::nosubs);
result tests::gen, tests::exe, tests::tes, tests::std, tests::va;
path tests::tmpdir;
void tests::init()
{
    this->generator = gen;
    this->rres = exe;
    this->tres = tes;
    this->standard = std;
    this->valid = va;
    getFiles();
    getArgs(this->generator);
    getArgs(this->standard);
    getArgs(this->rres);
    getArgs(this->tres);
    getArgs(this->valid);
    this->tpoint::getArgs(this->generator);
    this->tpoint::getArgs(this->standard);
    this->tpoint::getArgs(this->valid);
    this->tpoint::init();
}
void tests::generate()
{
    if (standard.cmd.empty() && valid.cmd.empty())
    {
        this->generator.exec();
        return;
    }
    unsigned int fv = maxVaRetries, fs = maxStdRetries;
    static auto tryRun = [](result& r, unsigned int& i) -> bool {
        if (r.cmd.empty())
            return true;
        r.exec();
        if (r.ret)
            --i;
        return !r.ret;
    };
    do
    {
        this->generator.exec();
        if (!tryRun(this->valid, fv))
            continue;
        if (!tryRun(this->standard, fs))
            continue;
        break;
    } while (fv && fs);
    if (standard.ret || valid.ret)
    {
        genFail = true;
        out = "(Null)";
        dif = "(Null)";
        s = new apdebug::exception::JudgeFail("", "Generate data failed.");
    }
}
bool tests::exec()
{
    if (genFail)
        return false;
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
        tab.newColumn(s->color());
    else if (ts != nullptr)
        tab.newColumn(ts->color());
    else
        tab.newColumn(s->color());
    tab.writeColumn(Id, id);
    tab.writeColumn(RState, s->name());
    if (ts != nullptr)
        tab.writeColumn(TState, ts->name());
    else
        tab.writeColumn(TState, "skip");
    tab.writeColumn(In, in);
    tab.writeColumn(Out, out);
    tab.writeColumn(Ans, ans);
    tab.writeColumn(Dif, dif);
    tab.writeColumn(MbMem, mem / 1024.0 / 1024.0);
    tab.writeColumn(KbMem, mem / 1024.0);
    tab.writeColumn(MsTim, tim / 1000);
    tab.writeColumn(UsTim, tim);
    tab.writeColumn(Det, s->details());
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
    r.args = regex_replace(r.args, rdiff, dif);
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

// for table setup
atomic_uint thrdCnt = 0;
mutex mTab;
condition_variable thrdCnd;
unsigned int tmpV;

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
    if (showall)
    {
        unique_lock lk(mTab);
        thrdCnd.wait(lk, []() { return thrdCnt == parallel; });
        cout << "Test Results:" << endl;
        tab.printHeader(cout);
    }
    while (wait.load() || isRun() || thrdCnt)
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
            tab.printAll(cout);
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
    if (showall)
    {
        const unsigned int l = tmpV + GetThreadId().length() + 1;
        unique_lock lk(mTab);
        tab.update(In, l + 3);
        tab.update(Out, l + 4);
        tab.update(Ans, l + 4);
        tab.update(Dif, l + 5);
        lk.unlock();
        ++thrdCnt;
        thrdCnd.notify_one();
    }
    for (unsigned long i = 0; isRun(); ++i, ++cur)
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
            {
                --thrdCnt;
                return;
            }
        }
        else
        {
            tp->release();
            PrintPass(tp);
        }
    }
    --thrdCnt;
}
int main(int argc, char* argv[])
{
    if (strcmp(argv[2], "-no-version"))
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
        else if (!strcmp(argv[i], "-test"))
            tests::tes.cmd = argv[++i];
        else if (ReadLimit(tests::lim, i, argv))
            continue;
        else if (readMemoryConf<tests>(i, argv))
            continue;
        else if (!strcmp(argv[i], "-args"))
        {
            cout << col::CYAN << "[Info] Arguments: ";
            ReadArgument(tests::exe, ++i, argv);
            cout << tests::exe.args << endl;
        }
        else if (!strcmp(argv[i], "-testargs"))
        {
            cout << col::CYAN << "[Info] Test command: ";
            ReadArgument(tests::tes, ++i, argv);
            cout << tests::tes.cmd << " " << tests::tes.args << endl;
        }
        else if (!strcmp(argv[i], "-generator"))
            tests::gen.cmd = argv[++i];
        else if (!strcmp(argv[i], "-genargs"))
        {
            cout << col::CYAN << "[Info] Generator command: ";
            ReadArgument(tests::gen, ++i, argv);
            cout << tests::gen.cmd << " " << tests::gen.args << endl;
        }
        else if (!strcmp(argv[i], "-validator"))
            tests::va.cmd = argv[++i];
        else if (!strcmp(argv[i], "-valargs"))
        {
            cout << col::CYAN << "[Info] Validator command: ";
            ReadArgument(tests::va, ++i, argv);
            cout << tests::va.cmd << " " << tests::va.args << endl;
        }
        else if (!strcmp(argv[i], "-times"))
            times = stoul(argv[++i]);
        else if (!strcmp(argv[i], "-stop-on-error"))
            stop = true;
        else if (!strcmp(argv[i], "-parallelism"))
            parallel = stoi(argv[++i]);
        else if (!strcmp(argv[i], "-fail-only"))
            showall = false;
        else if (!strcmp(argv[i], "-standard"))
            tests::std.cmd = argv[++i];
        else if (!strcmp(argv[i], "-stdargs"))
        {
            cout << col::CYAN << "[Info] Answer command: ";
            ReadArgument(tests::std, ++i, argv);
            cout << tests::std.cmd << " " << tests::std.args << endl;
        }
        else if (!strcmp(argv[i], "-swapaccount"))
            apdebug::memory::swapaccount = true;
    }
    cout << col::CYAN << "[Info] Test time: " << times << endl;
    cout << col::CYAN << "[Info] Parallelism: " << parallel << endl;
    cout << col::CYAN << "[Info] Stop on error: " << boolalpha << stop << endl;
    cout << col::CYAN << "[Info] Show all test: " << boolalpha << showall << endl;
    printMemConf<tests>(cout, true);
    PrintLimit(tests::lim, cout, false);
    cout << col::NONE << endl;
    //Set table width
    {
        unsigned int len = path(tests::exe.cmd).stem().string().length() + 1 + log10(times) + 1 + tests::tmpdir.string().length();
        tab.update(Id, log10(times) + 1);
        tab.update(MsTim, log10(tpoint::lim.hardlim / 1000));
        tab.update(UsTim, log10(tpoint::lim.hardlim));
        if (showall)
        {
            const unsigned int rlen = strlen("(Released)");
            tmpV = len;
            tab.update(In, rlen);
            tab.update(Out, rlen);
            tab.update(Ans, rlen);
            tab.update(Dif, rlen);
        }
    }

    tests::initMemLimit();
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
        for (auto i : fqueue)
            i->id = id++;
        for (auto i : fqueue)
        {
            i->print();
            delete i;
        }
        tab.printHeader(cout);
        tab.printAll(cout);
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
