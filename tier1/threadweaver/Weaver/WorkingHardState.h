/* -*- C++ -*-

   This file declares the WorkingHardState class.

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

   $Id: WorkingHardState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef WorkingHardState_H
#define WorkingHardState_H

#include "StateImplementation.h"
#include "WeaverImpl.h"

#ifndef THREADWEAVER_PRIVATE_API
#define THREADWEAVER_PRIVATE_API
#endif

namespace ThreadWeaver {

    /* WorkingHardState handles the state dependent calls in the state where
       jobs are executed.  */
    class WorkingHardState : public StateImplementation
    {
    public:
	explicit WorkingHardState( WeaverImpl *weaver)
	    : StateImplementation (weaver)
	    {
	    }
	/** Suspend job processing. */
        virtual void suspend();
        /** Resume job processing. */
        virtual void resume();
        /** Assign a job to an idle thread. */
        virtual Job* applyForWork ( Thread *th,  Job* previous );
        /** Wait (by suspending the calling thread) until a job becomes available. */
        virtual void waitForAvailableJob ( Thread *th );
	/** Overload. */
	void activated();

        /** reimpl */
        StateId stateId() const;
    };

}

#endif // WorkingHardIState_H
