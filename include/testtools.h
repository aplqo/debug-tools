#ifndef TESTTOOLS_H
#define TESTTOOLS_H

#include "include/dynArray.h"
#include "system.h"

#include <fmt/format.h>

#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <string>
#include <vector>

namespace apdebug::TestTools
{
    class AutoDiff
    {
    public:
        AutoDiff& instantiate(fmt::format_args args);
        AutoDiff& instantiate();
        void parseArgument(const YAML::Node& nod);
        void check(System::Command& cmd);
        void release();

        std::filesystem::path differ;

        bool enable = false, verbose = false;
        unsigned int size = 200;

        const char* differTemplate = nullptr;
        DynArray::DynArray<const char*> fileTemplate;

    private:
        bool redirect = false;
        DynArray::DynArray<std::filesystem::path> file;
    };
    class TemporaryFile
    {
    public:
        enum Phase
        {
            Run = 0,
            Test = 1
        };
        TemporaryFile& instantiate(fmt::format_args args);
        void parseArgument(const YAML::Node& nod);
        void release(const Phase p, const bool pass, const bool accept);

        DynArray::DynArray<const char*> filesTemplate[2][2][2] {};

    private:
        bool enable = false;
        DynArray::DynArray<std::filesystem::path> files[2][2][2]; // 0: run, 1: test
    };
}

#endif
