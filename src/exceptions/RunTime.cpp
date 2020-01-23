#include "include/define.h"
#include "include/exception.h"
#include "include/output.h"
#include <csignal>
#include <exception>
#include <iostream>
#include <string>

namespace apdebug
{
    namespace exception
    {
        using apdebug::timer::timType;
        using std::cerr;
        using std::endl;
        using std::string;
        using namespace apdebug::out;

        void RuntimeError::name()
        {
            cerr << "RE";
        }
        void RuntimeError::color()
        {
            cerr << col::PURPLE;
        }

        NormalRE::NormalRE(int t)
        {
            typ = t;
        }
        void NormalRE::verbose()
        {
            color();
            cerr << "[RE] Received SIG";
            switch (typ)
            {
            case SIGSEGV:
                cerr << "SEGV  invalid memory access (segmentation fault)" << endl;
                break;
            case SIGINT:
                cerr << "INT  external interrupt, usually initiated by the user" << endl;
                break;
            case SIGILL:
                cerr << "IGN  invalid program image, such as invalid instruction" << endl;
                break;
            case SIGTERM:
                cerr << "TERM  termination request, sent to the program" << endl;
                break;
            }
        }
        void NormalRE::details()
        {
            cerr << "Received SIG";
            switch (typ)
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
        }

        void FloatPoint::verbose()
        {
            color();
            cerr << "[RE] Received SIGFPE  erroneous arithmetic operation such as divide by zero " << endl;
            if (!stat)
                return;
            if (stat & FE_DIVBYZREO)
                cerr << "\tFE_DIVBYZREO: pole error occurred in an earlier floating-point operation." << endl;
            if (stat & FE_INEXACT)
                cerr << "\tFE_INEXACT: inexact result: rounding was necessary to store the result of an earlier floating-point operation." << endl;
            if (stat & FE_INVALID)
                cerr << "\tFE_INVALID: domain error occurred in an earlier floating-point operation" << endl;
            if (stat & FE_OVERFLOW)
                cerr << "\tFE_OVERFLOW: the result of the earlier floating-point operation was too large to be representable" << endl;
            if (stat & FE_UNDERFLOW)
                cerr << "\tFE_UNDERFLOW: the result of the earlier floating-point operation was subnormal with a loss of precision" << endl;
        }
        void FloatPoint::details()
        {
            cerr << "Received SIGFPE";
            if (!stat)
                return;
            cerr << " ( Float point exception: ";
            if (stat & FE_DIVBYZREO)
                cerr << "FE_DIVBYZREO ";
            if (stat & FE_INEXACT)
                cerr << "FE_INEXACT ";
            if (stat & FE_INVALID)
                cerr << "FE_INVALID ";
            if (stat & FE_OVERFLOW)
                cerr << "FE_OVERFLOW ";
            if (stat & FE_UNDERFLOW)
                cerr << "FE_UNDERFLOW ";
            cerr << ")";
        }

        DivByZero::DivByZero(const string typ)
        {
            type = typ;
        }
        void DivByZero::verbose()
        {
            color();
            cerr << "[RE] Divide by zero. type: " << type << endl;
        }
        void DivByZero::details()
        {
            cerr << "Divide by zero on type " << type;
        }

        STDExcept::STDExcept(const string typ, const string des)
        {
            type = typ;
            what = des;
        }
        void STDExcept::verbose()
        {
            color();
            cerr << "[RE] Throw an exception of type " << type << endl;
            cerr << "\t what() " << what << endl;
        }
        void STDExcept::details()
        {
            cerr << "Throw an exception " << type;
        }

        void UnknownExcept::verbose()
        {
            color();
            cerr << "[RE] Throw an unknown exception" << endl;
        }
        void UnknownExcept::details()
        {
            cerr << "Throw an unknown exception";
        }
    }
}