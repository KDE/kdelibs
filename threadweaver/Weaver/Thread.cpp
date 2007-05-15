/* -*- C++ -*-

   This file implements the Thread class.

   Thread is not a part of the public interface of the ThreadWeaver library.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005 Mirko Boehm $
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

   $Id: Thread.cpp 25 2005-08-14 12:41:38Z mirko $
*/

#include "Thread.h"
#include "Thread_p.h"

#include <QtCore/QMutex>
#include <QtCore/QDebug>

#include "ThreadWeaver.h"
#include "WeaverImpl.h"
#include "Job.h"
#include "DebuggingAids.h"

using namespace ThreadWeaver;

class Thread::Private
{
public:
    explicit Private ( WeaverImpl* theParent )
        : parent ( theParent )
        , runhelper ( 0 )
        , id ( makeId() )
    {}

    WeaverImpl *parent;

    ThreadRunHelper* runhelper;

    const unsigned int id;

    static unsigned int makeId()
    {
        static unsigned int s_id;
        static QMutex sm_mutex;
        QMutexLocker l (&sm_mutex);
        return ++s_id;
    }
};


ThreadWeaver::ThreadRunHelper::ThreadRunHelper()
    : QObject ( 0 )
    , m_job( 0 )
{
}

void ThreadWeaver::ThreadRunHelper::run ( WeaverImpl *parent, Thread* th )
{
    Q_ASSERT ( thread() == th );
    emit ( started ( th) );

    while (true)
    {
        debug ( 3, "Thread::run [%u]: trying to execute the next job.\n", th->id() );

        // this is the *only* assignment to m_job  in the Thread class!
        Job* tmp = m_job; m_job = 0;

        Job* job = parent->applyForWork ( th, tmp );

        if (job == 0)
        {
            break;
        } else {
            m_job = job;
            emit ( jobStarted ( th,  m_job ) );
            m_job->execute (th);
            emit ( jobDone ( m_job ) );
        }
    }
}

void ThreadWeaver::ThreadRunHelper::requestAbort()
{
    Job* job = m_job;
    if ( job )
    {
        job->requestAbort();
    }
}

Thread::Thread (WeaverImpl *parent)
    : QThread () // no parent, because the QObject hierarchy of this thread
                 // does not have a parent (see QObject::pushToThread)
    , d ( new Private ( parent ) )
{
}

Thread::~Thread()
{
    delete d;
}

unsigned int Thread::id()
{
    return d->id;
}

void Thread::run()
{
// disabled while testing movetothread...
//    Q_ASSERT ( thread() != this ); // this is created and owned by the main thread
    debug ( 3, "Thread::run [%u]: running.\n", id() );

    ThreadRunHelper helper;
    d->runhelper = &helper;

    connect ( &helper, SIGNAL ( started ( ThreadWeaver::Thread* ) ),
              SIGNAL ( started ( ThreadWeaver::Thread* ) ) );
    connect ( &helper, SIGNAL ( jobStarted ( ThreadWeaver::Thread*, ThreadWeaver::Job* ) ),
              SIGNAL ( jobStarted ( ThreadWeaver::Thread*, ThreadWeaver::Job* ) ) );
    connect ( &helper, SIGNAL ( jobDone ( ThreadWeaver::Job* ) ),
              SIGNAL ( jobDone ( ThreadWeaver::Job* ) ) );
    helper.run( d->parent,  this );

    d->runhelper = 0;
    debug ( 3, "Thread::run [%u]: exiting.\n", id() );
}

void Thread::msleep(unsigned long msec)
{
    QThread::msleep(msec);
}


void Thread::requestAbort ()
{
    if ( d->runhelper )
    {
        d->runhelper->requestAbort();
    } else {
        qDebug ( "Thread::requestAbort: not running." );
    }
}

#include "Thread.moc"
#include "Thread_p.moc"
