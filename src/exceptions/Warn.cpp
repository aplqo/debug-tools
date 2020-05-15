#include "include/define.h"
#include "include/exception.h"
#include "include/output.h"
#include "include/utility.h"
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

        Warn::Warn(const string typ, const string op)
        {
            type = typ;
            oper = op;
        }
        string Warn::name()
        {
            return "Warn";
        }
        string Warn::verbose()
        {
            ostringstream os;
            os << color() << "[Warn] Integer overflow ( Operation " << oper << " Type: " << type << ")" << endl;
            return os.str();
        }
        string Warn::details()
        {
            ostringstream os;
            os << "Integer overflow on " << oper << " of " << type;
            return os.str();
        }
        string Warn::color()
        {
            ostringstream os;
            os << col::YELLOW;
            return os.str();
        }

        state* Warn::read(std::istream& is)
        {
            logfile::Warning w;
            is.read(reinterpret_cast<char*>(&w), sizeof(w));
            std::string type = readString(is), op = readString(is);
            return new Warn(type, op);
        }
    }
}
