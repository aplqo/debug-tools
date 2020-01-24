#include "include/define.h"
#include "include/output.h"
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
using namespace std;
using namespace apdebug::out;
using namespace apdebug::info;
using namespace std::filesystem;

uintmax_t lim = 200;
bool red = false;
string cmd;

int main(int argc, char* argv[])
{
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
                if (file_size(f) > lim || (!red))
                {
                    red = true;
                    cerr << "Autodiff: File " << f << " tpp large, redirect stdout to file." << endl;
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
            }
        }
        if (!strcmp(argv[i], "-test"))
        {
            unsigned long n = stoul(argv[++i]);
            for (unsigned int j = 0; j < n; ++j)
                cmd = cmd + " \"" + argv[++i] + "\"";
            cerr << "Autodiff: Test command " << cmd << endl;
        }
    }
    cerr << col::NONE << endl;
    int ret = system(cmd.c_str());
    cerr << endl;
    if (ret)
        cerr << col::YELLOW;
    else
        cerr << col::GREEN;
    cerr << "Autodiff: Test program return " << ret << col::NONE << endl;
    return ret;
}