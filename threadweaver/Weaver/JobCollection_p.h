/* -*- C++ -*-

   This file implements the JobCollection class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005, 2006 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: DebuggingAids.h 30 2005-08-16 16:16:04Z mirko $
*/

#ifndef THREADWEAVER_JOBCOLLECTION_P_H
#define THREADWEAVER_JOBCOLLECTION_P_H

#include "JobCollection.h"

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
