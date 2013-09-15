#include <JobCollection.h>
#include <JobSequence.h>
#include <ManagedJobPointer.h>

#include "IdDecorator.h"

namespace ThreadWeaver {

//Pssst: IdDecorator uses the d pointer to hold decoratee. It has no other member variables.
IdDecorator::IdDecorator(JobInterface *decoratee)
    : d(reinterpret_cast<Private*>(decoratee))
{
}

IdDecorator::~IdDecorator()
{
    Q_ASSERT(d);
    delete job();
}

QMutex *IdDecorator::mutex() const
{
    Q_ASSERT(d);
    return job()->mutex();
}

void IdDecorator::setFinished(bool status)
{
    Q_ASSERT(d);
    job()->setFinished(status);
}

void IdDecorator::run(JobPointer self, Thread *thread)
{
    Q_ASSERT(d);
    job()->run(self, thread);
}

void IdDecorator::defaultBegin(JobPointer self, Thread *thread)
{
    Q_ASSERT(d);
    job()->defaultBegin(self, thread);
}

void IdDecorator::defaultEnd(JobPointer self, Thread *thread)
{
    Q_ASSERT(d);
    job()->defaultEnd(self, thread);
}

void IdDecorator::freeQueuePolicyResources(JobPointer j)
{
    Q_ASSERT(d);
    job()->freeQueuePolicyResources(j);
}

void IdDecorator::removeQueuePolicy(QueuePolicy* policy)
{
    Q_ASSERT(d);
    job()->removeQueuePolicy(policy);
}

QList<QueuePolicy *> IdDecorator::queuePolicies() const
{
    Q_ASSERT(d);
    return job()->queuePolicies();
}

void IdDecorator::assignQueuePolicy(QueuePolicy* policy)
{
    Q_ASSERT(d);
    job()->assignQueuePolicy(policy);
}

bool IdDecorator::isFinished() const
{
    Q_ASSERT(d);
    return job()->isFinished();
}

void IdDecorator::aboutToBeQueued(QueueAPI *api)
{
    Q_ASSERT(d);
    job()->aboutToBeQueued(api);
}

void IdDecorator::aboutToBeQueued_locked(QueueAPI *api)
{
    Q_ASSERT(d);
    job()->aboutToBeQueued_locked(api);
}

void IdDecorator::aboutToBeDequeued(QueueAPI *api)
{
    Q_ASSERT(d);
    job()->aboutToBeDequeued(api);
}

void IdDecorator::aboutToBeDequeued_locked(QueueAPI* api)
{
    Q_ASSERT(d);
    job()->aboutToBeDequeued_locked(api);
}

void IdDecorator::requestAbort()
{
    Q_ASSERT(d);
    job()->requestAbort();
}

bool IdDecorator::success() const
{
    Q_ASSERT(d);
    return job()->success();
}

int IdDecorator::priority() const
{
    Q_ASSERT(d);
    return job()->priority();
}

Executor *IdDecorator::executor() const
{
    Q_ASSERT(d);
    return job()->executor();
}

Executor *IdDecorator::setExecutor(Executor *executor)
{
    Q_ASSERT(d);
    return job()->setExecutor(executor);
}

void IdDecorator::execute(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread)
{
    Q_ASSERT(d);
    job()->execute(self, thread);
}

const ThreadWeaver::JobInterface* IdDecorator::job() const
{
    return reinterpret_cast<JobInterface*>(d);
}

JobInterface *IdDecorator::job()
{
    return reinterpret_cast<JobInterface*>(d);
}

const ThreadWeaver::JobCollection *IdDecorator::collection() const
{
   return dynamic_cast<const JobCollection*>(job());
}

JobCollection *IdDecorator::collection()
{
    return dynamic_cast<JobCollection*>(job());
}

const JobSequence *IdDecorator::sequence() const
{
    return dynamic_cast<const JobSequence*>(job());
}

JobSequence *IdDecorator::sequence()
{
    return dynamic_cast<JobSequence*>(job());
}

}
