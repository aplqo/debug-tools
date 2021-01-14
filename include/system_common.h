#ifndef PROCESS_COMMON_H
#define PROCESS_COMMON_H

#include <cstdint>
#include <cstring>
#include <random>
#include <string>

#include <fmt/format.h>

namespace apdebug::System
{
    static const uint32_t eof = 0xffffffff;

    class MemoryStream
    {
    public:
        inline void read(char* dest, size_t len)
        {
            memcpy(dest, ptr, len);
            ptr += len;
        }
        inline void write(const char* src, size_t len)
        {
            memcpy(ptr, src, len);
            ptr += len;
        }
        template <class T>
        inline void read(T& res) { read(reinterpret_cast<char*>(&res), sizeof(T)); }
        template <class T>
        inline void write(const T res) { write(reinterpret_cast<const char*>(&res), sizeof(T)); }
        bool eof() const { return *reinterpret_cast<uint32_t*>(ptr) == System::eof; }
        inline void ignore(const size_t len) { ptr += len; }

        char* ptr;
    };
    struct TimeUsage
    {
        unsigned long long real, user, sys;

        TimeUsage operator-(const TimeUsage& r) const
        {
            return TimeUsage {
                .real = real - r.real,
                .user = user - r.user,
                .sys = sys - r.sys
            };
        }
    };
    typedef unsigned long long MemoryUsage;
    enum class RedirectType
    {
        StdIn,
        StdOut,
        StdErr
    };

    template <class T>
    inline void randomName(T begin, const unsigned int len)
    {
        static const char character[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::mt19937 rnd((std::random_device())());
        std::uniform_int_distribution<unsigned int> dis(0, sizeof(character) - 2);
        std::generate_n(begin, len, [&dis, &rnd]() { return character[dis(rnd)]; });
    }
    std::string GetThreadId();

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
}

#endif