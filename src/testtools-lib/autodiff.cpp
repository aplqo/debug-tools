#include "include/color.h"
#include "include/testtools.h"
#include "include/utility.h"

#include <cstring>
#include <initializer_list>
#include <iomanip>
#include <iostream>

#include <fmt/core.h>

using std::strcmp;
namespace fs = std::filesystem;
namespace SGR = apdebug::Output::SGR;

namespace apdebug::TestTools
{
    AutoDiff& AutoDiff::replace(fmt::format_args args)
    {
        if (!enable)
            return *this;
        for (auto& i : file)
            i = fmt::vformat(i.c_str(), args);
        differ = fmt::vformat(differ.c_str(), args);
        return *this;
    }
    void AutoDiff::parseArgument(int& argc, const char* const argv[])
    {
        enable = true;
        ++argc;
        for (; strcmp(argv[argc], "]"); ++argc)
        {
            if (!strcmp(argv[argc], "-silent"))
                verbose = false;
            else if (!strcmp(argv[argc], "-verbose"))
                verbose = true;
            else if (!strcmp(argv[argc], "-limit"))
                size = atoll(argv[++argc]);
            else if (!strcmp(argv[argc], "-diff"))
                differ = argv[++argc];
            else if (!strcmp(argv[argc], "-files"))
                file = Utility::parseCmdArray<std::filesystem::path>(++argc, argv);
            else if (!strcmp(argv[argc], "-disable"))
                enable = false;
        }
    }
    void AutoDiff::check(System::Command& cmd)
    {
        if (!enable)
            return;
        if (!size)
        {
            if (verbose)
            {
                std::cout << SGR::TextCyan << "Autodiff: Redirect stdout to " << differ << "\n";
                std::cout << SGR::TextBlue << "Autodiff: Test command " << cmd << SGR::None << std::endl;
            }
            cmd.setRedirect(System::RedirectType::StdOut, differ.c_str());
        }
        else
        {
            const std::filesystem::path* exceed = nullptr;
            for (const auto& i : file)
                if (fs::file_size(i) > size)
                {
                    exceed = &i;
                    break;
                }
            if (verbose)
            {
                std::cout << SGR::TextCyan << "Autodiff: File size limit " << size << " byte\n";
                if (exceed)
                    std::cout << "Autodiff: File " << *exceed << " too large, redirect stdout to file\n"
                              << "Autodiff: Redirect stdout to " << differ << "\n";
                std::cout << SGR::TextBlue << "Autodiff: Test command " << cmd << SGR::None << std::endl;
            }
            if (exceed)
                cmd.setRedirect(System::RedirectType::StdOut, differ.c_str());
        }
    }
    void AutoDiff::release()
    {
        if (!enable)
            return;
        if (Utility::removeFile(differ) && verbose)
            std::cout << SGR::TextGreen << "Autodiff: Test passed removed output file." << SGR::None << "\n";
    }
}
