#include "include/define.h"
#include "include/output.h"
#include "include/testtools.h"
#include "process.h"
#include <cstdio>
#include <iostream>
using apdebug::Output::PrintVersion;
using std::cout;
namespace SGR = apdebug::Output::SGR;

apdebug::Process::Command exe;
apdebug::TestTools::AutoDiff diff;

int main(int argc, char* argv[])
{
    if (strcmp(argv[1], "-no-version"))
        PrintVersion("auto diff", cout);
    diff.enable = true;
    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-autodiff"))
            diff.parseArgument(++i, argv);
        else if (!strcmp(argv[i], "-test"))
            exe.path = argv[++i];
        else if (!strcmp(argv[i], "-test-args"))
            exe.parseArgument(++i, argv);
    }
    diff.check(exe);
    const int ret = exe.finalizeForExec().execute().wait();
    if (!ret)
        diff.release();
    else
        cout << SGR::TextYellow << "Autodiff: Test program return " << ret << SGR::None << "\n";
    return ret;
}