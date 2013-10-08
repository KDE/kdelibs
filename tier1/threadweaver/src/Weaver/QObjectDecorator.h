#ifndef THREADWEAVER_QOBJECTDECORATOR_H
#define THREADWEAVER_QOBJECTDECORATOR_H

#include <QtCore/QObject>

#include <threadweaver_export.h>
#include <IdDecorator.h>

namespace ThreadWeaver {

class JobCollection;
class JobSequence;

class THREADWEAVER_EXPORT QObjectDecorator : public QObject, public IdDecorator
{
    Q_OBJECT
public:
    explicit QObjectDecorator(JobInterface* decoratee, QObject *parent = 0);
    explicit QObjectDecorator(JobInterface* decoratee, bool autoDelete, QObject *parent = 0);

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
    void defaultBegin(JobPointer job, Thread* thread) Q_DECL_OVERRIDE;
    void defaultEnd(JobPointer job, Thread* thread) Q_DECL_OVERRIDE;
};

typedef QSharedPointer<QObjectDecorator> QJobPointer;

}

#endif // THREADWEAVER_QOBJECTDECORATOR_H
