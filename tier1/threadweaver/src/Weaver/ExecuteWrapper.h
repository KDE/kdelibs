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
    Executor* unwrap(Job*);
    void begin(Job *, Thread *);
    void execute(Job *job, Thread *thread);
    void executeWrapped(Job* job, Thread* thread);
    void end(Job *, Thread *);

private:
    QAtomicPointer<Executor> wrapped;
};

}

#endif // EXECUTEWRAPPER_H
