#ifndef JUDGE_H
#define JUDGE_H

#include <cstddef>
#include <cstdint>

namespace ns_run
{
    int run(int, const char* const[]);
}

extern "C"
{
    extern void writeName(const char* name); // write demangled name
    extern void writeLog(const char* obj, const size_t size);
    extern void writeString(const char* str);
    extern void abortProgram(const unsigned int dep);
    extern int judgeMain(int (*userMain)(int, const char* const[]), int argc, const char* const argv[]);
}

int main(int argc, const char* const argv[])
{
    return judgeMain(ns_run::run, argc, argv);
}

#define main(...) ns_run::run(int argc, const char* const argv[])

#endif