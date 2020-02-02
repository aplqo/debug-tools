#ifndef DECTOR_H
#define DECTOR_H

#pragma STDC FENV_ACCESS ON
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
        /*-----Signal support-----*/
        extern "C" void
        sig(int s)
        {
            cerr << "RE SIG";
            switch (s)
            {
            case SIGSEGV:
                cerr << "SEGV";
                break;
            case SIGINT:
                cerr << "INT";
                break;
            case SIGILL:
                cerr << "ILL";
                break;
            case SIGTERM:
                cerr << "TERM";
                break;
            }
            cerr << endl;
            quick_exit(1);
        }
        extern "C" void fpe_handler(int sig)
        {
            cerr << "RE SIGFPE " << endl;
            if (fetestexcept(FE_DIVBYZERO))
                cerr << "FE_DIVBYZERO " << endl;
            if (fetestexcept(FE_INEXACT))
                cerr << "FE_INEXACT " << endl;
            if (fetestexcept(FE_INVALID))
                cerr << "FE_INVALID " << endl;
            if (fetestexcept(FE_OVERFLOW))
                cerr << "FE_OVERFLOW " << endl;
            if (fetestexcept(FE_UNDERFLOW))
                cerr << "FE_UNDERFLOW " << endl;
            quick_exit(1);
        }
        void regsig()
        {
#ifdef _MSC_VER
            _controlfp(0, _MCW_EM); //Enable floating point exception
#else
	    feenableexcept(FE_DIVBYZERO | FE_INEXACT | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW );
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
