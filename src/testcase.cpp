#include "include/testcase.h"
#include "include/define.h"
#include "include/exception.h"
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <string>

namespace apdebug
{
    namespace testcase
    {
        using apdebug::timer::timType;
        using std::getline;
        using std::ifstream;
        using std::numeric_limits;
        using std::string;
        using std::system;
        using std::to_string;

        void result::exec()
        {
            if (cmd.empty())
                return;
            cmd += " ";
            cmd += args;
            ret = system(cmd.c_str());
        }

        timType tpoint::lim=1000*1000;
        timType tpoint::hardlim = 1000 * 1000 * 1000;
        void tpoint::run()
        {
            concat(in);
            concat(out);
            concat(log);
            rres.cmd += " " + to_string(lim);
            rres.cmd += " " + to_string(hardlim);
            rres.exec();
        }
        void tpoint::parse()
        {
            ifstream lo(log);
            while (lo)
            {
                string str;
                lo >> str;
                if (str == "Time")
                {
                    lo >> tim;
                    if (tim >= lim)
                        s = new exception::TimeLimit(lim);
                    return;
                }
                if (str == "Hlim")
                {
                    s = new exception::HardLimit(hardlim);
                    return;
                }
                if (str == "Ret")
                    return;
                if (str == "Warn")
                {
                    string op, typ;
                    lo >> op >> typ;
                    s = new exception::Warn(typ.c_str(), op.c_str());
                    return;
                }
                if (str == "RE")
                {
                    string typ;
                    lo >> typ;
                    if (typ == "STDException")
                    {
                        string what, typ;
                        lo >> typ;
                        lo.ignore(numeric_limits<std::streamsize>::max(), '\n');
                        while (lo)
                        {
                            string t;
                            getline(lo, t);
                            what += t + '\n';
                        }
                        s = new exception::STDExcept(typ.c_str(), what.c_str());
                        return;
                    }
                    if (typ == "UnknownException")
                    {
                        s = new exception::UnknownExcept;
                        return;
                    }
                    if (typ == "SIGFPE")
                    {
                        exception::FloatPoint* f = new exception::FloatPoint();
                        while (lo)
                        {
                            string st;
                            lo >> st;
                            if (st == "FE_DIVBYZERO")
                                f->stat |= exception::FloatPoint::FE_DIVBYZREO;
                            else if (st == "FE_INEXACT")
                                f->stat |= exception::FloatPoint::FE_INEXACT;
                            else if (st == "FE_INVALID")
                                f->stat |= exception::FloatPoint::FE_INVALID;
                            else if (st == "FE_OVERFLOW")
                                f->stat |= exception::FloatPoint::FE_OVERFLOW;
                            else if (st == "FE_UNDERFLOW")
                                f->stat |= exception::FloatPoint::FE_UNDERFLOW;
                        }
                        s = f;
                        return;
                    }
                    if (typ == "DIV")
                    {
                        string typ;
                        lo >> typ;
                        s = new exception::DivByZero(typ.c_str());
                        return;
                    }
                    {
                        if (typ == "SIGTERM")
                            s = new exception::NormalRE(SIGTERM);
                        else if (typ == "SIGINT")
                            s = new exception::NormalRE(SIGINT);
                        else if (typ == "SIGILL")
                            s = new exception::NormalRE(SIGILL);
                        else if (typ == "SIGSEGV")
                            s = new exception::NormalRE(SIGSEGV);
                    }
                }
            }
            s = new exception::Pass;
        }
        void tpoint::test()
        {
            if (rres.ret)
                return;
            tres.exec();
            delete s;
            if (tres.ret)
                s = new exception::WrongAnswer(tres.ret);
            else
                s = new exception::Accepted;
        }
        void tpoint::concat(string& s)
        {
            rres.cmd += " ";
            if (s.empty())
                rres.cmd += "\"\"";
            else
                rres.cmd += s;
        }
        tpoint::~tpoint()
        {
            delete s;
        }
    }
}