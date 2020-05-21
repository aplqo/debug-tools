#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "include/define.h"
#include "include/logfile.h"
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
            virtual ~state() { }
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
            apdebug::timer::timType tim = 0;
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

            static state* read(std::istream& is);

        private:
            std::string type, oper;
        };

        class RuntimeError : public state
        {
        public:
            std::string name();
            std::string color();

            static state* read(std::istream& is);
        };
        class NormalRE : public RuntimeError
        {
        public:
            NormalRE(logfile::Signal);
            std::string verbose();
            std::string details();

            static state* read(std::istream& is);

        private:
            logfile::Signal typ;
        };
        class FloatPoint : public RuntimeError
        {
        public:
            std::string verbose();
            std::string details();

            static state* read(std::istream& is);

            logfile::FPE stat = logfile::FPE::Normal;
        };
        class DivByZero : public RuntimeError
        {
        public:
            DivByZero(const std::string typ);
            std::string verbose();
            std::string details();

            static state* read(std::istream& is);

        private:
            std::string type;
        };
        class STDExcept : public RuntimeError
        {
        public:
            STDExcept(const std::string typ, const std::string des);
            std::string verbose();
            std::string details();

            static state* read(std::istream& is);

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
            std::string details();
            std::string color();

        private:
            int ret;
        };
    }
}
#endif
