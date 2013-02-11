/* -*- C++ -*-

This file implements the WorkingHardState class.

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

$Id: WorkingHardState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "WorkingHardState.h"

#include <QtCore/QByteRef>

#include "Job.h"
#include "Thread.h"
#include "WeaverImpl.h"
#include "ThreadWeaver.h"
#include "DebuggingAids.h"


namespace ThreadWeaver {

void WorkingHardState::activated()
{
    weaver()->assignJobs();
}

WorkingHardState::WorkingHardState(WeaverImpl *weaver)
    : WeaverImplState (weaver)
{
}

void WorkingHardState::suspend()
{
    weaver()->setState(Suspending);
}

void WorkingHardState::resume()
{
}

Job* WorkingHardState::applyForWork(Thread *th,  Job* previous)
{   // beware: this code is executed in the applying thread!
    debug ( 2, "WorkingHardState::applyForWork: thread %i applies for work in %s state.\n", th->id(),
            qPrintable ( weaver()->state().stateName() ) );
    Job *next = weaver()->takeFirstAvailableJobOrWait(th, previous);
    if ( next ) {
        return next;
    } else {
        // this is no infinite recursion: the state may have changed meanwhile, or jobs may have become available:
        return weaver()->applyForWork( th,  0 );
    }
}

StateId WorkingHardState::stateId() const
{
    return WorkingHard;
}

}
