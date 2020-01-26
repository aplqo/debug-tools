#include "include/exception.h"
#include "include/output.h"
#include <sstream>
#include <string>

namespace apdebug
{
    namespace exception
    {
        using std::endl;
        using std::ostringstream;
        using std::string;
        using namespace apdebug::out;

        string Unknown::name()
        {
            return "UKE";
        }
        string Unknown::color()
        {
            ostringstream os;
            os << col::NONE;
            return os.str();
        }
        string Unknown::verbose()
        {
            ostringstream os;
            os << "[UKE] Unknown error ( return " << ret << " ) " << endl;
            return os.str();
        }
        string Unknown::details()
        {
            ostringstream os;
            os << "Return " << ret;
            return os.str();
        }
    }
}