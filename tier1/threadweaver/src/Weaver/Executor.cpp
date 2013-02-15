#include "Executor.h"

namespace ThreadWeaver {

Executor::~Executor()
{
}

void Executor::run(Job *job)
{
    job->run();
}

}
