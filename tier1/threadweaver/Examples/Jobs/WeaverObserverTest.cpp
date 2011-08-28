/* -*- C++ -*-

   This file implements the WeaverObserverTest class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: WeaverObserverTest.cpp 31 2005-08-16 16:21:10Z mirko $
*/

#include "WeaverObserverTest.h"

#include "DebuggingAids.h"
#include "State.h"
#include "Thread.h"

namespace ThreadWeaver {

    WeaverObserverTest::WeaverObserverTest ( QObject *parent )
        : WeaverObserver ( parent )
    {
        connect ( this,  SIGNAL ( weaverStateChanged ( ThreadWeaver::State* ) ),
                  SLOT ( slotWeaverStateChanged ( ThreadWeaver::State* ) ) );
        connect ( this,  SIGNAL ( threadStarted( ThreadWeaver::Thread* ) ),
                  SLOT ( slotThreadStarted ( ThreadWeaver::Thread* ) ) );
        connect ( this,  SIGNAL ( threadBusy ( ThreadWeaver::Thread *,  ThreadWeaver::Job* ) ),
                  SLOT ( slotThreadBusy ( ThreadWeaver::Thread *,  ThreadWeaver::Job* ) ) );
        connect ( this,  SIGNAL ( threadSuspended ( ThreadWeaver::Thread* ) ),
                  SLOT ( slotThreadSuspended ( ThreadWeaver::Thread* ) ) );
        connect ( this,  SIGNAL ( threadExited ( ThreadWeaver::Thread* ) ),
                  SLOT ( slotThreadExited ( ThreadWeaver::Thread* ) ) );
    }

    void WeaverObserverTest::slotWeaverStateChanged ( State *state )
    {
        debug ( 0, "WeaverObserverTest: state changed to \"%s\".\n",
                state->stateName().toAscii().constData() );
    }

    void WeaverObserverTest::slotThreadStarted ( Thread* th )
    {
        debug ( 0, "WeaverObserverTest: thread %i started.\n",
                th->id() );
    }

    void WeaverObserverTest::slotThreadBusy ( Thread *th,  Job* )
    {
        debug ( 0, "WeaverObserverTest: thread %i busy.\n",
                th->id() );
    }

    void WeaverObserverTest::slotThreadSuspended ( Thread *th )
    {
        debug ( 0, "WeaverObserverTest: thread %i suspended.\n",
                th->id() );
    }

    void WeaverObserverTest::slotThreadExited ( Thread *th )
    {
        debug ( 0, "WeaverObserverTest: thread %i exited.\n",
                th->id() );
    }

}

#include "WeaverObserverTest.moc"
