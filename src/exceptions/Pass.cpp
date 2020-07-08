#include "include/define.h"
#include "include/exception.h"
#include "include/output.h"
#include <sstream>

namespace apdebug
{
    namespace exception
    {
        using apdebug::timer::timType;
        using std::endl;
        using std::ostringstream;
        using namespace apdebug::out;

        std::string Pass::name()
        {
            return "Pass";
        }
        std::string Pass::verbose()
        {
            ostringstream os;
            os << color() << "[Pass] Program finished. Time: ";
            PrintTime(tim, os);
            os << " Memory: ";
            PrintMemory(mem, os);
            os << endl;
            return os.str();
        }
        std::string Pass::color()
        {
            ostringstream os;
            os << col::GREEN;
            return os.str();
        }

        std::string Accepted::name()
        {
            return "AC";
        }
        std::string Accepted::verbose()
        {
            ostringstream os;
            os << color() << "[AC] Test program return code 0";
            os << endl;
            return os.str();
        }
        std::string Accepted::color()
        {
            ostringstream os;
            os << col::GREEN;
            return os.str();
        }

        WrongAnswer::WrongAnswer(int r)
        {
            ret = r;
        }
        std::string WrongAnswer::name()
        {
            return "WA";
        }
        std::string WrongAnswer::verbose()
        {
            ostringstream os;
            os << color();
            os << "[WA] Test program return " << ret << endl;
            return os.str();
        }
        std::string WrongAnswer::color()
        {
            ostringstream os;
            os << col::RED;
            return os.str();
        }

        std::string JudgeFail::name()
        {
            return "Fail";
        }
        std::string JudgeFail::verbose()
        {
            ostringstream os;
            os << color() << "[Fail] " << vmsg << endl;
            return os.str();
        }
        std::string JudgeFail::color()
        {
            ostringstream os;
            os << col::NONE;
            return os.str();
        }
        std::string JudgeFail::details()
        {
            return dmsg;
        }
    }
}
