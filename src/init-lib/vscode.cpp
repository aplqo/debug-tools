#include "include/init.h"
#include <filesystem>
#include <fstream>
#include <memory>

namespace fs = std::filesystem;

namespace apdebug::init::vscode
{

    class vscode : public editor
    {
    public:
        vscode()
            : editor("vscode", "Visual Studio Code")
        {
            editors()->append(this);
        }

        static vscode* instance()
        {
            static std::unique_ptr<vscode> obj(new vscode);
            return obj.get();
        }

    private:
        void initImpl(const fs::path& p, const bool upd = false)
        {
            fs::create_directory(p / ".vscode");
        }
        void deinitImpl(const fs::path& dest, const bool)
        {
            fs::remove_all(dest / ".vscode");
        }
    };
    class DirectCopy : public compiler
    {
    public:
        DirectCopy(const char* name, const char* descript, const fs::path confPath)
            : compiler(name, descript)
            , confPath(confPath)
        {
            vscode::instance()->add(this);
        }

    private:
        void initImpl(const fs::path& dest, const bool)
        {
            fs::directory_iterator it(confPath);
            for (auto& i : it)
                fs::copy(i.path(), dest / ".vscode", fs::copy_options::overwrite_existing | fs::copy_options::recursive);
        }
        void deinitImpl(const fs::path&, const bool) { }

        const fs::path confPath;
    };

// config for windows
#ifdef Windows
    inline static DirectCopy msvc("MSVC", "Microsoft c++ compiler", "./config/vscode/msvc");
    inline static DirectCopy gcc("gcc", "GCC in MinGW", "./config/vscode/gcc");
    inline static DirectCopy devc("Dev c++", "Dev-c++ (Header path only)", "./config/vscode/devc");
#endif
#ifdef Linux
    inline static DirectCopy gcc("gcc", "GCC for linux", "./config/vscode/gcc");
    inline static DirectCopy clang("Clang", "Clang for linux", "./config/vscode/clang");
#endif
}
