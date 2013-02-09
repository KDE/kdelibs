#ifndef QUEUEAPI_H
#define QUEUEAPI_H

#include "Queue.h"
#include "QueueInterface.h"
#include "State.h"

namespace ThreadWeaver {

class QueueAPI : public Queue, public QueueInterface
{
    Q_OBJECT

public:
    explicit QueueAPI(QObject *parent = 0);
    
    virtual void setState_p( StateId ) = 0;
    virtual const State& state_p() const = 0;
    virtual void setMaximumNumberOfThreads_p(int cap) = 0;
    virtual int maximumNumberOfThreads_p() const = 0;
    virtual int currentNumberOfThreads_p() const = 0;
    virtual void registerObserver_p(WeaverObserver*) = 0;
    virtual void enqueue_p(Job* job) = 0;
    virtual bool dequeue_p(Job* job) = 0;
    virtual void dequeue_p() = 0;
    virtual void finish_p() = 0;
    virtual void suspend_p() = 0;
    virtual void resume_p() = 0;
    virtual bool isEmpty_p() const = 0;
    virtual bool isIdle_p() const = 0;
    virtual int queueLength_p() const = 0;
    virtual void requestAbort_p() = 0;
};

}

#endif // QUEUEAPI_H
