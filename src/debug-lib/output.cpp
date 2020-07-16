#include "include/output.h"
#include "include/define.h"
#include <chrono>
#include <iostream>
#include <string>

namespace apdebug
{
    namespace out
    {
        using apdebug::timer::timType;
        using std::endl;
        using std::ostream;
        using std::string;
        using std::chrono::duration_cast;
        using std::chrono::microseconds;
        using std::chrono::milliseconds;

        /*-----Color-----*/
        const char* colors[] = { "\033[0m", "\033[31m", "\033[1m\033[32m", "\033[33m",
            "\033[1m\033[34m", "\033[1m\033[35m", "\033[36m", "\033[1m", "\033[4m" };

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

        /*---Print time and memory---*/
        void PrintMemory(const size_t m, ostream& os)
        {
            const double k = m / 1024.0;
            os << k / 1024.0 << " MiB "
               << " (" << k << " KiB)";
        }
        void PrintM(const size_t m, const char* str, ostream& os)
        {
            os << "[info] " << str << ": ";
            PrintMemory(m, os);
        }
        void PrintTime(microseconds::rep ti, ostream& os)
        {
            milliseconds ms = duration_cast<milliseconds>(microseconds(ti));
            os << ms.count() << "ms ( " << ti << "us )";
        }
        void printT(timType n, const char* in, ostream& os)
        {
            os << "[Info] " << in << ": ";
            os << n / 1000 << "ms ( " << (double)n / 1000000 << "s )";
        }
        /*-----Print test point config-----*/
        static inline bool print(const char* str, const string& st, ostream& os, bool n)
        {
            if (st.empty())
                return false;
            if (n)
                os << endl;
            os << col::CYAN << "[Info] " << str << ": " << st;
            return true;
        }
        void PrintRun(const testcase::tpoint& tp, ostream& os, bool n)
        {
            print("Input file", tp.in, os, false);
            print("Output file", tp.out, os, true);
            print("Run arguments", tp.rres.args, os, true);
            if (n)
                os << endl;
        }
        void PrintTest(const testcase::tpoint& tp, ostream& os, bool n)
        {
            const bool t1 = print("Answer file", tp.ans, os, false);
            const bool t2 = print("Test command", tp.tres.cmd + tp.tres.args, os, t1);
            if (n && (t1 || t2))
                os << endl;
        }
        void PrintLimit(const testcase::limits& o, ostream& os, bool n)
        {
            os << col::CYAN;
            printT(o.lim, "Time limit", os);
            os << col::CYAN << std::endl;
            printT(o.hardlim, "Hard time limit", os);
            if (o.hardMemByte)
            {
                os << col::CYAN << std::endl;
                PrintM(o.memLimByte, "Memory limit", os);
                os << col::CYAN << std::endl;
                PrintM(o.hardMemByte, "Hard memory limit", os);
            }
            else if (o.memLimByte)
            {
                os << col::CYAN << std::endl;
                PrintM(o.memLimByte, "Memory limit", os);
            }
            if (n)
                os << std::endl;
        }
    }
}
