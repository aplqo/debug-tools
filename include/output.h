#ifndef OUTPUT_H
#define OUTPUT_H

#if __cplusplus < 201103L
#error ISO c++11 is required to use test output!
#endif

#include "include/define.h"
#include "include/testcase.h"
#include <chrono>
#include <initializer_list>
#include <iostream>
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
            CYAN = 6
        };
        std::ostream& operator<<(std::ostream&, col);

        /*-----Time-----*/
        void PrintTime(std::chrono::microseconds::rep, std::ostream&);
        void printT(apdebug::timer::timType, const char*, std::ostream&);

        /*-----Print table-----*/
        class table
        {
        public:
            table(std::initializer_list<const char*> col);
            void update(int col, int val);
            void header(std::ostream& os);
            void setw(int col, std::ostream& os);
            ~table();

            template <class T>
            void print(int col, const T dat, std::ostream& os)
            {
                this->setw(col, os);
                os << dat << "  ";
            }

        private:
            const int num;
            int* width;
            const char** head;
        };
        /*-----Print test point config-----*/
        template <class T>
        void PrintLimit(std::ostream& os, bool n) // print time limit
        {
            static_assert(std::is_base_of_v<testcase::tpoint, T>);
            os << col::CYAN;
            printT(T::lim, "Time limit", os);
            os << col::CYAN << std::endl;
            printT(T::hardlim, "Hard time limit", os);
            if (n)
                os << std::endl;
        }
        void PrintRun(const testcase::tpoint&, std::ostream&, bool);
        void PrintTest(const testcase::tpoint&, std::ostream&, bool);
        /*-----Print version info-----*/
        static void PrintVersion(const char* str, std::ostream& os)
        {
            os << std::endl;
            os << "Aplqo debug tool: " << str << std::endl;
            os << "Version git@" << info::hash << " " << info::version << std::endl;
            os << "Build branch: " << info::branch << std::endl;
            os << "Build on " << __TIME__ << " " << __DATE__ << " by " << info::builder << std::endl;
            os << std::endl;
        }
    }
}
#endif