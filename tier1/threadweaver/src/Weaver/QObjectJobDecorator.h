#ifndef QOBJECTJOBDECORATOR_H
#define QOBJECTJOBDECORATOR_H

#include <QtCore/QObject>

#include <threadweaver_export.h>
#include <QObject>
#include <JobInterface.h>

#include <qlist.h>
namespace ThreadWeaver {

class JobCollection;
class JobSequence;

class THREADWEAVER_EXPORT QObjectJobDecorator : public QObject, public JobInterface
{
    Q_OBJECT
public:
    explicit QObjectJobDecorator(JobInterface* job, QObject *parent = 0);
    ~QObjectJobDecorator();
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

    void execute(Thread*, JobPointer job) Q_DECL_OVERRIDE;
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

Q_SIGNALS:
    /** This signal is emitted when this job is being processed by a thread. */
    void started(ThreadWeaver::JobPointer);
    /** This signal is emitted when the job has been finished (no matter if it succeeded or not). */
    void done(ThreadWeaver::JobPointer);
    /** This job has failed.
     *
     * This signal is emitted when success() returns false after the job is executed. */
    void failed(ThreadWeaver::JobPointer);
    
protected:
    void freeQueuePolicyResources() Q_DECL_OVERRIDE;
    void run() Q_DECL_OVERRIDE;
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

#endif // QOBJECTJOBDECORATOR_H
