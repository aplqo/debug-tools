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

        Warn::Warn(const char* typ, const char* op)
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
            cerr << col::YELLOW << "[Warn] Integer overflow ( Operation " << oper << " Type: " << type << endl;
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