#include "include/define.h"
#include "include/exception.h"
#include "include/output.h"
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

        Warn::Warn(const string typ, const string op)
        {
            type = typ;
            oper = op;
        }
        void Warn::name()
        {
            cerr << "Warn";
        }
        void Warn::verbose()
        {
            cerr << col::YELLOW << "[Warn] Integer overflow ( Operation " << oper << " Type: " << type << ")" << endl;
        }
        void Warn::details()
        {
            cerr << "Integer overflow on " << oper << " of " << type;
        }
        void Warn::color()
        {
            cerr << col::YELLOW;
        }
    }
}