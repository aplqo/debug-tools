#include "include/io.h"
#include "system.h"

#include <processthreadsapi.h>

namespace apdebug::System
{
    static HANDLE stdIO[3] = {
        GetStdHandle(STD_INPUT_HANDLE),
        GetStdHandle(STD_OUTPUT_HANDLE),
        GetStdHandle(STD_ERROR_HANDLE)
    };

    Command& Command::appendArgument(const std::string_view arg)
    {
        instantiated = true;
        cmdline += Output::writeToString(" ", std::quoted<char, std::char_traits<char>>(arg));
        return *this;
    }
    Command& Command::instantiate(fmt::format_args args)
    {
        instantiated = true;
        if (templateCmdline)
            cmdline = fmt::vformat(*templateCmdline, args);
        std::copy(stdIO, stdIO + 3, stdioHandle);
        return *this;
    }
    Command& Command::instantiate()
    {
        instantiated = true;
        if (templateCmdline)
            cmdline = *templateCmdline;
        std::copy(stdIO, stdIO + 3, stdioHandle);
        return *this;
    }
    Process Command::execute()
    {
        size_t size;
        InitializeProcThreadAttributeList(NULL, 1, 0, &size);
        STARTUPINFOEXA info {
            .StartupInfo {
                .cb = sizeof(STARTUPINFOEXA),
                .dwFlags = STARTF_USESTDHANDLES,
                .hStdInput = stdioHandle[0],
                .hStdOutput = stdioHandle[1],
                .hStdError = stdioHandle[2] },
            .lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(new unsigned char[size])
        };
        PROCESS_INFORMATION pinfo;
        cmdline = path.data() + (" " + cmdline);
        InitializeProcThreadAttributeList(info.lpAttributeList, 1, 0, &size);
        UpdateProcThreadAttribute(info.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, stdioHandle, sizeof(stdioHandle), NULL, NULL);

        CreateProcessA(
            path.data(), const_cast<char*>(cmdline.data()),
            NULL, NULL, true,
            EXTENDED_STARTUPINFO_PRESENT, NULL, NULL,
            reinterpret_cast<STARTUPINFOA*>(&info), &pinfo);

        DeleteProcThreadAttributeList(info.lpAttributeList);
        delete[] reinterpret_cast<unsigned char*>(info.lpAttributeList);
        CloseHandle(pinfo.hThread);
        return Process(pinfo.hProcess, pinfo.dwProcessId);
    }
    Command& Command::setRedirect(RedirectType typ, const std::filesystem::path& file)
    {
        SECURITY_ATTRIBUTES sec {
            .nLength = sizeof(SECURITY_ATTRIBUTES),
            .lpSecurityDescriptor = nullptr,
            .bInheritHandle = true
        };
        switch (typ)
        {
        case RedirectType::StdIn:
            stdioHandle[0] = openFile[0] = CreateFileW(file.c_str(),
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_DELETE,
                &sec, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
            break;
        case RedirectType::StdOut:
            stdioHandle[1] = openFile[1] = CreateFileW(file.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_DELETE,
                &sec, CREATE_ALWAYS,
                FILE_ATTRIBUTE_TEMPORARY, NULL);
            break;
        case RedirectType::StdErr:
            stdioHandle[2] = openFile[2] = CreateFileW(file.c_str(),
                GENERIC_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
                FILE_SHARE_READ | FILE_SHARE_DELETE,
                &sec, CREATE_ALWAYS,
                FILE_ATTRIBUTE_TEMPORARY, NULL);
            break;
        }
        return *this;
    }
    Command& Command::setRedirect(RedirectType typ, HANDLE had)
    {
        switch (typ)
        {
        case RedirectType::StdIn:
            stdioHandle[0] = had;
            break;
        case RedirectType::StdOut:
            stdioHandle[1] = had;
            break;
        case RedirectType::StdErr:
            stdioHandle[2] = had;
            break;
        }
        return *this;
    }
    void Command::parseArgument(const YAML::Node& node)
    {
        std::string* tmpl = new std::string;
        templateCmdline = tmpl;
        for (const auto& it : node)
            if (it.first.Scalar() == "path")
                path = it.second.Scalar().c_str();
            else if (it.first.Scalar() == "argument")
            {
                for (const auto& com : it.second)
                {
                    tmpl->push_back(' ');
                    if (com.IsScalar())
                        tmpl->append(Output::writeToString(std::quoted<char>(com.Scalar().c_str())));
                    else
                    {
                        const auto& v = com["val"];
                        if (com["quote"].as<bool>())
                            tmpl->append(Output::writeToString(std::quoted<char>(v.Scalar().c_str())));
                        else
                            tmpl->append(v.Scalar());
                    }
                }
            }
    }
    std::ostream& operator<<(std::ostream& os, const Command& c)
    {
        os << c.path;
        if (!c.instantiated)
        {
            if (c.templateCmdline)
                os << " " << *c.templateCmdline;
        }
        else
            os << " " << c.cmdline;
        return os;
    }
    void Command::release()
    {
        for (auto& i : openFile)
            if (i != INVALID_HANDLE_VALUE)
            {
                CloseHandle(i);
                i = INVALID_HANDLE_VALUE;
            }
    }
    Command::~Command()
    {
        release();
    }
}