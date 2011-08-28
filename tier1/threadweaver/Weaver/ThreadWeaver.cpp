/* -*- C++ -*-

This file implements the Weaver class.

$ Author: Mirko Boehm $
$ Copyright: (C) 2005, 2006 Mirko Boehm $
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

$Id: ThreadWeaver.cpp 30 2005-08-16 16:16:04Z mirko $
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
        : weaverinterface ( 0)
    {}

    WeaverInterface* weaverinterface;
};

Weaver::Weaver ( QObject* parent )
    : WeaverInterface( parent )
    , d (new Private)
{
    d->weaverinterface = makeWeaverImpl();
    connect ( d->weaverinterface, SIGNAL ( finished() ), SIGNAL ( finished() ) );
    connect ( d->weaverinterface, SIGNAL ( suspended() ), SIGNAL ( suspended() ) );
    connect ( d->weaverinterface, SIGNAL ( jobDone( ThreadWeaver::Job* ) ),
              SIGNAL ( jobDone ( ThreadWeaver::Job* ) ) );
}

Weaver::~Weaver()
{
    delete d->weaverinterface;
    delete d;
}

WeaverInterface* Weaver::makeWeaverImpl()
{
    return new WeaverImpl ( this );
}

const State& Weaver::state() const
{
    return d->weaverinterface->state();
}

void Weaver::registerObserver ( WeaverObserver *ext )
{
    d->weaverinterface->registerObserver ( ext );
}

Weaver* Weaver::instance()
{
    /** The application-global Weaver instance.

    This  instance will only be created if this method is actually called
    in the lifetime of the application. */
    static Weaver* s_instance;

    if ( s_instance == 0 )
    {   // we try to avoid the expensive mutex-lock operation if possible:
        static QMutex mutex;
        QMutexLocker l(&mutex);
        if ( s_instance == 0 )
        {
            s_instance = new Weaver();
        }
    }
    return s_instance;
}

void Weaver::enqueue (Job* j)
{
    d->weaverinterface->enqueue ( j );
}

bool Weaver::dequeue (Job* j)
{
    return d->weaverinterface->dequeue ( j );
}

void Weaver::dequeue ()
{
    return d->weaverinterface->dequeue();
}

void Weaver::finish ()
{
    return d->weaverinterface->finish ();
}

void Weaver::suspend ()
{
    return d->weaverinterface->suspend();
}

void Weaver::resume ()
{
    return d->weaverinterface->resume();
}

bool Weaver::isEmpty() const
{
    return d->weaverinterface->isEmpty();
}

bool Weaver::isIdle() const
{
    return d->weaverinterface->isIdle();
}

int Weaver::queueLength() const
{
    return d->weaverinterface->queueLength();
}

void Weaver::setMaximumNumberOfThreads( int cap )
{
    d->weaverinterface->setMaximumNumberOfThreads( cap );
}

int Weaver::currentNumberOfThreads() const
{
    return d->weaverinterface->currentNumberOfThreads();
}

int Weaver::maximumNumberOfThreads() const
{
    return d->weaverinterface->maximumNumberOfThreads();
}

void Weaver::requestAbort()
{
    d->weaverinterface->requestAbort();
}

#include "ThreadWeaver.moc"
