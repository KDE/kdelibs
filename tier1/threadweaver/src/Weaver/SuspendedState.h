/* -*- C++ -*-

   This file declares the SuspendedState class.

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

   $Id: SuspendedState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef SuspendedState_H
#define SuspendedState_H

#ifndef THREADWEAVER_PRIVATE_API
#define THREADWEAVER_PRIVATE_API
#endif

#include "WeaverImplState.h"
#include "WeaverImpl.h"

namespace ThreadWeaver {

/** In SuspendedState, jobs are queued, but will not be executed. All
 *  thread remains blocked.  */
class SuspendedState : public WeaverImplState
{
public:
    explicit SuspendedState( WeaverImpl *weaver);
    /** Suspend job processing. */
    void suspend();
    /** Resume job processing. */
    void resume();
    /** Assign a job to an idle thread. */
    Job* applyForWork ( Thread *th,  Job* previous );
    /** reimpl */
    StateId stateId() const;
};

}

#endif // SuspendedState_H
