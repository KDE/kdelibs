#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "Job.h"

namespace ThreadWeaver {

class Job;
class Thread;

//FIXME Pimpl, make part of the API
class Executor
{
public:
    virtual ~Executor();
    virtual void execute(Job*, Thread*) = 0;
    virtual void cleanup(Job*, Thread*);

    void run(Job* job);
};

}

#endif // EXECUTOR_H
