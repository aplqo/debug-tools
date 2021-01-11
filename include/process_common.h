#ifndef PROCESS_COMMON_H
#define PROCESS_COMMON_H

#include <cstdint>
#include <cstring>
#include <random>
#include <string>

namespace apdebug::Process
{
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
}

#endif