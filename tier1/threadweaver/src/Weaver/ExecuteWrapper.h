#ifndef EXECUTEWRAPPER_H
#define EXECUTEWRAPPER_H

#include "Job.h"
#include "Executor.h"

namespace ThreadWeaver {

class Job;
class Thread;

class ExecuteWrapper : public Executor
{
public:
    ExecuteWrapper();
    void wrap(Executor* previous);
    void executeWrapped(Job* job, Thread* thread);
private:
    Executor* wrapped;
};

}

#endif // EXECUTEWRAPPER_H
