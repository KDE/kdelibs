#include <QMutex>
#include <QtDebug>

#include "Job.h"
#include "DependencyPolicy.h"
#include "DebuggingAids.h"

using namespace ThreadWeaver;

class JobMultiMap : public QMultiMap<Job*, Job*> {};

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

void DependencyPolicy::addDependency( Job* jobA, Job* jobB )
{   // jobA depends on jobB
    jobA->assignQueuePolicy( this );
    QMutexLocker l( & d->mutex() );
    d->dependencies().insert( jobA, jobB );
}

bool DependencyPolicy::removeDependency( Job* jobA, Job* jobB )
{
    QMutexLocker l( & d->mutex() );

    // there may be only one (!) occurence of [this, dep]:
    QMutableMapIterator<Job*, Job*> it( d->dependencies () );
    while ( it.hasNext() )
	{
	    it.next();
	    if ( it.key()==jobA && it.value()==jobB )
	    {
		it.remove();
		return true;
	    }
	}
    return false;
}

void DependencyPolicy::resolveDependencies( Job* job )
{
    if ( job->success() )
    {
        QMutexLocker l( & d->mutex() );
        QMutableMapIterator<Job*, Job*> it( d->dependencies() );
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

QList<Job*> DependencyPolicy::getDependencies( Job* job ) const
{
    QList<Job*> result;
    QMutexLocker l( & d->mutex() );
    JobMultiMap::const_iterator it;

    for ( it = d->dependencies().begin(); it != d->dependencies().end(); ++it )
    {
        if ( it.key() == job )
        {
            result.append( it.value() );
        }
    }
    return result;
}

bool DependencyPolicy::hasUnresolvedDependencies( Job* job )
{
    QMutexLocker l( & d->mutex() );
    return d->dependencies().contains( job );
}

DependencyPolicy& DependencyPolicy::instance ()
{
    static DependencyPolicy policy;
    return policy;
}

bool DependencyPolicy::canRun( Job* job )
{
    return ! hasUnresolvedDependencies( job );
}

void DependencyPolicy::free( Job* job )
{
    resolveDependencies( job );
}

void DependencyPolicy::release( Job* )
{
}

void DependencyPolicy::destructed( Job* job )
{
    resolveDependencies ( job );
}

void DependencyPolicy::DumpJobDependencies()
{
    QMutexLocker l( & d->mutex() );

    debug ( 0, "Job Dependencies (left depends on right side):\n" );
    for ( JobMultiMap::const_iterator it = d->dependencies().begin(); it != d->dependencies().end(); ++it )
    {
        debug( 0, "  : %p (%s%s) <-- %p (%s%s)\n",
               it.key(),
               it.key()->objectName().isEmpty() ? "" : qPrintable ( it.key()->objectName() + QObject::tr ( " of type " ) ),
               it.key()->metaObject()->className(),
               it.value(),
               it.value()->objectName().isEmpty() ? "" : qPrintable ( it.value()->objectName() + QObject::tr ( " of type " ) ),
               it.value()->metaObject()->className() );
    }
    debug ( 0, "-----------------\n" );
}
