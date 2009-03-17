/*  This file is part of the KDE project
    Copyright (C) 2000 Matej Koss <koss@miesto.sk>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2009 Shaun Reich <shaun.reich@kdemail.net>

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

#ifndef KWIDGETJOBTRACKER_H
#define KWIDGETJOBTRACKER_H

#include <kabstractwidgetjobtracker.h>

/**
 * This class implements a job tracker with a widget suited for use as a
 * progress dialog.
 */
class KDEUI_EXPORT KWidgetJobTracker : public KAbstractWidgetJobTracker
{
    Q_OBJECT

public:
    /**
     * Creates a new KWidgetJobTracker
     *
     * @param parent the parent of this object and of the widget displaying the job progresses
     */
    KWidgetJobTracker(QWidget *parent = 0);

    /**
     * Destroys a KWidgetJobTracker
     */
    virtual ~KWidgetJobTracker();

    /**
     * The widget associated to this tracker.
     *
     * @param job the job that is assigned the widget we want to get
     * @return the widget displaying the job progresses
     */
    virtual QWidget *widget(KJob *job);

// KDE5: move this two virtual methods to be placed correctly (ereslibre)
public Q_SLOTS:
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

public:
    bool keepOpen(KJob *job) const;

protected Q_SLOTS:
    /**
     * The following slots are inherited from KJobTrackerInterface.
     */
    virtual void infoMessage(KJob *job, const QString &plain, const QString &rich);
    virtual void description(KJob *job, const QString &title,
                             const QPair<QString, QString> &field1,
                             const QPair<QString, QString> &field2);
    virtual void totalAmount(KJob *job, KJob::Unit unit, qulonglong amount);
    virtual void processedAmount(KJob *job, KJob::Unit unit, qulonglong amount);
    virtual void percent(KJob *job, unsigned long percent);
    virtual void speed(KJob *job, unsigned long value);
    virtual void slotClean(KJob *job);
    virtual void suspended(KJob *job);
    virtual void resumed(KJob *job);

    //TODO: Misses canResume()

private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void _k_showProgressWidget())
};

#endif
