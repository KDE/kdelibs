/* -*- C++ -*-

This file implements the public interfaces of the WeaverImpl class.

$ Author: Mirko Boehm $
$ Copyright: (C) 2005, 2006 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://www.hackerbuero.org $

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

#ifndef THREADWEAVER_PRIVATE_API
#define THREADWEAVER_PRIVATE_API
#endif

#include "State.h"
#include "WeaverInterface.h"

namespace ThreadWeaver {

    class Job;
    class Thread;
    class WeaverObserver;

    /** A WeaverImpl is the manager of worker threads (Thread objects) to
        which it assigns jobs from its queue. It extends the API of
        WeaverInterface to provide additional methods needed by the Thread
        objects. */
    class WeaverImpl : public WeaverInterface
    {
        Q_OBJECT
    public:
	/** Construct a WeaverImpl object. */
        explicit WeaverImpl (QObject* parent=0 );
	/** Destruct a WeaverImpl object. */
        virtual ~WeaverImpl ();
	const State& state() const;

        void setMaximumNumberOfThreads( int cap );
        int maximumNumberOfThreads() const;
        int currentNumberOfThreads () const;


        /** Set the object state. */
        void setState( StateId );
        void registerObserver ( WeaverObserver* );
        virtual void enqueue (Job*);
        virtual bool dequeue (Job*);
        virtual void dequeue ();
	virtual void finish();
        virtual void suspend( );
        virtual void resume();
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
        /** Blocks the calling thread until some actor calls assignJobs. */
        void blockThreadUntilJobsAreBeingAssigned(Thread *th);
        /** Wait for a job to become available. */
	void waitForAvailableJob(Thread *th);
        /** Increment the count of active threads. */
        void incActiveThreadCount();
        /** Decrement the count of active threads. */
        void decActiveThreadCount();
        /** Returns the number of active threads.
            Threads are active if they process a job.
        */
        int activeThreadCount();
        /** Take the first available job out of the queue and return it.
            The job will be removed from the queue (therefore, take).
            Only jobs that have no unresolved dependencies are considered
	    available. If only jobs that depened on other, unfinished jobs are
	    in the queue, this method returns a nil pointer. */
        Job* takeFirstAvailableJob();
        /** Schedule enqueued jobs to be executed by idle threads.
            This will try to distribute as many jobs as possible
            to all idle threads. */
        void assignJobs();
        void requestAbort();

        /** Dump the current jobs to the console. Not part of the API. */
        void dumpJobs();

    Q_SIGNALS:
        /** A Thread has been created. */
        void threadStarted ( ThreadWeaver::Thread* );
        /** A thread has exited. */
        void threadExited ( ThreadWeaver::Thread* );
        /** A thread has been suspended. */
        void threadSuspended ( ThreadWeaver::Thread* );
        /** The thread is busy executing job j. */
        void threadBusy ( ThreadWeaver::Thread*,  ThreadWeaver::Job* j);

        // FIXME (0.7) this seems to be unnecessary
        // some more private Q_SIGNALS: There are situations where other threads
        // call functions of (this). In this case, there may be confusion
        // about whether to handle the signals synchronously or not. The
        // following signals are asynchroneoulsy connected to their siblings.
        void asyncThreadSuspended( ThreadWeaver::Thread* );

    protected:
        /** Adjust active thread count.
            This is a helper function for incActiveThreadCount and decActiveThreadCount. */
        void adjustActiveThreadCount ( int diff );
        /** Factory method to create the threads.
            Overload in adapted Weaver implementations.
        */
        virtual Thread* createThread();
        /** Adjust the inventory size.

        This method creates threads on demand. Threads in the inventory
        are not created upon construction of the WeaverImpl object, but
        when jobs are queued. This avoids costly delays on the application
        startup time. Threads are created when the inventory size is under
        inventoryMin and new jobs are queued.
        */
        // @TODO: add code to raise inventory size over inventoryMin
        // @TODO: add code to quit unnecessary threads
        void adjustInventory ( int noOfNewJobs );
	/** Lock the mutex for this weaver. The threads in the
	    inventory need to lock the weaver's mutex to synchronize
	    the job management. */
// 	void lock ();
// 	/** Unlock. See lock(). */
// 	void unlock ();
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

    private:
	/** Mutex to serialize operations. */
	QMutex *m_mutex;

        /** Non-recursive mutex to serialize calls to finish(). */
        QMutex* m_finishMutex;

        /** Mutex used by m_jobAvailable wait condition. */
        QMutex* m_jobAvailableMutex;

        // @TODO: make state objects static
	/** The state of the art.
         * @see StateId
         */
	State*  m_state;
        /** The state objects. */
        State *m_states[NoOfStates];
    };

} // namespace ThreadWeaver

#endif // WeaverImpl_H
