/*  This file is part of the KDE project
    Copyright (C) 2000 Matej Koss <koss@miesto.sk>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#include <QWidget>
#include <QTimer>
#include <QEvent>

class KAbstractWidgetJobTracker::Private
{
public:
    Private(KAbstractWidgetJobTracker *parent)
        : q(parent), job(0),
          stopOnClose(true), onlyClean(false) { }

    KAbstractWidgetJobTracker *const q;
    KJob *job;
    bool stopOnClose;
    bool onlyClean;

    void _k_installEventFilter();
};

KAbstractWidgetJobTracker::KAbstractWidgetJobTracker(QWidget *parent)
    : KJobTrackerInterface(parent), d(new Private(this))
{
    QTimer::singleShot(0, this, SLOT(_k_installEventFilter()));
}

KAbstractWidgetJobTracker::~KAbstractWidgetJobTracker()
{
    delete d;
}

void KAbstractWidgetJobTracker::registerJob(KJob *job)
{
    if (d->job) {
        unregisterJob(d->job);
    }

    d->job = job;
    KJobTrackerInterface::registerJob(job);
}

void KAbstractWidgetJobTracker::unregisterJob(KJob *job)
{
    d->job = 0;
    KJobTrackerInterface::unregisterJob(job);
}

void KAbstractWidgetJobTracker::setStopOnClose(bool stopOnClose)
{
    d->stopOnClose = stopOnClose;
}

bool KAbstractWidgetJobTracker::stopOnClose() const
{
    return d->stopOnClose;
}

void KAbstractWidgetJobTracker::setOnlyClean(bool onlyClean)
{
    d->onlyClean = onlyClean;
}

bool KAbstractWidgetJobTracker::onlyClean() const
{
    return d->onlyClean;
}

void KAbstractWidgetJobTracker::finished(KJob */*job*/)
{
    // clean or delete dialog
    if (d->onlyClean) {
        slotClean();
    } else {
        deleteLater();
    }
}

void KAbstractWidgetJobTracker::slotStop()
{
    if (d->job) {
        d->job->kill(); // this will call slotFinished
        d->job = 0L;
    } else {
        finished(0); // here we call it ourselves
    }

    emit stopped();
}

void KAbstractWidgetJobTracker::slotSuspend()
{
    if (d->job) {
        d->job->suspend();
    }

    emit suspend();
}

void KAbstractWidgetJobTracker::slotResume()
{
    if (d->job ) {
        d->job->resume();
    }

    emit resume();
}

void KAbstractWidgetJobTracker::slotClean()
{
    widget()->hide();
}

bool KAbstractWidgetJobTracker::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==widget() && event->type()==QEvent::Close) {
        // kill job when desired
        if (d->stopOnClose) {
            slotStop();
        } else if (d->onlyClean) { // clean or delete dialog
            slotClean();
        } else {
            deleteLater();
        }
    }

    return KJobTrackerInterface::eventFilter(obj, event);
}

void KAbstractWidgetJobTracker::Private::_k_installEventFilter()
{
    q->widget()->installEventFilter(q);
}

#include "kabstractwidgetjobtracker.moc"
