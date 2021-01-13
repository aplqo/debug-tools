#ifndef UTILITY_H
#define UTILITY_H

#include <filesystem>
#include <string>
#include <vector>

#include <fmt/format.h>

namespace apdebug::Utility
{
    template <class T, class... Args>
    std::vector<T> parseCmdArray(int& pos, const char* const argv[], Args... args)
    {
        std::vector<T> ret;
        if (strcmp(argv[pos], "["))
        {
            ret.emplace_back(argv[pos], args...);
            return ret;
        }
        for (++pos; strcmp(argv[pos], "]"); ++pos)
            ret.emplace_back(argv[pos], args...);
        return ret;
    }
    inline bool removeFile(std::filesystem::path& s)
    {
        namespace fs = std::filesystem;
        if (fs::exists(s))
        {
            fs::remove(s);
            s = "<released>";
            return true;
        }
        else
            s = "<unused>";
        return false;
    }
} // namespace apdebug

#endif
