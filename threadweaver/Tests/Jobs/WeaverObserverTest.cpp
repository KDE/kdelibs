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

#include "DebuggingAids.h"
#include "State.h"
#include "Thread.h"

#include "WeaverObserverTest.h"

namespace ThreadWeaver {

    WeaverObserverTest::WeaverObserverTest ( QObject *parent )
        : WeaverObserver ( parent )
    {
        connect ( this,  SIGNAL ( weaverStateChanged ( State* ) ),
                  SLOT ( slotWeaverStateChanged ( State* ) ) );
        connect ( this,  SIGNAL ( threadStarted( Thread* ) ),
                  SLOT ( slotThreadStarted ( Thread* ) ) );
        connect ( this,  SIGNAL ( threadBusy ( Thread *,  Job* ) ),
                  SLOT ( slotThreadBusy ( Thread *,  Job* ) ) );
        connect ( this,  SIGNAL ( threadSuspended ( Thread* ) ),
                  SLOT ( slotThreadSuspended ( Thread* ) ) );
        connect ( this,  SIGNAL ( threadExited ( Thread* ) ),
                  SLOT ( slotThreadExited ( Thread* ) ) );
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
