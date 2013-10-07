/* -*- C++ -*-

   This file implements the StateImplementation class.

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

   $Id: InConstructionState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "WeaverImplState.h"

#include "WeaverImpl.h"

namespace ThreadWeaver {

WeaverImplState::WeaverImplState(Queue *weaver)
    : State(weaver)
{}

const State *WeaverImplState::state() const
{
    return this;
}

void WeaverImplState::shutDown()
{
    weaver()->shutDown_p();
}

WeaverImpl* WeaverImplState::weaver()
{
    Q_ASSERT ( dynamic_cast<WeaverImpl*> ( State::weaver() ) );
    return static_cast<WeaverImpl*> ( State::weaver() );
}

const WeaverImpl *WeaverImplState::weaver() const
{
    Q_ASSERT ( dynamic_cast<const WeaverImpl*> ( State::weaver() ) );
    return static_cast<const WeaverImpl*> ( State::weaver() );
}

void WeaverImplState::enqueueRaw(JobInterface *)
{
    Q_ASSERT(false); //should not be called, handled in WeaverImpl
}

bool WeaverImplState::dequeueRaw(JobInterface*)
{
    Q_ASSERT(false); //should not be called, handled in WeaverImpl
    return false;
}

void WeaverImplState::setMaximumNumberOfThreads(int cap)
{
    weaver()->setMaximumNumberOfThreads_p(cap);
}

int WeaverImplState::maximumNumberOfThreads() const
{
    return weaver()->maximumNumberOfThreads_p();
}

int WeaverImplState::currentNumberOfThreads() const
{
    return weaver()->currentNumberOfThreads_p();
}

void WeaverImplState::registerObserver(WeaverObserver *obs)
{
    weaver()->registerObserver_p(obs);
}

void WeaverImplState::enqueue(JobPointer job)
{
    weaver()->enqueue_p(job);
}

bool WeaverImplState::dequeue(JobPointer job)
{
    return weaver()->dequeue_p(job);
}

void WeaverImplState::dequeue()
{
    weaver()->dequeue_p();
}

void WeaverImplState::finish()
{
    weaver()->finish_p();
}

bool WeaverImplState::isEmpty() const
{
    return weaver()->isEmpty_p();
}

bool WeaverImplState::isIdle() const
{
    return weaver()->isIdle_p();
}

int WeaverImplState::queueLength() const
{
    return weaver()->queueLength_p();
}

void WeaverImplState::requestAbort()
{
    return weaver()->requestAbort_p();
}

void WeaverImplState::reschedule()
{
    // Not implemented, performed directly in WeaverImpl::reschedule().
}

void WeaverImplState::waitForAvailableJob(Thread *th)
{
    weaver()->blockThreadUntilJobsAreBeingAssigned(th);
}

}
