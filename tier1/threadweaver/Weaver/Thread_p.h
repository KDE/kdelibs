/* -*- C++ -*-

   This file implements the Thread class.

   Thread is not a part of the public interface of the ThreadWeaver library.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $

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

   $Id: Thread.cpp 25 2005-08-14 12:41:38Z mirko $
*/

#ifndef THREADWEAVER_THREAD_P_H
#define THREADWEAVER_THREAD_P_H

#include "Thread.h"

using namespace ThreadWeaver;


// FIXME (0.7) it looks like we do not need this if the Thread object is
// moved to its own thread (weaverimpl does that)
class ThreadWeaver::ThreadRunHelper : public QObject
{
    Q_OBJECT
public:
    explicit ThreadRunHelper ();

Q_SIGNALS: // see Thread:

    /** The thread has been started. */
    void started ( ThreadWeaver::Thread* );
    /** The thread started to process a job. */
    void jobStarted ( ThreadWeaver::Thread*, ThreadWeaver::Job* );
    /** The thread finished to execute a job. */
    void jobDone ( ThreadWeaver::Job* );

private:
    Job* m_job;

public:
    void run ( WeaverImpl *parent, Thread* th );

    void requestAbort();
};

#endif
