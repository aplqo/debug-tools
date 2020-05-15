#include "include/testcase.h"
#include "include/define.h"
#include "include/exception.h"
#include "include/logfile.h"
#include "include/utility.h"
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <regex>
#include <string>
#include <thread>

namespace apdebug
{
    namespace testcase
    {
        using apdebug::timer::timType;
        using std::getline;
        using std::ifstream;
        using std::numeric_limits;
        using std::regex;
        using std::regex_replace;
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
        void tpoint::init()
        {
            getArgs(rres);
            getArgs(tres);
            getLog();
        }
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
            using logfile::RStatus;
            ifstream lo(log, std::ios::binary | std::ios::in);
            RStatus st = RStatus::Unknown;
            lo.read(reinterpret_cast<char*>(&st), sizeof(st));
            switch (st)
            {
            case RStatus::Time:
                lo.read(reinterpret_cast<char*>(&tim), sizeof(tim));
                if (tim >= lim)
                    s = new exception::TimeLimit(tim);
                else
                {
                    fail = false;
                    s = new exception::Pass(tim);
                }
                return;
            case RStatus::HardLimit:
                s = new exception::HardLimit(hardlim);
                tim = hardlim;
                return;
            case RStatus::Return:
                return;
            case RStatus::Warn:
                s = exception::Warn::read(lo);
                return;
            case RStatus::Runtime:
                s = exception::RuntimeError::read(lo);
                return;
            default:
                s = new exception::Unknown(rres.ret);
                return;
            }
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
            p = p.stem().concat("-" + utility::GetThreadId());
            p.replace_extension(".log");
            log = p.string();
        }
        void tpoint::getArgs(result& r)
        {
            r.args = regex_replace(r.args, regex(R"(\[input\])"), in);
            r.args = regex_replace(r.args, regex(R"(\[output\])"), out);
            r.args = regex_replace(r.args, regex(R"(\[answer\])"), ans);
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
