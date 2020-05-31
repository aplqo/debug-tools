#ifndef DebugTools_LIMIT_H
#define DebugTools_LIMIT_H
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <thread>

namespace apdebug::limit
{
    using limit = std::chrono::milliseconds;
    class watchDog
    {
    public:
        watchDog(limit l, void (*f)())
            : lim(l)
            , onTimeout(f) {};
        watchDog(const watchDog&) = delete;
        void start()
        {
            isStart = true;
            wdt = std::thread(std::mem_fn(&watchDog::wdThrd), this);
        }
        void stop()
        {
            mtx.lock();
            isStart = false;
            mtx.unlock();
            cnd.notify_all();
            wdt.join();
        }

    private:
        void wdThrd()
        {
            std::unique_lock lk(mtx);
            if (cnd.wait_for(lk, lim, [&]() { return !isStart; }))
                return;
            onTimeout();
        }

        void (*onTimeout)();

        std::thread wdt;
        std::condition_variable cnd;
        std::mutex mtx;
        limit lim;
        bool isStart = false;
    };
    class LimitGuard
    {
    public:
        LimitGuard(watchDog& l)
            : l(l)
        {
            l.start();
        };
        ~LimitGuard()
        {
            l.stop();
        }

    private:
        watchDog& l;
    };
}

#endif