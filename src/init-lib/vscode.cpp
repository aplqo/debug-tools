#include "include/init.h"
#include <filesystem>
#include <fstream>
#include <memory>

namespace apdebug::init::vscode
{
    using std::ofstream;
    using std::unique_ptr;
    using std::filesystem::copy;
    using std::filesystem::copy_options;
    using std::filesystem::create_directory;
    using std::filesystem::path;
    using std::filesystem::remove;
    using std::filesystem::remove_all;

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
            static unique_ptr<vscode> obj(new vscode);
            return obj.get();
        }

    private:
        void initImpl(const path& p, const bool upd = false)
        {
            if (upd)
                return;
            create_directory(p / ".vscode");
        }
        void deinitImpl(const path& dest, const bool)
        {
            remove_all(dest / ".vscode");
        }
    };
    class DirectCopy : public compiler
    {
    public:
        DirectCopy(const char* name, const char* descript, const path confPath)
            : compiler(name, descript)
            , confPath(confPath)
        {
            vscode::instance()->add(this);
        }

    private:
        void initImpl(const path& dest, const bool)
        {
            std::filesystem::directory_iterator it(confPath);
            for (auto& i : it)
                copy(i.path(), dest / ".vscode", copy_options::overwrite_existing | copy_options::recursive);
        }
        void deinitImpl(const path&, const bool) {}

        const path confPath;
    };

// config for windows
#ifdef Windows
    inline static DirectCopy msvc("MSVC", "Microsoft c++ compiler", "./config/vscode/msvc");
    inline static DirectCopy gcc("gcc", "GCC in MinGW", "./config/vscode/gcc");
    inline static DirectCopy devc("Dev c++", "Dev-c++ (Header path only)", "./config/vscode/devc");
#endif
#ifdef Linux
    inline static DirectCopy gcc("gcc","GCC for linux","./config/vscode/gcc");
    inline static DirectCopy clang("Clang","Clang for linux","./config/vscode/clang");
#endif
}
