/* -*- C++ -*-

   This file declares the WeaverInterface class.

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

   $Id: WeaverImpl.h 29 2005-08-14 19:04:30Z mirko $
*/

#ifndef WeaverInterface_H
#define WeaverInterface_H

#include <QtCore/QObject>

#include <threadweaver/threadweaver_export.h>

namespace ThreadWeaver {

    class Job;
    class State;
    class WeaverObserver;

    /** WeaverInterface provides a common interface for weaver implementations.

        In most cases, it is sufficient for an application to hold exactly one
        ThreadWeaver job queue. To execute jobs in a specific order, use job
        dependencies. To limit the number of jobs of a certain type that can
        be executed at the same time, use resource restrictions. To handle
        special requirements of the application when it comes to the order of
        execution of jobs, implement a special queue policy and apply it to
        the jobs.

        Users of the ThreadWeaver API are encouraged to program to this
        interface, instead of the implementation. This way, implementation
        changes will not affect user programs.

        This interface can be used for example to implement adapters and
        decorators. The member documentation is provided in the Weaver and
        WeaverImpl classes.
    */

    class THREADWEAVER_EXPORT WeaverInterface : public QObject {
        Q_OBJECT

    public:
        /** A ThreadWeaver object manages a queue of Jobs.
            It inherits QObject.
        */
        explicit WeaverInterface ( QObject* parent = 0 );
        virtual ~WeaverInterface() {}
        /** Return the state of the weaver object. */
        virtual const State& state() const = 0;

      /** Set the maximum number of threads this Weaver object may start. */
      virtual void setMaximumNumberOfThreads( int cap ) = 0;

      /** Get the maximum number of threads this Weaver may start. */
      virtual int maximumNumberOfThreads() const = 0;

      /** Returns the current number of threads in the inventory. */
      virtual int currentNumberOfThreads () const = 0;

        /** Register an observer.

            Observers provides signals on different weaver events that are
            otherwise only available through objects of different classes
            (threads, jobs). Usually, access to the signals of those objects
            is not provided through the weaver API. Use an observer to reveice
            notice, for example, on thread activity.

            To unregister, simply delete the observer.
         */
        virtual void registerObserver ( WeaverObserver* ) = 0;
        /** Add a job to be executed.

            It depends on the state if execution of the job will be attempted
            immidiately. In suspended state, jobs can be added to the queue,
            but the threads remain suspended. In WorkongHard state, an idle
            thread may immidiately execute the job, or it might be queued if
            all threads are busy.
        */
        virtual void enqueue ( Job* ) = 0;
        /** Remove a job from the queue.
            If the job was queued but not started so far, it is simply
            removed from the queue. For now, it is unsupported to
            dequeue a job once its execution has started.
	    For that case, you will have to provide a method to interrupt your
	    job's execution (and receive the done signal).
            Returns true if the job has been dequeued, false if the
            job has already been started or is not found in the
            queue. */
        virtual bool dequeue ( Job* ) = 0;
        /** Remove all queued jobs.
            Please note that this will not kill the threads, therefore
            all jobs that are being processed will be continued. */
        virtual void dequeue () = 0;
	/** Finish all queued operations, then return.

	    This method is used in imperative (not event driven) programs that
	    cannot react on events to have the controlling (main) thread wait
	    wait for the jobs to finish. The call will block the calling
	    thread and return when all queued jobs have been processed.

	    Warning: This will suspend your thread!
	    Warning: If one of your jobs enters an infinite loop, this
	             will never return! */
        virtual void finish () = 0;
        /** Suspend job execution.
            When suspending, all threads are allowed to finish the
            currently assigned job but will not receive a new
            assignment.
            When all threads are done processing the assigned job, the
            signal suspended will() be emitted.
            If you call suspend() and there are no jobs left to
            be done, you will immidiately receive the suspended()
            signal. */
        virtual void suspend () = 0;
        /** Resume job queueing.
            @see suspend
        */
        virtual void resume () = 0;
        /** Is the queue empty?
	    The queue is empty if no more jobs are queued. */
        virtual bool isEmpty () const = 0;
	/** Is the weaver idle?
	    The weaver is idle if no jobs are queued and no jobs are processed
            by the threads. */
        virtual bool isIdle () const = 0;
	/** Returns the number of pending jobs.
            This will return the number of queued jobs. Jobs that are
	    currently being executed are not part of the queue. All jobs in
	    the queue are waiting to be executed.
        */
        virtual int queueLength () const = 0;

        /** Request aborts of the currently executed jobs.
            It is important to understand that aborts are requested, but
	    cannot be guaranteed, as not all Job classes support it. It is up
	    to the application to decide if and how job aborts are
	    necessary. */
        virtual void requestAbort() = 0;

    Q_SIGNALS:
	/** This signal is emitted when the Weaver has finished ALL currently
	    queued jobs.
	    If a number of jobs is enqueued sequentially, this signal might be
	    emitted a couple of times (what happens is that all already queued
	    jobs have been processed while you still add new ones). This is
	    not a bug, but the intended behaviour. */
        void finished ();

        /** Thread queueing has been suspended.
            When suspend is called with state = true, all threads are
            allowed to finish their job. When the last thread
            finished, this signal is emitted. */
        void suspended ();

	/** This signal is emitted when a job is finished.  In addition,
	    the Job itself emits a done() signal just before the jobDone() signal
		is emitted.  
	 	*/

        void jobDone ( ThreadWeaver::Job* );
        // FIXME (0.7) test if signal is emitted properly
        // FIXME (0.7) provide jobStarted and jobFailed signals
        // FIXME (0.7) or remove all these, and add them to WeaverObserver

        /** The Weaver's state has changed. */
        void stateChanged ( ThreadWeaver::State* );
    };

}

#endif
