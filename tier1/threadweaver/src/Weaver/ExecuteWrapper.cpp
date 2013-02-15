#include "ExecuteWrapper.h"

namespace ThreadWeaver {

ExecuteWrapper::ExecuteWrapper()
{
}

void ExecuteWrapper::wrap(Executor *previous)
{
    wrapped.fetchAndStoreOrdered(previous);
}

void ExecuteWrapper::executeWrapped(Job *job, Thread *thread)
{
    Executor* executor = wrapped.fetchAndAddOrdered(0);
    Q_ASSERT_X(executor!=0, Q_FUNC_INFO, "Wrapped Executor cannot be zero!");
    executor->execute(job, thread);
}

}

#include "ExecuteWrapper.h"

