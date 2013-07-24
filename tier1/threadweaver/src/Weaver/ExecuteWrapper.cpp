#include "ExecuteWrapper.h"

namespace ThreadWeaver {

ExecuteWrapper::ExecuteWrapper()
{
}

Executor *ExecuteWrapper::wrap(Executor *previous)
{
    return wrapped.fetchAndStoreOrdered(previous);
}

Executor *ExecuteWrapper::unwrap(JobPointer job)
{
    Executor* executor = job->setExecutor(wrapped.fetchAndAddOrdered(0));
    Q_ASSERT_X(executor == this, Q_FUNC_INFO, "ExecuteWrapper can only unwrap itself!");
    wrapped.fetchAndStoreOrdered(0);
    return executor;
}

void ExecuteWrapper::begin(JobPointer job, Thread *thread) {
    Q_ASSERT(wrapped.loadAcquire()!=0);
    wrapped.loadAcquire()->begin(job, thread);
}

void ExecuteWrapper::execute(JobPointer job, Thread *thread)
{
    executeWrapped(job, thread);
}

void ExecuteWrapper::executeWrapped(JobPointer job, Thread *thread)
{
    Executor* executor = wrapped.loadAcquire();
    Q_ASSERT_X(executor!=0, Q_FUNC_INFO, "Wrapped Executor cannot be zero!");
    executor->execute(job, thread);
}

void ExecuteWrapper::end(JobPointer job, Thread *thread) {
    Q_ASSERT(wrapped.loadAcquire()!=0);
    wrapped.loadAcquire()->end(job, thread);
}

}

#include "ExecuteWrapper.h"
