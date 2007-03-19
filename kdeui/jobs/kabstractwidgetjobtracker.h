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

#ifndef KABSTRACTWIDGETJOBTRACKER_H
#define KABSTRACTWIDGETJOBTRACKER_H

#include <kjobtrackerinterface.h>

class KJob;
class QWidget;

/**
 * The base class for widget base job trackers.
 */
class KDEUI_EXPORT KAbstractWidgetJobTracker : public KJobTrackerInterface
{
    Q_OBJECT

public:
    /**
     * Creates a new KWidgetJobTracker
     *
     * @param parent the parent of this object and of the widget displaying the job progresses
     */
    KAbstractWidgetJobTracker(QWidget *parent = 0);

    /**
     * Destroys a KWidgetJobTracker
     */
    virtual ~KAbstractWidgetJobTracker();

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
    virtual QWidget *widget() = 0;

    // should we stop the job when the dialog is closed ?
    void setStopOnClose(bool stopOnClose);
    bool stopOnClose() const;

    /**
     * This controls whether the dialog should be deleted or only cleaned when
     * the KJob is finished (or canceled).
     *
     * If your dialog is an embedded widget and not a separate window, you should
     * setOnlyClean(true) in the constructor of your custom dialog.
     *
     * @param onlyClean If true the dialog will only call method slotClean.
     * If false the dialog will be deleted.
     * @see onlyClean()
     */
    void setOnlyClean(bool onlyClean);

    /**
     * Checks whether the dialog should be deleted or cleaned.
     * @return true if the dialog only calls slotClean, false if it will be
     *         deleted
     * @see setOnlyClean()
     */
    bool onlyClean() const;

protected Q_SLOTS:
    /**
     * Called when a job is finished, in any case. It is used to notify
     * that the job is terminated and that progress UI (if any) can be hidden.
     *
     * @param job the job that emitted this signal
     */
    void finished(KJob *job);

public Q_SLOTS:
    /**
     * This method should be called for correct cancellation of IO operation
     * Connect this to the progress widgets buttons etc.
     */
    void slotStop();

    /**
     * This method should be called for pause/resume
     * Connect this to the progress widgets buttons etc.
     */
    void slotSuspend();

    /**
     * This method should be called for pause/resume
     * Connect this to the progress widgets buttons etc.
     */
    void slotResume();

    /**
     * This method is called when the widget should be cleaned (after job is finished).
     * redefine this for custom behavior.
     */
    virtual void slotClean();

Q_SIGNALS:
    /**
     * Emitted when the user aborted the operation
     */
    void stopped();

    /**
     * Emitted when the user suspended the operation
     */
    void suspend();

    /**
     * Emitted when the user resumed the operation
     */
    void resume();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Q_PRIVATE_SLOT(d, void _k_installEventFilter())

    class Private;
    Private *const d;
};

#endif
