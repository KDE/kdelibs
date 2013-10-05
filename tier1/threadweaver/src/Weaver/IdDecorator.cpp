#include <JobCollection.h>
#include <JobSequence.h>
#include <ManagedJobPointer.h>

#include "IdDecorator.h"

namespace ThreadWeaver {

const int IdDecorator_AutoDelete = 1;

// Pssst: IdDecorator uses the d pointer to hold decoratee. It also uses d2 as a bitfield to store the
// autoDelete setting. The goal is not to require a dynamic allocation on creation.
IdDecorator::IdDecorator(JobInterface *decoratee, bool autoDelete)
    : d1(reinterpret_cast<Private1*>(decoratee))
    , d2(0)
{
    setAutoDelete(autoDelete);
}

IdDecorator::~IdDecorator()
{
    Q_ASSERT(d1);
    delete job();
}

QMutex *IdDecorator::mutex() const
{
    Q_ASSERT(d1);
    return job()->mutex();
}

void IdDecorator::setFinished(bool status)
{
    Q_ASSERT(d1);
    job()->setFinished(status);
}

void IdDecorator::run(JobPointer self, Thread *thread)
{
    Q_ASSERT(d1);
    job()->run(self, thread);
}

void IdDecorator::defaultBegin(JobPointer self, Thread *thread)
{
    Q_ASSERT(d1);
    job()->defaultBegin(self, thread);
}

void IdDecorator::defaultEnd(JobPointer self, Thread *thread)
{
    Q_ASSERT(d1);
    job()->defaultEnd(self, thread);
}

void IdDecorator::freeQueuePolicyResources(JobPointer j)
{
    Q_ASSERT(d1);
    job()->freeQueuePolicyResources(j);
}

void IdDecorator::removeQueuePolicy(QueuePolicy* policy)
{
    Q_ASSERT(d1);
    job()->removeQueuePolicy(policy);
}

QList<QueuePolicy *> IdDecorator::queuePolicies() const
{
    Q_ASSERT(d1);
    return job()->queuePolicies();
}

void IdDecorator::assignQueuePolicy(QueuePolicy* policy)
{
    Q_ASSERT(d1);
    job()->assignQueuePolicy(policy);
}

bool IdDecorator::isFinished() const
{
    Q_ASSERT(d1);
    return job()->isFinished();
}

void IdDecorator::aboutToBeQueued(QueueAPI *api)
{
    Q_ASSERT(d1);
    job()->aboutToBeQueued(api);
}

void IdDecorator::aboutToBeQueued_locked(QueueAPI *api)
{
    Q_ASSERT(d1);
    job()->aboutToBeQueued_locked(api);
}

void IdDecorator::aboutToBeDequeued(QueueAPI *api)
{
    Q_ASSERT(d1);
    job()->aboutToBeDequeued(api);
}

void IdDecorator::aboutToBeDequeued_locked(QueueAPI* api)
{
    Q_ASSERT(d1);
    job()->aboutToBeDequeued_locked(api);
}

void IdDecorator::requestAbort()
{
    Q_ASSERT(d1);
    job()->requestAbort();
}

bool IdDecorator::success() const
{
    Q_ASSERT(d1);
    return job()->success();
}

int IdDecorator::priority() const
{
    Q_ASSERT(d1);
    return job()->priority();
}

Executor *IdDecorator::executor() const
{
    Q_ASSERT(d1);
    return job()->executor();
}

Executor *IdDecorator::setExecutor(Executor *executor)
{
    Q_ASSERT(d1);
    return job()->setExecutor(executor);
}

void IdDecorator::execute(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread)
{
    Q_ASSERT(d1);
    job()->execute(self, thread);
}

void IdDecorator::blockingExecute()
{
    Q_ASSERT(d1);
    job()->blockingExecute();
}

const ThreadWeaver::JobInterface* IdDecorator::job() const
{
    return reinterpret_cast<JobInterface*>(d1);
}

JobInterface *IdDecorator::job()
{
    return reinterpret_cast<JobInterface*>(d1);
}

void IdDecorator::setAutoDelete(bool onOff)
{

    if (onOff) {
        d2 = reinterpret_cast<IdDecorator::Private2*>(IdDecorator_AutoDelete);
    } else {
        d2 = 0;
    }
}

bool IdDecorator::autoDelete() const
{
    return d2 == reinterpret_cast<IdDecorator::Private2*>(IdDecorator_AutoDelete);
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
