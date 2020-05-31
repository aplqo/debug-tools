#ifndef TIMER_H
#define TIMER_H

#if __cplusplus < 201103L
#error ISO c++11 is required to use timer!
#endif

#include "debug_tools/define.h"
#include "debug_tools/log.h"
#include "debug_tools/logfile.h"
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <functional>
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
        using std::chrono::duration_cast;
        using std::chrono::microseconds;
        using std::chrono::steady_clock;
        using std::chrono::time_point;
        using namespace log;
        using namespace logfile;

        template <class tim, class rep, class period>
        class timer
        {
        public:
            timer(timType& l, timType& hlim)
                : lim(l)
                , hardlim(hlim)
            {
            }
            inline void start()
            {
                stat = true;
                watch = thread(std::mem_fn(&timer::watchdog), this);
                beg = tim::now();
            }
            void stop()
            {
                if (!stat)
                    return;
                aft = tim::now();
                {
                    std::lock_guard<std::mutex> lk(mstat);
                    stat = false;
                }
                cv.notify_all();
                pr = true;
                if (watch.joinable())
                    watch.join();
            }
            void print()
            {
                if (!pr)
                    return;
                pr = false;
                duration<rep, period> d1 = duration_cast<duration<rep, period>>(aft - beg);
                WriteObj(RStatus::Time);
                WriteObj(static_cast<timType>(d1.count()));
            }

        private:
            bool pr = false;
            typename tim::time_point beg, aft;
            timType &lim, &hardlim;

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
                WriteObj(RStatus::HardLimit);
                quick_exit(5);
            }
        };
    } // namespace timer
} // namespace apdebug

#endif