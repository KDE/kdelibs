/*  This file is part of the KDE project
    Copyright (C) 2000 Matej Koss <koss@miesto.sk>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2007 Rafael Fernández López <ereslibre@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "kabstractwidgetjobtracker.h"
#include "kabstractwidgetjobtracker_p.h"

#include <QWidget>
#include <QTimer>
#include <QEvent>
#include <QMap>

#include <kdebug.h>

KAbstractWidgetJobTracker::KAbstractWidgetJobTracker(QWidget *parent)
    : KJobTrackerInterface(parent)
    , d(new Private(this))
{
}

KAbstractWidgetJobTracker::~KAbstractWidgetJobTracker()
{
    delete d;
}

void KAbstractWidgetJobTracker::registerJob(KJob *job)
{
    KJobTrackerInterface::registerJob(job);
}

void KAbstractWidgetJobTracker::unregisterJob(KJob *job)
{
    KJobTrackerInterface::unregisterJob(job);
}

void KAbstractWidgetJobTracker::setStopOnClose(KJob *job, bool stopOnClose)
{
    d->setStopOnClose(job, stopOnClose);
}

bool KAbstractWidgetJobTracker::stopOnClose(KJob *job) const
{
    return d->stopOnClose(job);
}

void KAbstractWidgetJobTracker::setAutoDelete(KJob *job, bool autoDelete)
{
    d->setAutoDelete(job, autoDelete);
}

bool KAbstractWidgetJobTracker::autoDelete(KJob *job) const
{
    return d->autoDelete(job);
}

void KAbstractWidgetJobTracker::finished(KJob *job)
{
    Q_UNUSED(job);
}

void KAbstractWidgetJobTracker::slotStop(KJob *job)
{
    if (job) {
        job->kill(KJob::EmitResult); // notify that the job has been killed
        emit stopped(job);
    }
}

void KAbstractWidgetJobTracker::slotSuspend(KJob *job)
{
    if (job) {
        job->suspend();
        emit suspend(job);
    }
}

void KAbstractWidgetJobTracker::slotResume(KJob *job)
{
    if (job) {
        job->resume();
        emit resume(job);
    }
}

void KAbstractWidgetJobTracker::slotClean(KJob *job)
{
    Q_UNUSED(job);
}

#include "kabstractwidgetjobtracker.moc"
