/* -*- C++ -*-

   This file declares the DestructedState class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005-2013 Mirko Boehm $
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

   $Id: DestructedState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef DestructedState_H
#define DestructedState_H

#ifndef THREADWEAVER_PRIVATE_API
#define THREADWEAVER_PRIVATE_API
#endif

#include "WeaverImplState.h"

namespace ThreadWeaver {

class Queue;

/** DestructedState is only active after the thread have been destroyed by
 *  the destructor, but before superclass destructors have finished.
 */
class DestructedState : public WeaverImplState
{
public:
    explicit DestructedState(Queue *weaver);
    void shutDown();
    WeaverImpl* weaver();
    const WeaverImpl* weaver() const;
    void setMaximumNumberOfThreads(int cap);
    int maximumNumberOfThreads() const;
    int currentNumberOfThreads() const;
    void registerObserver(WeaverObserver *obs);
    void enqueue(Job *job);
    bool dequeue(Job* job);
    void dequeue();
    void finish();
    bool isEmpty() const;
    bool isIdle() const;
    int queueLength() const;
    void requestAbort();
    void suspend();
    void resume();
    Job* applyForWork ( Thread *th,  Job* previous );
    void waitForAvailableJob ( Thread *th );
    StateId stateId() const;
};

}

#endif // DestructedState_H
