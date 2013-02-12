/* -*- C++ -*-

This file implements the WeaverImpl class.


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
    : QueueAPI(parent)
    , m_active(0)
    , m_inventoryMax( qMax(4, 2 * QThread::idealThreadCount() ) )
    , m_mutex ( new QMutex( QMutex::NonRecursive ) )
    , m_state (0)
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    // initialize state objects:
    m_states[InConstruction] = QSharedPointer<State>(new InConstructionState(this));
    setState_p(InConstruction);
    m_states[WorkingHard] = QSharedPointer<State>(new WorkingHardState(this));
    m_states[Suspending] = QSharedPointer<State>(new SuspendingState(this));
    m_states[Suspended] = QSharedPointer<State>(new SuspendedState(this));
    m_states[ShuttingDown] = QSharedPointer<State>(new ShuttingDownState(this));
    m_states[Destructed] = QSharedPointer<State>(new DestructedState(this));
    setState_p(  WorkingHard );
}

WeaverImpl::~WeaverImpl()
{
    Q_ASSERT_X(state().stateId() == Destructed, Q_FUNC_INFO, "shutDown() method was not called before WeaverImpl destructor!");
    delete m_mutex;
}

void WeaverImpl::shutDown()
{
    m_state->shutDown();
}

void WeaverImpl::shutDown_p()
{
    // the constructor may only be called from the thread that owns this
    // object (everything else would be what we professionals call "insane")
    REQUIRE( QThread::currentThread() == thread() );
    debug ( 3, "WeaverImpl::shutDown: destroying inventory.\n" );
    setState ( ShuttingDown );

    m_jobAvailable.wakeAll();

    // problem: Some threads might not be asleep yet, just finding
    // out if a job is available. Those threads will suspend
    // waiting for their next job (a rare case, but not impossible).
    // Therefore, if we encounter a thread that has not exited, we
    // have to wake it again (which we do in the following for
    // loop).

    for (;;) {
        Thread* th = 0;
        {
            QMutexLocker l(m_mutex); Q_UNUSED(l);
            if (m_inventory.isEmpty()) break;
            th = m_inventory.takeFirst();
        }
        if ( !th->isFinished() )
        {
            for ( ;; )
            {
                m_jobAvailable.wakeAll();
                if ( th->wait( 100 ) ) break;
                debug ( 1,  "WeaverImpl::shutDown: thread %i did not exit as expected, "
                        "retrying.\n", th->id() );
            }
        }
        Q_EMIT ( threadExited ( th ) );
        delete th;
    }
    Q_ASSERT(m_inventory.isEmpty());
    debug ( 3, "WeaverImpl::shutDown: done\n" );
    setState ( Destructed ); // Destructed ignores all calls into the queue API
}

void WeaverImpl::setState ( StateId id )
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    setState_p(id);
}

//FIXME Use delayed signal emitter!
void WeaverImpl::setState_p(StateId id)
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
    if ( m_state==0 || m_state->stateId() != id ) {
        m_state = m_states[id].data();
        debug ( 2, "WeaverImpl::setState: state changed to \"%s\".\n",
                m_state->stateName().toLatin1().constData() );
        if ( id == Suspended ) {
            Q_EMIT ( suspended() );
        }

        m_state->activated();
        Q_EMIT ( stateChanged ( m_state ) );
    }
}

const State& WeaverImpl::state() const
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    return state_p();
}

const State& WeaverImpl::state_p() const
{
    return *m_state;
}

void WeaverImpl::setMaximumNumberOfThreads( int cap )
{
    //FIXME really? Why not 0?
    Q_ASSERT_X ( cap > 0, "Weaver Impl", "Thread inventory size has to be larger than zero." );
    QMutexLocker l (m_mutex);  Q_UNUSED(l);
    m_state->setMaximumNumberOfThreads(cap);
}

void WeaverImpl::setMaximumNumberOfThreads_p(int cap)
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
    m_inventoryMax = cap;
}

int WeaverImpl::maximumNumberOfThreads() const
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    return m_state->maximumNumberOfThreads();
}

int WeaverImpl::maximumNumberOfThreads_p() const
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
    return m_inventoryMax;
}

int WeaverImpl::currentNumberOfThreads () const
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    return m_state->currentNumberOfThreads();
}

int WeaverImpl::currentNumberOfThreads_p() const
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
    return m_inventory.count();
}

void WeaverImpl::registerObserver ( WeaverObserver *ext )
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    return m_state->registerObserver(ext);
}

void WeaverImpl::registerObserver_p(WeaverObserver *ext)
{
    connect ( this,  SIGNAL (stateChanged(ThreadWeaver::State*)),
              ext,  SIGNAL (weaverStateChanged(ThreadWeaver::State*)) );
    connect ( this,  SIGNAL (threadStarted(ThreadWeaver::Thread*)),
              ext,  SIGNAL (threadStarted(ThreadWeaver::Thread*)) );
    connect ( this,  SIGNAL (threadBusy(ThreadWeaver::Thread*,ThreadWeaver::Job*)),
              ext,  SIGNAL (threadBusy(ThreadWeaver::Thread*,ThreadWeaver::Job*)) );
    connect ( this,  SIGNAL (threadSuspended(ThreadWeaver::Thread*)),
              ext,  SIGNAL (threadSuspended(ThreadWeaver::Thread*)) );
    connect ( this,  SIGNAL (threadExited(ThreadWeaver::Thread*)) ,
              ext,  SIGNAL (threadExited(ThreadWeaver::Thread*)) );
}

void WeaverImpl::enqueue(Job* job)
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    return m_state->enqueue(job);
}

void WeaverImpl::enqueue_p(Job *job)
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
    if (job) {
        adjustInventory(1);
        debug(3, "WeaverImpl::enqueue: queueing job %p of type %s.\n", (void*)job, job->metaObject()->className());
        job->aboutToBeQueued(this);
        // find position for insertion:
        int i = m_assignments.size();
        if (i > 0) {
            while(i > 0 && m_assignments.at(i - 1)->priority() < job->priority()) --i;
            m_assignments.insert(i, job);
        } else {
            m_assignments.append(job);
        }
        assignJobs();
    }
}

bool WeaverImpl::dequeue ( Job* job )
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    return m_state->dequeue(job);
}

bool WeaverImpl::dequeue_p(Job* job)
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
    int i = m_assignments.indexOf ( job );
    if ( i != -1 ) {
        job->aboutToBeDequeued( this );
        m_assignments.removeAt( i );
        debug(3, "WeaverImpl::dequeue: job %p dequeued, %i jobs left.\n", (void*)job, queueLength_p());
        // from the queues point of view, a job is just as finished if it gets dequeued:
        m_jobFinished.wakeOne();
        return true;
    } else {
        debug( 3, "WeaverImpl::dequeue: job %p not found in queue.\n", (void*)job );
        return false;
    }
}

void WeaverImpl::dequeue ()
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    m_state->dequeue();
}

void WeaverImpl::dequeue_p()
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
    debug( 3, "WeaverImpl::dequeue: dequeueing all jobs.\n" );
    for ( int index = 0; index < m_assignments.size(); ++index ) {
        m_assignments.at( index )->aboutToBeDequeued( this );
    }
    m_assignments.clear();
    ENSURE ( m_assignments.isEmpty() );
}

void WeaverImpl::finish()
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    m_state->finish();
}

void WeaverImpl::finish_p()
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
#ifdef QT_NO_DEBUG
    const int MaxWaitMilliSeconds = 50;
#else
    const int MaxWaitMilliSeconds = 500;
#endif
    while (!isIdle_p()) {
        Q_ASSERT(state_p().stateId() == WorkingHard);
        debug (2, "WeaverImpl::finish: not done, waiting.\n" );
        if (m_jobFinished.wait(m_mutex, MaxWaitMilliSeconds) == false) {
            debug ( 2, "WeaverImpl::finish: wait timed out, %i jobs left, waking threads.\n",
                    queueLength_p() );
            m_jobAvailable.wakeAll();
        }
    }
    debug (2, "WeaverImpl::finish: done.\n\n\n" );
}

void WeaverImpl::suspend ()
{
    //FIXME?
    //QMutexLocker l(m_mutex); Q_UNUSED(l);
    m_state->suspend();
}

void WeaverImpl::suspend_p()
{
    //FIXME ?
}

void WeaverImpl::resume ( )
{
    //FIXME?
    //QMutexLocker l(m_mutex); Q_UNUSED(l);
    m_state->resume();
}

void WeaverImpl::resume_p()
{
    //FIXME ?
}

bool WeaverImpl::isEmpty() const
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    return m_state->isEmpty();
}

bool WeaverImpl::isEmpty_p() const
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
    return  m_assignments.isEmpty();
}

bool WeaverImpl::isIdle() const
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    return m_state->isIdle();
}

bool WeaverImpl::isIdle_p() const
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
    return isEmpty_p() && m_active == 0;
}

int WeaverImpl::queueLength() const
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    return m_state->queueLength();
}

int WeaverImpl::queueLength_p() const
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
    return m_assignments.count();
}

void WeaverImpl::requestAbort()
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    return m_state->requestAbort();
}

void WeaverImpl::requestAbort_p()
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
    for ( int i = 0; i<m_inventory.size(); ++i ) {
        m_inventory[i]->requestAbort();
    }
}

void WeaverImpl::adjustInventory ( int numberOfNewJobs )
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
    //number of threads that can be created:
    const int reserve = m_inventoryMax - m_inventory.count();

    if (reserve > 0) {
        for (int i = 0; i < qMin(reserve, numberOfNewJobs); ++i) {
            Thread *th = createThread();
            th->moveToThread( th ); // be sane from the start
            m_inventory.append(th);
            connect ( th,  SIGNAL (jobStarted(ThreadWeaver::Thread*,ThreadWeaver::Job*)),
                      SIGNAL (threadBusy(ThreadWeaver::Thread*,ThreadWeaver::Job*)) );
            connect ( th,  SIGNAL (jobDone(ThreadWeaver::Job*)),
                      SIGNAL (jobDone(ThreadWeaver::Job*)) );
            connect ( th,  SIGNAL (started(ThreadWeaver::Thread*)),
                      SIGNAL (threadStarted(ThreadWeaver::Thread*)) );

            th->start();
            debug(2, "WeaverImpl::adjustInventory: thread created, "
                  "%i threads in inventory.\n", currentNumberOfThreads_p());
        }
    }
}

Thread* WeaverImpl::createThread()
{
    return new Thread( this );
}

void WeaverImpl::assignJobs()
{
    m_jobAvailable.wakeAll();
}

void WeaverImpl::incActiveThreadCount()
{
    adjustActiveThreadCount ( 1 );
}

void WeaverImpl::decActiveThreadCount()
{
    adjustActiveThreadCount(-1);
    // the done job could have freed another set of jobs, and we do not know how
    // many - therefore we need to wake all threads:
    m_jobFinished.wakeAll();
}

void WeaverImpl::adjustActiveThreadCount(int diff)
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
    m_active += diff;
    debug(4, "WeaverImpl::adjustActiveThreadCount: %i active threads (%i jobs"
          " in queue).\n", m_active,  queueLength_p());

    if (m_assignments.isEmpty() && m_active == 0) {
        P_ASSERT ( diff < 0 ); // cannot reach Zero otherwise
        Q_EMIT ( finished() );
    }
}

int WeaverImpl::activeThreadCount()
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
    return m_active;
}

Job* WeaverImpl::takeFirstAvailableJobOrWait(Thread *th, Job *previous)
{
    QMutexLocker l (m_mutex); Q_UNUSED(l);
    if (previous) {
        // cleanup and send events:
        decActiveThreadCount();
    }
    Job *next = 0;
    for (int index = 0; index < m_assignments.size(); ++index) {
        if ( m_assignments.at(index)->canBeExecuted() ) {
            next = m_assignments.at(index);
            m_assignments.removeAt (index);
            break;
        }
    }
    if (next) {
        incActiveThreadCount();
        return next;
    } else {
        blockThreadUntilJobsAreBeingAssigned_locked(th);
        return 0;
    }
}

Job* WeaverImpl::applyForWork(Thread *th, Job* previous)
{
    return m_state->applyForWork ( th,  previous );
}

void WeaverImpl::waitForAvailableJob(Thread* th)
{
    m_state->waitForAvailableJob ( th );
}

void WeaverImpl::blockThreadUntilJobsAreBeingAssigned(Thread *th)
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    blockThreadUntilJobsAreBeingAssigned_locked(th);
}

void WeaverImpl::blockThreadUntilJobsAreBeingAssigned_locked(Thread *th)
{
    Q_ASSERT(!m_mutex->tryLock()); //mutex has to be held when this method is called
    debug ( 4,  "WeaverImpl::waitForAvailableJob_locked: thread %i blocked.\n", th->id());
    //FIXME use delayed signal emitter
    Q_EMIT threadSuspended(th);
    m_jobAvailable.wait(m_mutex);
    debug ( 4,  "WeaverImpl::waitForAvailableJob_locked: thread %i resumed.\n", th->id());
}

void WeaverImpl::dumpJobs()
{
    QMutexLocker l(m_mutex); Q_UNUSED(l);
    debug( 0, "WeaverImpl::dumpJobs: current jobs:\n" );
    for ( int index = 0; index < m_assignments.size(); ++index ) {
        debug( 0, "--> %4i: %p %s (priority %i, can be executed: %s)\n", index, (void*)m_assignments.at( index ),
               m_assignments.at( index )->metaObject()->className(),
               m_assignments.at(index)->priority(),
               m_assignments.at(index)->canBeExecuted() ? "yes" : "no");
    }
}

#include "WeaverImpl.moc"
