#ifndef INIT_LIST_H
#define INIT_LIST_H

#include "include/init.h"

#include <array>

namespace apdebug::init
{
    template <class T>
    list<T>::list(const char* typ)
        : tab(std::array<const char*, 3> { "Id", typ, "Description" }, Output::SGR::None) {};
    template <class T>
    void list<T>::append(T c)
    {
        m[c->name] = size();
        lst.push_back(c);
    }
    template <class T>
    void list<T>::print(unsigned int beg) const
    {
        for (auto i : lst)
        {
            tab.newColumn("");
            tab.writeColumn(0, std::to_string(beg));
            tab.writeColumn(1, i->name);
            tab.writeColumn(2, i->description);
            ++beg;
        }
        tab.printHeader(std::cout);
        tab.printAll(std::cout);
    }
    template <class T>
    T list<T>::read() const
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
    template <class T>
    T list<T>::find(const std::string& s) const
    {
        const auto it = m.find(s);
        return it == m.end() ? nullptr : lst[it->second];
    }
    template <class T>
    size_t list<T>::size() const
    {
        return lst.size();
    }
}

#endif