/* -*- C++ -*-

   This file implements the Thread Logger.

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

#include <weaver.h>
#include "weaverlogger.h"

namespace ThreadWeaver {

    extern void debug(int severity, const char * cformat, ...);

    WeaverThreadLogger::WeaverThreadLogger( QObject *parent, const char *name)
	: WeaverExtension (parent, name)
    {
    }

    WeaverThreadLogger::~WeaverThreadLogger ()
    {
    }

    void WeaverThreadLogger::threadCreated (ThreadWeaver::Thread *thread)
    {
	debug ( 1, "WeaverThreadLogger: thread (ID: %i) created.\n",
		thread->id() );
    }

    void WeaverThreadLogger::threadDestroyed (ThreadWeaver::Thread *thread)
    {
	debug ( 1, "WeaverThreadLogger: thread (ID: %i) destroyed.\n",
		thread->id() );
    }

    void WeaverThreadLogger::threadBusy (ThreadWeaver::Thread *thread)
    {
	debug ( 1, "WeaverThreadLogger: thread (ID: %i) is processing a job.\n",
		thread->id() );
    }

    void WeaverThreadLogger::threadSuspended (ThreadWeaver::Thread *thread)
    {
	debug ( 1, "WeaverThreadLogger: thread (ID: %i) suspended.\n",
		thread->id() );
    }

}
