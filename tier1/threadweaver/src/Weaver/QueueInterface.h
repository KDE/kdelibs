#ifndef THREADWEAVER_QUEUEINTERFACE_H
#define THREADWEAVER_QUEUEINTERFACE_H

namespace ThreadWeaver {

class Job;
class Thread;

class QueueInterface {
public:
    virtual ~QueueInterface() {}

    /** Assign a job to an idle thread.
     * @param th the thread to give a new Job to
     * @param previous the job this thread finished before calling
     */
    virtual Job* applyForWork ( Thread *th,  Job* previous ) = 0;

    /** Wait (by suspending the calling thread) until a job becomes available. */
    virtual void waitForAvailableJob ( Thread *th ) = 0;
};

}

#endif // THREADWEAVER_QUEUEINTERFACE_H
