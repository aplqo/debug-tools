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
        using std::copy;
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
            width = new size_t[num];
            head = new std::string[num];
            int j = 0;
            for (auto i = cols.begin(); i != cols.end(); ++i, ++j)
            {
                width[j] = strlen(*i) + 2;
                head[j] = *i;
            }
        }
        table::table(const table& t)
            : num(t.num)
        {
            if (&t == this)
                return;
            width = new size_t[num];
            head = new string[num];
            copy(t.head, t.head + num, head);
            copy(t.width, t.width + num, width);
        }
        table::table(table&& t)
            : num(t.num)
        {
            if (&t == this)
                return;
            width = t.width;
            head = t.head;
            t.width = nullptr;
            t.head = nullptr;
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
        void PrintRun(const testcase::tpoint& tp, ostream& os, bool n)
        {
            print("Input file", tp.in, os, true);
            print("Output file", tp.out, os, true);
            print("Run arguments", tp.rres.args, os, n);
        }
        void PrintTest(const testcase::tpoint& tp, ostream& os, bool n)
        {
            print("Answer file", tp.ans, os, true);
            print("Test command", tp.tres.cmd + tp.tres.args, os, n);
        }
    }
}
