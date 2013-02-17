#include "Executor.h"

namespace ThreadWeaver {

Executor::~Executor()
{
}

void Executor::defaultBegin(Job *job, Thread *thread)
{
    job->defaultBegin(job, thread);
}

void Executor::defaultEnd(Job *job, Thread *thread)
{
    job->defaultEnd(job, thread);
}

void Executor::cleanup(Job *, Thread *)
{
}

void Executor::run(Job *job)
{
    job->run();
}

}
