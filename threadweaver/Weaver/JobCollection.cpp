/* -*- C++ -*-

   This file implements the JobCollection class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005, 2006 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: DebuggingAids.h 30 2005-08-16 16:16:04Z mirko $
*/

#include <WeaverInterface.h>
#include <DebuggingAids.h>

#include <QList>
#include <QObject>
#include <QPointer>

#include "JobCollection.h"
#include "DependencyPolicy.h"

using namespace ThreadWeaver;

/* QPointers are used internally to be able to dequeue jobs at destruction
   time. The owner of the jobs could have deleted them in the meantime.
   We use a class instead of a typedef to be able to forward-declare the
   class in the declaration.
*/
class ThreadWeaver::JobCollectionJobRunner : public Job
{
    Q_OBJECT

public:
    JobCollectionJobRunner ( JobCollection* collection, Job* payload, QObject* parent )
        : Job( parent )
        , m_payload( payload )
        , m_collection( collection )
    {
        Q_ASSERT ( payload ); // will not accept zero jobs

        if ( ! m_payload->objectName().isEmpty() )
        {   // this is most useful for debugging...
            setObjectName( tr( "JobRunner executing " ) + m_payload->objectName() );
        } else {
            setObjectName( tr( "JobRunner (unnamed payload)" ) );
        }
    }

    bool canBeExecuted()
    {   // the JobCollectionJobRunner object never have any dependencies:
        return m_payload->canBeExecuted();
    }

    Job* payload ()
    {
        return m_payload;
    }

    void aboutToBeQueued ( WeaverInterface *weaver )
    {
        m_payload->aboutToBeQueued( weaver );
    }

    void aboutToBeDequeued ( WeaverInterface *weaver )
    {
        m_payload->aboutToBeDequeued( weaver );
    }

    void execute ( Thread *t )
    {
        if ( m_payload )
        {
            m_payload->execute ( t );
            m_collection->internalJobDone ( m_payload);
        } else {
            debug ( 1, "JobCollection: job in collection has been deleted." );
        }
        Job::execute ( t );
    }

    int priority () const
    {
        return m_payload->priority();
    }

private:
    void run () {}

    QPointer<Job> m_payload;
    JobCollection* m_collection;
};

class JobCollection::JobList : public QList <JobCollectionJobRunner*> {};

JobCollection::JobCollection ( QObject *parent )
    : Job ( parent )
    , m_elements ( new JobList() )
    , m_weaver ( 0 )
    , m_jobCounter (0)
{
}

JobCollection::~JobCollection()
{   // dequeue all remaining jobs:
    dequeueElements();
    delete m_elements;
    // QObject cleanup takes care of the job runners
}

void JobCollection::addJob ( Job *job )
{
    REQUIRE( m_weaver == 0 );
    REQUIRE( job != 0);
    m_elements->append ( new JobCollectionJobRunner( this, job, this ) );
}

void JobCollection::stop( Job *job )
{   // this only works if there is an event queue executed by the main
    // thread, and it is not blocked:
    Q_UNUSED( job );

    if ( m_weaver != 0 )
    {
        debug( 4, "JobCollection::stop: dequeueing %p.\n", this);
        m_weaver->dequeue( this );
    }
}

void JobCollection::aboutToBeQueued ( WeaverInterface *weaver )
{
    REQUIRE ( m_weaver == 0 ); // never queue twice

    m_weaver = weaver;

    if ( m_elements->size() > 0 )
    {
        m_elements->at( 0 )->aboutToBeQueued( weaver );
    }

    ENSURE(m_weaver != 0);
}

void JobCollection::aboutToBeDequeued( WeaverInterface* weaver )
{

    //  Q_ASSERT ( m_weaver != 0 );
    // I thought: "must have been queued first"
    // but the user can queue and dequeue in a suspended Weaver

    if ( m_weaver )
    {
        dequeueElements();

        m_elements->at( 0 )->aboutToBeDequeued( weaver );
    }

    m_weaver = 0;

    ENSURE ( m_weaver == 0 );
}

void JobCollection::execute ( Thread *t )
{
    REQUIRE ( m_weaver != 0);

    // FIXME make sure this is async:
    emit (started (this));

    if ( ! m_elements->isEmpty() )
    { // m_elements is supposedly constant at this time, since we are
      // already queued
      // set job counter:
        m_jobCounter = m_elements->size();

        // queue elements:
        for (int index = 1; index < m_elements->size(); ++index)
	{
            m_weaver->enqueue (m_elements->at(index));
	}

        // this is a hack (but a good one): instead of queueing (this), we
        // execute the last job, to avoid to have (this) wait for an
        // available thread (the last operation does not get queued in
        // aboutToBeQueued() )
        // NOTE: this also calls internalJobDone()
        m_elements->at( 0 )->execute ( t );
    } else {
        // otherwise, we are just a regular, empty job (sob...):
        Job::execute( t );
    }

    // do not emit done, done is emitted when the last job called
    // internalJobDone()
    // also, do not free the queue policies yet, since not the whole job
    // is done
}

Job* JobCollection::jobAt( int i )
{
    REQUIRE ( i >= 0 && i < m_elements->size() );
    return m_elements->at( i )->payload();
}

const int JobCollection::jobListLength()
{
    return m_elements->size();
}

bool JobCollection::canBeExecuted()
{
    bool inheritedCanRun = true;

    if ( m_elements->size() > 0 )
    {
        inheritedCanRun = m_elements->at( 0 )->canBeExecuted();
    }

    return Job::canBeExecuted() && inheritedCanRun;
}

void JobCollection::internalJobDone ( Job* job )
{
    REQUIRE (job != 0);
    REQUIRE (m_weaver != 0); // ... queued
    REQUIRE (m_jobCounter > 0); // ... still jobs left
    Q_UNUSED (job);
    --m_jobCounter;

    if (m_jobCounter == 0)
    {
        finalCleanup();

        if (! success())
	{
            emit failed(this);
	}
    }
    ENSURE (m_jobCounter >= 0);
}

void JobCollection::finalCleanup()
{
    freeQueuePolicyResources();
    setFinished(true);
    emit done(this);
}

void JobCollection::dequeueElements()
{   // dequeue everything:
    if ( m_weaver != 0 )
    {
        for ( int index = 1; index < m_elements->size(); ++index )
	{
            if ( m_elements->at( index ) && ! m_elements->at( index )->isFinished() ) // ... a QPointer
	    {
                debug( 4, "JobCollection::dequeueElements: dequeueing %p.\n",
                       m_elements->at( index ) );
                m_weaver->dequeue ( m_elements->at( index ) );
	    } else {
                debug( 4, "JobCollection::dequeueElements: not dequeueing %p, already finished.\n",
                       m_elements->at( index ) );
	    }
	}

        if (m_jobCounter != 0)
	{ // if jobCounter is not zero, then we where waiting for the
	  // last job to finish before we would have freed our queue
	  // policies, but in this case we have to do it here:
            finalCleanup();
	}
        m_jobCounter = 0;
    }
}

#include "JobCollection.moc"
