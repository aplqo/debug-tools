#include "include/testtools.h"
#include "include/utility.h"
#include <cstring>
#include <filesystem>
namespace fs = std::filesystem;
using apdebug::Utility::parseCmdArray;

namespace apdebug::TestTools
{
    TemporaryFile& TemporaryFile::instantiate(fmt::format_args args)
    {
        if (!enable)
            return *this;
        for (unsigned int i = 0; i < 2; ++i)
            for (unsigned int j = 0; j < 2; ++j)
                for (unsigned int k = 0; k < 2; ++k)
                    if (filesTemplate[i][j][k])
                    {
                        auto& cur = files[i][j][k];
                        const auto& pattern = *filesTemplate[i][j][k];
                        cur.reserve(pattern.size());
                        for (unsigned int i = 0; i < pattern.size(); ++i)
                            cur.emplace_back(fmt::vformat(pattern[i], args));
                    }
        return *this;
    }
    void TemporaryFile::parseArgument(int& argc, const char* const argv[])
    {
        enable = true;
        for (; !strcmp(argv[argc], "]"); ++argc)
        {
            if (!strcmp(argv[argc], "-disable"))
                enable = false;
            else
            {
                const char* const cptr = argv[argc++];
                filesTemplate[cptr[1] == 'T'][cptr[2] == 'P'][cptr[3] == 'P'] = new std::vector(parseCmdArray<const char*>(argc, argv));
            }
        }
    }
    void TemporaryFile::release(const Phase p, const bool pass, const bool accept)
    {
        if (!enable)
            return;
        for (const auto& i : files[p][pass][accept])
            fs::remove(i);
    }

}
