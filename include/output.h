#ifndef OUTPUT_H
#define OUTPUT_H

#if __cplusplus < 201103L
#error ISO c++11 is required to use test output!
#endif

#include "include/define.h"
#include "include/testcase.h"
#include <array>
#include <chrono>
#include <deque>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <type_traits>

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
            CYAN = 6,
            Bold = 7,
            Underline = 8
        };
        std::ostream& operator<<(std::ostream&, col);

        /*-----Time and memory-----*/
        void PrintMemory(const size_t, std::ostream&);
        void PrintM(const size_t, const char*, std::ostream&);
        void PrintTime(std::chrono::microseconds::rep, std::ostream&);
        void printT(apdebug::timer::timType, const char*, std::ostream&);

        /*-----Print table-----*/
        template <size_t siz>
        class table
        {
        public:
            template <class... Args>
            table(const std::array<const char*, siz>& col, const Args&... fmts)
            {
                header[0] = writeToString(fmts...);
                for (unsigned int i = 0; i < siz; ++i)
                {
                    update(i, strlen(col[i]));
                    header[i + 1] = col[i];
                }
            }
            inline void update(int col, size_t val)
            {
                width[col + 1] = std::max(width[col + 1], val + 2);
            }
            void printHeader(std::ostream& os)
            {
                printRow(header, os);
                os << header[0];
                for (unsigned int i = 1; i <= siz; ++i)
                {
                    for (unsigned int j = 0; j < width[i]; ++j)
                        os << "-";
                    os << "  ";
                }
                os << col::NONE << std::endl;
            }
            inline void newColumn(const std::string& col)
            {
                q.emplace_back();
                q.back()[0] = col;
            }
            template <class... Args>
            inline void newColumn(const Args&... fmts)
            {
                newColumn(writeToString(fmts...));
            }
            template <class... Args>
            void writeColumn(int col, const Args&... args)
            {
                q.back()[col + 1] = writeToString(args...);
                update(col, q.back()[col + 1].size());
            }
            void printAll(std::ostream& os)
            {
                while (!q.empty())
                {
                    printRow(q.front(), os);
                    q.pop_front();
                }
            }

        private:
            template <class T>
            void printRow(const std::array<T, siz + 1>& r, std::ostream& os)
            {
                os << r[0]; // write color code
                for (unsigned int i = 1; i <= siz; ++i)
                {
                    os.width(width[i]);
                    os << r[i] << "  ";
                }
                os << col::NONE << std::endl;
            }
            template <class... Args>
            static std::string writeToString(const Args&... args)
            {
                std::ostringstream s;
                (s << ... << args);
                return s.str();
            }

            using row = std::array<std::string, siz + 1>;
            row header;
            size_t width[siz + 1] = {};
            std::deque<row> q;
        };
        template <size_t siz, class... Args>
        table(const std::array<const char*, siz>&, Args... args) -> table<siz>;

        /*-----Print test point config-----*/
        void PrintLimit(const testcase::limits&, std::ostream&, bool n); // print time and memory limit
        template <class T>
        void printMemConf(std::ostream& os, bool n)
        {
            os << col::CYAN << "[Info] swapaccount: " << std::boolalpha << apdebug::memory::swapaccount;
            if (T::memLimit.enable)
            {
                os << col::CYAN << std::endl;
                os << "[info] Cgroup path: " << T::memLimit.group;
            }
            if (n)
                os << std::endl;
        }
        void PrintRun(const testcase::tpoint&, std::ostream&, bool);
        void PrintTest(const testcase::tpoint&, std::ostream&, bool);
        /*-----Print version info-----*/
        static void PrintVersion(const char* str, std::ostream& os)
        {
            os << std::endl;
            os << "Debug tool: " << str << std::endl;
            os << "Version " << info::branch << "@" << info::hash << " " << info::version << std::endl;
            os << "Build compiler: " << info::compier << std::endl;
            os << "Build on " << __TIME__ << " " << __DATE__ << " by " << info::builder << std::endl;
            os << std::endl;
        }
    }
}
#endif
