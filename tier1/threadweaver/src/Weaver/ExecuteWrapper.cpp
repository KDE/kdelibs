#include "ExecuteWrapper.h"

namespace ThreadWeaver {

ExecuteWrapper::ExecuteWrapper()
{
}

void ExecuteWrapper::wrap(Executor *previous)
{
    wrapped = previous;
}

void ExecuteWrapper::executeWrapped(Job *job, Thread *thread)
{
    wrapped->execute(job, thread);
}

}

#include "ExecuteWrapper.h"

