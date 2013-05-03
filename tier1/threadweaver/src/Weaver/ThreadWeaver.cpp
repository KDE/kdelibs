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

#include <QtCore/QCoreApplication>
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
    connect(d->implementation, SIGNAL (finished()), SIGNAL (finished()));
    connect(d->implementation, SIGNAL (suspended()), SIGNAL (suspended()));
    connect(d->implementation, SIGNAL (jobDone(ThreadWeaver::JobPointer)),
            SIGNAL(jobDone(ThreadWeaver::JobPointer)));
}

Weaver::~Weaver()
{
    if (d->implementation->state()->stateId()!=Destructed) {
        d->implementation->shutDown();
    }
    delete d->implementation;
    delete d;
}

Queue *Weaver::makeWeaverImpl()
{
    Q_ASSERT_X(qApp!=0, Q_FUNC_INFO, "Cannot create global ThreadWeaver instance before QApplication!");
    Queue *queue = new WeaverImpl(this);
    return queue;
}

void Weaver::shutDown()
{
    d->implementation->shutDown();
}

const State* Weaver::state() const
{
    return d->implementation->state();
}

void Weaver::registerObserver ( WeaverObserver *ext )
{
    d->implementation->registerObserver ( ext );
}

namespace {

class StaticThreadWeaverInstanceGuard : public QObject {
    Q_OBJECT
public:
    explicit StaticThreadWeaverInstanceGuard(QAtomicPointer<Weaver>& instance, QCoreApplication* app)
        : QObject(app)
        , instance_(instance)
    {
        Q_ASSERT_X(app!=0, Q_FUNC_INFO, "Calling ThreadWeaver::Weaver::instance() requires a QCoreApplication!");
        qAddPostRoutine(shutDownGlobalQueue);
    }

    ~StaticThreadWeaverInstanceGuard() {
        instance_.fetchAndStoreOrdered(0);
    }
private:
    static void shutDownGlobalQueue() {
        Weaver::instance()->shutDown();
        Q_ASSERT(Weaver::instance()->state()->stateId() == Destructed);
    }

    QAtomicPointer<Weaver>& instance_;
};

}

/** @brief The application-global Weaver instance.
 * This  instance will only be created if this method is actually called in the lifetime of the application.
 * The method will create the Weaver instance on first call. The Q(Core)Application object must exist at that time.
 * The instance will be deleted when Q(Core)Application is destructed. After that, the instance() method returns zero. */
Weaver* Weaver::instance()
{
    static QAtomicPointer<Weaver> s_instance(new Weaver(qApp));
    //Order is of importance here:
    //When s_instanceGuard is destructed (first, before s_instance), it sets the value of s_instance to zero. Next, qApp will delete
    //the object s_instance pointed to.
    static StaticThreadWeaverInstanceGuard* s_instanceGuard = new StaticThreadWeaverInstanceGuard(s_instance, qApp);
    Q_UNUSED(s_instanceGuard);
    Q_ASSERT_X(s_instance.load() == 0 || s_instance.load()->thread() == QCoreApplication::instance()->thread(),
               Q_FUNC_INFO,
               "The global ThreadWeaver queue needs to be instantiated (accessed first) from the main thread!");
    return s_instance.loadAcquire();
}

void Weaver::enqueue(JobPointer job)
{
    d->implementation->enqueue(job);
}

bool Weaver::dequeue(JobPointer job)
{
    return d->implementation->dequeue(job);
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
