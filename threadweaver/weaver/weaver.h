/* -*- C++ -*-

   This file declares the Weaver, Job and Thread classes.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Id$
*/

#ifndef THUD_WEAVER_H
#define THUD_WEAVER_H

extern "C"
{
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
}

#include <qobject.h>
#include <qptrlist.h>
#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>
#include <qevent.h>

namespace ThreadWeaver {

    /** This method prints a text message on the screen, if debugging is
	enabled. Otherwise, it does nothing. The message is thread safe,
	therefore providing that the messages appear in the order they where
	issued by the different threads.
	All messages are suppressed when Debug is false. All messages with a
	lower importance (higher number) than DebugLevel will be suppressed,
	too. Debug level 0 messages will always be printed as long as
	Debug is true.
        We use our own debugging method, since debugging threads is a more
        complicated experience than debugging single threaded
        contexts. This might change in future in the way that debug
        prints it's messages to another logging facility provided by
        the platform.
	Use setDebugLevel () to integrate adapt debug () to your platform.
    */

    extern bool Debug;
    extern int DebugLevel;

    inline void setDebugLevel (bool debug, int level)
        {
	    Debug = debug;
	    DebugLevel = level;
	}

    inline void debug(int severity, const char * cformat, ...)
#ifdef __GNUC__
	__attribute__ ( (format (printf, 2, 3 ) ) )
#endif
;

    inline void debug(int severity, const char * cformat, ...)
    {
	if ( Debug == true && ( severity<=DebugLevel || severity == 0) )
	{
	    static QMutex mutex;
	    QString text;

	    mutex.lock();
	    va_list ap;
	    va_start( ap, cformat );
	    vprintf (cformat, ap);
	    va_end (ap);
	    mutex.unlock();
	}
    }


    class Thread;
    class Job;

    /** A class to represent the events threads generate and send to the
	Weaver object. Examples include the start or end of the processing of a
	job. Threads create the event objects and discard them after posting
	the event, since the event receiver will assume ownership of the
	event.
	Events are associated to the sending thread and possibly to a
	processed job.

	Note: Do not create and use SPR/APR events, use Job::triggerSPR or
	Job::triggerAPR to create the requests. */

    class Event : public QCustomEvent
    {
    public:
	enum Action {
	    NoAction = 0,
            /// All jobs in the queue are done.
	    Finished,
            /// Thread queueing halted.
            Suspended,
            /// A thread started.
	    ThreadStarted,
            /// A thread exited.
	    ThreadExiting,
            /// A thread executes a job.
	    ThreadBusy,
            /// A thread has beend suspended.
	    ThreadSuspended,
            ///  A job has been started.
	    JobStarted,
            /// A job has been finished.
	    JobFinished,
            /// Synchronous Process Request
	    JobSPR,
            /// Asynchronous Process Request
	    JobAPR
	};
	Event ( Action = NoAction, Thread * = 0, Job *job = 0);
	/** Return the (custom defined) event type. */
	static const int type ();
	/** The ID of the sender thread. */
	Thread* thread();
	/** The associated job. */
	Job* job ();
	/** The action. */
	Action action ();
    private:
	Action m_action;
	Thread *m_thread;
	Job *m_job;
	static const int Type;
    };

    /** A Job is a simple abstraction of an action that is to be
        executed in a thread context.
	It is essential for the ThreadWeaver library that as a kind of
        convention, the different creators of Job objects do not touch the
        protected data members of the Job until somehow notified by the
        Job. See the SPR signal for an example.

	Jobs may emit process requests as signals. Consider process requests
	as a kind of synchronized call to the main thread.
	Process Requests are a generic means for Job derivate programmers to have
	the jobs interact with the creators (in the main thread) during
	processing time. To avoid race
	conditions and extensive locking and unlocking, the thread executing the
	job is suspended during the period needed to process the request.

	There are two kinds of process requests (we introduce abbreviations,
	also in the signal names and the code,
	only to save typing). Both are emitted by signals in the main thread:
	- Synchronous Process Requests (SPR): Synchronous requests expect that the
	complete request is performed in the slots connected to the signals. For
	example, to update a widget according to the progress of the job, a SPR
	may be used. In such cases, the Job's execution will be resumed
	immediately after the signal has been processed.
	- Asynchronous Process Requests (APR): For APRs, the job emitting the
	signal does not assume anything about the amount of time needed to
	perform the operation. Therefore, the thread is not waked after the
	signal returns. The creator has to wake to thread whenever it is
	ready by calling the wakeAPR method.

	Note: When using an APR, you better make sure to receive the signal
	with some object, otherwise the calling thread will block forever!
    */
    class Job : public QObject
    {
        Q_OBJECT
    public:
	/** Construct a Job object. */
        Job (QObject* parent=0, const char* name=0);

        /** Construct a Job object which depends on dep.
            dep will be considered a dependancy if it is not finished
            yet. Otherwise, no dependancy will be added.
         */
        Job (Job* dep, QObject* parent=0, const char* name=0);

	/** Destructor. */
        virtual ~Job();

	/** Perform the job. The thread in which this job is executed
	    is given as a parameter.
            Do not overload this method to create your own Job
            implementation, overload run(). */
        virtual void execute(Thread*);

        /** Returns true if the jobs's execute method finished.
            Thread Safe. */
        virtual bool isFinished() const;

	/** Wake the thread after an APR has been processed. */
	void wakeAPR ();

	/** Process events related to this job (created by the processing
	    thread or the weaver or whoever). */
	virtual void processEvent ( Event* );

        /** Add a dependancy.
            The object given will be added as a dependancy. The Job will not
            be executed until all dependancies have been processed.
            The job is automatically added to the dependancy as a dependant.

            Note: Adding the same dependancy more than once will be ignored.

            Note: Ignore internal_auto.
        */
        void addDependancy (Job*, bool internal_auto = false);

        /** Add a depandant.
            The object given will be added as a dependant. When the execution
            of the job finished, the dependants will be notified.
            The job is automatically added to the dependant as a dependancy.

            Note: Adding the same dependant more than once will be ignored.

            Note: Ignore internal_auto.
        */
        void addDependant (Job*, bool internal_auto = false);

        /** Remove dependancy.
            The given dependancy will be removed. If none are left, the job
            will be executed as soon as a waiting thread is available.
            The job will automatically be removed as a dependant of dep.

            Returns false if the given object is not dependancy of this job.

            Note: Ignore internal_auto.
        */
        bool removeDependancy (Job *dep, bool internal_auto = false);

        /** Remove dependant.
            The given dependant will be removed. This might happen if a job
            object is deleted before it is executed.
            The job will automatically be removed as a dependancy of dep.

            Returns false if the given object is not dependant of this job.

            Note: Ignore internal_auto.
        */
        bool removeDependant (Job* dep, bool internal_auto = false);

        /** Query whether the job has an unresolved dependancy.
            In case it does, it will not be processed by a thread trying to
            request a job. */
        bool hasUnresolvedDependancies();

    signals:
	/** This signal is emitted when a thread starts to process a job. */
	void started ( Job* );
	/** This signal is emitted when a job has been finished. */
	void done ( Job* );
	/** This signal is emitted when the job needs some operation done by
	    the main thread (usually the creator of the job).
	    It is important to understand that the emitting thread is
	    suspended until the signal returns.
	    When
	    the operation requested has been performed and this signal is
	    finished, the thread is automatically waked.
	    What operation needs to be performed has to be negotiated between
	    the two objects.
	    Note: This signal is an attempt to provide job programmers with a
	    generic way to interact while the job is executed. I am interested
	    in feedback about it's use. */
	void SPR ();
	/** Perform an Asynchronous Process Request. See SPR and the generic
	    Job documentation for a comparison. */
	void APR ();
    protected:
        /** Lock this Job's mutex. */
        void lock();
        /** Unlock this Job's mutex. */
        void unlock();
        /** The method that actually performs the job. It is called from
            execute(). This method is the one to overload it with the
            job's task. */
        virtual void run () = 0;
	/** Return the thread that executes this job.
	    Returns zero of the job is not currently executed. */
	Thread *thread();
	/** Call with status = true to mark this job as done. */
	virtual void setFinished(bool status);
	/** Trigger a SPR.
	    This emits a signal in the main thread indicating the necessity of
	    a synchronized operation. */
	void triggerSPR ();
	/** Trigger an APR.
	    This emit a signal in the main thread indicating the necessity of
	    an unsynchronized operation.
	    The calling thread needs to ensure to wake the thread when the
	    operation is done. */
	void triggerAPR ();

        /** Resolve all dependancies.
            This method needs to be called after the Job has been finished, or
            when it is deleted without being executed (performed by the
            destructor.
            The method will notify all dependants that the job has
            finished.
        */
        void resolveDependancies();

        bool m_finished;

	QMutex *m_mutex;

	Thread * m_thread;

	QWaitCondition *m_wc;

        /** A list of Job objects which DEPEND ON THIS job. */
        QPtrList<Job> m_dependants;

        /** A list of Job object which THIS job DEPENDS on. */
        QPtrList<Job> m_dependancies;
    };

    class Weaver;

    /** The class Thread is used to represent the worker threads in
        the weaver's inventory. It is not meant to be overloaded. */
    class Thread : public QThread
    {
    public:
	/** Create a thread.
	    These thread objects are only used inside the Weaver parent
	    object. */
        Thread(Weaver *parent);

	/** The destructor. */
        ~Thread();

        /** Overloaded to execute the assigned job.
	    This will NOT return until shutdown() is called. The
	    thread will try to execute one job after the other, asking
	    the Weaver parent for a new job when the assigned one is
	    finished.
            If no jobs are available, the thread will suspend.
	    After shutdown() is called, the thread will end as soon as
	    the currently assigned job is done.
	*/
        void run();

	/* Provide the msleep() method (protected in QThread) to be
	   available  for executed jobs. */
	void msleep(unsigned long msec);

	/** Returns the thread id.
	    This id marks the respective Thread object, and must
	    therefore not be confused with, e.g., the pthread thread
	    ID. */
	const unsigned int id();

	/** Post an event, will be received and processed by the Weaver. */
	void post (Event::Action, Job* = 0);

    private:
        Weaver *m_parent;

	const unsigned int m_id;

	static unsigned int sm_Id;

	static unsigned int makeId();
    };

    /** A weaver is the manager of worker threads (Thread objects) to
        which it assigns jobs from it's queue. */
    class Weaver : public QObject
    {
        Q_OBJECT
    public:
        Weaver (QObject* parent=0, const char* name=0,
                int inventoryMin = 4, // minimal number of provided threads
                int inventoryMax = 32); // maximum number of provided threads
        virtual ~Weaver ();
        /** Add a job to be executed. */
        virtual void enqueue (Job*);
	/** Enqueue all jobs in the given list.
	    This is an atomic operation, no jobs will start
	    before all jobs in the list are enqueued.
	    If you need a couple of jobs done and want to receive the
	    finished () signal afterwards, use this method to queue
	    them. Otherwise, when enqueueing your jobs
	    individually, there is a chance that you receive more than
	    one finished signal. */
	void enqueue (QPtrList<Job> jobs);
        /** Remove a job from the queue.
            If the job qas queued but not started so far, it is simple
            removed from the queue. For now, it is unsupported to
            dequeue a job once its execution has started.
	    For that case, you will have to provide a method to interrupt your
	    job's execution (and receive the done signal).
            Returns true if the job has been dequeued, false if the
            job has already been started or is not found in the
            queue. */
        virtual bool dequeue (Job*);
        /** Remove all queued jobs.
            Please note that this will not kill the threads, therefore
            all jobs that are being processed will be continued. */
        virtual void dequeue ();
        /** Get notified when a thread has finished a job.
            This is done automatically. */
        // virtual void jobFinished(Thread *);
	/** Finish all queued operations, then return.
	    This method is used in imperative programs that cannot react on
            events to have the controlling (main) thread wait wait for the
            jobs to finish.
	    Warning: This will suspend your thread!
	    Warning: If your jobs enter for example an infinite loop, this
	             will never return! */
	virtual void finish();
        /** Suspend job execution if state = true, otherwise resume
            job execution if it was suspended.
            When suspending, all threads are allowed to finish the
            currently assigned job but will not receive a new
            assignment.
            When all threads are done processing the assigned job, the
            signal suspended will() be emitted.
            If you call suspend (true) and there are no jobs left to
            be done, you will immidiately receive the suspended()
            signal. */
        virtual void suspend (bool state);
        /** Is the queue empty? */
        bool isEmpty ();
	/** Is the weaver idle?
	    The weaver is idle if no jobs are queued and no jobs are processed
            by the threads (m_active is zero). */
	bool isIdle ();
	/** Returns the number of pending jobs. */
	int queueLength ();
        /** Assign a job to the calling thread.
	    This is supposed to be called from the Thread objects in
	    the inventory.
	    Returns 0 if the weaver is shutting down, telling the
	    calling thread to finish and exit.
            If no jobs are available and shut down is not in progress,
            the calling thread is suspended until either condition is
            met.
	    In previous, threads give the job they have completed. If this is
	    the first job, previous is zero. */
        virtual Job* applyForWork (Thread *thread, Job *previous);
	/** Lock the mutex for this weaver. The threads in the
	    inventory need to lock the weaver's mutex to synchronize
	    the job management. */
	void lock ();
	/** Unlock. See lock(). */
	void unlock ();
	/** Post an event that is handled by this object, but in the main
	    (GUI) thread. Different threads may use this method to communicate
	    with the main thread.
	    thread and job mark the objects associated with this event. */
       	void post (Event::Action, Thread* = 0, Job* = 0);
	/** Returns the current number of threads in the inventory. */
	int threads ();
    signals:
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
	/** This signal is emitted when a job is done. It is up to the
	    programmer if this signal or the done signal of the job is more
	    handy. */
	void jobDone (Job*);
// The following signals are used mainly for debugging purposes.
	void threadCreated (Thread *);
	void threadDestroyed (Thread *);
	void threadBusy (Thread *);
	void threadSuspended (Thread *);

    protected:
        /** Schedule enqueued jobs to be executed by idle threads.
            This will try to distribute as many jobs as possible
            to all idle threads. */
        void assignJobs();
	/** Check incoming events for user defined ones. The threads use user
	    defined events to communicate with the Weaver. */
	bool event ( QEvent* );
        /** The thread inventory. */
        QPtrList<Thread> m_inventory;
        /** The job queue. */
        QPtrList<Job> m_assignments;
	/** The number of jobs that are assigned to the worker
	    threads, but not finished. */
	int m_active;
        /** Stored setting. */
        int m_inventoryMin;
        /** Stored setting . */
        int m_inventoryMax;
        /** Wait condition all idle or done threads wait for. */
        QWaitCondition m_jobAvailable;
	/** Wait for a job to finish. */
	QWaitCondition m_jobFinished;
        /** Indicates if the weaver is shutting down and exiting it's
            threads. */
        bool m_shuttingDown;
	/** m_running is set to true when a job is enqueued and set to false
	    when the job finishes that was the last in the queue.
	    E.g., this will flip from false to true to false when you
	    continuously enqueue one single job. */
	bool m_running;
        /** If m_suspend is true, no new jobs will be assigned to
            threads.
	    Jobs may be queued, but will not be processed until suspend
	    (false) is called. */
        bool m_suspend;
    private:
	/** Mutex to serialize operations. */
	QMutex *m_mutex;
    };
} // namespace ThreadWeaver

#endif // defined THUD_WEAVER_H
