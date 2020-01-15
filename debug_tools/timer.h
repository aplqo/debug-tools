#ifndef TIMER_H
#define TIMER_H

#if __cplusplus < 201103L
#error ISO c++11 is required to use timer!
#endif

#include "define.h"
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>

namespace apdebug
{
    namespace timer
    {
        using std::cerr;
        using std::condition_variable;
        using std::endl;
        using std::mutex;
        using std::quick_exit;
        using std::thread;
        using std::unique_lock;
        using std::chrono::duration;
        using std::chrono::steady_clock;
        using std::chrono::time_point;

        template <class tim, class rep, class period>
        class timer
        {
        public:
            timer(timType l, timType hlim)
                : lim(l)
                , hardlim(hlim)
            {
            }
            inline void start()
            {
                stat = true;
                watch = thread(&timer::watchdog, this);
                beg = tim::now();
            }
            void stop()
            {
                if (!stat)
                    return;
                aft = tim::now();
                stat = false;
                pr = true;
                cv.notify_all();
                if (watch.joinable())
                    watch.join();
            }
            void print()
            {
                if (!pr)
                    return;
                pr = false;
                duration<rep, period> d1 = duration_cast<duration<rep, period>>(aft - beg);
                cerr << "Time " << d1.count() << endl;
            }

        private:
            bool pr = false;
            typename tim::time_point beg, aft;
            unsigned int lim, hardlim;

            /*-thread support-*/
            condition_variable cv;
            mutex mstat;
            thread watch;
            bool stat = false;
            void watchdog()
            {
                unique_lock<mutex> lk(mstat);
                if (cv.wait_for(lk, microseconds(hardlim), [&]() -> bool { return !stat; }))
                    return;
                cerr << "Hlim" << endl;
                quick_exit(5);
            }
        };
    }
}

#endif