/* -*- C++ -*-

This file implements the JobCollection class.

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

$Id: DebuggingAids.h 30 2005-08-16 16:16:04Z mirko $
*/

#include "JobCollection.h"
#include "JobCollection_p.h"

#include "WeaverInterface.h"
#include "DebuggingAids.h"

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QPointer>

#include "DependencyPolicy.h"

using namespace ThreadWeaver;

JobCollectionJobRunner::JobCollectionJobRunner ( JobCollection* collection, Job* payload, QObject* parent )
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

bool JobCollectionJobRunner::canBeExecuted()
{   // the JobCollectionJobRunner object never have any dependencies:
    return m_payload->canBeExecuted();
}

Job* JobCollectionJobRunner::payload ()
{
    return m_payload;
}

void JobCollectionJobRunner::aboutToBeQueued ( WeaverInterface *weaver )
{
    m_payload->aboutToBeQueued( weaver );
}

void JobCollectionJobRunner::aboutToBeDequeued ( WeaverInterface *weaver )
{
    m_payload->aboutToBeDequeued( weaver );
}

void JobCollectionJobRunner::execute ( Thread *t )
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

int JobCollectionJobRunner::priority () const
{
    return m_payload->priority();
}

void JobCollectionJobRunner::run ()
{
}

class JobList : public QList <JobCollectionJobRunner*> {};

class JobCollection::Private
{
public:

    Private()
        : elements ( new JobList() )
        , weaver ( 0 )
        , jobCounter (0)
    {}

    ~Private()
    {
        delete elements;
    }

    /* The elements of the collection. */
    JobList* elements;

    /* The Weaver interface this collection is queued in. */
    WeaverInterface *weaver;

    /* Counter for the finished jobs.
       Set to the number of elements when started.
       When zero, all elements are done.
    */
    int jobCounter;

    QMutex mutex;
};

JobCollection::JobCollection ( QObject *parent )
    : Job ( parent )
    , d (new Private)
{
}

JobCollection::~JobCollection()
{   // dequeue all remaining jobs:
    if ( d->weaver != 0 ) // still queued
        dequeueElements();
    // QObject cleanup takes care of the job runners
    delete d;
}

void JobCollection::addJob ( Job *job )
{
    REQUIRE( d->weaver == 0 );
    REQUIRE( job != 0);

	JobCollectionJobRunner* runner = new JobCollectionJobRunner( this, job, this );
    d->elements->append ( runner );
	connect( runner , SIGNAL(done(ThreadWeaver::Job*)) , this , SLOT(jobRunnerDone()) );
}

void JobCollection::stop( Job *job )
{   // this only works if there is an event queue executed by the main
    // thread, and it is not blocked:
    Q_UNUSED( job );
    if ( d->weaver != 0 )
    {
        debug( 4, "JobCollection::stop: dequeueing %p.\n", (void*)this);
        d->weaver->dequeue( this );
    }
    // FIXME ENSURE ( d->weaver == 0 ); // verify that aboutToBeDequeued has been called
}

void JobCollection::aboutToBeQueued ( WeaverInterface *weaver )
{
    REQUIRE ( d->weaver == 0 ); // never queue twice

    d->weaver = weaver;

    if ( d->elements->size() > 0 )
    {
        d->elements->at( 0 )->aboutToBeQueued( weaver );
    }

    ENSURE(d->weaver != 0);
}

void JobCollection::aboutToBeDequeued( WeaverInterface* weaver )
{   //  Q_ASSERT ( d->weaver != 0 );
    // I thought: "must have been queued first"
    // but the user can queue and dequeue in a suspended Weaver

    if ( d->weaver )
    {
        dequeueElements();

        d->elements->at( 0 )->aboutToBeDequeued( weaver );
    }

    d->weaver = 0;
    ENSURE ( d->weaver == 0 );
}

void JobCollection::execute ( Thread *t )
{
    REQUIRE ( d->weaver != 0);

    // this is async,
    // JobTests::JobSignalsAreEmittedAsynchronouslyTest() proves it
    emit (started (this));

    if ( d->elements->isEmpty() )
    {   // we are just a regular, empty job (sob...):
        Job::execute( t );
        return;
    }

    {   // d->elements is supposedly constant at this time, since we are
        // already queued
        // set job counter:
        QMutexLocker l ( & d->mutex );
        d->jobCounter = d->elements->size();

        // queue elements:
        for (int index = 1; index < d->elements->size(); ++index)
	{
            d->weaver->enqueue (d->elements->at(index));
	}
    }
    // this is a hack (but a good one): instead of queueing (this), we
    // execute the last job, to avoid to have (this) wait for an
    // available thread (the last operation does not get queued in
    // aboutToBeQueued() )
    // NOTE: this also calls internalJobDone()
    d->elements->at( 0 )->execute ( t );

    // do not emit done, done is emitted when the last job called
    // internalJobDone()
    // also, do not free the queue policies yet, since not the whole job
    // is done
}

Job* JobCollection::jobAt( int i )
{
    QMutexLocker l( &d->mutex );
    REQUIRE ( i >= 0 && i < d->elements->size() );
    return d->elements->at( i )->payload();
}

const int JobCollection::jobListLength() // const qualifier is possibly BiC?
{
    QMutexLocker l( &d->mutex );
    return d->elements->size();
}

bool JobCollection::canBeExecuted()
{
    bool inheritedCanRun = true;

    QMutexLocker l( &d->mutex );

    if ( d->elements->size() > 0 )
    {
        inheritedCanRun = d->elements->at( 0 )->canBeExecuted();
    }

    return Job::canBeExecuted() && inheritedCanRun;
}

void JobCollection::jobRunnerDone()
{
	// Note:  d->mutex must be unlocked before emitting the done() signal
	// because this JobCollection may be deleted by a slot connected to done()
	// in another thread
	bool emitDone = false;

	{
		QMutexLocker l(&d->mutex);

		if ( d->jobCounter == 0 )
		{   // there is a small chance that (this) has been dequeued in the
			// meantime, in this case, there is nothing left to clean up:
			d->weaver = 0;
			return;
		}

		--d->jobCounter;

		ENSURE (d->jobCounter >= 0);

		if (d->jobCounter == 0)
		{
			if (! success())
			{
				emit failed(this);
			}

			finalCleanup();
			emitDone = true;
		}
	}

	if (emitDone)
		emit done(this);
}
void JobCollection::internalJobDone ( Job* job )
{
	REQUIRE( job != 0 );
    Q_UNUSED (job);
}

void JobCollection::finalCleanup()
{
    freeQueuePolicyResources();
    setFinished(true);
    d->weaver = 0;
}

void JobCollection::dequeueElements()
{
	// Note:  d->mutex must be unlocked before emitting the done() signal
	// because this JobCollection may be deleted by a slot connected to done() in another
	// thread
	
	bool emitDone = false;

	{
		// dequeue everything:
		QMutexLocker l( &d->mutex );

		if ( d->weaver != 0 )
		{
			for ( int index = 1; index < d->elements->size(); ++index )
			{
				if ( d->elements->at( index ) && ! d->elements->at( index )->isFinished() ) // ... a QPointer
				{
					debug( 4, "JobCollection::dequeueElements: dequeueing %p.\n",
							(void*)d->elements->at( index ) );
					d->weaver->dequeue ( d->elements->at( index ) );
				} else {
					debug( 4, "JobCollection::dequeueElements: not dequeueing %p, already finished.\n",
							(void*)d->elements->at( index ) );
				}
			}

			if (d->jobCounter != 0)
			{ 	// if jobCounter is not zero, then we where waiting for the
				// last job to finish before we would have freed our queue
				// policies, but in this case we have to do it here:
				finalCleanup();
			}
			d->jobCounter = 0;
		}
	}
	if (emitDone)
		emit done(this);
}

#include "JobCollection.moc"
#include "JobCollection_p.moc"
