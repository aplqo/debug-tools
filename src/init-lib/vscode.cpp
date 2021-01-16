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
        void initImpl(const fs::path&, const fs::path& dest) override
        {
            fs::create_directory(dest / ".vscode");
        }
        void deinitImpl(const fs::path& dest) override
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
        void initImpl(const fs::path& src, const fs::path& dest) override
        {
            fs::directory_iterator it(src / confPath);
            for (auto& i : it)
                fs::copy(i, dest / ".vscode", fs::copy_options::overwrite_existing | fs::copy_options::recursive);
        }
        void deinitImpl(const fs::path&) override { }

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
