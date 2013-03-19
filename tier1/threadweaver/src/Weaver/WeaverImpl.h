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

#ifndef THREADWEAVER_PRIVATE_API
#define THREADWEAVER_PRIVATE_API
#endif

#include "State.h"
#include "QueueAPI.h"
#include <QSemaphore>

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
class WeaverImpl : public QueueAPI
{
    Q_OBJECT
public:
    /** Construct a WeaverImpl object. */
    explicit WeaverImpl (QObject* parent=0 );
    /** Destruct a WeaverImpl object. */
    virtual ~WeaverImpl ();
    /** Enter Destructed state.
     * Once this method returns, it is save to delete this object. */
    void shutDown();
    void shutDown_p();

    const State* state() const;
    State* state();

    void setMaximumNumberOfThreads( int cap );
    int maximumNumberOfThreads() const;
    int currentNumberOfThreads () const;

    /** Set the object state. */
    void setState( StateId );
    void registerObserver ( WeaverObserver* );
    void enqueue (Job*);
    bool dequeue (Job*);
    void dequeue ();
    void finish();
    void suspend( );
    void resume();
    bool isEmpty () const;
    bool isIdle () const;
    int queueLength () const;
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
    virtual Job* applyForWork (Thread *thread, Job *previous);
    /** Wait for a job to become available. */
    void waitForAvailableJob(Thread *th);
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
     * available. If only jobs that depened on other, unfinished jobs are in the queue, this method blocks on m_jobAvailable. */
    Job* takeFirstAvailableJobOrWait(Thread* th, Job* previous);
    /** Schedule enqueued jobs to be executed by idle threads.
     * This will try to distribute as many jobs as possible to all idle threads. */
    void assignJobs();
    void requestAbort();

    /** Dump the current jobs to the console. Not part of the API. */
    void dumpJobs();

    //FIXME: rename _p to _locked:
    friend class WeaverImplState;
    friend class SuspendingState;
    void setState_p( StateId );
    void setMaximumNumberOfThreads_p(int cap);
    int maximumNumberOfThreads_p() const;
    int currentNumberOfThreads_p() const;
    void registerObserver_p(WeaverObserver*);
    void enqueue_p(Job* job);
    bool dequeue_p(Job* job);
    void dequeue_p();
    void finish_p();
    void suspend_p( );
    void resume_p();
    bool isEmpty_p() const;
    bool isIdle_p() const;
    int queueLength_p() const;
    void requestAbort_p();

Q_SIGNALS:
    /** A Thread has been created. */
    void threadStarted ( ThreadWeaver::Thread* );
    /** A thread has exited. */
    void threadExited ( ThreadWeaver::Thread* );
    /** A thread has been suspended. */
    void threadSuspended ( ThreadWeaver::Thread* );
    /** The thread is busy executing job j. */
    void threadBusy ( ThreadWeaver::Thread*,  ThreadWeaver::Job* j);

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
    // 	void lock ();
    // 	/** Unlock. See lock(). */
    // 	void unlock ();
private:
    /** The thread inventory. */
    QList<Thread*> m_inventory;
    /** The job queue. */
    QList<Job*> m_assignments;
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
