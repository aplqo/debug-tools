#ifndef TESTER_H
#define TESTER_H

#include <fstream>

namespace apdebug
{
    namespace tester
    {
        namespace exceptions
        {
            class Exception
            {
            };
            class ReadFail : public Exception
            {
            };
        }

        template <class T>
        T ReadOutput(std::ifstream& is)
        {
            T ret;
            is >> ret;
            if (is.fail())
                throw exceptions::ReadFail();
            return ret;
        }
    } // namespace tester
}

#endif