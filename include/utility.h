#ifndef UTILITY_H
#define UTILITY_H
#include "process.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <fmt/format.h>

namespace apdebug
{
    namespace Utility
    {
        template <class T>
        void writeFile(const std::filesystem::path& p, const T& dat)
        {
            std::ofstream f(p);
            f << dat;
            f.close();
        }
        template <class T>
        T readFileVal(const std::filesystem::path& p)
        {
            std::ifstream f(p);
            T ret;
            f >> ret;
            return ret;
        };
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
        inline bool removeFile(std::string& s)
        {
            namespace fs = std::filesystem;
            if (fs::path tmp(s); fs::exists(tmp))
            {
                fs::remove(tmp);
                s = "<released>";
                return true;
            }
            else
                s = "<unused>";
            return false;
        }
        template <class T>
        concept Replaceable = requires(fmt::format_args pat, T&& a) { { a.replace(pat) }; };
        template <Replaceable T>
        inline void ReplaceMaybe(fmt::format_args pat, T&& a)
        {
            a.replace(pat);
        }
        template <class T>
        inline void ReplaceMaybe(fmt::format_args, T&&) { }
        template <class T, class... Args>
        inline void ReplaceMaybe(fmt::format_args pat, T&& one, Args&&... other)
        {
            ReplaceMaybe(pat, one);
            ReplaceMaybe(pat, other...);
        }
        template <Replaceable... Args>
        inline void ReplaceStrict(fmt::format_args pat, Args&&... other)
        {
            ReplaceMaybe(pat, other...);
        }
    } // namespace utility
} // namespace apdebug

#endif
