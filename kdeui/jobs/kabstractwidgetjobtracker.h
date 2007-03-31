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

#include <kdeui_export.h>
#include <kjobtrackerinterface.h>

class KJob;
class QWidget;

/**
 * The base class for widget based job trackers.
 */
class KDEUI_EXPORT KAbstractWidgetJobTracker : public KJobTrackerInterface
{
    Q_OBJECT

public:
    /**
     * Creates a new KAbstractWidgetJobTracker
     *
     * @param parent the parent of this object and of the widget displaying the job progresses
     */
    KAbstractWidgetJobTracker(QWidget *parent = 0);

    /**
     * Destroys a KAbstractWidgetJobTracker
     */
    virtual ~KAbstractWidgetJobTracker();

    /**
     * Register a new job in this tracker.
     * Note that job trackers inheriting from this class can have only one job
     * registered at a time.
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

    /**
     * This controls whether the job should be canceled if the dialog is closed.
     *
     * @param stopOnClose If true the job will be stopped if the dialog is closed,
     * otherwise the job will continue even on close.
     * @see stopOnClose()
     */
    void setStopOnClose(bool stopOnClose);

    /**
     * Checks whether the job will be killed when the dialog is closed.
     *
     * @return true if the job is killed on close event, false otherwise.
     * @see setStopOnClose()
     */
    bool stopOnClose() const;

    /**
     * This controls whether the dialog should be deleted or only cleaned when
     * the KJob is finished (or canceled).
     *
     * If your dialog is an embedded widget and not a separate window, you should
     * setAutoDelete(false) in the constructor of your custom dialog.
     *
     * @param autoDelete If false the dialog will only call method slotClean.
     * If true the dialog will be deleted.
     * @see autoDelete()
     */
    void setAutoDelete(bool autoDelete);

    /**
     * Checks whether the dialog should be deleted or cleaned.
     * @return false if the dialog only calls slotClean, true if it will be
     *         deleted
     * @see setAutoDelete()
     */
    bool autoDelete() const;

protected Q_SLOTS:
    /**
     * Called when a job is finished, in any case. It is used to notify
     * that the job is terminated and that progress UI (if any) can be hidden.
     *
     * @param job the job that emitted this signal
     */
    void finished(KJob *job);

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
    /**
     * @internal
     */
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Q_PRIVATE_SLOT(d, void _k_installEventFilter())

    class Private;
    Private *const d;
};

#endif
