#ifndef JUDGE_H
#define JUDGE_H

#include <cstddef>
#include <cstdint>

namespace _interal
{
    int run(int, const char* const[]);
}

extern "C"
{
    extern void writeName(const char* name); // write demangled name
    extern void writeLog(const char* obj, const size_t size);
    extern void writeString(const char* str);
    namespace Judger
    {
        extern void stopWatch();
        extern void abortProgram(const unsigned int dep);
        extern int judgeMain(int (*userMain)(int, const char* const[]), int argc, const char* const argv[]);
    }
    namespace Interactor
    {
        extern void beginReportFail(const uint32_t id);
        extern void endReportFail();
        extern void reportAccept();
        extern int interactorMain(int (*userMain)(int, const char* const[]), int argc, const char* const argv[]);
    }
}

int main(int argc, const char* const argv[])
{
#ifndef Interactive
    return Judger::judgeMain(_interal::run, argc, argv);
#else
    return Interactor::interactorMain(_interal::run, argc, argv);
#endif
}

#define main(...) _interal::run(int argc, const char* const argv[])

#endif