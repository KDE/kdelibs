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

#ifndef KSTATUSBARJOBTRACKER_H
#define KSTATUSBARJOBTRACKER_H

#include <kabstractwidgetjobtracker.h>

/**
 * This class implements a job tracker with a widget suited for embedding in a
 * status bar.
 */
class KDEUI_EXPORT KStatusBarJobTracker : public KAbstractWidgetJobTracker
{
    Q_OBJECT

public:
    enum StatusBarMode
    {
        NoInformation = 0x0000, ///< Does not show any information
        LabelOnly     = 0x0001, ///< Shows an informative label for job progress
        ProgressOnly  = 0x0002  ///< Shows a progress bar with the job completion
    };

    Q_DECLARE_FLAGS(StatusBarModes, StatusBarMode)

    /**
     * Creates a new KStatusBarJobTracker
     *
     * @param parent the parent of this object and of the widget displaying the job progresses
     * @param button true to display a stop button allowing to kill the job, false otherwise
     */
    explicit KStatusBarJobTracker(QWidget *parent = 0, bool button = true);

    /**
     * Destroys a KStatusBarJobTracker
     */
    virtual ~KStatusBarJobTracker();

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

    /**
     * The widget associated to this tracker.
     *
     * @return the widget displaying the job progresses
     */
    virtual QWidget *widget(KJob *job);

    /**
     * Sets the mode of the status bar.
     *
     * @param statusBarMode what information the status bar will show (see StatusBarMode).
     *                      LabelOnly by default
     */
    void setStatusBarMode(StatusBarModes statusBarMode);

public Q_SLOTS:
    /**
     * The following slots are inherited from KJobTrackerInterface.
     */
    virtual void description(KJob *job, const QString &title,
                             const QPair<QString, QString> &field1,
                             const QPair<QString, QString> &field2);
    virtual void totalAmount(KJob *job, KJob::Unit unit, qulonglong amount);
    virtual void percent(KJob *job, unsigned long percent);
    virtual void speed(KJob *job, unsigned long value);
    virtual void slotClean(KJob *job);

private:
    class Private;
    Private *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KStatusBarJobTracker::StatusBarModes)

#endif
