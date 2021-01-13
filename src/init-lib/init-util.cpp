#include "include/init-list.h"
#include "include/init.h"
#include "include/utility.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
using apdebug::Input::readFileVal;
using apdebug::Output::writeFile;
using std::cout;
using std::string;
using std::unique_ptr;
using std::filesystem::path;
namespace apdebug::init
{
    template class list<compiler*>;
    list<compiler*>* shared()
    {
        static unique_ptr<list<compiler*>> obj(new list<compiler*>("Shared"));
        return obj.get();
    }
    template class list<editor*>;
    list<editor*>* editors()
    {
        static unique_ptr<list<editor*>> obj(new list<editor*>("Editors"));
        return obj.get();
    }

    string readFileLn(const path& p)
    {
        string ret;
        std::ifstream f(p);
        std::getline(f, ret);
        return ret;
    }

    compiler* editor::find(const string& c) const
    {
        compiler* ptr = com.find(c);
        return ptr ? ptr : shared()->find(c);
    }
    compiler* editor::read()
    {
        unsigned int sel;
        std::cout << "Enter compiler: ";
        std::cout.flush();
        std::cin >> sel;
        while (sel >= com.size() && sel - com.size() > shared()->size())
        {
            std::cout << "No such config!" << std::endl;
            std::cout << "Enter compiler: ";
            std::cout.flush();
            std::cin >> sel;
        }
        std::cout << std::endl;
        compiler* ret = sel >= com.size() ? shared()->lst[sel - com.size()] : com.lst[sel];
        ret->read();
        return ret;
    }
    void editor::add(compiler* c)
    {
        com.append(c);
    }
    void editor::init(const path& dest, compiler* c)
    {
        initImpl(dest);
        writeFile(dest / ".config" / "compiler", c->name);
        c->init(dest);
    }
    void editor::print() const
    {
        com.print();
        std::cout << std::endl;
        shared()->print(com.lst.size());
    }
    void editor::update(const path& dest)
    {
        deinitImpl(dest, true);
        initImpl(dest, true);
        find(readFileLn(dest / ".config" / "compiler"))->update(dest);
    }
    void editor::deinit(const path& dest)
    {
        find(readFileLn(dest / ".config" / "compiler"))->deinit(dest);
        deinitImpl(dest);
    }

    void compiler::init(const path& dest)
    {
        initImpl(dest);
    }
    void compiler::update(const path& dest)
    {
        deinitImpl(dest, true);
        initImpl(dest, true);
    }
    void compiler::deinit(const path& dest)
    {
        deinitImpl(dest);
    }
}
