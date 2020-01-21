#include "include/define.h"
#include "include/exception.h"
#include "include/output.h"
#include "include/testcase.h"
#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>
using namespace std;
using namespace std::chrono;
using namespace apdebug::out;
using namespace apdebug::testcase;
using apdebug::timer::timType;
using std::filesystem::path;

tpoint tp;
void info(const char* str, const char* val)
{
    cout << col::CYAN << "[Info] " << str << ": " << val << endl;
}
void printT(timType n, const char* in)
{
    cout << col::CYAN << "[Info] " << in << ": ";
    cout << n / 1000 << "ms ( " << (double)n / 1000000 << "s )" << endl;
}
void getLog()
{
    path p(tp.rres.cmd);
    p.replace_extension(".log");
    tp.log = p.string();
}
int main(int argc, char* argv[])
{
    tp.rres.cmd = argv[1];
    cout << "----------Aplqo debug tool----------" << endl;
    for (int i = 2; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-in"))
        {
            info("Input file", argv[++i]);
            tp.in = argv[i];
        }
        else if (!strcmp(argv[i], "-out"))
        {
            info("Output file", argv[++i]);
            tp.out = argv[i];
        }
        else if (!strcmp(argv[i], "-test"))
            tp.tres.cmd = argv[++i];
        else if (!strcmp(argv[i], "-time"))
        {
            tpoint::lim = atoi(argv[++i]) * timType(1000) * 1000;
            tpoint::hardlim = tpoint::lim * 10;
        }
        else if (!strcmp(argv[i], "-hlimit"))
            tpoint::hardlim = atoi(argv[++i]) * timType(1000) * 1000;
        else if (!strcmp(argv[i], "-args"))
        {
            int ccmd = atoi(argv[++i]);
            ++i;
            cout << col::CYAN << "[Info] Arguments: ";
            for (int j = 1; j <= ccmd; ++j, ++i)
            {
                tp.rres.cmd += argv[i];
                cout << argv[i] << " ";
            }
            cout << endl;
        }
        else if (!strcmp(argv[i], "-testargs"))
        {
            int num = atoi(argv[++i]);
            ++i;
            cerr << col::CYAN << "[Info] Test command: ";
            for (int j = 0; j < num; ++j, ++i)
                tp.tres.args = tp.tres.args + " " + argv[i];
            cout << tp.tres.cmd << " " << tp.tres.args << endl;
        }
    }
    printT(tpoint::lim, "Time limit");
    printT(tpoint::hardlim, "Hard time limit");
    cout << col::BLUE << "[Info] Start program" << col::NONE << endl;
    cout.flush();
    getLog();
    tp.run();
    tp.parse();
    remove(path(tp.log));
    tp.s->verbose();
    if (!tp.tres.cmd.empty() && tp.rres.ret == 0)
    {
        cout << col::BLUE << "[Info] Start testing" << col::NONE << endl;
        cout.flush();
        tp.test();
        tp.s->verbose();
    }
    return 0;
}