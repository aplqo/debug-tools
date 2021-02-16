#ifndef LOGFILE_H
#define LOGFILE_H

#ifndef Judge
#include "include/testcase.h"
#include "system.h"
#endif
#include <cstdint>

namespace apdebug {
namespace Logfile {
static const unsigned int maxDumpDepth = 30;

enum class RStatus : uint32_t {
  Return = 0x01,
  Warn = 0x03,
  RuntimeError = 0x04,
  WrongAnswer = 0x05,
  Protocol = 0x06,
  Accept = 0x07,
  Unknown = 0xffffffff
};
enum class Warning : uint32_t { Overflow = 0x01 };
enum class RtError : uint32_t {
  STDExcept = 0x01,
  UnknownExcept = 0x02,
  Sigfpe = 0x03,
  DivByZero = 0x04,
  Signal = 0x05
};
enum FPE : uint32_t {
  Normal = 0,
  FE_Divbyzero = 1,
  FE_Inexact = 1 << 1,
  FE_Invavid = 1 << 2,
  FE_Overflow = 1 << 3,
  FE_Underflow = 1 << 4
};
enum class Signal : uint32_t {
  Sigterm = 0x01,
  Sigint = 0x02,
  Sigill = 0x03,
  Sigsegv = 0x04
};

#ifndef Judge
const Testcase::Result* parseRE(System::MemoryStream& ms,
                                Testcase::Result*& cur);
const Testcase::Result* parseWarn(System::MemoryStream& ms,
                                  Testcase::Result*& cur);
std::string readString(System::MemoryStream& ms);
#endif
}  // namespace Logfile
}  // namespace apdebug

#endif
