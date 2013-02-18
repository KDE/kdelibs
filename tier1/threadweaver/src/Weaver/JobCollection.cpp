/* -*- C++ -*-

This file implements the JobCollection class.

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
*/

#include "JobCollection.h"

#include "QueueAPI.h"
#include "DebuggingAids.h"

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QPointer>

#include "DependencyPolicy.h"
#include "ExecuteWrapper.h"

namespace ThreadWeaver {

class CollectionExecuteWrapper : public ThreadWeaver::ExecuteWrapper {
public:
    CollectionExecuteWrapper()
        : collection(0)
    {}

    void setCollection(JobCollection* collection_) {
        collection = collection_;
    }

    void execute(ThreadWeaver::Job* job, ThreadWeaver::Thread *thread) {
        Q_ASSERT(collection);
        collection->elementStarted(job, thread);
        executeWrapped(job, thread);
        collection->elementFinished(job, thread);
    }

    void cleanup(Job *job, Thread *) {
        //Once job is unwrapped from us, this object is dangling. Job::executor points to the next higher up execute wrapper.
        //It is thus safe to "delete this". By no means add any later steps after delete!
        delete unwrap(job);
    }

private:
    ThreadWeaver::JobCollection* collection;
};

class CollectionSelfExecuteWrapper : public ThreadWeaver::ExecuteWrapper {
public:
    void begin(Job*, Thread*) {
    }

    void end(Job*, Thread*) {
    }
};


class JobCollection::Private
{
public:
    typedef QList<Job*> JobList;

    Private()
        : api ( 0 )
        , jobCounter (0)
    {
    }

    ~Private()
    {
    }

    /* The elements of the collection. */
    JobList elements;

    /* The Weaver interface this collection is queued in. */
    QueueAPI *api;

    /* Counter for the finished jobs.
       Set to the number of elements when started.
       When zero, all elements are done.
    */
    QAtomicInt jobCounter;
    QAtomicInt jobsStarted;
    CollectionSelfExecuteWrapper selfExecuteWrapper;
};

JobCollection::JobCollection(QObject *parent)
    : Job(parent)
    , d(new Private)
{
    d->selfExecuteWrapper.wrap(setExecutor(&d->selfExecuteWrapper));
    CollectionExecuteWrapper* wrapper = new CollectionExecuteWrapper();
    wrapper->setCollection(this);
    wrapper->wrap(setExecutor(wrapper));
}

JobCollection::~JobCollection()
{
    {   // dequeue all remaining jobs:
        QMutexLocker l(mutex()); Q_UNUSED(l);
        if (d->api != 0) // still queued
            dequeueElements(false);
    }
    // QObject cleanup takes care of the job runners
    delete d;
}

void JobCollection::addJob ( Job *job )
{
    QMutexLocker l(mutex()); Q_UNUSED(l);
    REQUIRE( d->api == 0 ); // not queued yet
    REQUIRE( job != 0);

    CollectionExecuteWrapper* wrapper = new CollectionExecuteWrapper();
    wrapper->setCollection(this);
    wrapper->wrap(job->setExecutor(wrapper));
    d->elements.append(job);
}

//FIXME add test!
//This method is unused and untested. And probably does not work.
void JobCollection::stop(Job *job)
{   // this only works if there is an event queue executed by the main
    // thread, and it is not blocked:
    Q_UNUSED( job );
    if ( d->api != 0 ) {
        debug( 4, "JobCollection::stop: dequeueing %p.\n", (void*)this);
        d->api->dequeue( this );
    }
    // FIXME ENSURE ( d->weaver == 0 ); // verify that aboutToBeDequeued has been called
}

void JobCollection::aboutToBeQueued_locked(QueueAPI *api)
{
    Q_ASSERT(!mutex()->tryLock());
    Q_ASSERT(d->api == 0); // never queue twice
    d->api = api;
    Job::aboutToBeQueued_locked(api);
}

void JobCollection::aboutToBeDequeued_locked(QueueAPI *api )
{   Q_ASSERT(!mutex()->tryLock());
    Q_ASSERT(api && d->api == api );
    dequeueElements(true);
    d->api = 0;
    Job::aboutToBeDequeued_locked(api);
}

void JobCollection::execute(Thread *thread)
{
    Q_ASSERT(d->api!= 0);
    {
        QMutexLocker l(mutex()); Q_UNUSED(l);
        d->jobCounter.fetchAndStoreOrdered(d->elements.count() + 1); //including self
        Q_FOREACH(Job* child, d->elements) {
            d->api->enqueue(child);
        }
    }
    Job::execute(thread);
}

void JobCollection::elementStarted(Job*, Thread* thread)
{
    if (d->jobsStarted.fetchAndAddOrdered(1) == 0) {
        //emit started() signal on beginning of first job execution
        executor()->defaultBegin(this, thread);
    }
}

void JobCollection::elementFinished(Job*, Thread *thread)
{
    QMutexLocker l(mutex()); Q_UNUSED(l);
    const int jobsStarted = d->jobsStarted.fetchAndAddOrdered(0);
    Q_ASSERT(jobsStarted >=0);
    const int remainingJobs = d->jobCounter.fetchAndAddOrdered(-1) -1;
    Q_ASSERT(remainingJobs >=0);
    if (remainingJobs == 0 ) {
        // there is a small chance that (this) has been dequeued in the
        // meantime, in this case, there is nothing left to clean up
        finalCleanup();
        executor()->defaultEnd(this, thread);
    }
}

Job* JobCollection::jobAt( int i )
{
    Q_ASSERT(!mutex()->tryLock());
    Q_ASSERT(i >= 0 && i < d->elements.size() );
    return d->elements.at(i);
}

int JobCollection::jobListLength() const
{
    QMutexLocker l(mutex()); Q_UNUSED(l);
    return jobListLength_locked();
}

int JobCollection::jobListLength_locked() const
{
    return d->elements.size();
}

//FIXME get rid off
void JobCollection::internalJobDone ( Job* job )
{
	REQUIRE( job != 0 );
    Q_UNUSED (job);
}

void JobCollection::finalCleanup()
{
    Q_ASSERT(!mutex()->tryLock());
    freeQueuePolicyResources();
    setFinished(true);
    d->api = 0;
}

void JobCollection::dequeueElements(bool queueApiIsLocked)
{   // dequeue everything:
    Q_ASSERT(!mutex()->tryLock());
    if ( d->api == 0 ) return; //not queued
    //FIXME only if not finished?
    for ( int index = 0; index < d->elements.size(); ++index ) {
        debug(4, "JobCollection::dequeueElements: dequeueing %p.\n", (void*)d->elements.at(index));
        if (queueApiIsLocked) {
            d->api->dequeue_p(d->elements.at(index));
        } else {
            d->api->dequeue(d->elements.at(index));
        }
    }

    const int jobCount = d->jobCounter.fetchAndAddOrdered(0);
    if (jobCount != 0) {
        // if jobCounter is not zero, then we where waiting for the
        // last job to finish before we would have freed our queue
        // policies, but in this case we have to do it here:
        finalCleanup();
    }
    d->jobCounter = 0;
}

}

#include "moc_JobCollection.cpp"
