#include "include/convert.h"
#include "system.h"

#include <filesystem>
#include <string_view>
#include <utility>

#include <fcntl.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace apdebug::System
{
    static const size_t maxArgsSize = 500;

    Command::Command(Command&& other)
        : path(other.path)
        , args(std::move(other.args))
    {
        if (&other == this)
            return;
        for (unsigned int i = 0; i < 3; ++i)
        {
            fd[i] = other.fd[i];
            other.fd[i] = -1;
        }
        other.args.clear();
    }
    Command& Command::appendArgument(const std::string_view val)
    {
        args.back() = val.data();
        args.push_back(nullptr);
        return *this;
    }
    Command& Command::instantiate(fmt::format_args args)
    {
        this->args.push_back(path.data());
        instantiated = true;
        if (templateArgs.data)
        {
            const size_t cnt = templateArgs.size;
            this->args.reserve(cnt + 1);
            for (size_t i = 1; i <= cnt; ++i)
            {
                char* buf = new char[maxArgsSize + 1];
                *fmt::vformat_to(buf, templateArgs.data[i - 1], args) = '\0';
                this->args.push_back(buf);
            }
        }
        this->args.push_back(nullptr);
        return *this;
    }
    Command& Command::instantiate()
    {
        instantiated = true;
        args.push_back(path.data());
        if (templateArgs.data)
        {
            args.reserve(10 + templateArgs.size);
            std::copy_n(templateArgs.data, templateArgs.size, std::back_inserter(args));
        }
        args.push_back(nullptr);
        return *this;
    }
    Process Command::execute() const
    {
        const pid_t p = vfork();
        if (!p)
        {
            if (fd[0] != -1)
                dup2(fd[0], STDIN_FILENO);
            if (fd[1] != -1)
                dup2(fd[1], STDOUT_FILENO);
            if (fd[2] != -1)
                dup2(fd[2], STDERR_FILENO);
            execv(args.front(), const_cast<char**>(args.data()));
            exit(1);
        }
        return Process { p };
    }
    Command& Command::setRedirect(const RedirectType which, const int fd)
    {
        switch (which)
        {
        case RedirectType::StdIn:
            this->fd[0] = fd;
            break;
        case RedirectType::StdOut:
            this->fd[1] = fd;
            break;
        case RedirectType::StdErr:
            this->fd[2] = fd;
            break;
        }
        return *this;
    }
    Command& Command::setRedirect(const RedirectType which, const fs::path& file)
    {
        setRedirect(which, open(file.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR));
        created[static_cast<unsigned int>(which)] = true;
        return *this;
    }
    void Command::parseArgument(const YAML::Node& node)
    {
        for (const auto& it : node)
        {
            if (it.first.Scalar() == "path")
                path = it.second.Scalar().c_str();
            else if (it.first.Scalar() == "argument")
                templateArgs.parseArgument(it.second, [](const char** c, const YAML::Node& node) {
                    *c = node.Scalar().c_str();
                });
        }
    }
    std::ostream& operator<<(std::ostream& os, const Command& c)
    {
        if (!c.instantiated)
        {
            os << std::quoted(c.path);
            if (c.templateArgs.data)
            {
                for (const auto i : c.templateArgs)
                    os << " " << std::quoted(i);
            }
        }
        else
        {
            for (unsigned int i = 0; i < c.args.size() - 1; ++i)
                os << " " << std::quoted(c.args[i]);
        }
        return os;
    }
    void Command::release()
    {
        for (unsigned int i = 0; i < 3; ++i)
        {
            if (created[i])
                close(fd[i]);
            created[i] = false;
        }
    }
    Command::~Command()
    {
        if (!instantiated)
            return;
        release();
        const unsigned int cnt = templateArgs.size;
        for (unsigned int i = 1; i <= cnt; ++i)
            delete[] args[i];
    }
}