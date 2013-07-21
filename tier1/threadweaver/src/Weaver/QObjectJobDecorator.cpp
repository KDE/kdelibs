#include "QObjectJobDecorator.h"

namespace ThreadWeaver {

//Pssst: QObjectJobDecorator uses the d pointer to hold decoratee. It has no other member variables.
QObjectJobDecorator::QObjectJobDecorator(JobInterface *decoratee, QObject *parent)
    : QObject(parent)
    , d(reinterpret_cast<Private*>(decoratee))
{
}

QObjectJobDecorator::~QObjectJobDecorator()
{
    delete reinterpret_cast<JobInterface*>(d);
}

QMutex *QObjectJobDecorator::mutex() const
{
    Q_ASSERT(d);
    return reinterpret_cast<JobInterface*>(d)->mutex();
}

void QObjectJobDecorator::setFinished(bool status)
{
    Q_ASSERT(d);
    reinterpret_cast<JobInterface*>(d)->setFinished(status);
}

Thread *QObjectJobDecorator::thread()
{
    Q_ASSERT(d);
    return reinterpret_cast<JobInterface*>(d)->thread();
}

void QObjectJobDecorator::run()
{
    Q_ASSERT(d);
    reinterpret_cast<JobInterface*>(d)->run();
}

void QObjectJobDecorator::freeQueuePolicyResources()
{
    Q_ASSERT(d);
    reinterpret_cast<JobInterface*>(d)->freeQueuePolicyResources();
}

void QObjectJobDecorator::removeQueuePolicy(QueuePolicy* policy)
{
    Q_ASSERT(d);
    reinterpret_cast<JobInterface*>(d)->removeQueuePolicy(policy);
}

void QObjectJobDecorator::assignQueuePolicy(QueuePolicy* policy)
{
    Q_ASSERT(d);
    reinterpret_cast<JobInterface*>(d)->assignQueuePolicy(policy);
}

bool QObjectJobDecorator::isFinished() const
{
    Q_ASSERT(d);
    return reinterpret_cast<JobInterface*>(d)->isFinished();
}

bool QObjectJobDecorator::canBeExecuted()
{
    Q_ASSERT(d);
    return reinterpret_cast<JobInterface*>(d)->canBeExecuted();
}

void QObjectJobDecorator::aboutToBeQueued(QueueAPI *api)
{
    Q_ASSERT(d);
    reinterpret_cast<JobInterface*>(d)->aboutToBeQueued(api);
}

void QObjectJobDecorator::aboutToBeQueued_locked(QueueAPI *api)
{
    Q_ASSERT(d);
    reinterpret_cast<JobInterface*>(d)->aboutToBeQueued_locked(api);
}

void QObjectJobDecorator::aboutToBeDequeued(QueueAPI *api)
{
    Q_ASSERT(d);
    reinterpret_cast<JobInterface*>(d)->aboutToBeDequeued(api);
}

void QObjectJobDecorator::aboutToBeDequeued_locked(QueueAPI* api)
{
    Q_ASSERT(d);
    reinterpret_cast<JobInterface*>(d)->aboutToBeDequeued_locked(api);
}

void QObjectJobDecorator::requestAbort()
{
    Q_ASSERT(d);
    return reinterpret_cast<JobInterface*>(d)->requestAbort();
}

bool QObjectJobDecorator::success() const
{
    Q_ASSERT(d);
    return reinterpret_cast<JobInterface*>(d)->success();
}

int QObjectJobDecorator::priority() const
{
    Q_ASSERT(d);
    return reinterpret_cast<JobInterface*>(d)->priority();
}

Executor *QObjectJobDecorator::executor() const
{
    Q_ASSERT(d);
    return reinterpret_cast<JobInterface*>(d)->executor();
}

Executor *QObjectJobDecorator::setExecutor(Executor *executor)
{
    Q_ASSERT(d);
    return reinterpret_cast<JobInterface*>(d)->setExecutor(executor);
}

void ThreadWeaver::QObjectJobDecorator::execute(ThreadWeaver::Thread* thread, ThreadWeaver::JobPointer job)
{
    Q_ASSERT(d);
    reinterpret_cast<JobInterface*>(d)->execute(thread, job);
}

}
