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
            inline auto operator+(U r)
            {
                auto ret = dat + r;
                if (ret - dat != r || ((r > 0) ^ (ret > this->dat)))
                    err("Add");
                return ret;
            }
            template <class U>
            inline auto operator*(U a)
            {
                auto ret = dat * a.dat;
                if (ret / a.dat != this->dat)
                    err("Multiply");
                return ret;
            }
            template <class U>
            inline auto operator-(U a)
            {
                auto ret = dat - a.dat;
                if (ret + a.dat != this->dat || ((a.dat < 0) ^ (ret < this->dat)))
                    err("Minus");
                return ret;
            }
            template <class U>
            inline auto operator/(U r)
            {
                if (r == 0)
                {
                    cerr << "RE DIV " << typeid(T).name() << endl;
                    quick_exit(1);
                }
                return dat / r;
            }
            operator T() { return dat; };
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
    inline auto operator op(U r) { return dat op r; }
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
            inline auto operator~()
            {
                return ~dat;
            }
            /*Stream extraction and insertion*/
            inline ostream operator<<(ostream& os)
            {
                os << dat;
                return os;
            }
            inline istream operator>>(istream& is)
            {
                is >> dat;
                return is;
            }

        private:
            static void err(char* op)
            {
                cerr << "Warn OVERFLOW " << op << " " << typeid(T).name() << endl;
                quick_exit(2);
            }
            T dat;
        };
    }
}
#endif