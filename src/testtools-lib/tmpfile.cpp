#include "include/testtools.h"
#include "include/utility.h"
#include <cstring>
#include <filesystem>
namespace fs = std::filesystem;
using apdebug::Utility::parseCmdArray;

namespace apdebug::TestTools
{
    TemporaryFile& TemporaryFile::replace(fmt::format_args args)
    {
        if (!enable)
            return *this;
        for (auto& phase : files)
            for (auto& currentState : phase)
                for (auto& isAccept : currentState)
                    for (auto& k : isAccept)
                        k = fmt::vformat(k, args);
        return *this;
    }
    void TemporaryFile::parseArgument(int& argc, const char* const argv[])
    {
        enable = true;
        for (; !strcmp(argv[argc], "]"); ++argc)
        {
            const char* const cptr = argv[argc++];
            files[cptr[1] == 'T'][cptr[2] == 'P'][cptr[3] == 'P'] = parseCmdArray<std::string>(argc, argv);
        }
    }
    void TemporaryFile::release(const Phase p, const bool pass, const bool accept)
    {
        if (!enable)
            return;
        for (const auto& i : files[p][pass][accept])
            if (const fs::path p(i); fs::exists(p))
                fs::remove(p);
    }

}
