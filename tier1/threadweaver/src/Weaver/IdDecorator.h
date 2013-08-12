#ifndef THREADWEAVER_IDDECORATOR_H
#define THREADWEAVER_IDDECORATOR_H

#include <QObject>

#include <threadweaver_export.h>
#include <JobInterface.h>

namespace ThreadWeaver {

class JobCollection;
class JobSequence;

/** @brief IdDecorator decorates a job without changing it's behaviour.
 *
 *  It is supposed to be used as the base class for actual decorators that do change the behaviour of jobs. */
class THREADWEAVER_EXPORT IdDecorator : public JobInterface
{
public:
    explicit IdDecorator(JobInterface* job);
    ~IdDecorator() Q_DECL_OVERRIDE;
    /** Retrieve the decorated job. */
    const JobInterface* job() const;
    /** Retrieve the decorated job. */
    JobInterface* job();
    /** Retrieve the decorated job as a JobCollection.
     *  If the decorated Job is not a JobCollection, 0 is returned. */
    const JobCollection* collection() const;
    /** Retrieve the decorated job as a JobCollection.
     *  If the decorated Job is not a JobCollection, 0 is returned. */
    JobCollection* collection();
    /** Retrieve the decorated job as a JobSequence.
     *  If the decorated Job is not a JobSequence, 0 is returned. */
    const JobSequence* sequence() const;
    /** Retrieve the decorated job as a JobSequence.
     *  If the decorated Job is not a JobSequence, 0 is returned. */
    JobSequence* sequence();

    void execute(JobPointer job, Thread*) Q_DECL_OVERRIDE;
    Executor* setExecutor(Executor* executor) Q_DECL_OVERRIDE;
    Executor* executor() const Q_DECL_OVERRIDE;
    int priority() const Q_DECL_OVERRIDE;
    bool success () const Q_DECL_OVERRIDE;
    void requestAbort() Q_DECL_OVERRIDE;
    void aboutToBeQueued(QueueAPI *api) Q_DECL_OVERRIDE;
    void aboutToBeQueued_locked(QueueAPI *api) Q_DECL_OVERRIDE;
    void aboutToBeDequeued(QueueAPI *api) Q_DECL_OVERRIDE;
    void aboutToBeDequeued_locked(QueueAPI *api) Q_DECL_OVERRIDE;
    bool canBeExecuted(JobPointer job) Q_DECL_OVERRIDE;
    bool isFinished() const Q_DECL_OVERRIDE;
    void assignQueuePolicy(QueuePolicy*) Q_DECL_OVERRIDE;
    void removeQueuePolicy(QueuePolicy*) Q_DECL_OVERRIDE;

protected:
    void freeQueuePolicyResources() Q_DECL_OVERRIDE;
    void run(JobPointer self, Thread* thread) Q_DECL_OVERRIDE;
    void defaultBegin(JobPointer job, Thread* thread) Q_DECL_OVERRIDE;
    void defaultEnd(JobPointer job, Thread* thread) Q_DECL_OVERRIDE;

    Thread* thread() Q_DECL_OVERRIDE;
    void setFinished(bool status) Q_DECL_OVERRIDE;
    QMutex* mutex() const Q_DECL_OVERRIDE;

private:
    class Private;
    Private* const d;
};

}

#endif // THREADWEAVER_IDDECORATOR_H
