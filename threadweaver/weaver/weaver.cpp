/* -*- C++ -*-

   This file implements the Weaver, Job and Thread classes.

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

extern "C" {
#include <signal.h>
}

#include <qevent.h>
#include <qapplication.h>

#include "weaver.h"

namespace ThreadWeaver {

    bool Debug = true;
    int DebugLevel = 2;

    Job::Job (QObject* parent, const char* name)
        : QObject (parent, name),
          m_finished (false),
	  m_mutex (new QMutex (true) ),
	  m_thread (0)
    {
    }

    Job::Job (Job *dep, QObject* parent, const char* name)
        : QObject (parent, name),
          m_finished (false),
	  m_mutex (new QMutex (true) ),
	  m_thread (0)
    {
        if (dep->isFinished() == false) addDependancy (dep);
    }

    Job::~Job()
    {
        resolveDependancies();
    }

    void Job::lock()
    {
        m_mutex->lock();
    }

    void Job::unlock()
    {
        m_mutex->unlock();
    }

    void Job::execute(Thread *th)
    {
	m_mutex->lock();
	m_thread = th;
	m_mutex->unlock();

        run ();

	m_mutex->lock();
        resolveDependancies();
        setFinished (true);
	m_thread = 0;
	m_mutex->unlock();
    }

    Thread *Job::thread ()
    {
        QMutexLocker l (m_mutex);
	return m_thread;
    }

    bool Job::isFinished() const
    {
        QMutexLocker l (m_mutex);
        return m_finished;
    }

    void Job::setFinished(bool status)
    {
        QMutexLocker l (m_mutex);
        m_finished = status;
    }

    void Job::processEvent (Event *e)
    {
	switch ( e->action() )
	{
	    case Event::JobStarted:
		emit ( started( this ) );
		break;
	    case Event::JobFinished:
		emit ( done( this ) );
		break;
	    case Event::JobSPR:
		emit ( SPR () );
		m_wc->wakeOne ();
		break;
	    case Event::JobAPR:
		emit ( APR () );
		//  no wake here !
		break;
	    default:
		break;
	}
    }

    void Job::triggerSPR ()
    {
	m_mutex->lock ();
	m_wc = new QWaitCondition;
	m_mutex->unlock ();

	thread()->post (ThreadWeaver::Event::JobSPR, this);
	m_wc->wait ();

	m_mutex->lock ();
	delete m_wc;
	m_wc = 0;
	m_mutex->unlock ();
    }

    void Job::triggerAPR ()
    {
	m_mutex->lock ();
	m_wc = new QWaitCondition;
	m_mutex->unlock ();

	thread()->post (ThreadWeaver::Event::JobAPR, this);
	m_wc->wait ();
    }

    void Job::wakeAPR ()
    {
	QMutexLocker l(m_mutex);
	if ( m_wc!=0 )
	{
	    m_wc->wakeOne ();
	    delete m_wc;
	    m_wc = 0;
	}
    }
// maybe use protected members for auto-add logic? :
    void Job::addDependancy (Job* dep, bool internal_auto)
    {
        QMutexLocker l(m_mutex);
        if (m_dependancies.contains (dep) == 0)
        {
            m_dependancies.append (dep);
        }
        if (internal_auto==false) dep->addDependant (this, true);
    }

    void Job::addDependant (Job* dep, bool internal_auto)
    {
        QMutexLocker l(m_mutex);
        if (m_dependants.contains (dep) == 0)
        {
            m_dependants.append (dep);
        }
        if (internal_auto==false) dep->addDependancy (this, true);
    }

    bool Job::removeDependancy (Job* dep, bool internal_auto)
    {
        QMutexLocker l(m_mutex);
        if (internal_auto==false) dep->removeDependant (this, true);
        return m_dependancies.remove (dep);
    }

    bool Job::removeDependant (Job* dep, bool internal_auto)
    {
        QMutexLocker l(m_mutex);
        if (internal_auto==false) dep->removeDependancy (this, true);
        return m_dependants.remove (dep);
    }

    bool Job::hasUnresolvedDependancies ()
    {
        QMutexLocker l(m_mutex);
        return m_dependancies.isEmpty() == false;
    }

    void Job::resolveDependancies ()
    {
        QMutexLocker l(m_mutex);
        while (!m_dependants.isEmpty())
        {
            removeDependant (m_dependants.first());
        }
    }

    const int Event::Type = QEvent::User + 1000;

    Event::Event ( Action action, Thread *thread, Job *job)
	: QCustomEvent ( type () ),
	  m_action (action),
	  m_thread (thread),
	  m_job (job)
    {
    }

    const int Event::type ()
    {
        return Type;
    }

    Thread* Event::thread ()
    {
	if ( m_thread != 0)
	{
	    return m_thread;
	} else {
	    return 0;
	}
    }

    Job* Event::job ()
    {
	return m_job;
    }

    Event::Action Event::action ()
    {
	return m_action;
    }

    unsigned int Thread::sm_Id;

    Thread::Thread (Weaver *parent)
        : QThread (),
          m_parent ( parent ),
          m_id ( makeId() )
    {
    }

    Thread::~Thread()
    {
    }

    unsigned int Thread::makeId()
    {
        static QMutex mutex;
        QMutexLocker l (&mutex);

        return ++sm_Id;
    }

    const unsigned int Thread::id()
    {
        return m_id;
    }

    void Thread::run()
    {
        Job *job = 0;

	post ( Event::ThreadStarted );

        while (true)
        {
	    debug ( 3, "Thread::run [%u]: trying to execute the next job.\n", id() );

            job = m_parent->applyForWork ( this, job );

            if (job == 0)
            {
                break;
            } else {
		post ( Event::JobStarted, job );
                job->execute (this);
		post ( Event::JobFinished, job );
            }
        }

	post (	Event::ThreadExiting );
    }

    void Thread::post (Event::Action a, Job *j)
    {
	m_parent->post ( a, this, j);
    }

    void Thread::msleep(unsigned long msec)
    {
        QThread::msleep(msec);
    }

    Weaver::Weaver(QObject* parent, const char* name,
                   int inventoryMin, int inventoryMax)
        : QObject(parent, name),
          m_active(0),
          m_inventoryMin(inventoryMin),
          m_inventoryMax(inventoryMax),
          m_shuttingDown(false),
          m_running (false),
          m_suspend (false),
          m_mutex ( new QMutex(true) )
    {
        lock();

        for ( int count = 0; count < m_inventoryMin; ++count)
        {
            Thread *th = new Thread(this);
            m_inventory.append(th);
	    // this will idle the thread, waiting for a job
            th->start();

	    emit (threadCreated (th) );
        }

        unlock();
    }

    Weaver::~Weaver()
    {
        lock();

        debug ( 1, "Weaver dtor: destroying inventory.\n" );

        m_shuttingDown = true;

        unlock();

        m_jobAvailable.wakeAll();

        // problem: Some threads might not be asleep yet, just finding
        // out if a job is available. Those threads will suspend
        // waiting for their next job (a rare case, but not impossible).
        // Therefore, if we encounter a thread that has not exited, we
        // have to wake it again (which we do in the following for
        // loop).

        for ( Thread *th = m_inventory.first(); th; th = m_inventory.next() )
        {
            if ( !th->finished() )
            {
                m_jobAvailable.wakeAll();
                th->wait();
            }

	    emit (threadDestroyed (th) );
            delete th;

        }

        m_inventory.clear();

        delete m_mutex;

	debug ( 1, "Weaver dtor: done\n" );

    }

    void Weaver::lock()
    {
	debug ( 3 , "Weaver::lock: lock (mutex is %s).\n",
		( m_mutex->locked() ? "locked" : "not locked" ) );
        m_mutex->lock();
    }

    void Weaver::unlock()
    {
        m_mutex->unlock();

	debug ( 3 , "Weaver::unlock: unlock (mutex is %s).\n",
		( m_mutex->locked() ? "locked" : "not locked" ) );
    }

    int Weaver::threads ()
    {
        QMutexLocker l (m_mutex);
        return m_inventory.count ();
    }

    void Weaver::enqueue(Job* job)
    {
        lock();

        m_assignments.append(job);
	m_running = true;

        unlock();

        assignJobs();
    }

    void Weaver::enqueue (QPtrList <Job> jobs)
    {
        lock();

        for ( Job * job = jobs.first(); job; job = jobs.next() )
        {
            m_assignments.append (job);
        }

        unlock();

        assignJobs();
    }

    bool Weaver::dequeue ( Job* job )
    {
        QMutexLocker l (m_mutex);
        return m_assignments.remove (job);
    }

    void Weaver::dequeue ()
    {
        QMutexLocker l (m_mutex);
        m_assignments.clear();
    }

    void Weaver::suspend (bool state)
    {
        lock();

        if (state)
        {
            // no need to wake any threads here
            m_suspend = true;
            if ( m_active == 0 && isEmpty() )
            {   //  instead of waking up threads:
                post (Event::Suspended);
            }
        } else {
            m_suspend = false;
            // make sure we emit suspended () even if all threads are sleeping:
            assignJobs ();
            debug (2, "Weaver::suspend: queueing resumed.\n" );
        }

        unlock();
    }

    void Weaver::assignJobs()
    {
        m_jobAvailable.wakeAll();
    }

    bool Weaver::event (QEvent *e )
    {
	if ( e->type() >= QEvent::User )
	{

            if ( e->type() == Event::type() )
            {
                Event *event = (Event*) e;

                switch (event->action() )
                {
		    case Event::JobFinished:
			if ( event->job() !=0 )
			{
			    emit (jobDone (event->job() ) );
			}
			break;
		    case Event::Finished:
			emit ( finished() );
			break;
		    case Event::Suspended:
			emit ( suspended() );
			break;
		    case Event::ThreadSuspended:
			if (!m_shuttingDown )
			{
			    emit (threadSuspended ( event->thread() ) );
			}
			break;
		    case Event::ThreadBusy:
			if (!m_shuttingDown )
			{
			    emit (threadBusy (event->thread() ) );
			}
			break;
		    default:
			break;
                }

                if ( event->job() !=0 )
                {
                    event->job()->processEvent (event);
                }
            } else {
                debug ( 0, "Weaver::event: Strange: received unknown user event.\n" );
	    }
	    return true;
	} else {
	    // others - please make sure we are a QObject!
	    return QObject::event ( e );
	}
    }

    void Weaver::post (Event::Action a, Thread* t, Job* j)
    {
	Event *e = new Event ( a, t, j);
	QApplication::postEvent (this, e);
    }

    bool Weaver::isEmpty()
    {
        QMutexLocker l (m_mutex);
        return  m_assignments.count()==0;
    }

    Job* Weaver::applyForWork(Thread *th, Job* previous)
    {
        Job *next = 0;
        bool lastjob = false;
        bool suspended = false;

        while (true)
        {
            lock();

            // find the first job without unresolved dependancies, which
            // is the next one to execute:
            for (next = m_assignments.first(); next;
                 next = m_assignments.next() )
            {
                if ( next->hasUnresolvedDependancies() == false )
                {
                    break;
                }
            }

            if (previous != 0)
            {   // cleanup and send events:
                --m_active;

                debug ( 3, "Weaver::applyForWork: job done, %i jobs left, "
                        "%i active jobs left.\n",
                        queueLength(), m_active );

                if ( m_active == 0 && isEmpty() )
                {
                    lastjob = true;
                    m_running = false;
                    post (Event::Finished);
                    debug ( 3, "Weaver::applyForWork: no more jobs without"
                            " dependancies, idling.\n" );
                }

                if (m_active == 0 && m_suspend == true)
                {
                    suspended = true;
                    post (Event::Suspended);
                    debug ( 2, "Weaver::applyForWork: queueing suspended.\n" );
                }

                m_jobFinished.wakeOne();
            }
            // ... still locked ...
            previous = 0;

            if (m_shuttingDown == true)
            {
                unlock();
                return 0;
            } else {
                if ( next !=0 && m_suspend == false )
                {
                    m_assignments.remove (next);
                    ++m_active;

		    debug ( 3, "Weaver::applyForWork: job assigned, "
			    "%i jobs in queue (%i active).\n",
			    m_assignments.count(), m_active );
                    unlock();

		    post (Event::ThreadBusy, th);

                    return next;
                } else {
                    unlock();
		    post (Event::ThreadSuspended, th);
                    m_jobAvailable.wait();
                }
            }
        }
    }

    int Weaver::queueLength()
    {
        QMutexLocker l (m_mutex);
        return m_assignments.count();
    }

    bool Weaver::isIdle ()
    {
        QMutexLocker l (m_mutex);
        return isEmpty() && m_active == 0;
    }

    void Weaver::finish()
    {
        while ( !isIdle() )
        {
            debug (2, "Weaver::finish: not done, waiting.\n" );
            m_jobFinished.wait();
        }
	debug (1, "Weaver::finish: done.\n\n\n" );
    }

}

// #include "weaver.moc"
