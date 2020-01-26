#include "include/output.h"
#include "include/define.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <string>

namespace apdebug
{
    namespace out
    {
        using apdebug::timer::timType;
        using std::endl;
        using std::initializer_list;
        using std::max;
        using std::ostream;
        using std::string;
        using std::strlen;
        using std::chrono::duration_cast;
        using std::chrono::microseconds;
        using std::chrono::milliseconds;

        /*-----Color-----*/
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
        void printT(timType n, const char* in, ostream& os)
        {
            os << "[Info] " << in << ": ";
            os << n / 1000 << "ms ( " << (double)n / 1000000 << "s )";
        }

        /*----- Print table-----*/
        table::table(initializer_list<const char*> cols)
            : num(cols.size())
        {
            width = new int[num];
            head = new const char*[num];
            int j = 0;
            for (auto i = cols.begin(); i != cols.end(); ++i, ++j)
            {
                width[j] = strlen(*i) + 2;
                head[j] = *i;
            }
        }
        void table::update(int col, int val)
        {
            width[col] = max(width[col], val);
        }
        void table::header(ostream& os)
        {
            for (int i = 0; i < num; ++i)
            {
                setw(i, os);
                os << head[i] << "  ";
            }
            os << endl;
            for (int i = 0; i < num; ++i)
            {
                for (int j = 0; j < width[i]; ++j)
                    os << "-";
                os << "  ";
            }
        }
        void table::setw(int col, ostream& os)
        {
            os.width(width[col]);
        }
        table::~table()
        {
            delete[] head;
            delete[] width;
        }
        /*-----Print test point config-----*/
        static inline void print(const char* str, const string& st, ostream& os, bool n)
        {
            if (st.empty())
                return;
            os << col::CYAN << "[Info] " << str << ": " << st;
            if (n)
                os << endl;
        }
        void PrintRun(const testcase::tpoint& tp, ostream& os,bool n)
        {
            print("Input file", tp.in, os, true);
            print("Output file", tp.out, os, true);
            print("Run arguments", tp.rres.args, os, n);
        }
        void PrintTest(const testcase::tpoint& tp, ostream& os,bool n)
        {
            print("Answer file", tp.ans, os, n);
            if (!tp.tres.cmd.empty())
            {
                if (!n)
                    os << endl;
                os << col::CYAN << "[Info] Test command: " << tp.tres.cmd << " " << tp.tres.args;
                if (n)
                    os << endl;
            }
        }
    }
}