/* -*- C++ -*-

   This file implements the Weaver class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
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

#include "ThreadWeaver.h"
#include "WeaverObserver.h"
#include "WeaverImpl.h"

namespace ThreadWeaver {

    Weaver* Weaver::m_instance;

    Weaver::Weaver ( QObject* parent, int inventoryMin, int inventoryMax )
        : WeaverInterface( parent )
    {
        m_weaverinterface = makeWeaverImpl ( inventoryMin,  inventoryMax );
        connect ( m_weaverinterface,  SIGNAL ( finished() ),  SIGNAL ( finished() ) );
        connect ( m_weaverinterface,  SIGNAL ( suspended() ),  SIGNAL ( suspended() ) );
        connect ( m_weaverinterface,  SIGNAL ( jobDone( Job* ) ),  SIGNAL ( jobDone ( Job* ) ) );
    }

    Weaver::~Weaver()
    {
        delete m_weaverinterface;
    }

    WeaverInterface* Weaver::makeWeaverImpl(int inventoryMin, int inventoryMax )
    {
        return new WeaverImpl ( this, inventoryMin,  inventoryMax );
    }

    const State& Weaver::state() const
    {
        return m_weaverinterface->state();
    }

    void Weaver::registerObserver ( WeaverObserver *ext )
    {
        m_weaverinterface->registerObserver ( ext );
    }

    Weaver* Weaver::instance()
    {
	if (m_instance == 0)
	{   // we try to avoid the expensive mutex-lock operation if possible:
            static QMutex mutex;
            QMutexLocker l(&mutex);
            if ( m_instance == 0 )
            {
                m_instance = new Weaver();
            }
        }
	return m_instance;
    }

    void Weaver::enqueue (Job* j)
    {
        m_weaverinterface->enqueue ( j );
    }

    void Weaver::enqueue (const QList<Job*>& jobs)
    {
        m_weaverinterface->enqueue ( jobs );
    }

    bool Weaver::dequeue (Job* j)
    {
        return m_weaverinterface->dequeue ( j );
    }

    void Weaver::dequeue ()
    {
        return m_weaverinterface->dequeue();
    }

    void Weaver::finish ()
    {
        return m_weaverinterface->finish ();
    }

    void Weaver::suspend ()
    {
        return m_weaverinterface->suspend();
    }

    void Weaver::resume ()
    {
        return m_weaverinterface->resume();
    }

    bool Weaver::isEmpty()
    {
        return m_weaverinterface->isEmpty();
    }

    bool Weaver::isIdle()
    {
        return m_weaverinterface->isIdle();
    }

    int Weaver::queueLength()
    {
        return m_weaverinterface->queueLength();
    }

    int Weaver::noOfThreads()
    {
        return m_weaverinterface->noOfThreads();
    }

}
