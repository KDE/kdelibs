/* -*- C++ -*-

   This file declares the JobCollection class.

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

#ifndef JOBCOLLECTION_H
#define JOBCOLLECTION_H

#include "Job.h"

namespace ThreadWeaver {

    class Thread;
    class JobCollectionJobRunner;

    /** A JobCollection is a vector of Jobs that will be queued together.
     *
     * In a JobCollection, the order of execution of the elements is not guaranteed.
     *
     * It is intended that the collection is set up first and then
     * queued. After queuing, no further jobs should be added to the collection.
	 *
	 * JobCollection emits a done(Job*) signal when all of the jobs in the collection
	 * have completed.
     */
    class THREADWEAVER_EXPORT JobCollection : public Job
    {
        friend class JobCollectionJobRunner;
        Q_OBJECT

    public:
        explicit JobCollection ( QObject *parent = 0 );
        ~JobCollection ();
        /** Append a job to the collection.

	To use JobCollection, create the Job objects first, add them to the
	collection, and then queue it. After the collection has been queued, no
	further Jobs are supposed to be added.
        */
        virtual void addJob ( Job* );

        /** Overload to manage recursive sets. */
        bool canBeExecuted();

    public Q_SLOTS:
        /** Stop processing, dequeue all remaining Jobs.
            job is supposed to be an element of the collection.
            */
        void stop ( ThreadWeaver::Job *job );

    protected:
        /** Overload to queue the collection. */
        void aboutToBeQueued ( WeaverInterface *weaver );

        /** Overload to dequeue the collection. */
        void aboutToBeDequeued ( WeaverInterface *weaver );

        /** Return a reference to the job in the job list at position i. */
        Job* jobAt( int i );

#ifdef __GNUC__
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 1)
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif
#endif
        /** Return the number of jobs in the joblist. */
        const int jobListLength(); // const qualifier is possibly BiC?

        /** Callback method for done jobs.
        */
        virtual void internalJobDone( Job* );

		/** 
	 	 * Perform the task usually done when one individual job is
	     * finished, but in our case only when the whole collection
	     * is finished or partly dequeued.  
		 */
		void finalCleanup();

	private Q_SLOTS:
		// called when an internal job runner emits its done(Job*) signal.
		// JobCollection emits done(Job*) after the last JobCollectionJobRunner finishes
		void jobRunnerDone();

    private:
        /** Overload the execute method. */
        void execute ( Thread * );

        /** Overload run().
            We have to. */
        void run() {}

        /** Dequeue all elements of the collection.
            Note: This will not dequeue the collection itself.
        */
        void dequeueElements();

      class Private;
      Private * const d;

    };

}

#endif
