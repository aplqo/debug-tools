#ifndef CHECKED_H
#define CHECKED_H
#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace apdebug
{
    namespace checked
    {
        using std::cerr;
        using std::common_type;
        using std::endl;
        using std::is_integral;
        using std::istream;
        using std::ostream;
        using std::quick_exit;

        template <class T>
        class CheckedInteger
        {
        public:
            static_assert(is_integral<T>::value, "Check type must be integral!");
            CheckedInteger() = default;
            CheckedInteger(T s)
                : dat(s)
            {
            }
            template <class U>
            inline auto operator+(const U r) const
            {
                auto ret = dat + r;
                if (ret - dat != r || ((r > 0) ^ (ret > this->dat)))
                    err("Add");
                return ret;
            }
            template <class U>
            inline auto operator*(const U a) const
            {
                auto ret = dat * a;
                if (ret / a.dat != this->dat)
                    err("Multiply");
                return ret;
            }
            template <class U>
            inline auto operator-(const U a) const
            {
                auto ret = dat - a;
                if (ret + a != this->dat || (((a > 0) ^ (ret < this->dat)) && a != 0))
                    err("Minus");
                return ret;
            }
            template <class U>
            inline auto operator/(const U r) const
            {
                if (r == 0)
                {
                    cerr << "RE DIV " << typeid(T).name() << endl;
                    quick_exit(1);
                }
                return dat / r;
            }
            operator T() const { return dat; };
            /*Increment and decrement*/
            inline typename CheckedInteger<T>& operator++()
            {
                T ret = dat;
                ++dat;
                if (ret >= dat)
                    err("post-incresaement");
                return *this;
            }
            inline typename CheckedInteger<T> operator++(int t)
            {
                T ret = dat;
                ++dat;
                if (ret >= dat)
                    err("pre-increasement");
                return ret;
            }
            inline typename CheckedInteger<T>& operator--()
            {
                T ret = dat;
                --dat;
                if (ret <= dat)
                    err("post-decreasement");
                return *this;
            }
            inline typename CheckedInteger<T> operator--(int t)
            {
                T ret = dat;
                --dat;
                if (ret <= dat)
                    err("pre-decreasement");
                return ret;
            }
#define oper(op)       \
    template <class U> \
    inline auto operator op(const U r) const { return dat op r; }
            oper(%);
            /*Bitwise arithmetic operators*/
            oper(&);
            oper(|);
            oper (^);
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
    inline typename CheckedInteger<T> operator op##=(U a) { return *this = *this op a; }
            assop(/);
            assop(*);
            assop(-);
            assop(+);
            assop(%);
            assop(&);
            assop(|);
            assop (^);
            assop(>>);
            assop(<<);
#undef assop
            inline auto operator~() const
            {
                return ~dat;
            }
            template <class U>
            friend inline ostream& operator<<(ostream&, const CheckedInteger<U>&);
            template <class U>
            friend inline istream& operator>>(istream&, CheckedInteger<U>&);

        private:
            static void err(char* op)
            {
                cerr << "Warn OVERFLOW " << op << " " << typeid(T).name() << endl;
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