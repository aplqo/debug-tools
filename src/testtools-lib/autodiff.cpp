#include "include/color.h"
#include "include/pathFormat.h"
#include "include/testtools.h"
#include "include/utility.h"

#include <cstring>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <unordered_map>

#include <fmt/core.h>

using std::strcmp;
namespace fs = std::filesystem;
namespace Escape = apdebug::Output::Escape;

namespace apdebug::TestTools
{
    namespace
    {
        enum class Param
        {
            Verbose,
            Limit,
            Diff,
            File
        };
        static const std::unordered_map<std::string, Param> par {
            { "verbose", Param::Verbose },
            { "limit", Param::Limit },
            { "differ", Param::Diff },
            { "file", Param::File }
        };
    }

    AutoDiff& AutoDiff::instantiate(fmt::format_args args)
    {
        if (!enable)
            return *this;
        using namespace fmt::literals;
        if (fileTemplate.size)
        {
            file.allocate(fileTemplate.size);
            for (unsigned int i = 0; i < fileTemplate.size; ++i)
                new (file.data + i) fs::path(fmt::vformat(fileTemplate.data[i], args));
        }
        if (differTemplate)
            differ = fmt::vformat(differTemplate, args);
        return *this;
    }
    AutoDiff& AutoDiff::instantiate()
    {
        if (fileTemplate.size)
        {
            file.allocate(fileTemplate.size);
            for (unsigned int i = 0; i < fileTemplate.size; ++i)
                new (file.data + i) fs::path(fileTemplate.data[i]);
        }
        if (differTemplate)
            differ = differTemplate;
        return *this;
    }
    void AutoDiff::parseArgument(const YAML::Node& nod)
    {
        if (nod.IsNull())
        {
            enable = false;
            return;
        }
        enable = true;
        for (const auto& it : nod)
            switch (par.at(it.first.Scalar()))
            {
            case Param::Verbose:
                verbose = it.second.as<bool>();
                break;
            case Param::Limit:
                size = it.second.as<size_t>();
                break;
            case Param::Diff:
                differTemplate = it.second.Scalar().c_str();
                break;
            case Param::File:
                fileTemplate.parseArgument(it.second, [](const char** dest, const YAML::Node& node) {
                    *dest = node.Scalar().c_str();
                });
                break;
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
    AutoDiff::~AutoDiff()
    {
        file.release();
    }
}
