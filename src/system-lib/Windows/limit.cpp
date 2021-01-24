#include "system.h"

namespace apdebug::System
{
    std::pair<bool, int> TimeLimit::waitFor(const Process& p)
    {
        if (WaitForSingleObject(p.handle, time) == WAIT_TIMEOUT)
        {
            p.terminate();
            exceed = true;
            return std::make_pair(true, 9);
        }
        exceed = false;
        DWORD ret;
        GetExitCodeProcess(p.handle, &ret);
        return std::make_pair(false, ret);
    }

    void watchThread(KillParam* const kp)
    {
        ULONG_PTR key;
        LPOVERLAPPED over;
        DWORD msg;
        while (GetQueuedCompletionStatus(kp->iocp, &msg, &key, &over, INFINITE))
        {
            if (msg == JOB_OBJECT_MSG_JOB_MEMORY_LIMIT)
            {
                TerminateJobObject(kp->job, 9);
                ++(kp->exceed);
            }
            else if (key)
                return;
        }
    }
    MemoryLimit::MemoryLimit()
    {
        job = CreateJobObject(NULL, NULL);
        iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
        cntrl = KillParam {
            .job = job,
            .iocp = iocp,
            .exceed = 0
        };
        {
            JOBOBJECT_ASSOCIATE_COMPLETION_PORT ass {
                .CompletionKey = 0, .CompletionPort = iocp
            };
            SetInformationJobObject(job, JobObjectAssociateCompletionPortInformation, &ass, sizeof(ass));
        }
        exceed = 0;
        watch = std::thread(watchThread, &cntrl);
    }
    void MemoryLimit::setLimit(const MemoryUsage kb)
    {

        JOBOBJECT_EXTENDED_LIMIT_INFORMATION lim {
            .BasicLimitInformation = JOBOBJECT_BASIC_LIMIT_INFORMATION {
                .LimitFlags = JOB_OBJECT_LIMIT_JOB_MEMORY },
            .JobMemoryLimit = kb * 1024
        };
        SetInformationJobObject(job, JobObjectExtendedLimitInformation, &lim, sizeof(lim));
    }
    void MemoryLimit::addProcess(const Process& p)
    {
        AssignProcessToJobObject(job, p.handle);
    }
    bool MemoryLimit::isExceed()
    {
        const unsigned int old = exceed;
        exceed = cntrl.exceed;
        return exceed > old;
    }
    MemoryLimit::~MemoryLimit()
    {
        PostQueuedCompletionStatus(iocp, 0, 1, 0);
        watch.join();
        CloseHandle(iocp);
        CloseHandle(job);
    }

}