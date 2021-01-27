#include "include/color.h"
#include "include/pathFormat.h"
#include "include/testtools.h"
#include "include/utility.h"

#include <cstring>
#include <initializer_list>
#include <iomanip>
#include <iostream>

#include <fmt/core.h>

using std::strcmp;
namespace fs = std::filesystem;
namespace Escape = apdebug::Output::Escape;

namespace apdebug::TestTools
{

    AutoDiff& AutoDiff::instantiate(fmt::format_args args)
    {
        if (!enable)
            return *this;
        using namespace fmt::literals;
        if (fileTemplate)
        {
            file.reserve(fileTemplate->size());
            for (unsigned int i = 0; i < fileTemplate->size(); ++i)
                file.emplace_back(fmt::vformat(fileTemplate->at(i), args));
        }
        if (differTemplate)
            differ = fmt::vformat(differTemplate, args);
        return *this;
    }
    AutoDiff& AutoDiff::instantiate()
    {
        if (fileTemplate)
        {
            file.reserve(fileTemplate->size());
            for (unsigned int i = 0; i < fileTemplate->size(); ++i)
                file.emplace_back(fileTemplate->at(i));
        }
        if (differTemplate)
            differ = differTemplate;
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
                differTemplate = argv[++argc];
            else if (!strcmp(argv[argc], "-files"))
                fileTemplate = new std::vector(Utility::parseCmdArray<const char*>(++argc, argv));
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
                std::cout << Escape::TextCyan << "Autodiff: Redirect stdout to " << differ << "\n";
                std::cout << Escape::TextBlue << "Autodiff: Test command " << cmd << Escape::None << std::endl;
            }
            cmd.setRedirect(System::RedirectType::StdOut, differ);
            redirect = true;
        }
        else
        {
            const fs::path* exceed = nullptr;
            for (const auto& i : file)
            {
                std::error_code ec;
                if (fs::file_size(i, ec) > size && !ec)
                {
                    exceed = &i;
                    break;
                }
            }
            if (verbose)
            {
                std::cout << Escape::TextCyan << "Autodiff: File size limit " << size << " byte\n";
                if (exceed)
                    std::cout << "Autodiff: File " << *exceed << " too large, redirect stdout to file\n"
                              << "Autodiff: Redirect stdout to " << differ << "\n";
                std::cout << Escape::TextBlue << "Autodiff: Test command " << cmd << Escape::None << std::endl;
            }
            redirect = exceed;
            if (exceed)
                cmd.setRedirect(System::RedirectType::StdOut, differ);
        }
        if (!redirect)
            differ = "<unused>";
    }
    void AutoDiff::release()
    {
        if (!enable)
            return;
        if (redirect)
        {
            if (Utility::removeFile(differ) && verbose)
                std::cout << Escape::TextGreen << "Autodiff: Test passed removed output file." << Escape::None << "\n";
        }
        else
            differ = "<unused>";
    }
}
