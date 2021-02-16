#ifndef JUDGE_H
#define JUDGE_H
#define Judge

#include <cstddef>
#include <cstdint>

#include "debug_tools/logfile.h"

namespace _User {
int main(int, const char* const[]);
}

namespace apdebug {
extern "C" {
extern void writeName(const char* name);  // write demangled name
extern void writeLog(const char* obj, const size_t size);
extern void writeString(const char* str);
namespace Judger {
extern void stopWatch(const uint32_t stat);
extern void abortProgram();
extern int judgeMain(int (*userMain)(int, const char* const[]), int argc,
                     const char* const argv[]);
}  // namespace Judger
namespace Interactor {
extern void beginReportFail(const uint32_t id);
extern void endReportFail();
extern void reportAccept();
extern int interactorMain(int (*userMain)(int, const char* const[]), int argc,
                          const char* const argv[]);
}  // namespace Interactor
}
template <class T>
inline void writeObject(const T dat)
{
  writeLog(reinterpret_cast<const char*>(&dat), sizeof(T));
}
}  // namespace apdebug
int main(int argc, const char* const argv[])
{
#ifndef Interactive
  return apdebug::Judger::judgeMain(_User::main, argc, argv);
#else
  return apdebug::Interactor::interactorMain(_User::main, argc, argv);
#endif
}

#define main(...) _User::main(int argc, const char* const argv[])

#endif