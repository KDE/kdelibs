#include "Executor.h"

namespace ThreadWeaver {

Executor::~Executor()
{
}

void Executor::defaultBegin(JobPointer job, Thread *thread)
{
    job->defaultBegin(job, thread);
}

void Executor::defaultEnd(JobPointer job, Thread *thread)
{
    job->defaultEnd(job, thread);
}

void Executor::cleanup(JobPointer, Thread *)
{
}

void Executor::run(JobPointer job, Thread *thread)
{
    job->run(job, thread);
}

}
