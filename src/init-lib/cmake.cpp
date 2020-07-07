#include "include/init.h"
#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <regex>
#include <string>

namespace apdebug::init::cmake
{
    using std::array;
    using std::endl;
    using std::getline;
    using std::regex;
    using std::regex_match;
    using std::smatch;
    using std::string;
    using std::system;
    using std::unique_ptr;
    using std::filesystem::copy;
    using std::filesystem::copy_options;
    using std::filesystem::create_directory;
    using std::filesystem::directory_iterator;
    using std::filesystem::path;
    using std::filesystem::remove_all;

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
            static unique_ptr<cmake> obj(new cmake);
            return obj.get();
        }

    private:
        void initImpl(const path& dest, const bool)
        {
            copy("./config/cmake/CMakeLists.txt", dest, copy_options::overwrite_existing);
        }
        void deinitImpl(const path& dest, const bool)
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
            cout << "Available generators:" << endl;
            system("cmake --help");
            cout << "Enter selection(. for empty):";
            cout.flush();
            cin.ignore(10, '\n');
            getline(cin, gen);
        }
        void init(const path& dest)
        {
            writeFile(dest / ".config" / "cmake_gen", gen);
            initImpl(dest);
        }
        void update(const path& dest)
        {
            gen = readFileLn(dest / ".config" / "cmake_gen");
            deinitImpl(dest);
            initImpl(dest);
        }
        void deinit(const path& dest)
        {
            deinitImpl(dest);
        }

    private:
        void initImpl(const path& dest, const bool upd = false)
        {
            create_directory(dest / "build");
            string cmd = "cmake ";
            if (gen != ".")
                cmd += "-G \"" + gen + "\" ";
            cmd += "-S \"" + dest.string() + "\" -B \"" + (dest / "build").string() + "\"";
            system(cmd.c_str());
        }
        void deinitImpl(const path& dest, const bool upd = false)
        {
            remove_all(dest / "build");
        }

        string gen;
    };
    Generator gen;

    template <size_t siz>
    class DirectOpen : public compiler
    {
    public:
        DirectOpen(const char* name, const char* description, const array<regex, siz> rmList)
            : compiler(name, description)
            , rmList(rmList)
        {

            cmake::instance()->add(this);
        }

    private:
        void initImpl(const path&, const bool) override {}
        void deinitImpl(const path& dest, const bool) override
        {
            for (const auto& i : rmList)
            {
                directory_iterator it(dest);
                for (auto& j : it)
                {
                    if (regex_match(j.path().filename().string(), i))
                        remove_all(j.path());
                }
            }
        }

        const array<regex, siz> rmList;
    };
    template <size_t i>
    DirectOpen(const char*, const char*, const array<regex, i>)->DirectOpen<i>;

#ifdef Windows
    DirectOpen vs("VS", "Visual Studio CMake project", array<regex, 3> { regex(R"(\.vs$)"), regex("out"), regex("CMakeSettings.json") });
#endif
#ifdef Linux
    DirectOpen kdevelop("kdevelop", "KDevelop cmake project", array<regex, 2> { regex(R"(.*\.kdev4$)"), regex("build") });
#endif
}
