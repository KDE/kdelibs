/* -*- C++ -*-

   This file implements the Job class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Id: Job.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#include <QMutex>
#include <QObject>
#include <QWaitCondition>

#include <DebuggingAids.h>
#include <Thread.h>

#include "Job.h"

namespace ThreadWeaver {

    QMultiMap<Job*, Job*> Job::sm_dep;
    QMutex *Job::sm_mutex;

    Job::Job (Job *dep, QObject* parent)
        : QObject (parent),
	  m_thread (0),
	  m_mutex (new QMutex (QMutex::NonRecursive) ),
          m_finished (false)
    {
        // initialize the process global mutex that protects the dependancy tracker:
	if (sm_mutex == 0)
	{
	    sm_mutex=new QMutex();
	}
        if ( dep != 0 )
        {
            if (dep->isFinished() == false) addDependancy (dep);
        }
    }

    Job::~Job()
    {
        resolveDependancies();
    }

    void Job::execute(Thread *th)
    {
	P_ASSERT (sm_dep.values(this).isEmpty());

	debug(3, "Job::execute: executing job in thread %i.\n", th->id());
        emit ( started ( this ) );
	m_mutex->lock();
	m_thread = th;
	m_mutex->unlock();

        run ();

	m_mutex->lock();
        m_thread = 0;
	setFinished (true);
	m_mutex->unlock();
        resolveDependancies(); // notify dependants
        emit ( done( this ) );
	debug(3, "Job::execute: finished execution of job in thread %i.\n", th->id());
    }

/*
    void Job::triggerSPR ()
    {
        m_mutex->lock(); // protect creation of m_wc
        QMutex mutex;
        m_wc = new QWaitCondition ();
        m_mutex->unlock();


        // thread()->post (ThreadWeaver::Event::JobSPR, this);
	mutex.lock();
        m_wc->wait( &mutex );
	mutex.unlock();
        delete m_wc;
        m_mutex->unlock();
    }

    void Job::triggerAPR ()
    {
	m_mutex->lock ();
        m_wcmutex = new QMutex;
	m_wc = new QWaitCondition;
	m_mutex->unlock ();

	// thread()->post (ThreadWeaver::Event::JobAPR, this);
	m_wc->wait ( m_wcmutex );
    }

    void Job::wakeAPR ()
    {
	QMutexLocker l(m_mutex);
	if ( m_wc!=0 )
	{
	    m_wc->wakeOne ();
	    delete m_wc;
            delete m_wcmutex;
	    m_wc = 0;
	}
    }
*/
    void Job::addDependancy (Job *dep)
    {   // if *this* depends on dep, *this* will be the key and dep the value:
	QMutexLocker l(sm_mutex);
	sm_dep.insert( this, dep );
    }

    bool Job::removeDependancy (Job* dep)
    {
	QMutexLocker l(sm_mutex);
	// there may be only one (!) occurence of [this, dep]:
	QMutableMapIterator<Job*, Job*> it(sm_dep);
	while ( it.hasNext() )
	{
	    it.next();
	    if ( it.key()==this && it.value()==dep )
	    {
		it.remove();
		return true;
	    }
	}
	return false;
    }

    bool Job::hasUnresolvedDependancies ()
    {
        QMutexLocker l(sm_mutex);
        return sm_dep.contains(this);
   }

    void Job::resolveDependancies ()
    {
        QMutexLocker l(sm_mutex);
//        debug ( 5, "Job::resolveDependancies: entered.\n" );
        QMutableMapIterator<Job*, Job*> it(sm_dep);
        while ( it.hasNext() )
        {   // we remove all entries where jobs depend on *this* :
            it.next();
            if ( it.value()==this )
            {
                it.remove();
            }
        }
//        debug ( 5, "Job::resolveDependancies: left.\n" );
    }

}

