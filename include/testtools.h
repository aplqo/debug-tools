#ifndef TESTTOOLS_H
#define TESTTOOLS_H

#include "process.h"

#include <fmt/format.h>

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace apdebug::TestTools
{
    class AutoDiff
    {
    public:
        AutoDiff& replace(fmt::format_args args);
        void parseArgument(int& argc, const char* const argv[]);
        void check(Process::Command& cmd);
        void release();

        std::string differ;

        bool enable = false;
        unsigned int size = 200;

    private:
        bool verbose = false;
        std::vector<std::string> file;
    };
    class TemporaryFile
    {
    public:
        enum Phase
        {
            Run = 0,
            Test = 1
        };
        TemporaryFile& replace(fmt::format_args args);
        void parseArgument(int& argc, const char* const argv[]);
        void release(const Phase p, const bool pass, const bool accept);

    private:
        bool enable = false;
        std::vector<std::string> files[2][2][2]; // 0: run, 1: test
    };
}

#endif
