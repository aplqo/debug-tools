#include "include/init.h"
#include <filesystem>
#include <iostream>
#include <string>

using std::string;
using std::filesystem::path;
namespace apdebug::init::none
{
    class noeditor : public editor
    {
    public:
        noeditor()
            : editor("none", "Don't init editor config")
        {
            editors()->append(this);
        }
        compiler* find(const string&) const
        {
            return nullptr;
        }
        compiler* read()
        {
            return nullptr;
        }
        void print() const { }
        void init(const path&, compiler*) { }
        void update(const path&) { }
        void deinit(const path&) { }

    private:
        void initImpl(const path&, bool) { }
        void deinitImpl(const path&, bool) { }
    } noedit;

    class nocompiler : public compiler
    {
    public:
        nocompiler()
            : compiler("none", "Don't init compiler config")
        {
            shared()->append(this);
        }
        void init(const path&) { }
        void update(const path&) { }
        void deinit(const path&) { }

    private:
        void initImpl(const path&, bool) { }
        void deinitImpl(const path&, bool) { }
    } nocompile;
}