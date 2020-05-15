#ifndef LOGFILE_H
#define LOGFILE_H

namespace apdebug
{
    namespace logfile
    {
        enum class RStatus : uint32_t
        {
            Return = 0x00,
            Time = 0x01,
            HardLimit = 0x02,
            Warn = 0x03,
            Runtime = 0x04,
            Unknown = 0xffffffff
        };
        enum class Warning : uint32_t
        {
            Overflow = 0x01
        };
        enum class RtError : uint32_t
        {
            STDExcept = 0x01,
            UnknownExcept = 0x02,
            Sigfpe = 0x03,
            DivByZero = 0x04,
            Signal = 0x05
        };
        enum class FPE : uint32_t
        {
            Normal = 0,
            FE_Divbyzero = 1,
            FE_Inexact = 1 << 1,
            FE_Invavid = 1 << 2,
            FE_Overflow = 1 << 3,
            FE_Underflow = 1 << 4
        };
        inline uint32_t operator&(FPE f1, FPE f2)
        {
            return static_cast<uint32_t>(f1) & static_cast<uint32_t>(f2);
        }
        enum class Signal : uint32_t
        {
            Sigterm = 0x01,
            Sigint = 0x02,
            Sigill = 0x03,
            Sigsegv = 0x04
        };
    }
}

#endif
