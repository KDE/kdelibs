#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "Job.h"

namespace ThreadWeaver {

class Job;
class Thread;

//FIXME Pimpl, make part of the API, document
//TODO can JobPointer references be used?
class Executor
{
public:
    virtual ~Executor();
    virtual void begin(JobPointer, Thread*) = 0;
    void defaultBegin(JobPointer job, Thread* thread);
    virtual void execute(JobPointer, Thread*) = 0;
    virtual void end(JobPointer, Thread*) = 0;
    void defaultEnd(JobPointer job, Thread* thread);
    virtual void cleanup(JobPointer, Thread*);

    void run(JobPointer job, Thread* thread);
};

}

#endif // EXECUTOR_H
