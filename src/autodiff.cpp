#include "include/cmdarg.h"
#include "include/define.h"
#include "include/output.h"
#include "include/testcase.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;
using namespace apdebug::out;
using namespace apdebug::info;
using namespace std::filesystem;
using apdebug::args::ReadArgument;
using apdebug::testcase::result;

uintmax_t lim = 200;
bool red = false, silent = false;
ostringstream nul;
string fil;
result exe;

bool testSize(const path& p)
{
    if (!exists(p))
    {
        cerr << col::RED << "Autodiff: can't find file " << p << endl;
        std::exit(1);
    }
    if (file_size(p) > lim && (!red))
    {
        cerr << "Autodiff: File " << p << " too large, redirect stdout to file." << endl;
        return true;
    }
    return false;
}
int main(int argc, char* argv[])
{
    if (!strcmp(argv[1], "-quiet"))
    {
        silent = true;
        cerr.rdbuf(nul.rdbuf());
    }
    else if (strcmp(argv[1], "-no-version"))
        PrintVersion("auto diff", cerr);

    cerr << col::CYAN;

    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-limit"))
        {
            lim = stoull(argv[++i]);
            cerr << "Autodiff: File size limit: " << lim << " byte." << endl;
            continue;
        }
        if (!strcmp(argv[i], "-files"))
        {
            if (strcmp(argv[++i], "["))
                red = testSize(argv[i]);
            else
            {
                for (++i; strcmp(argv[i], "]") && !red; ++i)
                    red |= testSize(argv[i]);
            }
            continue;
        }
        if (!strcmp(argv[i], "-diff"))
        {
            ++i;
            if (red)
            {
                cerr << "Autodiff: Redirect stdout to " << argv[i] << endl;
                freopen(argv[i], "w", stdout);
                fil = argv[i];
            }
        }
        if (!strcmp(argv[i], "-test"))
            exe.cmd = argv[++i];
        if (!strcmp(argv[i], "-testargs"))
            ReadArgument(exe, ++i, argv);
    }
    cerr << col::BLUE << "Autodiff: Test command " << exe.cmd << "  " << exe.args << endl;
    if (!red)
    {
        cerr << col::NONE;
        cerr.flush();
    }
    exe.exec();
    if (red)
        fclose(stdout);
    if (exe.ret)
        cerr << col::YELLOW;
    else
    {
        cerr << col::GREEN;
        if (red && exists(fil))
        {
            remove(fil);
            cerr << "Autodiff: Test passed removed output file" << endl;
        }
    }
    cerr << "Autodiff: Test program return " << exe.ret << col::NONE << endl;
    if (exe.ret)
        return exe.ret + 10;
    else
        return 0;
}
