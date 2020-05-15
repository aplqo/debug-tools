#include "include/cmdarg.h"
#include "include/define.h"
#include "include/output.h"
#include "include/testcase.h"
#include <cstdio>
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
            unsigned long n = stoul(argv[++i]);
            for (unsigned int j = 0; j < n; ++j)
            {
                path f(argv[++i]);
                if (!exists(f))
                {
                    cerr << col::RED << "Autodiff: can't find file " << f << endl;
                    return 1;
                }
                if (file_size(f) > lim && (!red))
                {
                    red = true;
                    cerr << "Autodiff: File " << f << " too large, redirect stdout to file." << endl;
                }
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
