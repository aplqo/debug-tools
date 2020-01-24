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
        using std::ostringstream;
        using std::endl;
        using std::string;
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
            os << col::YELLOW << "[Warn] Integer overflow ( Operation " << oper << " Type: " << type << ")" << endl;
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
    }
}