#include <QObjectJobDecorator.h>
#include <JobCollection.h>
#include <JobSequence.h>
#include <ManagedJobPointer.h>

namespace ThreadWeaver {

//Pssst: QObjectJobDecorator uses the d pointer to hold decoratee. It has no other member variables.
QObjectJobDecorator::QObjectJobDecorator(JobInterface *decoratee, QObject *parent)
    : QObject(parent)
    , d(reinterpret_cast<Private*>(decoratee))
{
}

QObjectJobDecorator::~QObjectJobDecorator()
{
    Q_ASSERT(d);
    delete job();
}

QMutex *QObjectJobDecorator::mutex() const
{
    Q_ASSERT(d);
    return job()->mutex();
}

void QObjectJobDecorator::setFinished(bool status)
{
    Q_ASSERT(d);
    job()->setFinished(status);
}

Thread *QObjectJobDecorator::thread()
{
    Q_ASSERT(d);
    return job()->thread();
}

void QObjectJobDecorator::run()
{
    Q_ASSERT(d);
    job()->run();
}

void QObjectJobDecorator::defaultBegin(JobPointer self, Thread *thread)
{
    Q_ASSERT(d);
    Q_EMIT started(self);
    job()->defaultBegin(self, thread);
}

void QObjectJobDecorator::defaultEnd(JobPointer self, Thread *thread)
{
    Q_ASSERT(d);
    job()->defaultEnd(self, thread);
    if (!self->success()) {
        Q_EMIT failed(self);
    }
    Q_EMIT done(self);
}

void QObjectJobDecorator::freeQueuePolicyResources()
{
    Q_ASSERT(d);
    job()->freeQueuePolicyResources();
}

void QObjectJobDecorator::removeQueuePolicy(QueuePolicy* policy)
{
    Q_ASSERT(d);
    job()->removeQueuePolicy(policy);
}

void QObjectJobDecorator::assignQueuePolicy(QueuePolicy* policy)
{
    Q_ASSERT(d);
    job()->assignQueuePolicy(policy);
}

bool QObjectJobDecorator::isFinished() const
{
    Q_ASSERT(d);
    return job()->isFinished();
}

bool QObjectJobDecorator::canBeExecuted(JobPointer self)
{
    Q_ASSERT(d);
    return job()->canBeExecuted(self);
}

void QObjectJobDecorator::aboutToBeQueued(QueueAPI *api)
{
    Q_ASSERT(d);
    job()->aboutToBeQueued(api);
}

void QObjectJobDecorator::aboutToBeQueued_locked(QueueAPI *api)
{
    Q_ASSERT(d);
    job()->aboutToBeQueued_locked(api);
}

void QObjectJobDecorator::aboutToBeDequeued(QueueAPI *api)
{
    Q_ASSERT(d);
    job()->aboutToBeDequeued(api);
}

void QObjectJobDecorator::aboutToBeDequeued_locked(QueueAPI* api)
{
    Q_ASSERT(d);
    job()->aboutToBeDequeued_locked(api);
}

void QObjectJobDecorator::requestAbort()
{
    Q_ASSERT(d);
    job()->requestAbort();
}

bool QObjectJobDecorator::success() const
{
    Q_ASSERT(d);
    return job()->success();
}

int QObjectJobDecorator::priority() const
{
    Q_ASSERT(d);
    return job()->priority();
}

Executor *QObjectJobDecorator::executor() const
{
    Q_ASSERT(d);
    return job()->executor();
}

Executor *QObjectJobDecorator::setExecutor(Executor *executor)
{
    Q_ASSERT(d);
    return job()->setExecutor(executor);
}

void QObjectJobDecorator::execute(ThreadWeaver::Thread* thread, ThreadWeaver::JobPointer self)
{
    Q_ASSERT(d);
    job()->execute(thread, self);
}

const ThreadWeaver::JobInterface* QObjectJobDecorator::job() const
{
    return reinterpret_cast<JobInterface*>(d);
}

JobInterface *QObjectJobDecorator::job()
{
    return reinterpret_cast<JobInterface*>(d);
}

const ThreadWeaver::JobCollection *ThreadWeaver::QObjectJobDecorator::collection() const
{
   return dynamic_cast<const JobCollection*>(job());
}

JobCollection *QObjectJobDecorator::collection()
{
    return dynamic_cast<JobCollection*>(job());
}

const JobSequence *QObjectJobDecorator::sequence() const
{
    return dynamic_cast<const JobSequence*>(job());
}

JobSequence *QObjectJobDecorator::sequence()
{
    return dynamic_cast<JobSequence*>(job());
}

}
