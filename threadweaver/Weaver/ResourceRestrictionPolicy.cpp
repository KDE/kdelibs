#include <QList>
#include <QMutex>
#include <QMutexLocker>

#include "DebuggingAids.h"
#include "ResourceRestrictionPolicy.h"

class Job;

using namespace ThreadWeaver;

class ResourceRestrictionPolicy::Private
{
public:
    Private ( int theCap )
        : cap ( theCap)
    {}

    int cap;
    QList<Job*> customers;
    QMutex mutex;
};

ResourceRestrictionPolicy::ResourceRestrictionPolicy ( int cap)
    : QueuePolicy ()
    , d (new Private (cap))
{
}

ResourceRestrictionPolicy::~ResourceRestrictionPolicy()
{
    delete d;
}

void ResourceRestrictionPolicy::setCap (int cap)
{
    QMutexLocker l ( & d->mutex );
    d->cap = cap;
}

bool ResourceRestrictionPolicy::canRun( Job* job )
{
    QMutexLocker l ( & d->mutex );
    if ( d->customers.size() < d->cap )
    {
        d->customers.append( job );
        return true;
    } else {
        return false;
    }
}

void ResourceRestrictionPolicy::free ( Job* job )
{
    QMutexLocker l ( & d->mutex );
    int position = d->customers.indexOf (job);

    if (position != -1)
    {
        debug ( 4, "ResourceRestrictionPolicy::free: job %p done.\n", job );
        d->customers.removeAt (position);
    }
}

void ResourceRestrictionPolicy::release ( Job* job )
{
    free (job);
}

void ResourceRestrictionPolicy::destructed ( Job* job )
{
    free (job);
}
