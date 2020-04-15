#ifndef GENERATOR_H
#define GENERATOR_H

#include <random>

namespace apdebug::generator
{
    std::mt19937_64 default_rnd(std::random_device().operator());
    template <class T, class R>
    T GetRandom(const T a, const T b, R rnd = default_rnd) // get random number once
    {
        std::uniform_int_distribution<T> dis(a, b);
        return dis(rnd);
    }
    template <class T>
    T MayBe(const T val, const T def)
    {
        return val ? val : def;
    }
} // namespace apdebug

#endif