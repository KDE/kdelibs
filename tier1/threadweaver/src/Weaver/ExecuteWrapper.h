#ifndef EXECUTEWRAPPER_H
#define EXECUTEWRAPPER_H

#include <QAtomicPointer>

#include "Job.h"
#include "Executor.h"

namespace ThreadWeaver {

class Job;
class Thread;

//FIXME Pimpl, make part of the API
//Find a way to avoid new/delete operation, this is rather performance sensitive area
class ExecuteWrapper : public Executor
{
public:
    ExecuteWrapper();
    void wrap(Executor* previous);
    void executeWrapped(Job* job, Thread* thread);

private:
    QAtomicPointer<Executor> wrapped;
};

}

#endif // EXECUTEWRAPPER_H
