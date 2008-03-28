/* -*- C++ -*-

   This file implements the JobSequence class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005, 2006 Mirko Boehm $
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

   $Id: DebuggingAids.h 30 2005-08-16 16:16:04Z mirko $
*/

#include "JobSequence.h"
#include "WeaverInterface.h"
#include "DebuggingAids.h"
#include "DependencyPolicy.h"

using namespace ThreadWeaver;

JobSequence::JobSequence ( QObject *parent )
    : JobCollection ( parent ), d ( 0 )
{
}

void JobSequence::aboutToBeQueued ( WeaverInterface *weaver )
{
    REQUIRE (weaver != 0);

    if ( jobListLength() > 1 )
    {
        // set up the dependencies:
        for ( int i = 1; i < jobListLength(); ++i )
        {
            Job* jobA = jobAt(i);
            Job* jobB = jobAt(i-1);
            P_ASSERT ( jobA != 0 );
            P_ASSERT ( jobB != 0 );
            DependencyPolicy::instance().addDependency ( jobA, jobB );
        }
    }

    JobCollection::aboutToBeQueued( weaver );
}

void JobSequence::internalJobDone( Job* job)
{
    REQUIRE ( job != 0 );
    
	if ( !job->success() )
    {
        stop( job );
    }

	JobCollection::internalJobDone(job);
}

#include "JobSequence.moc"
