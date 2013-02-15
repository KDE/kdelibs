#include "Executor.h"

namespace ThreadWeaver {

Executor::~Executor()
{
}

void Executor::cleanup(Job *, Thread *)
{
}

void Executor::run(Job *job)
{
    job->run();
}

}
