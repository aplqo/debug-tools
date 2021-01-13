#include "include/init.h"
#include "include/io.h"
#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <regex>
#include <string>

namespace fs = std::filesystem;
using apdebug::Output::writeFile;
namespace apdebug::init::cmake
{

    class cmake : public editor
    {
    public:
        cmake()
            : editor("cmake", "Use cmake to generate ide config")
        {
            editors()->append(this);
        }

        static cmake* instance()
        {
            static std::unique_ptr<cmake> obj(new cmake);
            return obj.get();
        }

    private:
        void initImpl(const fs::path& dest, const bool)
        {
            fs::copy("./config/cmake/CMakeLists.txt", dest, fs::copy_options::overwrite_existing);
        }
        void deinitImpl(const fs::path& dest, const bool)
        {
            remove(dest / "CMakeLists.txt");
        }
    };
    class Generator : public compiler
    {
    public:
        Generator()
            : compiler("generate", "Use cmake generator.")
        {
            cmake::instance()->add(this);
        }
        void read()
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
        void init(const fs::path& dest)
        {
            writeFile(dest / ".config" / "cmake_gen", gen);
            initImpl(dest);
        }
        void update(const fs::path& dest)
        {
            gen = readFileLn(dest / ".config" / "cmake_gen");
            deinitImpl(dest);
            initImpl(dest);
        }
        void deinit(const fs::path& dest)
        {
            deinitImpl(dest);
        }

    private:
        void initImpl(const fs::path& dest, const bool upd = false)
        {
            fs::create_directory(dest / "build");
            std::string cmd = "cmake ";
            if (gen != ".")
                cmd += "-G \"" + gen + "\" ";
            cmd += "-S \"" + dest.string() + "\" -B \"" + (dest / "build").string() + "\"";
            std::system(cmd.c_str());
        }
        void deinitImpl(const fs::path& dest, const bool upd = false)
        {
            fs::remove_all(dest / "build");
        }

        std::string gen;
    };
    Generator gen;

    template <size_t siz>
    class DirectOpen : public compiler
    {
    public:
        DirectOpen(const char* name, const char* description, const std::array<std::regex, siz> rmList)
            : compiler(name, description)
            , rmList(rmList)
        {

            cmake::instance()->add(this);
        }

    private:
        void initImpl(const fs::path&, const bool) override { }
        void deinitImpl(const fs::path& dest, const bool) override
        {
            for (const auto& i : rmList)
            {
                fs::directory_iterator it(dest);
                for (auto& j : it)
                {
                    if (regex_match(j.path().filename().string(), i))
                        remove_all(j.path());
                }
            }
        }

        const std::array<std::regex, siz> rmList;
    };
    template <size_t i>
    DirectOpen(const char*, const char*, const std::array<std::regex, i>) -> DirectOpen<i>;

#ifdef Windows
    DirectOpen vs("VS", "Visual Studio CMake project", std::array<std::regex, 3> { std::regex(R"(\.vs$)"), std::regex("out"), std::regex("CMakeSettings.json") });
#endif
#ifdef Linux
    DirectOpen kdevelop("kdevelop", "KDevelop cmake project", std::array<std::regex, 2> { std::regex(R"(.*\.kdev4$)"), std::regex("build") });
#endif
}
