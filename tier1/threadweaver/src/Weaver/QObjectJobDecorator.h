#ifndef QOBJECTJOBDECORATOR_H
#define QOBJECTJOBDECORATOR_H

#include <QtCore/QObject>

#include <threadweaver_export.h>
#include <QObject>
#include <JobInterface.h>

namespace ThreadWeaver {

class THREADWEAVER_EXPORT QObjectJobDecorator : public QObject, public JobInterface
{
    Q_OBJECT
public:
    explicit QObjectJobDecorator(JobInterface* decoratee, QObject *parent = 0);
    ~QObjectJobDecorator();
    const JobInterface* decoratee() const;
    JobInterface* decoratee();
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
    bool canBeExecuted() Q_DECL_OVERRIDE;
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
