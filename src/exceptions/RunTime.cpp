#include "include/define.h"
#include "include/exception.h"
#include "include/logfile.h"
#include "include/output.h"
#include "include/utility.h"
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

namespace apdebug
{
    namespace exception
    {
        using apdebug::timer::timType;
        using std::endl;
        using std::ostringstream;
        using std::string;
        using utility::readString;
        using namespace apdebug::out;

        std::string RuntimeError::name()
        {
            return "RE";
        }
        std::string RuntimeError::color()
        {
            ostringstream os;
            os << col::PURPLE;
            return os.str();
        }
        state* RuntimeError::read(std::istream& is)
        {
            using logfile::RtError;
            RtError typ;
            is.read(reinterpret_cast<char*>(&typ), sizeof(typ));
            switch (typ)
            {
            case RtError::STDExcept:
                return STDExcept::read(is);
            case RtError::UnknownExcept:
                return new UnknownExcept;
            case RtError::Sigfpe:
                return FloatPoint::read(is);
            case RtError::DivByZero:
                return DivByZero::read(is);
            case RtError::Signal:
                return NormalRE::read(is);
            }
            return nullptr;
        }

        NormalRE::NormalRE(logfile::Signal t)
        {
            typ = t;
        }
        std::string NormalRE::verbose()
        {
            using logfile::Signal;
            ostringstream os;
            os << color();
            os << "[RE] Received SIG";
            switch (typ)
            {
            case Signal::Sigsegv:
                os << "SEGV  invalid memory access (segmentation fault)" << endl;
                break;
            case Signal::Sigint:
                os << "INT  external interrupt, usually initiated by the user" << endl;
                break;
            case Signal::Sigill:
                os << "ILL  invalid program image, such as invalid instruction" << endl;
                break;
            case Signal::Sigterm:
                os << "TERM  termination request, sent to the program" << endl;
                break;
            }
            return os.str();
        }
        std::string NormalRE::details()
        {
            using logfile::Signal;
            ostringstream os;
            os << "Received SIG";
            switch (typ)
            {
            case Signal::Sigsegv:
                os << "SEGV";
                break;
            case Signal::Sigint:
                os << "INT";
                break;
            case Signal::Sigill:
                os << "ILL";
                break;
            case Signal::Sigterm:
                os << "TERM";
                break;
            }
            return os.str();
        }
        state* NormalRE::read(std::istream& is)
        {
            logfile::Signal sig;
            is.read(reinterpret_cast<char*>(&sig), sizeof(sig));
            return new NormalRE(sig);
        }

        std::string FloatPoint::verbose()
        {
            using logfile::FPE;
            ostringstream os;
            os << color();
            os << "[RE] Received SIGFPE  erroneous arithmetic operation such as divide by zero " << endl;
            if (stat == FPE::Normal)
                return os.str();
            if (stat & FPE::FE_Divbyzero)
                os << "\tFE_DIVBYZERO: pole error occurred in an earlier floating-point operation." << endl;
            if (stat & FPE::FE_Inexact)
                os << "\tFE_INEXACT: inexact result: rounding was necessary to store the result of an earlier floating-point operation." << endl;
            if (stat & FPE::FE_Invavid)
                os << "\tFE_INVALID: domain error occurred in an earlier floating-point operation" << endl;
            if (stat & FPE::FE_Overflow)
                os << "\tFE_OVERFLOW: the result of the earlier floating-point operation was too large to be representable" << endl;
            if (stat & FPE::FE_Underflow)
                os << "\tFE_UNDERFLOW: the result of the earlier floating-point operation was subnormal with a loss of precision" << endl;
            return os.str();
        }
        std::string FloatPoint::details()
        {
            using logfile::FPE;
            ostringstream os;
            os << "Received SIGFPE";
            if (stat == FPE::Normal)
                return os.str();
            os << " ( Float point exception: ";
            if (stat & FPE::FE_Divbyzero)
                os << "FE_DIVBYZERO ";
            if (stat & FPE::FE_Inexact)
                os << "FE_INEXACT ";
            if (stat & FPE::FE_Invavid)
                os << "FE_INVALID ";
            if (stat & FPE::FE_Overflow)
                os << "FE_OVERFLOW ";
            if (stat & FPE::FE_Underflow)
                os << "FE_UNDERFLOW ";
            os << ")";
            return os.str();
        }
        state* FloatPoint::read(std::istream& is)
        {
            FloatPoint* ret = new FloatPoint;
            is.read(reinterpret_cast<char*>(&(ret->stat)), sizeof(ret->stat));
            return ret;
        }

        DivByZero::DivByZero(const string typ)
        {
            type = typ;
        }
        std::string DivByZero::verbose()
        {
            ostringstream os;
            os << color();
            os << "[RE] Divide by zero. type: " << type << endl;
            return os.str();
        }
        std::string DivByZero::details()
        {
            return "Divide by zero on type " + type;
        }
        state* DivByZero::read(std::istream& is)
        {
            string typ = utility::readString(is);
            return new DivByZero(typ);
        }

        STDExcept::STDExcept(const string typ, const string des)
        {
            type = typ;
            what = des;
        }
        std::string STDExcept::verbose()
        {
            ostringstream os;
            os << color();
            os << "[RE] Throw an exception of type " << type << endl;
            os << "\t what() " << what << endl;
            return os.str();
        }
        std::string STDExcept::details()
        {
            return "Throw an exception " + type;
        }
        state* STDExcept::read(std::istream& is)
        {
            string typ = readString(is), des = readString(is);
            return new STDExcept(typ, des);
        }

        std::string UnknownExcept::verbose()
        {
            ostringstream os;
            os << color();
            os << "[RE] Throw an unknown exception" << endl;
            return os.str();
        }
        std::string UnknownExcept::details()
        {
            return "Throw an unknown exception";
        }
    }
}