#ifndef PROGRAM_H
#define PROGRAM_H

#include "debug_tools/dector.h"
#include "debug_tools/timer.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace ns_run
{
    inline int run(int argc, char* c[]);
}
namespace program
{
    using namespace apdebug;
    using namespace std::chrono;
    apdebug::timer::timType li, hli;
    timer::timer<steady_clock, microseconds::rep, microseconds::period> t(li, hli);

    void atex() noexcept
    {
        t.stop();
        t.print();
    }
}
int main(int argc, char* argv[])
{
    using std::strcmp;
    using namespace program;
    if (strcmp(argv[1], "*") != 0)
        std::freopen(argv[1], "r", stdin);
    if (strcmp(argv[2], "*") != 0)
        std::freopen(argv[2], "w", stdout);
    if (strcmp(argv[3], "*") != 0)
        std::freopen(argv[3], "w", stderr);
    li = atoi(argv[4]);
    hli = atoi(argv[5]);
    dector::regsig();
    std::atexit(atex);
    int ret;
    argv[5] = argv[0];
    std::cerr.flush();
    t.start();
    try
    {
        ret = ns_run::run(argc - 5, argv + 5);
    }
    catch (const std::exception& e)
    {
        std::cerr << "RE STDException " << typeid(e).name() << std::endl
                  << e.what() << std::endl;
        std::quick_exit(1);
    }
    catch (...)
    {
        std::cerr << "RE UnknownException" << std::endl;
        std::quick_exit(1);
    }

    t.stop();
    t.print();
    std::cerr << "Ret " << ret << std::endl;
    return 0;
}
#define main(...) ns_run::run(int argc, char* argv[])
#endif