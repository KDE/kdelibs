/* -*- C++ -*-

This file implements the Job class.

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

$Id: Job.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#include "Job.h"

#include <QtCore/QList>
#include <QtCore/QMutex>
#include <DebuggingAids.h>
#include <Thread.h>
#include <QtDebug>
#include <QAtomicPointer>

#include "QueuePolicy.h"
#include "DependencyPolicy.h"
#include "Executor.h"
#include "ExecuteWrapper.h"

namespace {

//TODO QObjectExecutor? that would allows for Jobs to no inherit QObject?
//maaaaaaybeeee...
class DefaultExecutor : public ThreadWeaver::Executor {
public:
    void begin(ThreadWeaver::JobPointer job, ThreadWeaver::Thread *thread) {
        defaultBegin(job, thread);
    }

    void execute(ThreadWeaver::JobPointer job, ThreadWeaver::Thread *) Q_DECL_OVERRIDE {
        run(job);
    }

    void end(ThreadWeaver::JobPointer job, ThreadWeaver::Thread *thread) {
        defaultEnd(job, thread);
    }
};

class DebugExecuteWrapper : public ThreadWeaver::ExecuteWrapper {
public:
    void execute(ThreadWeaver::JobPointer job,ThreadWeaver::Thread *th) Q_DECL_OVERRIDE {
        Q_ASSERT_X(job, Q_FUNC_INFO, "job may not be zero!");
        ThreadWeaver::debug(3, "DefaultExecuteWrapper::execute: executing job %p in thread %i.\n", job.data(), th ? th->id() : 0);
        executeWrapped(job, th);
        ThreadWeaver::debug(3, "Job::execute: finished execution of job in thread %i.\n", th ? th->id() : 0);
    }
};

static DefaultExecutor defaultExecutor;
}

namespace ThreadWeaver {

class QueuePolicyList : public QList<QueuePolicy*> {};

class Job::Private
{
public:
    Private ()
        : mutex(QMutex::NonRecursive)
        , finished (false)
        , executor(&defaultExecutor)
    {
    }

    ~Private()
    {}

    /* The thread that executes this job. Zero when the job is not executed. */
    QAtomicPointer<Thread> thread;
    /* The list of QueuePolicies assigned to this Job. */
    QueuePolicyList queuePolicies;

    mutable QMutex mutex;
    /* d->finished is set to true when the Job has been executed. */
    bool finished;

    /** The Executor that will execute this Job. */
    QAtomicPointer<Executor> executor;

    //FIXME What is the correct KDE frameworks no debug switch?
#if not defined NDEBUG
    /** DebugExecuteWrapper for logging of Job execution. */
    DebugExecuteWrapper debugExecuteWrapper;
#endif
};

Job::Job(QObject *parent)
    : QObject (parent)
    , d(new Private())
{
    //FIXME What is the correct KDE frameworks no debug switch?
#if not defined NDEBUG
    d->debugExecuteWrapper.wrap(setExecutor(&d->debugExecuteWrapper));
#endif
}

Job::~Job()
{
    for (int index = 0; index < d->queuePolicies.size(); ++index ) {
        d->queuePolicies.at(index)->destructed(this);
    }
    delete d;
}

void Job::execute(Thread *th, JobPointer job)
{
    d->thread.fetchAndStoreOrdered(th);
    Executor* executor = d->executor.fetchAndAddOrdered(0);
    Q_ASSERT(executor); //may never be unset!
    executor->begin(job, th);
    executor->execute(job, th);
    executor->end(job, th);
    d->thread.fetchAndStoreOrdered(0);
    setFinished (true);
    executor->cleanup(job, th);
}

void Job::operator ()()
{
    Q_ASSERT(false); //TODO NI
    //execute(0);
}

Executor *Job::setExecutor(Executor *executor)
{
    return d->executor.fetchAndStoreOrdered(executor == 0 ? &defaultExecutor : executor);
}

Executor *Job::executor() const
{
    return d->executor.fetchAndAddOrdered(0);
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
    for (int index = 0; index < d->queuePolicies.size(); ++index) {
        d->queuePolicies.at(index)->free(this);
    }
}

void Job::defaultBegin(JobPointer job, Thread *)
{
    Q_ASSERT(job.data() == this);
    emit started(job);
}

void Job::defaultEnd(JobPointer job, Thread *)
{
    Q_ASSERT(job.data() == this);
    if (!success()) {
        emit failed(job);
    }
    emit done(job);
    //FIXME this requires the job lock?
    freeQueuePolicyResources();
}

void Job::aboutToBeQueued(QueueAPI* api)
{
    QMutexLocker l(mutex()); Q_UNUSED(l);
    aboutToBeQueued_locked(api);
}

void Job::aboutToBeQueued_locked (QueueAPI*)
{
}

void Job::aboutToBeDequeued(QueueAPI* api)
{
    QMutexLocker l(mutex()); Q_UNUSED(l);
    aboutToBeDequeued_locked(api);
}

void Job::aboutToBeDequeued_locked (QueueAPI*)
{
}

bool Job::canBeExecuted()
{
    QueuePolicyList acquired;

    bool success = true;

    if (!d->queuePolicies.isEmpty()) {
        debug( 4, "Job::canBeExecuted: acquiring permission from %i queue %s.\n",
               d->queuePolicies.size(), d->queuePolicies.size()==1 ? "policy" : "policies" );
        for (int index = 0; index < d->queuePolicies.size(); ++index) {
            if (d->queuePolicies.at(index)->canRun(this)) {
                acquired.append(d->queuePolicies.at(index));
            } else {
                success = false;
                break;
            }
        }

        debug(4, "Job::canBeExecuted: queue policies returned %s.\n", success ? "true" : "false");

        if (!success) {
            for (int index = 0; index < acquired.size(); ++index) {
                acquired.at(index)->release(this);
            }
        }
    } else {
        debug(4, "Job::canBeExecuted: no queue policies, this job can be executed.\n");
    }
    return success;
}

void Job::assignQueuePolicy(QueuePolicy* policy)
{
    if (! d->queuePolicies.contains(policy)) {
        d->queuePolicies.append(policy);
    }
}

void Job::removeQueuePolicy(QueuePolicy* policy)
{
    int index = d->queuePolicies.indexOf(policy);
    if (index != -1) {
        d->queuePolicies.removeAt(index);
    }
}

bool Job::isFinished() const
{
    return d->finished;
}

Thread* Job::thread()
{
    return d->thread.fetchAndAddOrdered(0);
}

void Job::setFinished(bool status)
{
    d->finished = status;
}

QMutex* Job::mutex() const
{
    return &d->mutex;
}

}

#include "moc_Job.cpp"
