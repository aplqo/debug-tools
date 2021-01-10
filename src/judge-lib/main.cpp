#include "include/logfile.h"
#include "process.h"

#include <boost/core/demangle.hpp>
#include <boost/stacktrace.hpp>

#include <cfenv>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <exception>
using namespace apdebug::Process;
using namespace apdebug::Logfile;
static const unsigned int maxStackDumpDepth = 30;

SharedMemory* shm;
MemoryStream ms;
TimeUsage startTime;

void writeString(const std::string& s)
{
    ms.write(s.length());
    ms.write(s.data(), s.length());
}
extern "C" void writeLog(const char* obj, const size_t size)
{
    ms.write(obj, size);
}
extern "C" void writeString(const char* str)
{
    ms.write(strlen(str));
    ms.write(str, std::strlen(str));
}
extern "C" void writeName(const char* name)
{
    writeString(boost::core::demangle(name));
}
extern "C" void abortProgram(const unsigned int dep)
{
    using namespace boost::stacktrace;
    const auto st = boost::stacktrace::stacktrace();
    const size_t dumpDepth = std::min<size_t>(st.size(), maxStackDumpDepth + dep);
    ms.write(dumpDepth);
    ms.write(st.size());
    ms.write(dep);
    for (unsigned int i = 0; i < dumpDepth; ++i)
    {
        ms.write<const void*>(st[i].address());
        writeString(st[i].source_file());
        writeString(st[i].name());
        ms.write(st[i].source_line());
    }
    shm->~SharedMemory();
    quick_exit(1);
}
void finishProgram()
{
    const TimeUsage t = getTimeUsage() - startTime;
    const MemoryUsage m = getMemoryUsage();
    ms.write(RStatus::Return);
    ms.write(t);
    ms.write(m);
    delete shm;
}

void signalHandler(int sig)
{
    ms.write(RStatus::RuntimeError);
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
    abortProgram(2);
}
void fpeHandler(int)
{
    ms.write(RStatus::RuntimeError);
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
    abortProgram(2);
}
void registerHandler()
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

extern "C" int judgeMain(int*(userMain)(int, const char* const[]), int argc, const char* const argv[])
{
    shm = new SharedMemory(argv[argc - 1]);
    ms.ptr = reinterpret_cast<char*>(shm->ptr);
    registerHandler();
    std::atexit(finishProgram);
    try
    {
        startTime = getTimeUsage();
        userMain(argc - 1, argv);
        finishProgram();
    }
    catch (const std::exception& e)
    {
        ms.write(RStatus::RuntimeError);
        ms.write(RtError::STDExcept);
        writeName(typeid(e).name());
        writeString(e.what());
        delete shm;
        quick_exit(1);
    }
    catch (...)
    {
        ms.write(RStatus::RuntimeError);
        ms.write(RtError::UnknownExcept);
        delete shm;
        quick_exit(1);
    }
    return 0;
}