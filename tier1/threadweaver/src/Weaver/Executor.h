#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "Job.h"

namespace ThreadWeaver {

class Job;
class Thread;

//FIXME Pimpl, make part of the API, document
class Executor
{
public:
    virtual ~Executor();
    virtual void begin(Job*, Thread*) = 0;
    void defaultBegin(Job* job, Thread* thread);
    virtual void execute(Job*, Thread*) = 0;
    virtual void end(Job*, Thread*) = 0;
    void defaultEnd(Job* job, Thread* thread);
    virtual void cleanup(Job*, Thread*);

    void run(Job* job);
};

}

#endif // EXECUTOR_H
