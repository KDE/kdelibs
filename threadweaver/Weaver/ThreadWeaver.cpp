/* -*- C++ -*-

This file implements the Weaver class.

$ Author: Mirko Boehm $
$ Copyright: (C) 2005, 2006 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://www.hackerbuero.org $
$ License: LGPL with the following explicit clarification:
This code may be linked against any version of the Qt toolkit
from Trolltech, Norway. $

$Id: ThreadWeaver.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include <QMutex>
#include <QMutexLocker>

#include "WeaverImpl.h"
#include "ThreadWeaver.h"
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
    connect ( d->weaverinterface, SIGNAL ( jobDone( Job* ) ), SIGNAL ( jobDone ( Job* ) ) );
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
