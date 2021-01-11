#include "include/logfile.h"
#include "include/output.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <string>
#include <vector>

#include <fmt/compile.h>
#include <fmt/format.h>

using apdebug::Process::MemoryStream;
using apdebug::Testcase::Result;

namespace apdebug::Logfile
{
    void readCString(MemoryStream& ms, char* dest)
    {
        size_t len;
        ms.read(len);
        ms.read(dest, len);
        dest[len] = '\0';
    }
    std::string readString(MemoryStream& ms)
    {
        size_t len;
        ms.read(len);
        std::string ret;
        ret.resize(len);
        ms.read(ret.data(), len);
        return ret;
    }
    struct Frame
    {
        std::string name, file;
        size_t line;
        void* address;
    };
    struct Stacktrace
    {
        std::vector<Frame> frames;
        size_t totalDepth, maxName, maxFile;
        unsigned int formatLength, userFrameDep;

        void print(std::string& dest) const;
        void parse(MemoryStream& ms);
    };
    void Stacktrace::parse(MemoryStream& ms)
    {
        size_t dump, dep;
        ms.read(dump);
        ms.read(totalDepth);
        ms.read(userFrameDep);
        maxName = maxFile = 0;
        for (unsigned int i = 0; i < dump; ++i)
        {
            void* addr;
            size_t line;
            ms.read(addr);
            std::string &&file = readString(ms), &&name = readString(ms);
            maxFile = std::max(maxFile, file.size());
            maxName = std::max(maxName, name.size());
            ms.read(line);
            frames.push_back(
                Frame { .name = name, .file = file, .line = line, .address = addr });
        }
        formatLength = frames.size() * (maxName + maxFile + 50) + 60;
    }
    void Stacktrace::print(std::string& dest) const
    {
        fmt::format_to(std::back_inserter(dest),
            "\tStack backtrace: (dumped depth {} total depth {})", frames.size(), totalDepth);
        const int idLen = ceil(log10(frames.size() + 1)), nameLen = maxName;
        for (unsigned int i = 0; i < frames.size(); ++i)
            fmt::format_to(std::back_inserter(dest),
                "\n\t\t#{0:{5}} {1} in {2:{6}} {3}:{4}",
                i, frames[i].address, frames[i].name, frames[i].file, frames[i].line, idLen, nameLen);
    }

    const Result* parseWarn(MemoryStream& ms, Result*& cur)
    {
        Warning w;
        ms.read(w);
        const std::string type = readString(ms), operate = readString(ms);
        *cur = Result {
            .type = Result::Type::Warn,
            .name = "Warn",
            .color = Output::SGR::TextYellow
        };
        Stacktrace st;
        st.parse(ms);
        cur->verbose.reserve(st.formatLength + 60);
        fmt::format_to(std::back_inserter(cur->verbose),
            "[Warn] Interger overflow on operate {} of {}.", operate, type);
        cur->verbose.push_back('\n');
        st.print(cur->verbose);
        cur->details = fmt::format("Interger overflow on {} of {} at {}", operate, type, st.frames[st.userFrameDep]);
        return cur++;
    }

    const static Result UnknownExcept {
        .type = Result::Type::RE,
        .name = "RE",
        .color = Output::SGR::TextPurple,
        .verbose = "[RE] Throw an unknown exception",
        .details = "Throw an unknown exception"
    };
    const Result* parseFPE(MemoryStream& ms, Result*& cur)
    {
        FPE stat;
        ms.read(stat);
        Stacktrace st;
        st.parse(ms);
        cur->type = Result::Type::RE;
        cur->name = "RE";
        cur->color = Output::SGR::TextPurple;
        cur->verbose.reserve(300 + st.formatLength);
        cur->verbose = "[RE] Received SIGFPE  erroneous arithmetic operation such as divide by zero.";
        cur->details.reserve(300 + st.maxFile + st.maxName + 60);
        cur->details = "Received SIGFPE";
        if (stat != Normal)
            cur->details += " (Float point exception:";
        if (stat & FPE::FE_Divbyzero)
        {
            cur->verbose += "\n\tFE_DIVBYZERO: pole error occurred in an earlier floating-point operation.";
            cur->details += " FE_DIVBYZERO";
        }
        if (stat & FPE::FE_Inexact)
        {
            cur->verbose += "\n\tFE_INEXACT: inexact result: rounding was necessary to store the result of an earlier floating-point operation.";
            cur->details += " FE_INEXACT";
        }
        if (stat & FPE::FE_Invavid)
        {
            cur->verbose += "\n\tFE_INVALID: domain error occurred in an earlier floating-point operation";
            cur->details += " FE_INVALID";
        }
        if (stat & FPE::FE_Overflow)
        {
            cur->verbose += "\n\tFE_OVERFLOW: the result of the earlier floating-point operation was too large to be representable";
            cur->details += " FE_OVERFLOW";
        }
        if (stat & FPE::FE_Underflow)
        {
            cur->verbose += "\n\tFE_UNDERFLOW: the result of the earlier floating-point operation was subnormal with a loss of precision";
            cur->details += " FE_UNDERFLOW";
        }
        if (stat != Normal)
            cur->details += ")";
        fmt::format_to(std::back_inserter(cur->details), " at {}", st.frames[st.userFrameDep]);
        cur->verbose.push_back('\n');
        st.print(cur->verbose);
        return cur++;
    }
    const Result* parseDivbyzero(MemoryStream& ms, Result*& cur)
    {
        const std::string type = readString(ms);
        Stacktrace st;
        st.parse(ms);
        cur->type = Result::Type::RE;
        cur->name = "RE";
        cur->color = Output::SGR::TextPurple;
        cur->details = fmt::format("Divide by zero on {} at {}", type, st.frames[st.userFrameDep]);
        cur->verbose.reserve(st.formatLength + 60);
        cur->verbose = "[RE] Divide by zero on" + type;
        cur->verbose.push_back('\n');
        st.print(cur->verbose);
        return cur++;
    }

    const Result* parseSignal(MemoryStream& ms, Result*& cur)
    {
        Signal sig;
        ms.read(sig);
        Stacktrace st;
        st.parse(ms);
        cur->type = Result::Type::RE;
        cur->name = "RE";
        cur->color = Output::SGR::TextPurple;
        cur->verbose.reserve(60 + st.formatLength);
        cur->verbose = "[RE] Received SIG";
        cur->details.reserve(60 + st.maxFile + st.maxName + 30);
        cur->details = "Received SIG";
        switch (sig)
        {
        case Signal::Sigterm:
            cur->verbose += "TERM: termination request, sent to the program";
            cur->details += "TERM";
            break;
        case Signal::Sigint:
            cur->verbose += "INT: external interrupt, usually initiated by the user";
            cur->details += "INT";
            break;
        case Signal::Sigill:
            cur->verbose += "ILL: invalid program image, such as invalid instruction";
            cur->details += "ILL";
            break;
        case Signal::Sigsegv:
            cur->verbose += "SEGV: invalid memory access (segmentation fault)";
            cur->details += "SEGV";
            break;
        }
        cur->verbose.push_back('\n');
        st.print(cur->verbose);
        fmt::format_to(std::back_inserter(cur->details), " at {}", st.frames[st.userFrameDep]);
        return cur++;
    }

    const Result* parseRE(MemoryStream& ms, Result*& cur)
    {
        RtError typ;
        ms.read(typ);
        switch (typ)
        {
        case RtError::STDExcept:
        {
            const std::string name = readString(ms), what = readString(ms);
            *cur = Result {
                .type = Result::Type::RE,
                .name = "RE",
                .color = Output::SGR::TextPurple,
                .verbose = fmt::format("[RE] Throw an exception of {} \n\t what(): {}", name, what),
                .details = "Throw an exception of " + name
            };
            return cur++;
        }
        case RtError::UnknownExcept:
            return &UnknownExcept;
        case RtError::Sigfpe:
            return parseFPE(ms, cur);
        case RtError::DivByZero:
            return parseDivbyzero(ms, cur);
        case RtError::Signal:
            return parseSignal(ms, cur);
        }
    }
} // namespace apdebug::Logfile

using apdebug::Logfile::Frame;
template <>
struct fmt::formatter<Frame>
{
    constexpr auto parse(fmt::format_parse_context& c)
    {
        return c.begin();
    }
    template <class FormatContext>
    auto format(const Frame& f, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), FMT_COMPILE("{} {} {}:{}"), f.address, f.name, f.file, f.line);
    }
};