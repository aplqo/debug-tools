#include "include/io.h"
#include "system.h"

namespace apdebug::System
{
    static HANDLE stdIO[3];

    Process::Process(NativeHandle v)
        : nativeHandle(v)
        , owns(false) {};
    Process::Process(Process&& other)
        : nativeHandle(other.nativeHandle)
        , owns(other.owns)
    {
        other.owns = false;
    }
    Process::~Process()
    {
        if (owns)
            CloseHandle(nativeHandle);
    }
    int Process::wait() const
    {
        WaitForSingleObject(nativeHandle, INFINITE);
        DWORD ret;
        GetExitCodeProcess(nativeHandle, &ret);
        return ret;
    }
    void Process::terminate() const
    {
        TerminateProcess(nativeHandle, 9);
        WaitForSingleObject(nativeHandle, INFINITE);
    }

    static inline void initStartInfo(STARTUPINFO& inf)
    {
        ZeroMemory(&inf, sizeof(inf));
        inf.cb = sizeof(inf);
        inf.hStdInput = stdIO[0];
        inf.hStdOutput = stdIO[1];
        inf.hStdError = stdIO[2];
    }
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
        initStartInfo(info);
        return *this;
    }
    Command& Command::instantiate()
    {
        instantiated = true;
        if (templateCmdline)
            cmdline = *templateCmdline;
        initStartInfo(info);
        return *this;
    }
    Process Command::execute()
    {
        PROCESS_INFORMATION pinfo;
        cmdline = path.data() + (" " + cmdline);
        CreateProcessA(NULL, const_cast<char*>(cmdline.data()), NULL, NULL, true, 0, NULL, NULL, &info, &pinfo);
        CloseHandle(pinfo.hThread);
        Process ret(pinfo.hProcess);
        ret.owns = true;
        return ret;
    }
    Command& Command::setRedirect(RedirectType typ, const std::filesystem::path& file)
    {
        info.dwFlags = STARTF_USESTDHANDLES;
        SECURITY_ATTRIBUTES sec {
            .nLength = sizeof(SECURITY_ATTRIBUTES),
            .lpSecurityDescriptor = nullptr,
            .bInheritHandle = true
        };
        switch (typ)
        {
        case RedirectType::StdIn:
            info.hStdInput = openFile[0] = CreateFileW(file.c_str(),
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_DELETE,
                &sec, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
            break;
        case RedirectType::StdOut:
            info.hStdOutput = openFile[1] = CreateFileW(file.c_str(),
                GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_DELETE,
                &sec, CREATE_ALWAYS,
                FILE_ATTRIBUTE_TEMPORARY, NULL);
            break;
        case RedirectType::StdErr:
            info.hStdError = openFile[2] = CreateFileW(file.c_str(),
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
        info.dwFlags = STARTF_USESTDHANDLES;
        switch (typ)
        {
        case RedirectType::StdIn:
            info.hStdInput = had;
            break;
        case RedirectType::StdOut:
            info.hStdOutput = had;
            break;
        case RedirectType::StdErr:
            info.hStdError = had;
            break;
        }
        return *this;
    }
    void Command::parseArgument(int& argc, const char* const argv[])
    {
        std::string* tmpl = new std::string;
        templateCmdline = tmpl;
        if (std::strcmp(argv[argc], "["))
        {
            tmpl->append(Output::writeToString(std::quoted<char>(argv[argc])));
            return;
        }
        unsigned int dep = 1;
        ++argc;
        for (; dep; ++argc)
        {
            if (!std::strcmp(argv[argc], "]") && !--dep)
                break;
            else if (!std::strcmp(argv[argc], "["))
                ++dep;
            *tmpl += Output::writeToString(" ", std::quoted<char>(argv[argc]));
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
        {
            if (i != INVALID_HANDLE_VALUE)
            {
                FlushFileBuffers(i);
                CloseHandle(i);
            }
            i = INVALID_HANDLE_VALUE;
        }
    }
    Command::~Command()
    {
        release();
    }

    void systemInit()
    {
        stdIO[0] = GetStdHandle(STD_INPUT_HANDLE);
        stdIO[1] = GetStdHandle(STD_OUTPUT_HANDLE);
        stdIO[2] = GetStdHandle(STD_ERROR_HANDLE);
    }
}