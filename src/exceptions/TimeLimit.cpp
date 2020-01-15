#include "include/define.h"
#include "include/exception.h"
#include "include/output.h"
#include <iostream>

namespace apdebug
{
    namespace exception
    {
        using apdebug::timer::timType;
        using std::cerr;
        using std::endl;
        using namespace apdebug::out;

        TimeLimit::TimeLimit(timType t)
        {
            p = Pass(t);
        }
        void TimeLimit::name()
        {
            cerr << "TLE";
        }
        void TimeLimit::verbose()
        {
            p.verbose();
            cerr << col::YELLOW << "[TLE] Time limit exceed." << endl;
        }
        void TimeLimit::color()
        {
            cerr << col::YELLOW;
        }

        HardLimit::HardLimit(timType lim)
        {
            hardlim = lim;
        }
        void HardLimit::verbose()
        {
            cerr << col::RED << "[MLE/TLE] Hard time limit exceed. ";
            PrintTime(hardlim, cerr);
        }
        void HardLimit::name()
        {
            cerr << "MLE/TLE";
        }
        void HardLimit::color()
        {
            cerr << col::RED;
        }
    }
}