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

        string LimitExceed::name()
        {
            switch (typ)
            {
            case Time:
                return "TLE";
            case Memory:
                return "MLE";
            case Time | Memory:
                return "TLE MLE";
            }
        }
        string LimitExceed::verbose()
        {
            ostringstream os;
            os << p.verbose();
            os << color();
            if (typ & Time)
                os << "[TLE] Time limit exceed." << endl;
            if (typ & Memory)
                os << "[MLE] Memory limit exceed." << endl;
            return os.str();
        }
        string LimitExceed::color()
        {
            std::ostringstream os;
            os << col::YELLOW;
            return os.str();
        }

        HardTimeLimit::HardTimeLimit(timType lim)
        {
            hardlim = lim;
        }
        string HardTimeLimit::verbose()
        {
            ostringstream os;
            os << color() << "[Hard TLE] Hard time limit exceed.";
            PrintTime(hardlim, os);
            os << endl;
            return os.str();
        }
        string HardTimeLimit::name()
        {
            return "Hard TLE";
        }
        string HardTimeLimit::color()
        {
            ostringstream os;
            os << col::RED;
            return os.str();
        }

        string HardMemoryLimit::verbose()
        {
            ostringstream os;
            os << color() << "[Hard MLE] Hard memory limit exceed.";
            PrintMemory(mem, os);
            os << endl;
            return os.str();
        }
        string HardMemoryLimit::name()
        {
            return "Hard MLE";
        }
        string HardMemoryLimit::color()
        {
            ostringstream os;
            os << col::RED;
            return os.str();
        }
    }
}
