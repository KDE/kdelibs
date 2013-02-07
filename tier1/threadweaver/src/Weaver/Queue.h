/* -*- C++ -*-

   This file is part of ThreadWeaver, a KDE framework.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2013 Mirko Boehm $
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

#ifndef THREADWEAVER_QUEUE_H
#define THREADWEAVER_QUEUE_H

#include <QtCore/QObject>
#include "WeaverInterface.h"

namespace ThreadWeaver {

class Queue : public QObject, public WeaverInterface
{
    Q_OBJECT
public:
    /** A Queue manages a queue of Jobs. */
    explicit Queue(QObject *parent = 0);
    
Q_SIGNALS:
    /** This signal is emitted when the Weaver has finished ALL currently
    queued jobs.
    If a number of jobs is enqueued sequentially, this signal might be
    emitted a couple of times (what happens is that all already queued
    jobs have been processed while you still add new ones). This is
    not a bug, but the intended behaviour. */
    void finished ();

    /** Thread queueing has been suspended.
    When suspend is called with, all threads are
    allowed to finish their job. When the last thread
    finished, this signal is emitted. */
    void suspended ();

    /** This signal is emitted when a job is finished.  In addition,
    the Job itself emits a done() signal just before the jobDone() signal
    is emitted.
    */
    void jobDone ( ThreadWeaver::Job* );
    // FIXME (0.7) provide jobStarted and jobFailed signals
    // FIXME (0.7) or remove all these, and add them to WeaverObserver

    /** The Weaver's state has changed. */
    void stateChanged ( ThreadWeaver::State* );
};

}

#endif // THREADWEAVER_QUEUE_H
