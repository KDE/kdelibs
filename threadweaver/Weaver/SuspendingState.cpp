/* -*- C++ -*-

   This file implements the SuspendingState class.

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

   $Id: SuspendingState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "SuspendingState.h"

#include "State.h"
#include "WeaverImpl.h"
#include "ThreadWeaver.h"

using namespace ThreadWeaver;

void SuspendingState::suspend()
{
    // this request is not handled in Suspending state (we are already
    // suspending...)
}

void SuspendingState::resume()
{
    weaver()->setState ( WorkingHard );
}

void SuspendingState::activated()
{
    if ( weaver()->activeThreadCount() == 0 )
    {
        weaver()->setState( Suspended );
    }
}

Job* SuspendingState::applyForWork ( Thread *th,  Job* previous )
{
    if ( weaver()->activeThreadCount() == 0 )
    {
        weaver()->setState ( Suspended );
    }
    weaver()->waitForAvailableJob ( th );
    return weaver()->applyForWork ( th,  previous );
}

void SuspendingState::waitForAvailableJob ( Thread *th )
{
    weaver()->blockThreadUntilJobsAreBeingAssigned( th );
}

StateId SuspendingState::stateId() const
{
    return Suspending;
}
