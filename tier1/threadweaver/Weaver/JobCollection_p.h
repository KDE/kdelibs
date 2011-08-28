/* -*- C++ -*-

   This file implements the JobCollection class.

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

#ifndef THREADWEAVER_JOBCOLLECTION_P_H
#define THREADWEAVER_JOBCOLLECTION_P_H

#include "JobCollection.h"
#include "WeaverInterface.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>

using namespace ThreadWeaver;

/* QPointers are used internally to be able to dequeue jobs at destruction
   time. The owner of the jobs could have deleted them in the meantime.
   We use a class instead of a typedef to be able to forward-declare the
   class in the declaration.
*/
class ThreadWeaver::JobCollectionJobRunner : public Job
{
    Q_OBJECT

public:
    JobCollectionJobRunner ( JobCollection* collection, Job* payload, QObject* parent );

    bool canBeExecuted();

    Job* payload ();

    void aboutToBeQueued ( WeaverInterface *weaver );

    void aboutToBeDequeued ( WeaverInterface *weaver );

    void execute ( Thread *t );

    int priority () const;

private:
    void run ();

    QPointer<Job> m_payload;
    JobCollection* m_collection;
};

#endif // THREADWEAVER_JOBCOLLECTION_P_H
