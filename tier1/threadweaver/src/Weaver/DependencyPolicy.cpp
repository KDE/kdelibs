/* -*- C++ -*-

This file implements the DependencyPolicy class.

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

$Id: DebuggingAids.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#include "DependencyPolicy.h"

#include <QtCore/QMutex>
#include <QtCore/QDebug>

#include "Job.h"
#include "DebuggingAids.h"

using namespace ThreadWeaver;

typedef QMultiMap<JobInterface*, JobInterface*> JobMultiMap;

class DependencyPolicy::Private
{
public:
    /** A container to keep track of Job dependencies.
        For each dependency A->B, which means Job B depends on Job A and
        may only be executed after A has been finished, an entry will be
        added with key A and value B. When A is finished, the entry will
        be removed. */
    JobMultiMap& dependencies()
    {
        static JobMultiMap depMap;
        return depMap;
    }

    QMutex& mutex()
    {
        static QMutex s_mutex;
        return s_mutex;
    }

};

DependencyPolicy::DependencyPolicy()
    : QueuePolicy()
    , d ( new Private() )
{
}

DependencyPolicy::~DependencyPolicy()
{
    delete d;
}

void DependencyPolicy::addDependency(JobInterface *jobA, JobInterface *jobB )
{
    // jobA depends on jobB
    REQUIRE ( jobA != 0 && jobB != 0 && jobA != jobB );

    jobA->assignQueuePolicy( this );
    jobB->assignQueuePolicy( this );
    QMutexLocker l( & d->mutex() );
    d->dependencies().insert( jobA, jobB );

    ENSURE ( d->dependencies().contains (jobA));
}

bool DependencyPolicy::removeDependency(JobInterface *jobA, JobInterface *jobB )
{
    REQUIRE (jobA != 0 && jobB != 0);
    bool result = false;
    QMutexLocker l( & d->mutex() );

    // there may be only one (!) occurrence of [this, dep]:
    QMutableMapIterator<JobInterface*, JobInterface*> it( d->dependencies () );
    while ( it.hasNext() )
    {
        it.next();
        if ( it.key()==jobA && it.value()==jobB )
        {
            it.remove();
            result = true;
            break;
        }
    }

    ENSURE ( ! d->dependencies().keys(jobB).contains(jobA) );
    return result;
}

void DependencyPolicy::resolveDependencies( JobInterface* job )
{
    if ( job->success() )
    {
        QMutexLocker l( & d->mutex() );
        QMutableMapIterator<JobInterface*, JobInterface*> it( d->dependencies() );
        // there has to be a better way to do this: (?)
        while ( it.hasNext() )
        {   // we remove all entries where jobs depend on *this* :
            it.next();
            if ( it.value()==job )
            {
                it.remove();
            }
        }
    }
}

QList<JobInterface *> DependencyPolicy::getDependencies(JobInterface *job ) const
{
    REQUIRE (job != 0);
    QList<JobInterface*> result;
    JobMultiMap::const_iterator it;
    QMutexLocker l( & d->mutex() );

    for ( it = d->dependencies().constBegin(); it != d->dependencies().constEnd(); ++it )
    {
        if ( it.key() == job )
        {
            result.append( it.value() );
        }
    }
    return result;
}

bool DependencyPolicy::hasUnresolvedDependencies(JobInterface *job ) const
{
    REQUIRE (job != 0);
    QMutexLocker l( & d->mutex() );
    return d->dependencies().contains( job );
}

DependencyPolicy& DependencyPolicy::instance ()
{
    static DependencyPolicy policy;
    return policy;
}

bool DependencyPolicy::canRun(JobInterface *job )
{
    REQUIRE (job != 0);
    return ! hasUnresolvedDependencies( job );
}

void DependencyPolicy::free(JobInterface *job )
{
    REQUIRE (job != 0);
    if ( job->success() )
    {
        resolveDependencies( job );
        debug( 3, "DependencyPolicy::free: dependencies resolved for job %p.\n", (void*)job);
    } else {
        debug( 3, "DependencyPolicy::free: not resolving dependencies for %p (execution not successful).\n",
               (void*)job);
    }
    ENSURE ( ( ! hasUnresolvedDependencies( job ) && job->success() ) || ! job->success() );
}

void DependencyPolicy::release(JobInterface *job )
{
    REQUIRE (job != 0); Q_UNUSED(job)
}

void DependencyPolicy::destructed(JobInterface *job )
{
    REQUIRE (job != 0);
    resolveDependencies ( job );
}

void DependencyPolicy::dumpJobDependencies()
{
    QMutexLocker l( & d->mutex() );

    debug ( 0, "Job Dependencies (left depends on right side):\n" );
    for ( JobMultiMap::const_iterator it = d->dependencies().constBegin(); it != d->dependencies().constEnd(); ++it )
    {
        debug( 0, "  : %p <-- %p\n", (void*)it.key(), (void*)it.value());
    }
    debug ( 0, "-----------------\n" );
}

