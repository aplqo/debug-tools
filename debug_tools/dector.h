#ifndef DECTOR_H
#define DECTOR_H

#pragma STDC FENV_ACCESS ON
#include "debug_tools/log.h"
#include "debug_tools/logfile.h"
#include <cfenv>
#include <csignal>
#include <cstdlib>
#include <iostream>

namespace apdebug
{
    namespace dector
    {
        using std::cerr;
        using std::endl;
        using std::fetestexcept;
        using std::quick_exit;
        using std::signal;
        using namespace logfile;
        using namespace log;
        /*-----Signal support-----*/
        extern "C" void
        sig(int s)
        {
            WriteObj(RStatus::Runtime);
            WriteObj(RtError::Signal);
            switch (s)
            {
            case SIGSEGV:
                WriteObj(Signal::Sigsegv);
                break;
            case SIGINT:
                WriteObj(Signal::Sigint);
                break;
            case SIGILL:
                WriteObj(Signal::Sigill);
                break;
            case SIGTERM:
                WriteObj(Signal::Sigterm);
                break;
            }
            quick_exit(1);
        }
        uint32_t& operator|=(uint32_t& l, FPE r)
        {
            return l |= static_cast<uint32_t>(r);
        }
        extern "C" void fpe_handler(int sig)
        {
            WriteObj(RStatus::Runtime);
            WriteObj(RtError::Sigfpe);
            uint32_t v = 0;
            if (fetestexcept(FE_DIVBYZERO))
                v |= FPE::FE_Divbyzero;
            if (fetestexcept(FE_INEXACT))
                v |= FPE::FE_Inexact;
            if (fetestexcept(FE_INVALID))
                v |= FPE::FE_Invavid;
            if (fetestexcept(FE_OVERFLOW))
                v |= FPE::FE_Overflow;
            if (fetestexcept(FE_UNDERFLOW))
                v |= FPE::FE_Underflow;
            WriteObj(v);
            quick_exit(1);
        }
        void regsig()
        {
#ifdef _MSC_VER
            _controlfp(0, _MCW_EM); //Enable floating point exception
#else
            feenableexcept(FE_DIVBYZERO | FE_INEXACT | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);
#endif
            signal(SIGSEGV, sig);
            signal(SIGFPE, fpe_handler);
            signal(SIGILL, sig);
            signal(SIGINT, sig);
            signal(SIGTERM, sig);
        }
    }
}

#endif
