#include "include/output.h"
#include "include/testcase.h"
#include <cstring>
#include <ostream>

namespace SGR = apdebug::Output::SGR;
using std::strcmp;

namespace apdebug::Testcase
{
    /*----- Result constants-----*/
    namespace ResultConstant
    {
        const char* name[13] = {
            "AC", "WA", "Pass", "TLE", "RE",
            "MLE", "Warn", "UKE", "Hard MLE", "Hard TLE",
            "Skip", "PV", "Other"
        };
        const char* color[13] = {
            SGR::TextGreen, SGR::TextRed, SGR::TextGreen, SGR::TextYellow, SGR::TextPurple,
            SGR::TextYellow, SGR::TextYellow, SGR::None, SGR::TextRed, SGR::TextRed,
            SGR::None, SGR::TextRed, SGR::None
        };
        const Result hardTLE {
            .type = Result::Type::HardTLE,
            .name = "Hard TLE",
            .color = Output::SGR::TextRed,
            .verbose = "[Hard TLE] Hard time limit exceed"
        };
        const Result hardMLE {
            .type = Result::Type::HardMLE,
            .name = "Hard MLE",
            .color = Output::SGR::TextRed,
            .verbose = "[Hard MLE] Hard memory limit exceed"
        };
        const Result TLE {
            .type = Result::Type::TLE,
            .name = "TLE",
            .color = Output::SGR::TextYellow,
            .verbose = "[TLE] Time limit exceed"
        };
        const Result MLE {
            .type = Result::Type::MLE,
            .name = "MLE",
            .color = Output::SGR::TextYellow,
            .verbose = "[MLE] Memory limit exceed"
        };
        const Result Skip {
            .type = Result::Type::Skip,
            .name = "skip",
            .color = ""
        };
        const Result Accept {
            .type = Result::Type::AC,
            .name = "AC",
            .color = Output::SGR::TextGreen,
            .verbose = "[AC] Test program return code 0"
        };
    }

    bool LimitInfo::parseArgument(int& argc, const char* const argv[])
    {
        if (!strcmp(argv[argc], "-memory"))
            memoryLimit = atoll(argv[++argc]) * 1024;
        else if (!strcmp(argv[argc], "-hmem"))
            hardMemoryLimit = atoll(argv[++argc]) * 1024;
        else if (!strcmp(argv[argc], "-time"))
            timeLimit = atoll(argv[++argc]) * 1000;
        else if (!strcmp(argv[argc], "-htime"))
            hardTimeLimit = atoll(argv[argc]) * 1000;
        else
            return false;
        return true;
    }
    std::ostream& operator<<(std::ostream& os, const LimitInfo& lim)
    {
        os << "[info] Time limit: " << lim.timeLimit / 1000.0 << " ms ( " << lim.timeLimit / 1e6 << " s )\n";
        os << "[info] Hard time limit: " << lim.hardTimeLimit / 1000.0 << " ms ( " << lim.hardTimeLimit / 1e6 << " s )\n";
        os << "[info] Memory limit: " << lim.memoryLimit / 1024.0 << " MiB ( " << lim.memoryLimit / (1024 * 1024.0) << " GiB )\n";
        os << "[info] Hard memory limit: " << lim.hardMemoryLimit / 1024.0 << " MiB ( " << lim.hardMemoryLimit / (1024.0 * 1024) << " GiB)";
        return os;
    }

    void Platform::init()
    {
        threadId = Process::GetThreadId();
        timeProtect.create();
    }
}
