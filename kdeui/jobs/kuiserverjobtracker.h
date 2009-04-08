/*  This file is part of the KDE project
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

#ifndef KUISERVERJOBTRACKER_H
#define KUISERVERJOBTRACKER_H

#include <kdeui_export.h>
#include <kjobtrackerinterface.h>

class KJob;

/**
 * The interface to implement to track the progresses of a job.
 */
class KDEUI_EXPORT KUiServerJobTracker : public KJobTrackerInterface
{
    Q_OBJECT

public:
    /**
     * Creates a new KJobTrackerInterface
     *
     * @param parent the parent object
     */
    KUiServerJobTracker(QObject *parent=0);

    /**
     * Destroys a KJobTrackerInterface
     */
    virtual ~KUiServerJobTracker();

    /**
     * Register a new job in this tracker.
     *
     * @param job the job to register
     */
    virtual void registerJob(KJob *job);

    /**
     * Unregister a job from this tracker.
     *
     * @param job the job to unregister
     */
    virtual void unregisterJob(KJob *job);

protected Q_SLOTS:
    /**
     * The following slots are inherited from KJobTrackerInterface.
     */
    virtual void finished(KJob *job);
    virtual void suspended(KJob *job);
    virtual void resumed(KJob *job);
    virtual void description(KJob *job, const QString &title,
                             const QPair<QString, QString> &field1,
                             const QPair<QString, QString> &field2);
    virtual void infoMessage(KJob *job, const QString &plain, const QString &rich);
    virtual void totalAmount(KJob *job, KJob::Unit unit, qulonglong amount);
    virtual void processedAmount(KJob *job, KJob::Unit unit, qulonglong amount);
    virtual void percent(KJob *job, unsigned long percent);
    virtual void speed(KJob *job, unsigned long value);

private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void _k_killJob())
};

#endif
