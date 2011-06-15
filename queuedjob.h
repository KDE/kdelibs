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

#ifndef QUEUEDJOB_H
#define QUEUEDJOB_H

#include <QtCore/QObject>

// forward declaration
class JobQueuePrivate;
class JobQueue;
class QueuedJobPrivate;

/**
 * Job-class that provides the ability to queue jobs in a JobQueue.
 * 
 * TODO: switch to KCompositeJob infrastructure
 *
 * @note Just like KJob, QueuedJob is meant to be used in a fire-and-forget
 *       way. It's deleting itself when it has finished using deleteLater()
 *       so the job instance disappears after the next event loop run.
 */
class QueuedJob : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param queue queue the job will be enqueued into
     */
    explicit QueuedJob(JobQueue *queue);

    /**
     * Destructor.
     */
    virtual ~QueuedJob();

    /**
     * Checks if this call can be made immediately/synchronously.
     *
     * @return true if the call can be made immediately, false if it needs to
     *         be queued
     * @note The base implementation always returns false, so if your job class
     *       isn't immediate, you don't have to reimplement this method.
     */
    virtual bool isImmediate() const;

    /**
     * Check whether this job is finished.
     *
     * @return true if the job is finished, false else
     */
    bool isFinished() const;

    /**
     * Enqueue this job into the JobQueue passed in its constructor.
     *
     * @param inFront true to enqueue the job in front so it is the next job
     *                to be processed
     */
    void enqueue(bool inFront = false);

    /**
     * Execute this job synchronously. Implementation of this method should
     * not involve an event-loop and it should only be available if the job
     * advertised that it can be called immediately.
     *
     * @note This method has to call emitResult() before returning. Even if
     *       no one is interested in the signal it's used to mark this job
     *       as finished and call deleteLater().
     */
    virtual void exec() = 0;

protected:
    /**
     * Start the job asynchronously.
     *
     * When the job is finished, result() is emitted.
     */
    virtual void start() = 0;

    /**
     * Emit the result of this job, notifying every object listening that
     * it's finished.
     */
    void emitResult();

Q_SIGNALS:
    /**
     * Emit the result of this job.
     *
     * As this signal can't be emitted directly from derived classes, use
     * emitResult() to mark the job as finished and emit this signal.
     *
     * @param job The job that finished (this object)
     */
    void result(QueuedJob *job);

private:
    friend class JobQueuePrivate;
    QueuedJobPrivate *d;
};

#endif // QUEUEDJOB_H
