#ifndef OUTPUT_H
#define OUTPUT_H

#if __cplusplus < 201103L
#error ISO c++11 is required to use test output!
#endif

#include <chrono>
#include <iostream>
namespace apdebug
{
    namespace out
    {
        /*-----Color-----*/
        enum class col
        {
            NONE = 0,
            RED = 1,
            GREEN = 2,
            YELLOW = 3,
            BLUE = 4,
            PURPLE = 5,
            CYAN = 6
        };
        std::ostream& operator<<(std::ostream&, col);
        void PrintTime(std::chrono::microseconds::rep, std::ostream&);
    }
}
#endif