#include "include/exception.h"
#include "include/output.h"
#include <iostream>

namespace apdebug
{
    namespace exception
    {
        using std::cerr;
        using std::endl;
        using namespace apdebug::out;

        void Unknown::name()
        {
            cerr << "UKE";
        }
        void Unknown::color()
        {
            cerr << col::NONE;
        }
        void Unknown::verbose()
        {
            cerr << "[UKE] Unknown error" << endl;
        }
    }
}