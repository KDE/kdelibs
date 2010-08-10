/*
 * Copyright 2010, Michael Leupold <lemma@confuego.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef JOBQUEUE_H
#define JOBQUEUE_H

// forward declarations
class QueuedJob;
class JobQueuePrivate;

/**
 * Generic class which is used for queueing objects of class QueuedJob
 * and executing them one after another.
 *
 * This class does intentionally not inherit QObject so it can be inherited
 * by other QObject-based classes.
 */
class JobQueue
{
public:
   /**
    * Constructor.
    */
   JobQueue();
   
   /**
    * Destructor.
    */
   virtual ~JobQueue();
   
   /**
    * Enqueue a job for processing.
    *
    * @param job the job to enqueue
    * @param inFront true to enqueue the job in front so it is the next
    *                job to be processed
    */
   void enqueue(QueuedJob *job, bool inFront = false);
   
private:
   JobQueuePrivate *d;
};

#endif // JOBQUEUE_H
