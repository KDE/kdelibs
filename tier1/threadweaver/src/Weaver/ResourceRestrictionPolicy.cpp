/* -*- C++ -*-

   This file implements the ResourceRestrictionPolicy class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005 Mirko Boehm $
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

   $Id: Job.h 32 2005-08-17 08:38:01Z mirko $
*/

#include "ResourceRestrictionPolicy.h"

#include <QtCore/QList>
#include <QtCore/QMutex>

#include "DebuggingAids.h"

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

int ResourceRestrictionPolicy::cap() const
{
    QMutexLocker l ( & d->mutex );
    return d->cap;
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
        debug ( 4, "ResourceRestrictionPolicy::free: job %p done.\n", (void*)job );
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
