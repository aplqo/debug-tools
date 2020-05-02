#ifndef INIT_H
#define INIT_H

#include "include/output.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace apdebug::init
{
    // simple file operations
    template <class T>
    void writeFile(const std::filesystem::path& p, const T& dat)
    {
        std::ofstream f(p);
        f << dat;
        f.close();
    }
    template <class T>
    T readFileVal(const std::filesystem::path& p)
    {
        std::ifstream f(p);
        T ret;
        f >> ret;
        return ret;
    }
    std::string readFileLn(const std::filesystem::path& p);

    template <class T>
    class list
    {
    public:
        list(const char* typ)
            : tab({ "Id", typ, "Description" })
        {
        }
        void append(T c)
        {
            m[c->name] = size();
            lst.push_back(c);
            tab.update(1, c->name.length() + 2);
            tab.update(2, c->description.length() + 2);
        }
        void print(unsigned int beg = 0) const
        {
            tab.header(std::cout);
            std::cout << std::endl;
            for (auto i : lst)
            {
                tab.print(0, beg, std::cout);
                tab.print(1, i->name, std::cout);
                tab.print(2, i->description, std::cout);
                std::cout << std::endl;
                ++beg;
            }
        }
        T read() const
        {
            unsigned int sel;
            std::cout << "Enter selection: ";
            std::cout.flush();
            std::cin >> sel;
            while (size() <= sel)
            {
                std::cout << "No such configuration!" << std::endl;
                std::cout << "Enter selection: ";
                std::cout.flush();
                std::cin >> sel;
            }
            return lst[sel];
        }
        T find(const std::string& s) const
        {
            const auto it = m.find(s);
            return it == m.end() ? nullptr : lst[it->second];
        }
        size_t size() const
        {
            return lst.size();
        }

        std::vector<T> lst;

    private:
        std::map<std::string, size_t> m;
        mutable out::table tab;
    };

    class compiler
    {
    public:
        compiler(const char* name, const char* descript)
            : name(name)
            , description(descript)
        {
        }
        virtual void init(const std::filesystem::path& dest);
        virtual void update(const std::filesystem::path& dest);
        virtual void deinit(const std::filesystem::path& dest);
        virtual void read() {}
        virtual ~compiler() {}

        const std::string name, description;

    protected:
        virtual void initImpl(const std::filesystem::path& dest, bool upd = false) = 0;
        virtual void deinitImpl(const std::filesystem::path& dest, bool upd = false) = 0;
    };
    list<compiler*>* shared();

    class editor
    {
    public:
        editor(const char* name, const char* description)
            : name(name)
            , description(description)
            , com("Compiler")
        {
        }
        virtual compiler* find(const std::string& c) const;
        virtual compiler* read();
        virtual void add(compiler* c);
        virtual void print() const;
        virtual void init(const std::filesystem::path& dest, compiler* c);
        virtual void update(const std::filesystem::path& dest);
        virtual void deinit(const std::filesystem::path& dest);
        virtual ~editor() {}

        const std::string name, description;

    protected:
        virtual void initImpl(const std::filesystem::path& dest, bool upd = false) = 0;
        virtual void deinitImpl(const std::filesystem::path& dest, bool upd = false) = 0;

        list<compiler*> com;
    };
    list<editor*>* editors();
}
#endif
