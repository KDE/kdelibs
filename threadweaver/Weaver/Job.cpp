/* -*- C++ -*-

This file implements the Job class.

$ Author: Mirko Boehm $
$ Copyright: (C) 2004, 2005, 2006 Mirko Boehm $
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

$Id: Job.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#include "Job.h"
#include "Job_p.h"

#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QArgument>
#include <QtCore/QWaitCondition>
#include <DebuggingAids.h>
#include <Thread.h>

#include "QueuePolicy.h"
#include "DependencyPolicy.h"

using namespace ThreadWeaver;

class ThreadWeaver::QueuePolicyList : public QList<QueuePolicy*> {};

class Job::Private
{
public:
    Private ()
        : thread (0)
        , queuePolicies ( new QueuePolicyList )
        , mutex (new QMutex (QMutex::NonRecursive) )
        , finished (false)
    {}

    ~Private()
    {
        delete queuePolicies;
        delete mutex;
    }

    /* The thread that executes this job. Zero when the job is not executed. */
    Thread * thread;

    /* The list of QueuePolicies assigned to this Job. */
    QueuePolicyList* queuePolicies;

    QMutex *mutex;
    /* d->finished is set to true when the Job has been executed. */
    bool finished;
};

Job::Job ( QObject *parent )
    : QObject (parent)
    , d(new Private())
{
}

Job::~Job()
{
    for ( int index = 0; index < d->queuePolicies->size(); ++index )
    {
        d->queuePolicies->at( index )->destructed( this );
    }

    delete d;
}

ThreadWeaver::JobRunHelper::JobRunHelper()
    : QObject ( 0 )
{
}

void ThreadWeaver::JobRunHelper::runTheJob ( Thread* th, Job* job )
{
    P_ASSERT ( th == thread() );
    job->d->mutex->lock();
    job->d->thread = th;
    job->d->mutex->unlock();

    emit ( started ( job ) );

    job->run();

    job->d->mutex->lock();
    job->d->thread = 0;
    job->setFinished (true);
    job->d->mutex->unlock();
    job->freeQueuePolicyResources();

    if ( ! job->success() )
    {
        emit ( failed( job ) );
    }

    emit ( done( job ) );
}

void Job::execute(Thread *th)
{
//    P_ASSERT (sm_dep()->values(this).isEmpty());
    JobRunHelper helper;
    connect ( &helper,  SIGNAL ( started ( ThreadWeaver::Job* ) ),
              SIGNAL ( started ( ThreadWeaver::Job* ) ) );
    connect ( &helper,  SIGNAL ( done ( ThreadWeaver::Job* ) ),
              SIGNAL ( done ( ThreadWeaver::Job* ) ) );
    connect ( &helper, SIGNAL( failed( ThreadWeaver::Job* ) ),
              SIGNAL( failed( ThreadWeaver::Job* ) ) );

    debug(3, "Job::execute: executing job of type %s %s in thread %i.\n",
          metaObject()->className(), objectName().isEmpty() ? "" : qPrintable( objectName() ), th->id());
    helper.runTheJob( th, this );
    debug(3, "Job::execute: finished execution of job in thread %i.\n", th->id());
}

int Job::priority () const
{
    return 0;
}

bool Job::success () const
{
    return true;
}

void Job::freeQueuePolicyResources()
{
    for ( int index = 0; index < d->queuePolicies->size(); ++index )
    {
        d->queuePolicies->at( index )->free( this );
    }
}

void Job::aboutToBeQueued ( WeaverInterface* )
{
}

void Job::aboutToBeDequeued ( WeaverInterface* )
{
}

bool Job::canBeExecuted()
{
    QueuePolicyList acquired;

    bool success = true;

    if ( d->queuePolicies->size() > 0 )
    {
        debug( 4, "Job::canBeExecuted: acquiring permission from %i queue %s.\n",
               d->queuePolicies->size(), d->queuePolicies->size()==1 ? "policy" : "policies" );
        for ( int index = 0; index < d->queuePolicies->size(); ++index )
        {
            if ( d->queuePolicies->at( index )->canRun( this ) )
            {
                acquired.append( d->queuePolicies->at( index ) );
            } else {
                success = false;
                break;
            }
        }

        debug( 4, "Job::canBeExecuted: queue policies returned %s.\n", success ? "true" : "false" );

        if ( ! success )
        {

            for ( int index = 0; index < acquired.size(); ++index )
            {
                acquired.at( index )->release( this );
            }
        }
    } else {
        debug( 4, "Job::canBeExecuted: no queue policies, this job can be executed.\n" );
    }

    return success;
}

void Job::assignQueuePolicy( QueuePolicy* policy )
{
    if ( ! d->queuePolicies->contains( policy ) )
    {
        d->queuePolicies->append( policy );
    }
}

void Job::removeQueuePolicy( QueuePolicy* policy )
{
    int index = d->queuePolicies->indexOf( policy );
    if ( index != -1 )
    {
        d->queuePolicies->removeAt( index );
    }
}

bool Job::isFinished() const
{
    return d->finished;
}

Thread* Job::thread()
{
    return d->thread;
}

void Job::setFinished ( bool status )
{
    d->finished = status;
}

// QMutex& Job::mutex()
// {
//     return * d->mutex;
// }

#include "Job.moc"
#include "Job_p.moc"
