#ifndef GENERATOR_H
#define GENERATOR_H

#include <map>
#include <random>

namespace apdebug
{
    namespace generator
    {
        std::mt19937_64 default_rnd;

        template <class Rnd = std::mt19937_64>
        void Init(Rnd& r = default_rnd)
        {
            r.seed(std::random_device()());
        }
        template <class T, class R = std::mt19937_64>
        T GetRandom(const T a, const T b, R& rnd = default_rnd) // get random number once
        {
            std::uniform_int_distribution<T> dis(a, b);
            return dis(rnd);
        }
        template <class T, T a, T b, class Rnd = std::mt19937_64>
        T GetRandom(Rnd& rnd = default_rnd)
        {
            static std::uniform_int_distribution<T> dis(a, b);
            return dis(rnd);
        }
        template <class T>
        T MayBe(const T val, const T def)
        {
            return val ? val : def;
        }
    }
} // namespace apdebug

#endif