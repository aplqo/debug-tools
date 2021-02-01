#ifndef CHECKED_H
#define CHECKED_H

#if __cplusplus < 201402L
#error C++14 is required to use checked.h
#endif

#define Judge
#include "debug_tools/judge.h"
#include "debug_tools/logfile.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <type_traits>

namespace apdebug
{
    namespace checked
    {
        using std::istream;
        using std::ostream;

        template <class T>
        class CheckedInteger
        {
        public:
            static_assert(std::is_integral<T>::value, "Check type must be integral!");
            constexpr CheckedInteger() = default;
            constexpr CheckedInteger(T s)
                : dat(s)
            {
            }
            template <class U>
            constexpr CheckedInteger(CheckedInteger<U> s)
            {
                if (s.dat > std::numeric_limits<T>::max())
                    err("assign");
                dat = s.dat;
            }
            template <class U>
            CheckedInteger& operator=(const CheckedInteger<U> v)
            {
                if (v.dat > std::numeric_limits<T>::max())
                    err("assign");
                dat = v.dat;
                return *this;
            }
            template <class U>
            CheckedInteger& operator=(const U v)
            {
                if (v > std::numeric_limits<T>::max())
                    err("assign");
                dat = v;
                return *this;
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
                    Judger::stopWatch(static_cast<uint32_t>(Logfile::RStatus::RuntimeError));
                    writeObject(Logfile::RtError::DivByZero);
                    writeName(typeid(T).name());
                    Judger::abortProgram();
                }
                return CheckedInteger(dat / r);
            }
#define checkOp(op)    \
    template <class U> \
    constexpr auto operator op(const CheckedInteger<U> r) const { return this->operator op(r.dat); }
            checkOp(+);
            checkOp(-);
            checkOp(*);
            checkOp(/);
#undef checkOP
            constexpr operator T() const
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
#undef oper
#define oper(op)                                                            \
    template <class U>                                                      \
    constexpr inline bool operator op(const U r) const { return dat op r; } \
    template <class U>                                                      \
    constexpr inline bool operator op(const CheckedInteger<U> r) const { return dat op r.dat; }
            /*Relational operators*/
            oper(>);
            oper(<);
            oper(==);
            oper(!=);
            oper(>=);
            oper(<=);
            /*Binary arithmetic operators*/
#define assop(op)      \
    template <class U> \
    inline CheckedInteger<T> operator op##=(U a) { return *this = *this op a; }
            assop(%);
            assop(&);
            assop(|);
            assop(^);
            assop(>>);
            assop(<<);
#undef assop
#define assop(op)                                                                         \
    template <class U>                                                                    \
    inline auto operator op##=(const U dat) { return *this = this->operator op<U>(dat); } \
    template <class U>                                                                    \
    inline auto operator op##=(const CheckedInteger<U> dat) { return *this = this->operator op<U>(dat.dat); }
            assop(+);
            assop(-);
            assop(*);
            assop(/);
#undef assop
            constexpr auto operator~() const
            {
                return CheckedInteger(~dat);
            }
            template <class U>
            friend ostream& operator<<(ostream&, const CheckedInteger<U>&);
            template <class U>
            friend istream& operator>>(istream&, CheckedInteger<U>&);

            T dat;

        private:
            static void err(const char* op)
            {
                Judger::stopWatch(static_cast<uint32_t>(Logfile::RStatus::Warn));
                writeObject(Logfile::Warning::Overflow);
                writeName(typeid(T).name());
                writeString(op);
                Judger::abortProgram();
            }
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
