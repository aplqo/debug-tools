#include "include/init.h"
#include "include/output.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
using namespace apdebug::init;
using namespace std;
using namespace std::filesystem;
const auto copyOpt = copy_options::recursive | copy_options::overwrite_existing;

enum class Script
{
    Batch = 0,
    Powershell = 1,
    Bash = 2
};
namespace PublicFiles
{
    void initPublic(const path& dest, Script sc)
    {
        const path c = current_path();
        copy(c / "bin", dest / "bin", copyOpt);
        copy(c / "debug_tools", dest / "debug_tools", copyOpt);
        copy(c / "config" / ".clang-format", dest, copyOpt);
        switch (sc)
        {
        case Script::Batch:
            copy(c / "scripts" / "batch", dest / "scripts", copyOpt);
            break;
        case Script::Powershell:
            copy(c / "scripts" / "powershell", dest / "scripts", copyOpt);
            break;
        case Script::Bash:
            copy(c / "scripts" / "bash", dest / "scripts", copyOpt);
            break;
        }
    }
    void init(const path& dest, Script sc)
    {
        create_directory(dest / ".config");
        writeFile(dest / ".config" / "src", current_path().string());
        writeFile(dest / ".config" / "src", static_cast<unsigned int>(sc));
        initPublic(dest, sc);
    }
    void update(const path& dest)
    {
        initPublic(dest, Script(readFileVal<unsigned int>(dest / ".config" / "stype")));
    }
    void deinit(const path& dest)
    {
        remove_all(dest / "bin");
        remove_all(dest / "debug_tools");
        remove_all(dest / "scripts");
        remove_all(dest / ".config");
        remove(dest / ".clang-format");
    }
}

void install(const path& dest)
{
    if (!exists(dest))
        create_directory(dest);
    unsigned int sc;
    editor* e;
    compiler* c;
    {
        cout << "Enter script type (0: batch, 1:powershell, 2:bash): ";
        cout.flush();
        cin >> sc;
        cout << endl;
    }
    {
        cout << "Available editors:" << endl;
        editors()->print();
        e = editors()->read();
    }
    {
        cout << "Available compilers" << endl;
        e->print();
        c = e->read();
    }
    PublicFiles::init(dest, static_cast<Script>(sc));
    writeFile(dest / ".config" / "editor", e->name);
    e->init(dest, c);
}
void update(const path& dest)
{
    editor* e = editors()->find(readFileLn(dest / ".config" / "editor"));
    PublicFiles::update(dest);
    e->update(dest);
}
void uninatll(const path& dest)
{
    editor* e = editors()->find(readFileLn(dest / ".config" / "editor"));
    e->deinit(dest);
    PublicFiles::deinit(dest);
}

int main(int argc, char* argv[])
{
    apdebug::out::PrintVersion("Config installer", cout);
    path d;
    if (argc <= 1 || !strcmp(argv[1], "install"))
    {
        if (argc <= 1)
        {
            cout << "Enter destination: ";
            cout.flush();
            cin >> d;
            cout << endl;
        }
        else
            d = argv[2];
        install(d);
    }
    else if (!strcmp(argv[1], "update"))
        update(argv[2]);
    else if (!strcmp(argv[1], "deinit"))
        uninatll(argv[1]);
    return 0;
}