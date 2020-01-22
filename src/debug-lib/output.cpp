#include <chrono>
#include <iostream>

namespace apdebug
{
    namespace out
    {
        using std::ostream;
        using std::chrono::duration_cast;
        using std::chrono::microseconds;
        using std::chrono::milliseconds;

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
        const char* colors[] = { "\033[0m", "\033[31m", "\033[32m", "\033[33m",
            "\033[34m", "\033[35m", "\033[36m" };

#ifdef COLOR
        ostream& operator<<(ostream& os, col c)
        {
            os << colors[int(c)];
            return os;
        }
#else
        ostream& operator<<(ostream& os, col c)
        {
            return os;
        }
#endif

        /*---Print time---*/
        void PrintTime(microseconds::rep ti, ostream& os)
        {
            milliseconds ms = duration_cast<milliseconds>(microseconds(ti));
            os << ms.count() << "ms ( " << ti << "us )";
        }
    }
}