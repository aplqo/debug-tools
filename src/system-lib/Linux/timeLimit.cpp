#include "system.h"

#include <sys/wait.h>

namespace apdebug::System
{
    static void killer(union ::sigval v)
    {
        killParam* t = reinterpret_cast<killParam*>(v.sival_ptr);
        t->killed = true;
        kill(t->pid, SIGKILL);
    }

    void TimeLimit::create()
    {
        sigev.sigev_notify = SIGEV_THREAD;
        sigev.sigev_value.sival_ptr = &cntrl;
        sigev.sigev_notify_function = killer;
        sigev.sigev_notify_attributes = nullptr;
        id = timer_create(CLOCK_MONOTONIC, &sigev, &timer);
    }
    void TimeLimit::setExpire(const unsigned long long us)
    {
        spec = itimerspec {
            .it_interval = {},
            .it_value {
                .tv_sec = us / 1000000,
                .tv_nsec = (us % 1000000) * 1000 }
        };
    }
    std::pair<bool, int> TimeLimit::waitFor(const Process& p)
    {
        static const itimerspec stop {};
        cntrl = killParam { .pid = p.nativeHandle, .killed = false };
        timer_settime(timer, 0, &spec, nullptr);
        int stat;
        waitpid(p.nativeHandle, &stat, 0);
        if (!cntrl.killed)
            timer_settime(timer, 0, &stop, nullptr);
        return std::make_pair(cntrl.killed, stat);
    }
    bool TimeLimit::isExceed()
    {
        return cntrl.killed;
    }
    TimeLimit::~TimeLimit()
    {
        timer_delete(timer);
    }
}