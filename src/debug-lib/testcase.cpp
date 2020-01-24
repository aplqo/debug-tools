#include "include/testcase.h"
#include "include/define.h"
#include "include/exception.h"
#include <csignal>
#include <cstdlib>
#include <filesystem>
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
        using std::filesystem::path;
        using std::filesystem::remove;

        void result::exec()
        {
            if (cmd.empty())
                return;
            cmd += " ";
            cmd += args;
            ret = system(cmd.c_str());
        }

        timType tpoint::lim = 1000 * 1000;
        timType tpoint::hardlim = 1000 * 10 * 1000;
        void tpoint::run()
        {
            getLog();
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
                        s = new exception::TimeLimit(tim);
                    else
                    {
                        fail = false;
                        s = new exception::Pass(tim);
                    }
                    return;
                }
                if (str == "Hlim")
                {
                    s = new exception::HardLimit(hardlim);
                    tim = hardlim;
                    return;
                }
                if (str == "Ret")
                    return;
                if (str == "Warn")
                {
                    string op, typ;
                    lo >> op >> op >> typ;
                    s = new exception::Warn(typ, op);
                    tim = 0;
                    return;
                }
                if (str == "RE")
                {
                    tim = 0;
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
                        s = new exception::STDExcept(typ, what);
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
                        s = new exception::DivByZero(typ);
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
                        return;
                    }
                }
            }
            s = new exception::Unknown;
        }
        void tpoint::test()
        {
            tres.exec();
            if (tres.ret)
                ts = new exception::WrongAnswer(tres.ret);
            else
                ts = new exception::Accepted;
        }
        bool tpoint::success()
        {
            return !(rres.ret || tres.ret);
        }
        void tpoint::release()
        {
            remove(log);
            if (success() && !out.empty())
            {
                remove(out);
                out = "(Released)";
            }
        }
        void tpoint::getLog() 
        {
            path p(rres.cmd);
            p.replace_extension(".log");
            log = p.string();
        }
        void tpoint::concat(string& s)
        {
            rres.cmd += " ";
            if (s.empty())
                rres.cmd += "\"\"";
            else
                rres.cmd += "\"" + s + "\"";
        }
        tpoint::~tpoint()
        {
            delete s;
        }
    }
}