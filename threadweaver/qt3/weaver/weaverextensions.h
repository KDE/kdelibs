/* -*- C++ -*-

   This file declares the Weaver Extensions basics.

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

#ifndef THUD_WEAVEREXTENSIONS_H
#define THUD_WEAVEREXTENSIONS_H

#include <qobject.h>

namespace ThreadWeaver {

    class Weaver;
    class Thread;

    /** A WeaverExtension can be attached to an existing Weaver object and
	will then receive signals on actions the Weaver takes, like starting
	to process a specific job, assigning a job to a thread or suspending a
	thread. It can be used to monitor the state of a program, but also,
	for example, to provide visualization of the Weaver's work load in GUI
	programs.  Derive from it to actually create an extension. */
    class WeaverExtension : public QObject
	{
	    Q_OBJECT
        public:
	    WeaverExtension ( QObject *parent = 0, const char *name = 0);
	    /** Attach() is a convenience method that will connect all our
	    slots to signals emitted by the weaver. It is also possible to
	    avoid attach() and only connect necessary signals. */
	    void attach (Weaver *);
	    virtual ~WeaverExtension() = 0;
        public slots:
            // these methods are implemented, but do nothing in the default
            // configuration
            // a thread is created:
            virtual void threadCreated (Thread *);
	    // a thread is destroyed:
	    virtual void threadDestroyed (Thread *);
	    // the thread is processing a job
	    virtual void threadBusy (Thread *);
	    // the thread is suspended and will be waked when jobs become
	    // available
	    virtual void threadSuspended (Thread *);
	};

} // namespace ThreadWeaver

#endif // THUD_WEAVEREXTENSIONS_H
