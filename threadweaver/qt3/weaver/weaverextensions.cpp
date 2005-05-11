/* -*- C++ -*-

   This file implements the Weaver Extensions basics.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Id$
*/

#include "weaverextensions.h"
#include "weaver.h"

namespace ThreadWeaver {

    WeaverExtension::WeaverExtension ( QObject *parent, const char *name)
        : QObject (parent, name)
    {
    }

    void WeaverExtension::attach (Weaver *w)
    {
	connect (w, SIGNAL (threadCreated (Thread *) ),
		 SLOT (threadCreated (Thread *) ) );
	connect (w, SIGNAL (threadDestroyed (Thread *) ),
		 SLOT (threadDestroyed (Thread *) ) );
	connect (w, SIGNAL (threadBusy (Thread *) ),
		 SLOT (threadBusy (Thread *) ) );
	connect (w, SIGNAL (threadSuspended (Thread *) ),
		 SLOT (threadSuspended (Thread *) ) );
    }

    WeaverExtension::~WeaverExtension()
    {
    }

    void WeaverExtension::threadCreated (Thread *)
    {
    }

    void WeaverExtension::threadDestroyed (Thread *)
    {
    }

    void WeaverExtension::threadBusy (Thread *)
    {
    }

    void WeaverExtension::threadSuspended (Thread *)
    {
    }

}

// #include "weaverextension.moc"
