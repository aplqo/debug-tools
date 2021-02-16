#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <regex>
#include <string>

#include "include/init.h"
#include "include/io.h"

namespace fs = std::filesystem;
using apdebug::Output::writeFile;
namespace apdebug::init::cmake {

class cmake : public editor {
 public:
  cmake() : editor("cmake", "Use cmake to generate ide config")
  {
    editors()->append(this);
  }

  static cmake* instance()
  {
    static std::unique_ptr<cmake> obj(new cmake);
    return obj.get();
  }

 private:
  void initImpl(const fs::path& src, const fs::path& dest)
  {
    fs::copy(src / "config" / "cmake" / "CMakeLists.txt", dest,
             fs::copy_options::overwrite_existing);
  }
  void deinitImpl(const fs::path& dest) { remove(dest / "CMakeLists.txt"); }
};
class Generator : public compiler {
 public:
  Generator() : compiler("generate", "Use cmake generator.")
  {
    cmake::instance()->add(this);
  }
  void read() override
  {
    using std::cin;
    using std::cout;
    cout << "Available generators:" << std::endl;
    std::system("cmake --help");
    cout << "Enter selection(. for empty):";
    cout.flush();
    cin.ignore(10, '\n');
    std::getline(cin, gen);
  }
  void load(const fs::path& dest, const Operate) override
  {
    gen = readFileLn(dest / ".config" / "cmake_gen");
  }
  void install(const fs::path& src, const fs::path& dest) override
  {
    writeFile(dest / ".config" / "cmake_gen", gen);
    init(src, dest);
  }
  void init(const fs::path&, const fs::path& dest) override
  {
    fs::create_directory(dest / "build");
    std::string cmd = "cmake ";
    if (gen != ".") cmd += "-G \"" + gen + "\" ";
    cmd +=
        "-S \"" + dest.string() + "\" -B \"" + (dest / "build").string() + "\"";
    std::system(cmd.c_str());
  }
  void update(const fs::path& src, const fs::path& dest) override
  {
    deinit(dest);
    init(src, dest);
  }
  void deinit(const fs::path& dest) { fs::remove_all(dest / "build"); }

 private:
  void initImpl(const fs::path&, const fs::path&) override{};
  void deinitImpl(const fs::path& dest) override{};

  std::string gen;
};
Generator gen;

template <size_t siz>
class DirectOpen : public compiler {
 public:
  DirectOpen(const char* name, const char* description,
             const std::array<std::regex, siz> rmList)
      : compiler(name, description), rmList(rmList)
  {
    cmake::instance()->add(this);
  }

 private:
  void initImpl(const fs::path&, const fs::path&) override {}
  void deinitImpl(const fs::path& dest) override
  {
    for (const auto& i : rmList) {
      fs::directory_iterator it(dest);
      for (auto& j : it) {
        if (regex_match(j.path().filename().string(), i)) remove_all(j.path());
      }
    }
  }

  const std::array<std::regex, siz> rmList;
};
template <size_t i>
DirectOpen(const char*, const char*, const std::array<std::regex, i>)
    -> DirectOpen<i>;

#ifdef Windows
DirectOpen vs("VS", "Visual Studio CMake project",
              std::array<std::regex, 3>{std::regex(R"(\.vs$)"),
                                        std::regex("out"),
                                        std::regex("CMakeSettings.json")});
#endif
#ifdef Linux
DirectOpen kdevelop("kdevelop", "KDevelop cmake project",
                    std::array<std::regex, 2>{std::regex(R"(.*\.kdev4$)"),
                                              std::regex("build")});
#endif
}  // namespace apdebug::init::cmake
