#ifndef DYNARRAY_H
#define DYNARRAY_H

#include "include/dynArray.h"

#include <yaml-cpp/yaml.h>

namespace apdebug::DynArray
{
    template <class T, class U>
    concept Function = requires(T f, U* a, const YAML::Node& node)
    {
        f(a++, node);
    };
    template <class T>
    struct DynArray
    {
        size_t size = 0;
        T* data = nullptr;
        const T* begin() const { return data; }
        const T* end() const { return data + size; }
        T* begin() { return data; }
        T* end() { return data + size; }
        inline void allocate(const unsigned int count)
        {
            size = count;
            data = reinterpret_cast<T*>(std::malloc(sizeof(T) * size));
        }
        inline void release()
        {
            if (!data)
                return;
            for (T* i = data; i < data + size; ++i)
                i->~T();
            free(data);
            data = nullptr;
        }
        template <class U>
        requires Function<U, T> void parseArgument(const YAML::Node& node, U param)
        {
            allocate(node.size());
            T* ptr = data;
            for (const auto& it : node)
                param(ptr++, it);
        }
        void parseArgument(const YAML::Node& node)
        {
            allocate(node.size());
            T* ptr = data;
            for (const auto& it : node)
                *(ptr++) = it.as<T>();
        }
    };
}

#endif