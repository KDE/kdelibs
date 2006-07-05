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

#include <QSet>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QMultiMap>
#include <QMetaObject>
#include <QWaitCondition>
#include <DebuggingAids.h>
#include <Thread.h>

#include "Job.h"
#include "QueuePolicy.h"
#include "DependencyPolicy.h"

using namespace ThreadWeaver;

class ThreadWeaver::QueuePolicyList : public QList<QueuePolicy*> {};

Job::Job ( QObject *parent )
    : QObject (parent)
    , m_thread (0)
    , m_queuePolicies ( new QueuePolicyList )
    , m_mutex (new QMutex (QMutex::NonRecursive) )
    , m_finished (false)
{
}

Job::~Job()
{
    for ( int index = 0; index < m_queuePolicies->size(); ++index )
    {
        m_queuePolicies->at( index )->destructed( this );
    }
}

class ThreadWeaver::JobRunHelper : public QObject
{
    Q_OBJECT
public:
    JobRunHelper()
        : QObject ( 0 )
    {
    }

signals:
    void started ( Job* );
    void done ( Job* );
    void failed( Job* );

public:

    void runTheJob ( Thread* th, Job* job )
    {
        P_ASSERT ( th == thread() );
        job->m_mutex->lock();
        job->m_thread = th;
        job->m_mutex->unlock();

        emit ( started ( job ) );

        job->run();

        job->m_mutex->lock();
        job->m_thread = 0;
        job->setFinished (true);
        job->m_mutex->unlock();
        job->freeQueuePolicyResources();

        if ( ! job->success() )
        {
            emit ( failed( job ) );
        }

        emit ( done( job ) );
    }
};

void Job::execute(Thread *th)
{
//    P_ASSERT (sm_dep()->values(this).isEmpty());
    JobRunHelper helper;
    connect ( &helper,  SIGNAL ( started ( Job* ) ), SIGNAL ( started ( Job* ) ) );
    connect ( &helper,  SIGNAL ( done ( Job* ) ), SIGNAL ( done ( Job* ) ) );
    connect ( &helper, SIGNAL( failed( Job* ) ), SIGNAL( failed( Job* ) ) );

    debug(3, "Job::execute: executing job of type %s in thread %i.\n",
          metaObject()->className(), th->id());
    helper.runTheJob( th, this );
    debug(3, "Job::execute: finished execution of job in thread %i.\n", th->id());
}

int Job::priority () const
{
  return 0;
}

void Job::freeQueuePolicyResources()
{
    for ( int index = 0; index < m_queuePolicies->size(); ++index )
    {
        m_queuePolicies->at( index )->free( this );
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

    if ( m_queuePolicies->size() > 0 )
    {
        debug( 4, "Job::canBeExecuted: acquiring permission from %i queue %s.\n",
               m_queuePolicies->size(), m_queuePolicies->size()==1 ? "policy" : "policies" );
        for ( int index = 0; index < m_queuePolicies->size(); ++index )
        {
            if ( m_queuePolicies->at( index )->canRun( this ) )
            {
                acquired.append( m_queuePolicies->at( index ) );
            } else {
                success = false;
                break;
            }
        }

        debug( 4, "Job::canBeExecuted: queue policies returned %s.\n", success ? "true" : "false" );

        if ( ! success )
        {   // FIXME maybe this hase to be done in reverse order?
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
    if ( ! m_queuePolicies->contains( policy ) )
    {
        m_queuePolicies->append( policy );
    }
}

void Job::removeQueuePolicy( QueuePolicy* policy )
{
    int index = m_queuePolicies->indexOf( policy );
    if ( index != -1 )
    {
        m_queuePolicies->removeAt( index );
    }
}

#include "Job.moc"
#ifdef USE_CMAKE
#include "Job_moc.cpp"
#endif
