#ifndef INIT_H
#define INIT_H

#include "include/io.h"
#include "include/table.h"

#include <concepts>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace apdebug::init
{
    // simple file operations
    std::string readFileLn(const std::filesystem::path& p);

    template <class T>
    concept ListEntry = requires(T&& a, Table::Table<3> b)
    {
        { b.writeColumn(0, a) };
    };
    template <class T>
    class list // when use it, include init-list.h
    {
    public:
        list(const char* typ);
        void append(T c);
        void print(unsigned int beg = 0) const;
        T read() const;
        T find(const std::string& s) const;
        size_t size() const;

        std::vector<T> lst;

    private:
        std::unordered_map<std::string, size_t> m;
        mutable Table::Table<3> tab;
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
        virtual void read() { }
        virtual ~compiler() { }

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
        virtual ~editor() { }

        const std::string name, description;

    protected:
        virtual void initImpl(const std::filesystem::path& dest, bool upd = false) = 0;
        virtual void deinitImpl(const std::filesystem::path& dest, bool upd = false) = 0;

        list<compiler*> com;
    };
    list<editor*>* editors();
}
#endif
