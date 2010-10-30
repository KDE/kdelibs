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

#include "jobqueue.h"
#include "jobqueue_p.h"
#include "queuedjob.h"

#include <QtCore/QTimer>

JobQueuePrivate::JobQueuePrivate()
{
}

JobQueuePrivate::~JobQueuePrivate()
{
    // TODO: make sure there's no jobs left running when we go down
}

void JobQueuePrivate::enqueue(QueuedJob *job, bool inFront)
{
    Q_ASSERT(job);
    if(inFront) {
        m_jobs.prepend(job);
    } else {
        m_jobs.enqueue(job);
    }

    if(m_currentJob.isNull()) {
        QTimer::singleShot(0, this, SLOT(process()));
    }
}

void JobQueuePrivate::process()
{
    // check if already processing
    if(!m_currentJob.isNull()) {
        return;
    }

    // get a job to execute
    // as the job queue consists of QPointers, jobs which have been dismissed
    // (deleted) are automatically weeded out.
    while(m_currentJob.isNull()) {
        if(m_jobs.isEmpty()) {
            return;
        }
        m_currentJob = m_jobs.dequeue();
    }

    connect(m_currentJob.data(), SIGNAL(result(QueuedJob*)),
            SLOT(jobFinished(QueuedJob*)));
    m_currentJob->start();
}

void JobQueuePrivate::jobFinished(QueuedJob *job)
{
    Q_UNUSED(job);
    Q_ASSERT(job == m_currentJob);
    m_currentJob = 0;
    // keep processing if there's more jobs
    if(!m_jobs.isEmpty()) {
        QTimer::singleShot(0, this, SLOT(process()));
    }
}

JobQueue::JobQueue() : d(new JobQueuePrivate)
{
}

JobQueue::~JobQueue()
{
    delete d;
}

void JobQueue::enqueue(QueuedJob *job, bool inFront)
{
    d->enqueue(job, inFront);
}

#include "jobqueue_p.moc"
