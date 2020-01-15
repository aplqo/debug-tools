#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "include/define.h"

namespace apdebug
{
    namespace exception
    {
        class state
        {
        public:
            virtual void name() = 0;
            virtual void verbose() = 0;
            virtual void details(){};
            virtual void color() = 0;
            ~state();
        };

        class Pass : public state
        {
        public:
            Pass() = default;
            Pass(apdebug::timer::timType t)
                : tim(t){};
            void name();
            void verbose();
            void color();

        private:
            apdebug::timer::timType tim;
        };
        class Accepted : public state
        {
        public:
            void name();
            void verbose();
            void color();
        };
        class WrongAnswer : public state
        {
        public:
            WrongAnswer(int);
            void name();
            void verbose();
            void color();

        private:
            int ret;
        };

        class TimeLimit : public state
        {
        public:
            TimeLimit(apdebug::timer::timType);
            void name();
            void verbose();
            void color();

        private:
            Pass p;
        };
        class HardLimit : public state
        {
        public:
            HardLimit(apdebug::timer::timType);
            void name();
            void verbose();
            void color();

        private:
            apdebug::timer::timType hardlim;
        };

        class Warn : public state
        {
        public:
            Warn(const char* typ, const char* op);
            void name();
            void verbose();
            void details();
            void color();

        private:
            const char *type, *oper;
        };

        class RuntimeError : public state
        {
        public:
            void name();
            void color();
        };
        class NormalRE : public RuntimeError
        {
        public:
            NormalRE(int);
            void verbose();
            void details();

        private:
            int typ;
        };
        class FloatPoint : public RuntimeError
        {
        public:
            void verbose();
            void details();

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
            DivByZero(const char* typ);
            void verbose();
            void details();

        private:
            const char* type;
        };
        class STDExcept : public RuntimeError
        {
        public:
            STDExcept(const char* typ, const char* des);
            void verbose();
            void details();

        private:
            const char *type, *what;
        };
        class UnknownExcept : public RuntimeError
        {
        public:
            void verbose();
            void details();
        };
    }
}
#endif