#ifndef CHECKED_H
#define CHECKED_H

#if __cplusplus < 201402L
#error C++14 is required to use checked.h
#endif

#include "debug_tools/log.h"
#include "debug_tools/logfile.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <type_traits>

namespace apdebug
{
    namespace checked
    {
        using logfile::RStatus;
        using logfile::Warning;
        using std::cerr;
        using std::common_type;
        using std::endl;
        using std::is_integral;
        using std::istream;
        using std::ostream;
        using std::quick_exit;
        using namespace log;

        template <class T>
        class CheckedInteger
        {
        public:
            static_assert(is_integral<T>::value, "Check type must be integral!");
            constexpr CheckedInteger() = default;
            constexpr CheckedInteger(T s)
                : dat(s)
            {
            }
            template <class U>
            constexpr auto operator+(const U r) const
            {
                auto ret = dat + r;
                if (ret - dat != r || ((r > 0) ^ (ret > this->dat)))
                    err("Add");
                return CheckedInteger(ret);
            }
            template <class U>
            constexpr auto operator*(const U a) const
            {
                auto ret = dat * a;
                if (this->dat != 0 && a != 0 && (ret == 0 || ret / a != this->dat))
                    err("Multiply");
                return CheckedInteger(ret);
            }
            template <class U>
            constexpr auto operator-(const U a) const
            {
                auto ret = dat - a;
                if (ret + a != this->dat || (((a > 0) ^ (ret < this->dat)) && a != 0))
                    err("Minus");
                return CheckedInteger(ret);
            }
            template <class U>
            constexpr auto operator/(const U r) const
            {
                if (r == 0)
                {
                    WriteObj(RStatus::Runtime);
                    WriteObj(logfile::RtError::DivByZero);
                    WriteString(typeid(T).name());
                    quick_exit(1);
                }
                return CheckedInteger(dat / r);
            }
#define checkOp(op)    \
    template <class U> \
    constexpr auto operator##op(const CheckedInteger<U> r) const { return *this op r.dat; }
            checkOp(+);
            checkOp(-);
            checkOp(*);
            checkOp(/);
#undef checkOP
            operator T() const
            {
                return dat;
            };
            /*Increment and decrement*/
            inline CheckedInteger<T>& operator++()
            {
                T ret = dat;
                ++dat;
                if (ret >= dat)
                    err("post-incresaement");
                return *this;
            }
            inline CheckedInteger<T> operator++(int t)
            {
                T ret = dat;
                ++dat;
                if (ret >= dat)
                    err("pre-increasement");
                return ret;
            }
            inline CheckedInteger<T>& operator--()
            {
                T ret = dat;
                --dat;
                if (ret <= dat)
                    err("post-decreasement");
                return *this;
            }
            inline CheckedInteger<T> operator--(int t)
            {
                T ret = dat;
                --dat;
                if (ret <= dat)
                    err("pre-decreasement");
                return ret;
            }
#define oper(op)       \
    template <class U> \
    constexpr auto operator op(const U r) const { return CheckedInteger(dat op r); }
            oper(%);
            /*Bitwise arithmetic operators*/
            oper(&);
            oper(|);
            oper(^);
            oper(>>);
            oper(<<);
            /*Relational operators*/
            oper(>);
            oper(<);
            oper(==);
            oper(!=);
            oper(>=);
            oper(<=);
#undef oper
            /*Binary arithmetic operators*/
#define assop(op)      \
    template <class U> \
    inline CheckedInteger<T> operator op##=(U a) { return *this = *this op a; }
            assop(/);
            assop(*);
            assop(-);
            assop(+);
            assop(%);
            assop(&);
            assop(|);
            assop(^);
            assop(>>);
            assop(<<);
#undef assop
            constexpr auto operator~() const
            {
                return CheckedInteger(~dat);
            }
            template <class U>
            friend ostream& operator<<(ostream&, const CheckedInteger<U>&);
            template <class U>
            friend istream& operator>>(istream&, CheckedInteger<U>&);

        private:
            static void err(const char* op)
            {
                WriteObj(RStatus::Warn);
                WriteObj(Warning::Overflow);
                WriteString(typeid(T).name());
                WriteString(op);
                quick_exit(2);
            }
            T dat;
        };
        /*Stream extraction and insertion*/
        template <class T>
        ostream& operator<<(ostream& os, const CheckedInteger<T>& d)
        {
            os << d.dat;
            return os;
        }
        template <class T>
        istream& operator>>(istream& is, CheckedInteger<T>& d)
        {
            is >> d.dat;
            return is;
        }
    }
}
#endif
