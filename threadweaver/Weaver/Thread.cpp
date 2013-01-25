/* -*- C++ -*-

   This file implements the Thread class.

   Thread is not a part of the public interface of the ThreadWeaver library.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004-2013 Mirko Boehm $
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

   $Id: Thread.cpp 25 2005-08-14 12:41:38Z mirko $
*/

#include "Thread.h"

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
    explicit Private(WeaverImpl* theParent)
        : parent(theParent)
        , id(makeId())
        , job(0)
    {
        Q_ASSERT(parent);
    }

    WeaverImpl *parent;
    const unsigned int id;
    Job* job;
    QMutex mutex;

    static unsigned int makeId()
    {
        static unsigned int s_id;
        static QMutex sm_mutex;
        QMutexLocker l (&sm_mutex);
        return ++s_id;
    }
};

Thread::Thread (WeaverImpl *parent)
    : QThread() // no parent, because the QObject hierarchy of this thread
                 // does not have a parent (see QObject::pushToThread)
    , d(new Private(parent))
{
}

Thread::~Thread()
{
    delete d;
}

unsigned int Thread::id()
{
    return d->id; //id is const
}

void Thread::run()
{
    debug(3, "Thread::run [%u]: running.\n", id());
    emit(started ( this));

    while (true) {
        debug(3, "Thread::run [%u]: trying to execute the next job.\n", id());
        Job* oldJob = 0;
        {
            QMutexLocker l(&d->mutex); Q_UNUSED(l);
            oldJob = d->job; d->job = 0;
        }
        // this is the *only* assignment to m_job  in the Thread class!
        Job* newJob = d->parent->applyForWork(this, oldJob);

        if (newJob == 0) {
            break;
        } else {
            {
                QMutexLocker l(&d->mutex); Q_UNUSED(l);
                d->job = newJob;
            }
            emit(jobStarted(this, newJob));
            newJob->execute (this);
            emit(jobDone(newJob));
        }
    }
    debug ( 3, "Thread::run [%u]: exiting.\n", id() );
}

void Thread::msleep(unsigned long msec)
{
    QThread::msleep(msec);
}

void Thread::requestAbort ()
{
    QMutexLocker l(&d->mutex); Q_UNUSED(l);
    if (d->job) {
        d->job->requestAbort();
    } else {
        qDebug ( "Thread::requestAbort: not running." );
    }
}

#include "Thread.moc"
