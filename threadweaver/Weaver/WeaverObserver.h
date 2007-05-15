/* -*- C++ -*-

   This file declares the WeaverObserver class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, 2006 Mirko Boehm $
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

   $Id: WeaverObserver.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef WEAVEROBSERVER_H
#define WEAVEROBSERVER_H

#include <QtCore/QObject>

#include <threadweaver/threadweaver_export.h>

namespace ThreadWeaver {

    class Job;
    class State;
    class Thread;

    /** Observers provides signals on some Weaver events that are
        otherwise only available through objects of different classes
        (threads, jobs). Usually, access to the signals of those objects
        is not provided through the ThreadWeaver API. Use an observer to receive
        notice, for example, on thread activity.

        Observers are registered through the WeaverInterface API
        (see WeaverInterface::registerObserver).

        To unregister, simply delete the observer.

        Observers are meant to be used within the controlling thread (usually
        the main thread), that creates the Jobs. If this is not the case, be
        aware not to delete Job objects externally that you reference in a
        WeaverObserver implementation.
    */
    class THREADWEAVER_EXPORT WeaverObserver : public QObject
    {
        Q_OBJECT
    public:
        explicit WeaverObserver ( QObject *parent = 0 );
        virtual ~WeaverObserver();
    Q_SIGNALS:
        void threadStarted ( ThreadWeaver::Thread* );
        void threadBusy( ThreadWeaver::Thread*, ThreadWeaver::Job* );
        void threadSuspended ( ThreadWeaver::Thread* );
        void threadExited ( ThreadWeaver::Thread* );
        void weaverStateChanged ( ThreadWeaver::State* );

    private:
        class Private;
        Private * const d;
    };

}

#endif
