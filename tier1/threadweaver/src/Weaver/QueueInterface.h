#ifndef THREADWEAVER_QUEUEINTERFACE_H
#define THREADWEAVER_QUEUEINTERFACE_H

#include <JobPointer.h>

namespace ThreadWeaver {

class Job;
class Thread;

class THREADWEAVER_EXPORT QueueInterface {
public:
    virtual ~QueueInterface() {}

    /** Assign a job to an idle thread.
     * @param th the thread to give a new Job to
     * @param wasBusy true if a job was previously assigned to the calling thread
     */
    virtual JobPointer applyForWork(Thread *th, bool wasBusy) = 0;

    /** Wait (by suspending the calling thread) until a job becomes available. */
    virtual void waitForAvailableJob(Thread *th) = 0;
};

}

#endif // THREADWEAVER_QUEUEINTERFACE_H
