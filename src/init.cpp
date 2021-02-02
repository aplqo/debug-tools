#include "include/init.h"
#include "include/io.h"
#include "include/utility.h"
#include "system.h"
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;
using namespace std::filesystem;
using namespace apdebug;
using apdebug::Input::readFileVal;
using apdebug::Output::writeFile;
const auto copyOpt = copy_options::recursive | copy_options::overwrite_existing;

namespace apdebug::init
{
    enum class Script
    {
        Batch = 0,
        Powershell = 1,
        Bash = 2
    };
    namespace PublicFiles
    {
        Script typ;

        void init(const path& c, const path& dest)
        {
            copy(c / "bin", dest / "bin", copyOpt);
            copy(c / "lib", dest / "lib", copyOpt);
#ifdef STATIC_LINK
#ifdef Linux
            copy(c / "lib" / "libjudge.so", dest, copyOpt);
#else
            copy(c / "lib" / "judge.dll", dest, copyOpt);
#endif
#endif
            copy(c / "config" / "config", dest / "config", copyOpt);
            copy(c / "debug_tools", dest / "debug_tools", copyOpt);
            copy(c / "config" / ".clang-format", dest, copyOpt);
            switch (typ)
            {
            case Script::Batch:
                copy(c / "scripts" / "batch", dest / "scripts", copyOpt);
                break;
            case Script::Powershell:
                copy(c / "scripts" / "powershell", dest / "scripts", copyOpt);
                break;
            case Script::Bash:
                copy(c / "scripts" / "shell", dest / "scripts", copyOpt);
                break;
            }
        }
        void deinit(const path& dest)
        {
            remove_all(dest / "bin");
            remove_all(dest / "debug_tools");
            remove_all(dest / "lib");
            remove_all(dest / "scripts");
            remove_all(dest / "config");
            remove(dest / ".clang-format");
#ifdef STATIC_LINK
#ifdef Linux
            remove(dest / "libjudge.so");
#else
            remove(dest / "judge.dll");
#endif
#endif
        }
        void install(const path& src, const path& dest)
        {
            writeFile(dest / ".config" / "stype", static_cast<unsigned int>(typ));
            init(src, dest);
        }
        void read()
        {
            unsigned int sc;
            do
            {
                cout << "Enter script type (0: batch, 1:powershell, 2:bash): ";
                cout.flush();
                cin >> sc;
                cout << endl;
                if (sc > 2)
                    std::cout << "Invalid select!\n";
                else
                    break;
            } while (true);
            typ = static_cast<Script>(sc);
        }
        void load(const path& dest, const Operate op)
        {
            const path config = dest / ".config";
            typ = Script(readFileVal<unsigned int>(config / "stype"));
            switch (op)
            {
            case Operate::Update:
                readCommandFile<false>(config / "pre-update", preScript());
                readCommandFile<true>(config / "post-update", postScript());
                break;
            case Operate::Init:
                readCommandFile<true>(config / "pre-init", preScript());
                readCommandFile<false>(config / "post-init", postScript());
                break;
            case Operate::Deinit:
                readCommandFile<false>(config / "pre-deinit", preScript());
                readCommandFile<true>(config / "post-deinit", postScript());
                break;
            case Operate::Uninstall:
                readCommandFile<false>(config / "pre-uninstall", preScript());
                readCommandFile<true>(config / "post-uninstall", postScript());
                break;
            }
        }
        void update(const path& src, const path& dest)
        {
            deinit(dest);
            init(src, dest);
        }
    }

    void execSeq(init::CommandList* seq)
    {
        for (const auto& i : *seq)
            i();
    }

    editor* select;
    void read()
    {
        PublicFiles::read();
        cout << "Available editors:" << endl;
        editors()->print();
        select = editors()->read();
        cout << endl;
        select->read();
    }
    void install(const path& src, const path& dest)
    {
        if (!exists(dest))
            create_directory(dest);
        create_directory(dest / ".config");
        writeFile(dest / ".config" / "src", src.string());
        writeFile(dest / ".config" / "editor", select->name);
        PublicFiles::install(src, dest);
        select->install(src, dest);
    }
    void load(const path& dest, const Operate op)
    {
        PublicFiles::load(dest, op);
        select = editors()->find(readFileLn(dest / ".config" / "editor"));
        select->load(dest, op);
    }
    void update(const path& src, const path& dest)
    {
        writeFile(dest / ".config" / "src", src.string());
        PublicFiles::update(src, dest);
        select->update(src, dest);
    }
    void uninstall(const path& dest)
    {
        select->uninstall(dest);
        PublicFiles::deinit(dest);
        remove_all(dest / ".config");
    }
    void init(const path& src, const path& dest)
    {
        writeFile(dest / ".config" / "src", src.string());
        PublicFiles::init(src, dest);
        select->init(src, dest);
    }
    void deinit(const path& dest)
    {
        select->deinit(dest);
        PublicFiles::deinit(dest);
    }
}
int main(int argc, char* argv[])
{
    apdebug::System::consoleInit();
    apdebug::Output::PrintVersion("Config installer", cout);
    void (*func1)(const path&) = nullptr;
    void (*func2)(const path&, const path&) = nullptr;
    path dest, src = canonical(path(argv[0]).parent_path());
    if (argc <= 1 || !strcmp(argv[1], "install"))
    {
        if (argc <= 1)
        {
            cout << "Enter destination: ";
            cout.flush();
            cin >> dest;
            cout << endl;
        }
        else
            dest = argv[2];
        init::read();
        if (!exists(dest))
            create_directory(dest);
        dest = canonical(dest);
        func2 = init::install;
    }
    else
    {
        dest = canonical(argv[2]);
        init::Operate op;
        if (!strcmp(argv[1], "init"))
        {
            op = init::Operate::Init;
            func2 = init::init;
        }
        else if (!strcmp(argv[1], "update"))
        {
            op = init::Operate::Update;
            func2 = init::update;
        }
        else if (!strcmp(argv[1], "deinit"))
        {
            op = init::Operate::Deinit;
            func1 = init::deinit;
        }
        else if (!strcmp(argv[1], "uninstall"))
        {
            op = init::Operate::Uninstall;
            func1 = init::uninstall;
        }
        init::load(dest, op);
    }
    current_path(dest);
    init::execSeq(init::preScript());
    if (func1)
        func1(dest);
    else
        func2(src, dest);
    init::execSeq(init::postScript());
    return 0;
}
