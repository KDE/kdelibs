#ifndef JOBINTERFACE_H
#define JOBINTERFACE_H

#include <QSharedPointer>

#include <threadweaver_export.h>
#include <JobPointer.h>

class QMutex;

namespace ThreadWeaver {

class Thread;
class Executor;
class QueueAPI;
class QueuePolicy;

class JobInterface;
typedef QSharedPointer<JobInterface> JobPointer;

class THREADWEAVER_EXPORT JobInterface {
public:
    virtual ~JobInterface() {}
    virtual void execute(JobPointer job, Thread*) = 0;
    virtual Executor* setExecutor(Executor* executor) = 0;
    virtual Executor* executor() const = 0;
    virtual int priority() const = 0;
    virtual bool success () const = 0;
    virtual void requestAbort() = 0;
    virtual void aboutToBeQueued(QueueAPI *api) = 0;
    virtual void aboutToBeQueued_locked(QueueAPI *api) = 0;
    virtual void aboutToBeDequeued(QueueAPI *api) = 0;
    virtual void aboutToBeDequeued_locked(QueueAPI *api) = 0;
    virtual bool isFinished() const = 0;
    virtual void assignQueuePolicy(QueuePolicy*) = 0;
    virtual void removeQueuePolicy(QueuePolicy*) = 0;
    virtual QList<QueuePolicy*> queuePolicies() const = 0;
    virtual void freeQueuePolicyResources(JobPointer) = 0;
    virtual void run(JobPointer self, Thread* thread) = 0;
    friend class Executor;
    virtual void defaultBegin(JobPointer job, Thread* thread) = 0;
    virtual void defaultEnd(JobPointer job, Thread* thread) = 0;
    virtual void setFinished(bool status) = 0;
    virtual QMutex* mutex() const = 0;
};

}

Q_DECLARE_METATYPE(ThreadWeaver::JobPointer)

#endif // JOBINTERFACE_H
