#include "ExecuteWrapper.h"

namespace ThreadWeaver {

ExecuteWrapper::ExecuteWrapper()
{
}

Executor *ExecuteWrapper::wrap(Executor *previous)
{
    return wrapped.fetchAndStoreOrdered(previous);
}

Executor *ExecuteWrapper::unwrap(Job* job)
{
    Executor* executor = job->setExecutor(wrapped.fetchAndAddOrdered(0));
    Q_ASSERT_X(executor == this, Q_FUNC_INFO, "ExecuteWrapper can only unwrap itself!");
    wrapped.fetchAndStoreOrdered(0);
    return executor;
}

void ExecuteWrapper::begin(Job *job, Thread *thread) {
    Q_ASSERT(wrapped.fetchAndAddOrdered(0));
    wrapped.fetchAndAddOrdered(0)->begin(job, thread);
}

void ExecuteWrapper::execute(Job *job, Thread *thread)
{
    executeWrapped(job, thread);
}

void ExecuteWrapper::executeWrapped(Job *job, Thread *thread)
{
    Executor* executor = wrapped.fetchAndAddOrdered(0);
    Q_ASSERT_X(executor!=0, Q_FUNC_INFO, "Wrapped Executor cannot be zero!");
    executor->execute(job, thread);
}

void ExecuteWrapper::end(Job *job, Thread *thread) {
    Q_ASSERT(wrapped.fetchAndAddOrdered(0));
    wrapped.fetchAndAddOrdered(0)->end(job, thread);
}

}

#include "ExecuteWrapper.h"
