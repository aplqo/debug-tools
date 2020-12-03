#ifndef GENERATOR_H
#define GENERATOR_H

#include <algorithm>
#include <functional>
#include <map>
#include <numeric>
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

        template <class T, class OutIt, class Rnd = decltype(default_rnd)>
        void createPermutation(const T n, const T ini, OutIt buf, Rnd& rd = default_rnd)
        {
            T* tp = new T[n];
            std::iota(tp, tp + n, ini);
            std::shuffle(tp, tp + n, rd);
            std::copy(tp, tp + n, buf);
            delete[] tp;
        }
        template <class T, class OutIt, class Rnd = decltype(default_rnd)>
        void createArray(const T mn, const T mx, const size_t len, OutIt buf, Rnd& rd = default_rnd)
        {
            std::uniform_int_distribution<T> dis(mn, mx);
            std::generate_n(buf, len, std::bind(dis, std::ref(rd)));
        }
    }
} // namespace apdebug

#endif