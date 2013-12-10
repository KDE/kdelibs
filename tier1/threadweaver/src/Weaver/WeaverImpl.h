/* -*- C++ -*-

This file implements the public interfaces of the WeaverImpl class.

$ Author: Mirko Boehm $
$ Copyright: (C) 2005-2013 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://creative-destruction.me $

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

$Id: WeaverImpl.h 32 2005-08-17 08:38:01Z mirko $
*/
#ifndef WeaverImpl_H
#define WeaverImpl_H

#include <QtCore/QObject>
#include <QtCore/QWaitCondition>
#include <QSharedPointer>
#include <QAtomicPointer>
#include <QAtomicInt>
#include <QSemaphore>
#include <QVector>

#ifndef THREADWEAVER_PRIVATE_API
#define THREADWEAVER_PRIVATE_API
#endif

#include "State.h"
#include "QueueAPI.h"

namespace ThreadWeaver {

class Job;
class Thread;
class WeaverObserver;
class WeaverImplState;
class SuspendingState;

/** A WeaverImpl is the manager of worker threads (Thread objects) to
        which it assigns jobs from its queue. It extends the API of
        WeaverInterface to provide additional methods needed by the Thread
        objects. */
class THREADWEAVER_EXPORT WeaverImpl : public QueueAPI
{
    Q_OBJECT
public:
    /** Construct a WeaverImpl object. */
    explicit WeaverImpl (QObject* parent=0 );
    /** Destruct a WeaverImpl object. */
    virtual ~WeaverImpl ();
    /** Enter Destructed state.
     * Once this method returns, it is save to delete this object. */
    void shutDown() Q_DECL_OVERRIDE;
    void shutDown_p() Q_DECL_OVERRIDE;

    const State* state() const Q_DECL_OVERRIDE;
    State* state() Q_DECL_OVERRIDE;

    void setMaximumNumberOfThreads(int cap) Q_DECL_OVERRIDE;
    int maximumNumberOfThreads() const Q_DECL_OVERRIDE;
    int currentNumberOfThreads () const Q_DECL_OVERRIDE;

    /** Set the object state. */
    void setState( StateId );
    void registerObserver(WeaverObserver*) Q_DECL_OVERRIDE;
    void enqueue(const QVector<JobPointer>& jobs) Q_DECL_OVERRIDE;
    bool dequeue(const JobPointer& job) Q_DECL_OVERRIDE;
    void dequeue() Q_DECL_OVERRIDE;
    void finish() Q_DECL_OVERRIDE;
    void suspend() Q_DECL_OVERRIDE;
    void resume() Q_DECL_OVERRIDE;
    bool isEmpty () const Q_DECL_OVERRIDE;
    bool isIdle () const Q_DECL_OVERRIDE;
    int queueLength () const Q_DECL_OVERRIDE;
    /** Assign a job to the calling thread.
        This is supposed to be called from the Thread objects in
        the inventory. Do not call this method from your code.

        Returns 0 if the weaver is shutting down, telling the
        calling thread to finish and exit.
        If no jobs are available and shut down is not in progress,
        the calling thread is suspended until either condition is
        met.
        In *previous*, threads give the job they have completed. If this is
        the first job, previous is zero. */
    virtual JobPointer applyForWork (Thread *thread, bool wasBusy) Q_DECL_OVERRIDE;
    /** Wait for a job to become available. */
    void waitForAvailableJob(Thread *th) Q_DECL_OVERRIDE;
    /** Blocks the calling thread until some actor calls assignJobs. */
    void blockThreadUntilJobsAreBeingAssigned(Thread* th);
    /** Blocks the calling thread until some actor calls assignJobs.
      * Mutex must be held when calling this method. */
    void blockThreadUntilJobsAreBeingAssigned_locked(Thread* th);
    /** Increment the count of active threads. */
    void incActiveThreadCount();
    /** Decrement the count of active threads. */
    void decActiveThreadCount();
    /** Returns the number of active threads.
     * Threads are active if they process a job.
     * Requires that the mutex is being held when called.
     */
    int activeThreadCount();

    /** Called from a new thread when entering the run method. */
    void threadEnteredRun(Thread* thread);
    /** Take the first available job out of the queue and return it.
     * The job will be removed from the queue (therefore, take). Only jobs that have no unresolved dependencies are considered
     * available. If only jobs that depened on other, unfinished jobs are in the queue, this method blocks on m_jobAvailable.
     * Go to suspended state if the active thread count is now zero and suspendIfAllThreadsInactive is true.
     * If justReturning is true, do not assign a new job, just process the completed previous one. */
    JobPointer takeFirstAvailableJobOrSuspendOrWait(Thread* th, bool threadWasBusy,
                                                    bool suspendIfAllThreadsInactive, bool justReturning);
    void requestAbort() Q_DECL_OVERRIDE;

    void reschedule() Q_DECL_OVERRIDE;

    /** Dump the current jobs to the console. Not part of the API. */
    void dumpJobs();

    //FIXME: rename _p to _locked:
    friend class WeaverImplState;
    friend class SuspendingState;
    void setState_p( StateId );
    void setMaximumNumberOfThreads_p(int cap) Q_DECL_OVERRIDE;
    int maximumNumberOfThreads_p() const Q_DECL_OVERRIDE;
    int currentNumberOfThreads_p() const Q_DECL_OVERRIDE;
    void registerObserver_p(WeaverObserver*) Q_DECL_OVERRIDE;
    void enqueue_p(const QVector<JobPointer>& jobs);
    bool dequeue_p(JobPointer job) Q_DECL_OVERRIDE;
    void dequeue_p() Q_DECL_OVERRIDE;
    void finish_p() Q_DECL_OVERRIDE;
    void suspend_p() Q_DECL_OVERRIDE;
    void resume_p() Q_DECL_OVERRIDE;
    bool isEmpty_p() const Q_DECL_OVERRIDE;
    bool isIdle_p() const Q_DECL_OVERRIDE;
    int queueLength_p() const Q_DECL_OVERRIDE;
    void requestAbort_p() Q_DECL_OVERRIDE;

Q_SIGNALS:
    /** A Thread has been created. */
    void threadStarted(ThreadWeaver::Thread*);
    /** A thread has exited. */
    void threadExited(ThreadWeaver::Thread*);
    /** A thread has been suspended. */
    void threadSuspended(ThreadWeaver::Thread*);
    /** The thread is busy executing job j. */
    void threadBusy(ThreadWeaver::JobPointer, ThreadWeaver::Thread*);

protected:
    /** Adjust active thread count.
            This is a helper function for incActiveThreadCount and decActiveThreadCount. */
    void adjustActiveThreadCount ( int diff );
    /** Factory method to create the threads.
            Overload in adapted Weaver implementations.
        */
    virtual Thread* createThread();
    /** Adjust the inventory size.
     * Requires that the mutex is being held when called
     *
     * This method creates threads on demand. Threads in the inventory
     * are not created upon construction of the WeaverImpl object, but
     * when jobs are queued. This avoids costly delays on the application
     * startup time. Threads are created when the inventory size is under
     * inventoryMin and new jobs are queued.
     */
    //TODO add code to raise inventory size over inventoryMin
    //TODO add code to quit unnecessary threads
    void adjustInventory ( int noOfNewJobs );
    /** Lock the mutex for this weaver. The threads in the
        inventory need to lock the weaver's mutex to synchronize
        the job management. */
private:
    /** Check with the assigned queue policies if the job can be executed.
     *
     * If it returns true, it expects that the job is executed right after that. The done() methods of the
     * queue policies will be automatically called when the job is finished.
     *
     * If it returns false, all queue policy resources have been freed, and the method can be called again
     * at a later time. */
    bool canBeExecuted(JobPointer);
    /** The thread inventory. */
    QList<Thread*> m_inventory;
    /** The job queue. */
    QList<JobPointer> m_assignments;
    /** The number of jobs that are assigned to the worker
        threads, but not finished. */
    int m_active;
    /** Stored setting . */
    int m_inventoryMax;
    /** Wait condition all idle or done threads wait for. */
    QWaitCondition m_jobAvailable;
    /** Wait for a job to finish. */
    QWaitCondition m_jobFinished;
    /** Mutex to serialize operations. */
    QMutex *m_mutex;
    /** Semaphore to ensure thread startup is  in sequence. */
    QSemaphore m_semaphore;
    /** Before shutdown can proceed to close the running threads, it needs to ensure that all of them
     *  entered the run method. */
    QAtomicInt m_createdThreads;
    /** The state of the art.
    * @see StateId
    */
    QAtomicPointer<State> m_state;
    /** The state objects. */
    QSharedPointer<State> m_states[NoOfStates];
};

} // namespace ThreadWeaver

#endif // WeaverImpl_H
