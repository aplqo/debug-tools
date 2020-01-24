#include "include/define.h"
#include "include/exception.h"
#include "include/output.h"
#include <csignal>
#include <exception>
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

        NormalRE::NormalRE(int t)
        {
            typ = t;
        }
        std::string NormalRE::verbose()
        {
            ostringstream os;
            os << color();
            os << "[RE] Received SIG";
            switch (typ)
            {
            case SIGSEGV:
                os << "SEGV  invalid memory access (segmentation fault)" << endl;
                break;
            case SIGINT:
                os << "INT  external interrupt, usually initiated by the user" << endl;
                break;
            case SIGILL:
                os << "IGN  invalid program image, such as invalid instruction" << endl;
                break;
            case SIGTERM:
                os << "TERM  termination request, sent to the program" << endl;
                break;
            }
            return os.str();
        }
        std::string NormalRE::details()
        {
            ostringstream os;
            os << "Received SIG";
            switch (typ)
            {
            case SIGSEGV:
                os << "SEGV";
                break;
            case SIGINT:
                os << "INT";
                break;
            case SIGILL:
                os << "ILL";
                break;
            case SIGTERM:
                os << "TERM";
                break;
            }
            return os.str();
        }

        std::string FloatPoint::verbose()
        {
            ostringstream os;
            os << color();
            os << "[RE] Received SIGFPE  erroneous arithmetic operation such as divide by zero " << endl;
            if (!stat)
                return os.str();
            if (stat & FE_DIVBYZREO)
                os << "\tFE_DIVBYZREO: pole error occurred in an earlier floating-point operation." << endl;
            if (stat & FE_INEXACT)
                os << "\tFE_INEXACT: inexact result: rounding was necessary to store the result of an earlier floating-point operation." << endl;
            if (stat & FE_INVALID)
                os << "\tFE_INVALID: domain error occurred in an earlier floating-point operation" << endl;
            if (stat & FE_OVERFLOW)
                os << "\tFE_OVERFLOW: the result of the earlier floating-point operation was too large to be representable" << endl;
            if (stat & FE_UNDERFLOW)
                os << "\tFE_UNDERFLOW: the result of the earlier floating-point operation was subnormal with a loss of precision" << endl;
            return os.str();
        }
        std::string FloatPoint::details()
        {
            ostringstream os;
            os << "Received SIGFPE";
            if (!stat)
                return os.str();
            os << " ( Float point exception: ";
            if (stat & FE_DIVBYZREO)
                os << "FE_DIVBYZREO ";
            if (stat & FE_INEXACT)
                os << "FE_INEXACT ";
            if (stat & FE_INVALID)
                os << "FE_INVALID ";
            if (stat & FE_OVERFLOW)
                os << "FE_OVERFLOW ";
            if (stat & FE_UNDERFLOW)
                os << "FE_UNDERFLOW ";
            os << ")";
            return os.str();
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