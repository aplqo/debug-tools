#ifndef JUDGE_H
#define JUDGE_H
#define Judge

#include "debug_tools/logfile.h"
#include <cstddef>
#include <cstdint>

namespace user
{
    int main(int, const char* const[]);
}

namespace apdebug
{
    extern "C"
    {
        extern void writeName(const char* name); // write demangled name
        extern void writeLog(const char* obj, const size_t size);
        extern void writeString(const char* str);
        namespace Judger
        {
            extern void stopWatch(const apdebug::Logfile::RStatus stat);
            extern void abortProgram();
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
    template <class T>
    inline void writeObject(const T dat)
    {
        writeLog(reinterpret_cast<const char*>(&dat), sizeof(T));
    }
}
int main(int argc, const char* const argv[])
{
#ifndef Interactive
    return apdebug::Judger::judgeMain(user::main, argc, argv);
#else
    return apdebug::Interactor::interactorMain(user::main, argc, argv);
#endif
}

#define main(...) user::main(int argc, const char* const argv[])

#endif