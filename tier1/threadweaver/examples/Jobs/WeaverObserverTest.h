/* -*- C++ -*-

   This file declares the WeaverObserverTest class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: WeaverObserverTest.h 31 2005-08-16 16:21:10Z mirko $
*/

#ifndef WEAVEROBSERVERTEST_H
#define WEAVEROBSERVERTEST_H

#include "WeaverObserver.h"

namespace ThreadWeaver {

    /** WeaverObserverTest is a simple test of the WeaverObserver class that
        prints debug messages when signals are received. */
    class WeaverObserverTest : public WeaverObserver
    {
        Q_OBJECT
    public:
        WeaverObserverTest ( QObject *parent=0 );
    protected Q_SLOTS:
        void slotWeaverStateChanged ( ThreadWeaver::State* );
        void slotThreadStarted ( ThreadWeaver::Thread* );
        void slotThreadBusy ( ThreadWeaver::Thread*, ThreadWeaver::Job* );
        void slotThreadSuspended ( ThreadWeaver::Thread* );
        void slotThreadExited ( ThreadWeaver::Thread* );
    };

}

#endif
