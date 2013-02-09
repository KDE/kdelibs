/* -*- C++ -*-

This file implements the Weaver class.

$ Author: Mirko Boehm $
$ Copyright: (C) 2005-2013 Mirko Boehm $
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

#include "ThreadWeaver.h"

#include <QtCore/QMutex>

#include "WeaverImpl.h"
#include "WeaverObserver.h"

using namespace ThreadWeaver;

class Weaver::Private
{
public:
    Private ()
        : implementation(0)
    {}

    Queue* implementation;
};

Weaver::Weaver ( QObject* parent )
    : Queue( parent )
    , d (new Private)
{
    d->implementation = makeWeaverImpl();
    //FIXME move to makeWeaverImpl(), so that implementations can be replaced
    connect ( d->implementation, SIGNAL (finished()), SIGNAL (finished()) );
    connect ( d->implementation, SIGNAL (suspended()), SIGNAL (suspended()) );
    connect ( d->implementation, SIGNAL (jobDone(ThreadWeaver::Job*)),
              SIGNAL (jobDone(ThreadWeaver::Job*)) );
}

Weaver::~Weaver()
{
    delete d->implementation;
    delete d;
}

Queue *Weaver::makeWeaverImpl()
{
    return new WeaverImpl ( this );
}

const State& Weaver::state() const
{
    return d->implementation->state();
}

void Weaver::registerObserver ( WeaverObserver *ext )
{
    d->implementation->registerObserver ( ext );
}

Weaver* Weaver::instance()
{
    /** The application-global Weaver instance.

    This  instance will only be created if this method is actually called
    in the lifetime of the application. */
    static Weaver* s_instance;

    static QMutex mutex;
    QMutexLocker l(&mutex);
    if ( s_instance == 0 )
    {
        s_instance = new Weaver();
    }
    return s_instance;
}

void Weaver::enqueue (Job* j)
{
    d->implementation->enqueue ( j );
}

bool Weaver::dequeue (Job* j)
{
    return d->implementation->dequeue ( j );
}

void Weaver::dequeue ()
{
    return d->implementation->dequeue();
}

void Weaver::finish ()
{
    return d->implementation->finish ();
}

void Weaver::suspend ()
{
    return d->implementation->suspend();
}

void Weaver::resume ()
{
    return d->implementation->resume();
}

bool Weaver::isEmpty() const
{
    return d->implementation->isEmpty();
}

bool Weaver::isIdle() const
{
    return d->implementation->isIdle();
}

int Weaver::queueLength() const
{
    return d->implementation->queueLength();
}

void Weaver::setMaximumNumberOfThreads( int cap )
{
    d->implementation->setMaximumNumberOfThreads( cap );
}

int Weaver::currentNumberOfThreads() const
{
    return d->implementation->currentNumberOfThreads();
}

int Weaver::maximumNumberOfThreads() const
{
    return d->implementation->maximumNumberOfThreads();
}

void Weaver::requestAbort()
{
    d->implementation->requestAbort();
}

#include "ThreadWeaver.moc"
