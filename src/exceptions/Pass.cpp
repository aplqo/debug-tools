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

        void Pass::name()
        {
            cerr << "Pass";
        }
        void Pass::verbose()
        {
            cerr << col::GREEN << "[Pass] Program finished after ";
            PrintTime(tim, cerr);
            cerr << endl;
        }
        void Pass::color()
        {
            cerr << col::GREEN;
        }

        void Accepted::name()
        {
            cerr << "AC";
        }
        void Accepted::verbose()
        {
            cerr << col::GREEN << "[AC] Test program return code 0";
            cerr << endl;
        }
        void Accepted::color()
        {
            cerr << col::GREEN;
        }

        WrongAnswer::WrongAnswer(int r)
        {
            ret = r;
        }
        void WrongAnswer::name()
        {
            cerr << "WA";
        }
        void WrongAnswer::verbose()
        {
            cerr << col::RED << "[WA] Test program return " << ret << endl;
        }
        void WrongAnswer::color()
        {
            cerr << col::RED;
        }
    }
}