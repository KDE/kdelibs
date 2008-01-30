/* -*- C++ -*-

This file implements the WeaverImpl class.


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

$Id: WeaverImpl.cpp 30 2005-08-16 16:16:04Z mirko $

*/

#include "WeaverImpl.h"

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QDebug>

#include "Job.h"
#include "State.h"
#include "Thread.h"
#include "ThreadWeaver.h"
#include "DebuggingAids.h"
#include "WeaverObserver.h"
#include "SuspendedState.h"
#include "SuspendingState.h"
#include "DestructedState.h"
#include "WorkingHardState.h"
#include "ShuttingDownState.h"
#include "InConstructionState.h"

using namespace ThreadWeaver;

WeaverImpl::WeaverImpl( QObject* parent )
    : WeaverInterface(parent)
    , m_active(0)
    , m_inventoryMax( 4 )
    , m_mutex ( new QMutex( QMutex::Recursive ) )
    , m_finishMutex( new QMutex )
    , m_jobAvailableMutex ( new QMutex )
    , m_state (0)
{
    // initialize state objects:
    m_states[InConstruction] = new InConstructionState( this );
    setState ( InConstruction );
    m_states[WorkingHard] = new WorkingHardState( this );
    m_states[Suspending] = new SuspendingState( this );
    m_states[Suspended] = new SuspendedState( this );
    m_states[ShuttingDown] = new ShuttingDownState( this );
    m_states[Destructed] = new DestructedState( this );

    // FIXME (0.7) this is supposedly unnecessary
    connect ( this, SIGNAL ( asyncThreadSuspended( ThreadWeaver::Thread* ) ),
              SIGNAL ( threadSuspended( ThreadWeaver::Thread* ) ),
              Qt::QueuedConnection );
    setState(  WorkingHard );
}

WeaverImpl::~WeaverImpl()
{   // the constructor may only be called from the thread that owns this
    // object (everything else would be what we professionals call "insane")
    REQUIRE( QThread::currentThread() == thread() );
    debug ( 3, "WeaverImpl dtor: destroying inventory.\n" );
    setState ( ShuttingDown );

    m_jobAvailable.wakeAll();

    // problem: Some threads might not be asleep yet, just finding
    // out if a job is available. Those threads will suspend
    // waiting for their next job (a rare case, but not impossible).
    // Therefore, if we encounter a thread that has not exited, we
    // have to wake it again (which we do in the following for
    // loop).

    while (!m_inventory.isEmpty())
    {
        Thread* th=m_inventory.takeFirst();
        if ( !th->isFinished() )
	{
            for ( ;; )
	    {
                m_jobAvailable.wakeAll();
                if ( th->wait( 100 ) ) break;
                debug ( 1,  "WeaverImpl::~WeaverImpl: thread %i did not exit as expected, "
                        "retrying.\n", th->id() );
	    }
	}
        emit ( threadExited ( th ) );
        delete th;
    }

    m_inventory.clear();
    delete m_mutex;
    delete m_finishMutex;
    delete m_jobAvailableMutex;
    debug ( 3, "WeaverImpl dtor: done\n" );
    setState ( Destructed ); // m_state = Halted;
    // FIXME: delete state objects. what sense does DestructedState make then?
    // FIXME: make state objects static, since they are
}

void WeaverImpl::setState ( StateId id )
{
    if ( m_state==0 || m_state->stateId() != id )
    {
        m_state = m_states[id];
        debug ( 2, "WeaverImpl::setState: state changed to \"%s\".\n",
                m_state->stateName().toAscii().constData() );
        if ( id == Suspended )
	{
            emit ( suspended() );
	}

        m_state->activated();

        emit ( stateChanged ( m_state ) );
    }
}

const State& WeaverImpl::state() const
{
    return *m_state;
}

void WeaverImpl::setMaximumNumberOfThreads( int cap )
{
    Q_ASSERT_X ( cap > 0, "Weaver Impl", "Thread inventory size has to be larger than zero." );
    QMutexLocker l (m_mutex);
    m_inventoryMax = cap;
}

int WeaverImpl::maximumNumberOfThreads() const
{
    QMutexLocker l (m_mutex);
    return m_inventoryMax;
}

int WeaverImpl::currentNumberOfThreads () const
{
    QMutexLocker l (m_mutex);
    return m_inventory.count ();
}

void WeaverImpl::registerObserver ( WeaverObserver *ext )
{
    connect ( this,  SIGNAL ( stateChanged ( ThreadWeaver::State* ) ),
              ext,  SIGNAL ( weaverStateChanged ( ThreadWeaver::State* ) ) );
    connect ( this,  SIGNAL ( threadStarted ( ThreadWeaver::Thread* ) ),
              ext,  SIGNAL ( threadStarted ( ThreadWeaver::Thread* ) ) );
    connect ( this,  SIGNAL ( threadBusy( ThreadWeaver::Thread*, ThreadWeaver::Job* ) ),
              ext,  SIGNAL ( threadBusy ( ThreadWeaver::Thread*, ThreadWeaver::Job* ) ) );
    connect ( this,  SIGNAL ( threadSuspended ( ThreadWeaver::Thread* ) ),
              ext,  SIGNAL ( threadSuspended ( ThreadWeaver::Thread* ) ) );
    connect ( this,  SIGNAL ( threadExited ( ThreadWeaver::Thread* ) ) ,
              ext,  SIGNAL ( threadExited ( ThreadWeaver::Thread* ) ) );
}

void WeaverImpl::enqueue(Job* job)
{
    adjustInventory ( 1 );
    if (job)
    {
        debug ( 3, "WeaverImpl::enqueue: queueing job %p of type %s.\n",
                (void*)job, job->metaObject()->className() );
        QMutexLocker l (m_mutex);
        job->aboutToBeQueued ( this );
        // find positiEon for insertion:;
        // FIXME (after 0.6) optimize: factor out queue management into own class,
        // and use binary search for insertion (not done yet because
        // refactoring already planned):
        int i = m_assignments.size();
        if (i > 0)
	{
            while ( i > 0 && m_assignments.at(i - 1)->priority() < job->priority() ) --i;
            m_assignments.insert( i, (job) );
	} else {
            m_assignments.append (job);
	}
        assignJobs();
    }
}

void WeaverImpl::adjustInventory ( int numberOfNewJobs )
{
    QMutexLocker l (m_mutex);

    // no of threads that can be created:
    const int reserve = m_inventoryMax - m_inventory.count();

    if ( reserve > 0 )
    {
        for ( int i = 0; i < qMin ( reserve,  numberOfNewJobs ); ++i )
	{
            Thread *th = createThread();
            th->moveToThread( th ); // be sane from the start
            m_inventory.append(th);
            connect ( th,  SIGNAL ( jobStarted ( ThreadWeaver::Thread*, ThreadWeaver::Job* ) ),
                      SIGNAL ( threadBusy ( ThreadWeaver::Thread*, ThreadWeaver::Job* ) ) );
            connect ( th,  SIGNAL ( jobDone( ThreadWeaver::Job* ) ),
                      SIGNAL ( jobDone( ThreadWeaver::Job* ) ) );
            connect ( th,  SIGNAL ( started ( ThreadWeaver::Thread* ) ),
                      SIGNAL ( threadStarted ( ThreadWeaver::Thread* ) ) );

            th->start ();
            debug ( 2, "WeaverImpl::adjustInventory: thread created, "
                    "%i threads in inventory.\n", currentNumberOfThreads() );
	}
    }
}

Thread* WeaverImpl::createThread()
{
    return new Thread( this );
}

bool WeaverImpl::dequeue ( Job* job )
{
    bool result;
    {
        QMutexLocker l (m_mutex);

        int i = m_assignments.indexOf ( job );
        if ( i != -1 )
        {
            job->aboutToBeDequeued( this );

            m_assignments.removeAt( i );
            result = true;
            debug( 3, "WeaverImpl::dequeue: job %p dequeued, %i jobs left.\n",
                   (void*)job, m_assignments.size() );
        } else {
            debug( 3, "WeaverImpl::dequeue: job %p not found in queue.\n", (void*)job );
            result = false;
        }
    }

    // from the queues point of view, a job is just as finished if
    // it gets dequeued:
    m_jobFinished.wakeOne();
    return result;
}

void WeaverImpl::dequeue ()
{
    debug( 3, "WeaverImpl::dequeue: dequeueing all jobs.\n" );
    QMutexLocker l (m_mutex);
    for ( int index = 0; index < m_assignments.size(); ++index )
    {
        m_assignments.at( index )->aboutToBeDequeued( this );
    }
    m_assignments.clear();

    ENSURE ( m_assignments.isEmpty() );
}

void WeaverImpl::suspend ()
{
    m_state->suspend();
}

void WeaverImpl::resume ( )
{
    m_state->resume();
}

void WeaverImpl::assignJobs()
{
    m_jobAvailable.wakeAll();
}

bool WeaverImpl::isEmpty() const
{
    QMutexLocker l (m_mutex);
    return  m_assignments.isEmpty();
}


void WeaverImpl::incActiveThreadCount()
{
    adjustActiveThreadCount ( 1 );
}

void WeaverImpl::decActiveThreadCount()
{
    adjustActiveThreadCount ( -1 );
    // the done job could have freed another set of jobs, and we do not know how
    // many - therefore we need to wake all threads:
    m_jobFinished.wakeAll();
}

void WeaverImpl::adjustActiveThreadCount( int diff )
{
    QMutexLocker l (m_mutex);
    m_active += diff;
    debug ( 4, "WeaverImpl::adjustActiveThreadCount: %i active threads (%i jobs"
            " in queue).\n", m_active,  queueLength() );

    if ( m_assignments.isEmpty() && m_active == 0)
    {
        P_ASSERT ( diff < 0 ); // cannot reach Zero otherwise
        emit ( finished() );
    }
}

int WeaverImpl::activeThreadCount()
{
    QMutexLocker l (m_mutex);
    return m_active;
}

Job* WeaverImpl::takeFirstAvailableJob()
{
    QMutexLocker l (m_mutex);
    Job *next = 0;
    for (int index = 0; index < m_assignments.size(); ++index)
    {
        if ( m_assignments.at(index)->canBeExecuted() )
	{
            next = m_assignments.at(index);
            m_assignments.removeAt (index);
            break;
	}
    }
    return next;
}

Job* WeaverImpl::applyForWork(Thread *th, Job* previous)
{
    if (previous)
    {   // cleanup and send events:
        decActiveThreadCount();
    }
    return m_state->applyForWork ( th,  0 );
}

void WeaverImpl::waitForAvailableJob(Thread* th)
{
    m_state->waitForAvailableJob ( th );
}

void WeaverImpl::blockThreadUntilJobsAreBeingAssigned ( Thread *th )
{   // th is the thread that calls this method:
    Q_UNUSED ( th );
    debug ( 4,  "WeaverImpl::blockThread...: thread %i blocked.\n", th->id());
    emit asyncThreadSuspended ( th );
    QMutexLocker l( m_jobAvailableMutex );
    m_jobAvailable.wait( m_jobAvailableMutex );
    debug ( 4,  "WeaverImpl::blockThread...: thread %i resumed.\n", th->id());
}

int WeaverImpl::queueLength() const
{
    QMutexLocker l (m_mutex);
    return m_assignments.count();
}

bool WeaverImpl::isIdle () const
{
    QMutexLocker l (m_mutex);
    return isEmpty() && m_active == 0;
}

void WeaverImpl::finish()
{
#ifdef QT_NO_DEBUG
    const int MaxWaitMilliSeconds = 200;
#else
    const int MaxWaitMilliSeconds = 2000;
#endif

    while ( !isIdle() )
    {
        debug (2, "WeaverImpl::finish: not done, waiting.\n" );
        QMutexLocker l( m_finishMutex );
        if ( m_jobFinished.wait( m_finishMutex, MaxWaitMilliSeconds ) == false )
	{
            debug ( 2, "WeaverImpl::finish: wait timed out, %i jobs left, waking threads.\n",
                    queueLength() );
            m_jobAvailable.wakeAll();
	}
    }
    debug (2, "WeaverImpl::finish: done.\n\n\n" );
}

void WeaverImpl::requestAbort()
{
    QMutexLocker l (m_mutex);
    for ( int i = 0; i<m_inventory.size(); ++i )
    {
        m_inventory[i]->requestAbort();
    }
}

void WeaverImpl::dumpJobs()
{
    QMutexLocker l (m_mutex);
    debug( 0, "WeaverImpl::dumpJobs: current jobs:\n" );
    for ( int index = 0; index < m_assignments.size(); ++index )
    {
        debug( 0, "--> %4i: %p %s (priority %i)\n", index, (void*)m_assignments.at( index ),
               m_assignments.at( index )->metaObject()->className(),
               m_assignments.at(index)->priority() );
    }
}

#include "WeaverImpl.moc"
