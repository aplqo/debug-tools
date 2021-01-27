#include "include/logfile.h"
#include "system.h"

#include <boost/core/demangle.hpp>
#include <boost/stacktrace.hpp>

#include <cfenv>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <exception>
using namespace apdebug::System;
using namespace apdebug::Logfile;
static constexpr unsigned int maxStackDumpDepth = 40;

struct ProtectedVariable
{
    SharedMemory shm;
    TimeUsage startTime;
};
static unsigned char protectMemory[pageSize * 3];

static MemoryStream ms;

static void writeString(const std::string& s)
{
    ms.write(s.length());
    ms.write(s.data(), s.length());
}

/*-----Exported functions (don't use internal)------*/
extern "C" void writeLog(const char* obj, const size_t size) { ms.write(obj, size); }
extern "C" void writeString(const char* str)
{
    ms.write(strlen(str));
    ms.write(str, std::strlen(str));
}
extern "C" void writeName(const char* name) { writeString(boost::core::demangle(name)); }

inline static ProtectedVariable* getProtectVar()
{
    return reinterpret_cast<ProtectedVariable*>(roundToPage(&protectMemory));
}

namespace Judger
{
    static std::string findUserMain(const boost::stacktrace::stacktrace& st)
    {
        for (auto i = st.rbegin(); i != st.rend(); ++i)
            if (i->name().starts_with("_User::main"))
                return i->source_file();
        return "";
    }
    static void abortProgramImpl()
    {
        using namespace boost::stacktrace;
        char* const ptr = ms.ptr;
        ms.write(apdebug::System::eof);
        const auto st = boost::stacktrace::stacktrace();
        const size_t dumpDepth = std::min<size_t>(st.size(), maxStackDumpDepth);
        ms.write(dumpDepth);
        ms.write(st.size());
        writeString(findUserMain(st));
        for (unsigned int i = 0; i < dumpDepth; ++i)
        {
            ms.write<const void*>(st[i].address());
            writeString(st[i].source_file());
            writeString(st[i].name());
            ms.write(st[i].source_line());
        }
        ms.ptr = ptr;
        ms.write(uint32_t(0));
        std::_Exit(1);
    }
    extern "C" void abortProgram() // Don't use internal
    {
        abortProgramImpl();
    }
    static void stopWatchImpl(const uint32_t stat)
    {
        const ProtectedVariable* const pvar = getProtectVar();
        ms.ptr = pvar->shm.ptr + interactArgsSize;
        ms.write(stat);
        char* const ptr = ms.ptr;
        ms.write(TimeUsage {});
        ms.write(MemoryUsage {});
        ms.write(apdebug::System::eof);
        const auto [ct, cm] = Usage::getUsage();
        const TimeUsage t = ct - pvar->startTime;
        ms.ptr = ptr;
        ms.write(t);
        ms.write(cm);
    }
    static void inline stopWatch(const RStatus s)
    {
        stopWatchImpl(static_cast<uint32_t>(s));
    }
    extern "C" void stopWatch(const uint32_t s) // Don't use internal
    {
        stopWatchImpl(s);
    }
    static void finishProgram()
    {
        stopWatch(RStatus::Return);
    }

    static void signalHandler(int sig)
    {
        stopWatch(RStatus::RuntimeError);
        ms.write(RtError::Signal);
        switch (sig)
        {
        case SIGSEGV:
            ms.write(Signal::Sigsegv);
            break;
        case SIGINT:
            ms.write(Signal::Sigint);
            break;
        case SIGILL:
            ms.write(Signal::Sigill);
            break;
        case SIGTERM:
            ms.write(Signal::Sigterm);
            break;
        }
        abortProgramImpl();
    }
    static void fpeHandler(int)
    {
        stopWatch(RStatus::RuntimeError);
        ms.write(RtError::Sigfpe);
        uint32_t v = 0;
        if (fetestexcept(FE_DIVBYZERO))
            v |= FPE::FE_Divbyzero;
        if (fetestexcept(FE_INEXACT))
            v |= FPE::FE_Inexact;
        if (fetestexcept(FE_INVALID))
            v |= FPE::FE_Invavid;
        if (fetestexcept(FE_OVERFLOW))
            v |= FPE::FE_Overflow;
        if (fetestexcept(FE_UNDERFLOW))
            v |= FPE::FE_Underflow;
        ms.write(v);
        abortProgramImpl();
    }
    static void registerHandler()
    {
#ifdef _WIN32
        _controlfp(EM_INEXACT, _MCW_EM); // Replace EM_INEXACT with 0 to enable all exceptions.
#else
        feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW); // Add FE_INEXACT to enable all exception.
#endif
        signal(SIGSEGV, signalHandler);
        signal(SIGFPE, fpeHandler);
        signal(SIGILL, signalHandler);
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
    }
    extern "C" int judgeMain(int (*userMain)(int, const char* const[]), int argc, const char* const argv[])
    {
        ProtectedVariable* const pvar = getProtectVar();
        new (pvar) ProtectedVariable {
            { argv[argc - 1] }, {}
        };
        registerHandler();
        std::atexit(finishProgram);
        try
        {
            pvar->startTime = Usage::getTimeUsage();
            protectPage(pvar, sizeof(ProtectedVariable), false);
            userMain(argc - 1, argv);
            finishProgram();
        }
        catch (const std::exception& e)
        {
            stopWatch(RStatus::RuntimeError);
            ms.write(RtError::STDExcept);
            writeString(boost::core::demangle(typeid(e).name()));
            writeString(e.what());
            std::_Exit(1);
        }
        catch (...)
        {
            stopWatch(RStatus::RuntimeError);
            ms.write(RtError::UnknownExcept);
            std::_Exit(1);
        }
        return 0;
    }
}
namespace Interactor
{
    static Process judged;

    extern "C" void beginReportFail(const uint32_t id)
    {
        judged.terminate();
        Judger::stopWatch(static_cast<RStatus>(id));
    }
    extern "C" void endReportFail()
    {
        std::exit(1);
    }
    extern "C" void reportAccept()
    {
        judged.wait();
        ms.write(apdebug::Logfile::RStatus::Accept);
        exit(0);
    }
    extern "C" int interactorMain(int (*userMain)(int, const char* const[]), int argc, const char* const argv[])
    {
        ProtectedVariable* const pvar = getProtectVar();
        new (pvar) ProtectedVariable {
            { argv[argc - 1] }, {}
        };
        Process::Pid p;
        ms.ptr = pvar->shm.ptr;
        ms.read(p);
        judged = Process { p };
        return userMain(argc - 1, argv);
    }
}