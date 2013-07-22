#include <QObjectJobDecorator.h>
#include <JobCollection.h>
#include <JobSequence.h>

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

void QObjectJobDecorator::defaultBegin(JobPointer job, Thread *thread)
{
    Q_ASSERT(d);
    reinterpret_cast<JobInterface*>(d)->defaultBegin(job, thread);
}

void QObjectJobDecorator::defaultEnd(JobPointer job, Thread *thread)
{
    Q_ASSERT(d);
    reinterpret_cast<JobInterface*>(d)->defaultEnd(job, thread);
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

void QObjectJobDecorator::execute(ThreadWeaver::Thread* thread, ThreadWeaver::JobPointer job)
{
    Q_ASSERT(d);
    Q_EMIT started(job);
    reinterpret_cast<JobInterface*>(d)->execute(thread, job);
    if (!job->success()) {
        Q_EMIT failed(job);
    }
    Q_EMIT done(job);

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
    const JobInterface* i = reinterpret_cast<JobInterface*>(d);
    return dynamic_cast<const JobCollection*>(i);
}

JobCollection *QObjectJobDecorator::collection()
{
    JobInterface* i = reinterpret_cast<JobInterface*>(d);
    return dynamic_cast<JobCollection*>(i);
}

const JobSequence *QObjectJobDecorator::sequence() const
{
    const JobInterface* i = reinterpret_cast<JobInterface*>(d);
    return dynamic_cast<const JobSequence*>(i);
}

JobSequence *QObjectJobDecorator::sequence()
{
    JobInterface* i = reinterpret_cast<JobInterface*>(d);
    return dynamic_cast<JobSequence*>(i);
}

}
