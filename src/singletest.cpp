#include "include/cmdarg.h"
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
using namespace apdebug::args;
using apdebug::timer::timType;
using std::filesystem::path;

tpoint tp;
int main(int argc, char* argv[])
{
    tp.rres.cmd = argv[1];
    PrintVersion("single test runner", cout);

    for (int i = 2; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-in"))
            tp.in = argv[++i];
        else if (!strcmp(argv[i], "-out"))
            tp.out = argv[++i];
        else if (!strcmp(argv[i], "-test"))
            tp.tres.cmd = argv[++i];
        else if (ReadLimit<tpoint>(i, argv))
            continue;
        else if (!strcmp(argv[i], "-args"))
            ReadArgument(tp.rres, ++i, argv);
        else if (!strcmp(argv[i], "-testargs"))
            ReadArgument(tp.tres, ++i, argv);
    }
    tp.init();
    PrintRun(tp, cout, true);
    PrintTest(tp, cout, true);
    PrintLimit<tpoint>(cout, true);
    cout << col::BLUE << "[Info] Start program" << col::NONE << endl;
    cout.flush();
    tp.run();
    tp.parse();
    cout << tp.s->verbose();
    if (tp.success() && !tp.tres.cmd.empty())
    {
        cout << col::BLUE << "[Info] Start testing" << col::NONE << endl;
        cout.flush();
        tp.test();
        cout << tp.ts->verbose();
    }
    tp.release();
    cout << col::NONE;
    cout.flush();
    return 0;
}