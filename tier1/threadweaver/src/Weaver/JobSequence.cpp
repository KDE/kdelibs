/* -*- C++ -*-

   This file implements the JobSequence class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004-2013 Mirko Boehm $
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

   $Id: DebuggingAids.h 30 2005-08-16 16:16:04Z mirko $
*/

#include "JobSequence.h"
#include "ManagedJobPointer.h"
#include "QueueAPI.h"
#include "DebuggingAids.h"
#include "DependencyPolicy.h"

namespace ThreadWeaver {

JobSequence::JobSequence(QObject *parent)
    : JobCollection ( parent )
    , d(0)
{
}

void JobSequence::aboutToBeQueued_locked (QueueAPI *api )
{
    Q_ASSERT(!mutex()->tryLock());
    REQUIRE (api != 0);

    const int jobs = jobListLength_locked();
    if (jobs > 0) {
        DependencyPolicy::instance().addDependency(jobAt(0).data(), this);
        // set up the dependencies:
        for (int i = 1; i < jobs; ++i) {
            JobPointer jobA = jobAt(i);
            JobPointer jobB = jobAt(i-1);
            P_ASSERT(jobA != 0);
            P_ASSERT(jobB != 0);
            DependencyPolicy::instance().addDependency(jobA.data(), jobB.data());
        }
    }
    JobCollection::aboutToBeQueued_locked(api);
}

void JobSequence::elementFinished(JobPointer job, Thread *thread)
{
    REQUIRE ( job != 0 );
    JobCollection::elementFinished(job, thread);
    if (!job->success()) {
        stop(job);
    }
    QMutexLocker l(mutex()); Q_UNUSED(l);
    if (jobListLength_locked() > 0) {
        DependencyPolicy::instance().removeDependency(jobAt(0).data(), this);
    }
}

}

#include "JobSequence.moc"
