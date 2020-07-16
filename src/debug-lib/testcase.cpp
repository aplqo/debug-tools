#include "include/testcase.h"
#include "include/define.h"
#include "include/exception.h"
#include "include/logfile.h"
#include "include/memory.h"
#include "include/utility.h"
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <limits>
#include <memory>
#include <regex>
#include <string>
#include <thread>

namespace apdebug
{
    namespace testcase
    {
        using apdebug::memory::MemoryLimit;
        using apdebug::timer::timType;
        using std::getline;
        using std::ifstream;
        using std::numeric_limits;
        using std::regex;
        using std::regex_replace;
        using std::string;
        using std::system;
        using std::to_string;
        using std::filesystem::exists;
        using std::filesystem::path;
        using std::filesystem::remove;
        using std::regex_constants::syntax_option_type;

        void result::exec()
        {
            if (cmd.empty())
                return;
            cmd += " ";
            cmd += args;
            ret = system(cmd.c_str());
        }
        result& result::concat(const std::string& s)
        {
            cmd += " ";
            if (s.empty())
                cmd += "\"*\"";
            else
                cmd += "\"" + s + "\"";
            return *this;
        }

        const thread_local string tpoint::thrdId = utility::GetThreadId();
        limits tpoint::lim;
        MemoryLimit tpoint::memLimit;

        constexpr syntax_option_type regFlag = std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::nosubs;
        const std::regex tpoint::rin(R"(<input>)", regFlag),
            tpoint::rout(R"(<output>)", regFlag),
            tpoint::rans(R"(<answer>)", regFlag),
            tpoint::rthr(R"(<thread>)", regFlag);

        void tpoint::init()
        {
            getArgs(rres);
            getArgs(tres);
            getLog();
        }
        void tpoint::run()
        {
            m = memLimit.addProcess(thrdId);
            rres.concat(in).concat(out).concat(log).concat(m.getArg());
            rres.cmd += " " + to_string(lim.hardlim);
            rres.exec();
            mem = m.getUsage();
            if (m.isExceed())
            {
                tim = 0;
                s = new exception::HardMemoryLimit(lim.hardMemByte);
                return;
            }
        }
        void tpoint::parse()
        {
            if (s)
                return;
            using logfile::RStatus;
            ifstream lo(log, std::ios::binary | std::ios::in);
            RStatus st = RStatus::Unknown;
            lo.read(reinterpret_cast<char*>(&st), sizeof(st));
            switch (st)
            {
            case RStatus::Time:
                lo.read(reinterpret_cast<char*>(&tim), sizeof(tim));
                if (tim >= lim.lim || (lim.memLimByte && mem > lim.memLimByte))
                {
                    unsigned int t = 0;
                    if (tim >= lim.lim)
                        t |= exception::LimitExceed::Time;
                    if (lim.memLimByte && mem >= lim.memLimByte)
                        t |= exception::LimitExceed::Memory;
                    s = new exception::LimitExceed(exception::LimitExceed::type(t), tim, mem);
                }
                else
                {
                    fail = false;
                    s = new exception::Pass(tim, mem);
                }
                return;
            case RStatus::HardLimit:
                s = new exception::HardTimeLimit(lim.hardlim);
                tim = lim.hardlim;
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
            if (success() && !out.empty())
            {
                remove(out);
                out = "(Released)";
            }
            m.release();
        }
        void tpoint::getLog()
        {
            static thread_local std::unique_ptr<path, std::function<void(path*)>> fil(
                new path(thrdId + ".log"),
                [](path* s) {if (exists(*s)) remove(*s); delete s; });
            log = fil->string();
        }
        void tpoint::getArgs(result& r)
        {
            r.args = regex_replace(r.args, rin, in);
            r.args = regex_replace(r.args, rout, out);
            r.args = regex_replace(r.args, rans, ans);
            r.args = regex_replace(r.args, rthr, thrdId);
        }
        tpoint::~tpoint()
        {
            delete s;
            delete ts;
        }
        void tpoint::initMemLimit()
        {
            if (lim.hardMemByte)
                memLimit.setLimit(lim.hardMemByte);
        }
    }
}
