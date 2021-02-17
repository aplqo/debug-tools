#include <filesystem>
#include <fstream>
#include <memory>

#include "include/init.h"

namespace fs = std::filesystem;

namespace apdebug::init::vscode {

static void copyDir(const fs::path& from, const fs::path& to)
{
  fs::directory_iterator it(from);
  for (const auto& i : it)
    fs::copy(
        i, to,
        fs::copy_options::recursive | fs::copy_options::overwrite_existing);
}

class vscode : public editor {
 public:
  vscode() : editor("vscode", "Visual Studio Code") { editors()->append(this); }

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
class DirectCopy : public compiler {
 public:
  DirectCopy(const char* name, const char* descript, const fs::path confPath)
      : compiler(name, descript), confPath(confPath)
  {
    vscode::instance()->add(this);
  }

 private:
  void initImpl(const fs::path& src, const fs::path& dest) override
  {
    const fs::path destPath = dest / ".vscode";
    copyDir(src / confPath, destPath);
    copyDir(src / "config" / "compiler-shared" / "vscode", destPath);
  }
  void deinitImpl(const fs::path&) override {}

  const fs::path confPath;
};

// config for windows
#ifdef Windows
inline static DirectCopy msvc("MSVC", "Microsoft c++ compiler",
                              "./config/vscode/msvc");
inline static DirectCopy devc("Dev c++", "Dev-c++ (Header path only)",
                              "./config/vscode/devc");
#endif
#ifdef Linux
inline static DirectCopy gcc("gcc", "GCC for linux", "./config/vscode/gcc");
inline static DirectCopy clang("Clang", "Clang for linux",
                               "./config/vscode/clang");
#endif
}  // namespace apdebug::init::vscode
