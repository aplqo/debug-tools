#include "include/define.h"
#include "include/exception.h"
#include "include/output.h"
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

        TimeLimit::TimeLimit(timType t)
        {
            p = Pass(t);
        }
        string TimeLimit::name()
        {
            return "TLE";
        }
        string TimeLimit::verbose()
        {
            ostringstream os;
            os << p.verbose();
            os<< col::YELLOW << "[TLE] Time limit exceed." << endl;
            return os.str();
        }
        string TimeLimit::color()
        {
            ostringstream os;
            os << col::YELLOW;
            return os.str();
        }

        HardLimit::HardLimit(timType lim)
        {
            hardlim = lim;
        }
        string HardLimit::verbose()
        {
            ostringstream os;
            os << col::RED << "[MLE/TLE] Hard time limit exceed. ";
            PrintTime(hardlim, os);
            os << endl;
            return os.str();
        }
        string HardLimit::name()
        {
            return "MLE/TLE";
        }
        string HardLimit::color()
        {
            ostringstream os;
            os << col::RED;
            return os.str();
        }
    }
}