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

#include "queuedjob.h"
#include "jobqueue.h"

class QueuedJobPrivate
{
public:
    JobQueue *m_queue;
    bool m_finished;
    bool m_enqueued;
};

QueuedJob::QueuedJob(JobQueue *queue)
    : d(new QueuedJobPrivate)
{
    Q_ASSERT(queue);

    d->m_queue = queue;
    d->m_finished = false;
    d->m_enqueued = false;
}

QueuedJob::~QueuedJob()
{
    delete d;
}

bool QueuedJob::isImmediate() const
{
    return false;
}

bool QueuedJob::isFinished() const
{
    return d->m_finished;
}

void QueuedJob::enqueue(bool inFront)
{
    // if the job has already been finished or enqueued, handle this
    // gracefully.
    if(d->m_enqueued || d->m_finished) {
        return;
    }
    d->m_queue->enqueue(this, inFront);
    d->m_enqueued = true;
}

void QueuedJob::emitResult()
{
    d->m_finished = true;
    emit result(this);
    deleteLater();
}

#include "queuedjob.moc"
