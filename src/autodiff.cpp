#include "include/define.h"
#include "include/output.h"
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

uintmax_t lim = 200;
bool red = false, silent = false;
ostringstream nul;
string cmd, fil;

int main(int argc, char* argv[])
{
    if (!strcmp(argv[1], "-quiet"))
    {
        silent = true;
        cerr.rdbuf(nul.rdbuf());
    }
    cerr << "Aplqo debug tool: auto diff" << endl;
    cerr << "Version git@" << apdebug::info::hash << " " << apdebug::info::version << endl;
    cerr << "Build branch: " << apdebug::info::branch << endl;
    cerr << "Build on " << __TIME__ << " " << __DATE__ << " by " << apdebug::info::builder << endl;

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
        {
            unsigned long n = stoul(argv[++i]);
            cmd = argv[++i];
            for (unsigned int j = 1; j < n; ++j)
                cmd = cmd + " \"" + argv[++i] + "\"";
            cerr << col::BLUE << "Autodiff: Test command " << cmd << endl;
        }
    }
    if (!red)
        cerr << col::NONE << endl;
    int ret = system(cmd.c_str());
    if (!red)
        cerr << endl;
    else
        fclose(stdout);
    if (ret)
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
    cerr << "Autodiff: Test program return " << ret << col::NONE << endl;
    return ret;
}