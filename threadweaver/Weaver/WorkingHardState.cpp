/* -*- C++ -*-

This file implements the WorkingHardState class.

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

$Id: WorkingHardState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "WorkingHardState.h"

#include <QtCore/QByteRef>

#include "Job.h"
#include "Thread.h"
#include "WeaverImpl.h"
#include "ThreadWeaver.h"
#include "DebuggingAids.h"


using namespace ThreadWeaver;

void WorkingHardState::activated()
{
    weaver()->assignJobs();
}

void WorkingHardState::suspend()
{
    weaver()->setState ( Suspending );
}

void WorkingHardState::resume()
{
}

Job* WorkingHardState::applyForWork ( Thread *th,  Job* )
{   // beware: this code is executed in the applying thread!
    debug ( 2, "WorkingHardState::applyForWork: thread %i applies for work "
            "in %s state.\n", th->id(),
            qPrintable ( weaver()->state().stateName() ) );

    Job *next = weaver()->takeFirstAvailableJob();

    if ( next )
    {
        weaver()->incActiveThreadCount();
        return next;
    } else {
        debug ( 2, "WorkingHardState::applyForWork: no work for thread %i, "
                "blocking it.\n", th->id() );
        weaver()->waitForAvailableJob( th );
        // this is no infinite recursion: the state may have changed
        // meanwhile, or jobs may have come available:
        return weaver()->applyForWork( th,  0 );
    }
}

void WorkingHardState::waitForAvailableJob ( Thread *th )
{
    weaver()->blockThreadUntilJobsAreBeingAssigned ( th );
}

StateId WorkingHardState::stateId() const
{
    return WorkingHard;
}
