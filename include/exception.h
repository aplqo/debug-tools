#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "include/define.h"
#include <string>

namespace apdebug
{
    namespace exception
    {
        class state
        {
        public:
            virtual std::string name() = 0;
            virtual std::string verbose() = 0;
            virtual std::string details() { return ""; }
            virtual std::string color() = 0;
        };

        class Pass : public state
        {
        public:
            Pass() = default;
            Pass(apdebug::timer::timType t)
                : tim(t) {};
            std::string name();
            std::string verbose();
            std::string color();

        private:
            apdebug::timer::timType tim;
        };
        class Accepted : public state
        {
        public:
            std::string name();
            std::string verbose();
            std::string color();
        };
        class WrongAnswer : public state
        {
        public:
            WrongAnswer(int);
            std::string name();
            std::string verbose();
            std::string color();

        private:
            int ret;
        };

        class TimeLimit : public state
        {
        public:
            TimeLimit(apdebug::timer::timType);
            std::string name();
            std::string verbose();
            std::string color();

        private:
            Pass p;
        };
        class HardLimit : public state
        {
        public:
            HardLimit(apdebug::timer::timType);
            std::string name();
            std::string verbose();
            std::string color();

        private:
            apdebug::timer::timType hardlim;
        };

        class Warn : public state
        {
        public:
            Warn(const std::string typ, const std::string op);
            std::string name();
            std::string verbose();
            std::string details();
            std::string color();

        private:
            std::string type, oper;
        };

        class RuntimeError : public state
        {
        public:
            std::string name();
            std::string color();
        };
        class NormalRE : public RuntimeError
        {
        public:
            NormalRE(int);
            std::string verbose();
            std::string details();

        private:
            int typ;
        };
        class FloatPoint : public RuntimeError
        {
        public:
            std::string verbose();
            std::string details();

            enum fexcept
            {
                Normal = 0,
                FE_DIVBYZREO = 1,
                FE_INEXACT = 1 << 1,
                FE_INVALID = 1 << 2,
                FE_OVERFLOW = 1 << 3,
                FE_UNDERFLOW = 1 << 4
            };
            uint32_t stat = 0;
        };
        class DivByZero : public RuntimeError
        {
        public:
            DivByZero(const std::string typ);
            std::string verbose();
            std::string details();

        private:
            std::string type;
        };
        class STDExcept : public RuntimeError
        {
        public:
            STDExcept(const std::string typ, const std::string des);
            std::string verbose();
            std::string details();

        private:
            std::string type, what;
        };
        class UnknownExcept : public RuntimeError
        {
        public:
            std::string verbose();
            std::string details();
        };
        class Unknown : public state
        {
        public:
            Unknown(int r)
                : ret(r)
            {
            }
            std::string name();
            std::string verbose();
            std::string color();

        private:
            int ret;
        };
    }
}
#endif