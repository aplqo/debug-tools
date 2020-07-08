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
            virtual ~state() {}
        };

        class Pass : public state
        {
        public:
            Pass() = default;
            Pass(apdebug::timer::timType t, size_t m)
                : tim(t)
                , mem(m) {};
            std::string name();
            std::string verbose();
            std::string color();

        private:
            size_t mem;
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
        class JudgeFail : public state
        {
        public:
            JudgeFail(const char* vstr = "", const char* dstr = "")
                : dmsg(dstr)
                , vmsg(vstr) {};
            std::string name();
            std::string verbose();
            std::string color();
            std::string details();

        private:
            const char *dmsg, *vmsg;
        };

        class LimitExceed : public state
        {
        public:
            enum type
            {
                Time = 1,
                Memory = 2
            };

            LimitExceed(type typ, apdebug::timer::timType t, size_t m)
                : typ(typ)
                , p(t, m) {};
            std::string name();
            std::string verbose();
            std::string color();

        private:
            type typ;
            Pass p;
        };
        class HardTimeLimit : public state
        {
        public:
            HardTimeLimit(apdebug::timer::timType);
            std::string name();
            std::string verbose();
            std::string color();

        private:
            apdebug::timer::timType hardlim;
        };
        class HardMemoryLimit : public state
        {
        public:
            HardMemoryLimit(size_t m)
                : mem(m) {};
            std::string name();
            std::string verbose();
            std::string color();

        private:
            size_t mem;
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
