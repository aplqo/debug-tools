#include "include/color.h"
#include "include/define.h"
#include "include/io.h"
#include "include/testtools.h"
#include "system.h"
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <unordered_map>

#include <yaml-cpp/yaml.h>
using apdebug::Output::PrintVersion;
using std::cout;
namespace Escape = apdebug::Output::Escape;

namespace
{
    enum class Param
    {
        Version,
        Autodiff,
        Test
    };
    static const std::unordered_map<std::string, Param> par {
        { "version", Param::Version },
        { "autodiff", Param::Autodiff },
        { "test", Param::Test }
    };
}

YAML::Node config, file;
apdebug::System::Command exe;
apdebug::TestTools::AutoDiff diff;
bool version = true;

int main(int argc, char* argv[])
{
    apdebug::System::consoleInit();
    {
        file = YAML::LoadFile(argv[1]);
        config = file["autodiff"];
        for (const auto& it : config)
            switch (par.at(it.first.Scalar()))
            {
            case Param::Version:
                version = it.second.as<bool>();
                break;
            case Param::Autodiff:
                diff.parseArgument(it.second);
                break;
            case Param::Test:
                exe.parseArgument(it.second);
                break;
            }
    }
    if (version)
        PrintVersion("auto diff", cout);
    if (diff.verbose)
        std::cout << Escape::TextCyan << "Autodiff: Using config file " << std::quoted(argv[1]) << "\n";
    diff.enable = true;
    diff.instantiate().check(exe);
    const int ret = exe.instantiate().execute().wait();
    if (!ret)
        diff.release();
    else if (diff.verbose)
        cout << Escape::TextYellow << "Autodiff: Test program return " << ret << Escape::None << "\n";
    return ret;
}