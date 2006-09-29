/* -*- C++ -*-

   This file implements the Thread class.

   Thread is not a part of the public interface of the ThreadWeaver library.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Id: Thread.cpp 25 2005-08-14 12:41:38Z mirko $
*/

#ifndef THREADWEAVER_THREAD_P_H
#define THREADWEAVER_THREAD_P_H

#include "Thread.h"

using namespace ThreadWeaver;


// FIXME (0.7) it looks like we do not need this if the Thread object is
// moved to it's own thread (weaverimpl does that)
class ThreadWeaver::ThreadRunHelper : public QObject
{
    Q_OBJECT
public:
    explicit ThreadRunHelper ();

signals: // see Thread:

    /** The thread has been started. */
    void started ( Thread* );
    /** The thread started to process a job. */
    void jobStarted ( Thread*,  Job* );
    /** The thread finished to execute a job. */
    void jobDone ( Job* );

private:
    Job* m_job;

public:
    void run ( WeaverImpl *parent, Thread* th );

    void requestAbort();
};

#endif
