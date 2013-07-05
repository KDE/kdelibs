#ifndef EXECUTEWRAPPER_H
#define EXECUTEWRAPPER_H

#include <QAtomicPointer>

#include "Job.h"
#include "Executor.h"

namespace ThreadWeaver {

class Job;
class Thread;

//FIXME Pimpl, make part of the API, document
//Find a way to avoid new/delete operation, this is rather performance sensitive area
class ExecuteWrapper : public Executor
{
public:
    ExecuteWrapper();
    Executor* wrap(Executor* previous);
    Executor* unwrap(JobPointer job);
    void begin(JobPointer job, Thread *);
    void execute(JobPointer job, Thread *thread);
    void executeWrapped(JobPointer job, Thread* thread);
    void end(JobPointer job, Thread *);

private:
    QAtomicPointer<Executor> wrapped;
};

}

#endif // EXECUTEWRAPPER_H
