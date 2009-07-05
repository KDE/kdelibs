/***************************************************************************
 *   Copyright (C) 2009 by Dario Freddi <drf@kde.org>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 **************************************************************************/

#ifndef KIDLETIME_H
#define KIDLETIME_H

#include <QObject>
#include <kdemacros.h>

class KIdleTimePrivate;

/**
 * KIdleTime is a singleton reporting information on idle time. It is useful not
 * only for finding out about the current idle time of the PC, but also for getting
 * notified upon idle time events, such as custom timeouts, or user activity.
 *
 * @author Dario Freddi
 */
class KDE_EXPORT KIdleTime : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KIdleTime)
    Q_DISABLE_COPY(KIdleTime)

public:
    /**
     * @returns the instance of KIdleTime
     *
     */
    static KIdleTime *instance();

    /**
     * The destructor
     */
    virtual ~KIdleTime();

    /**
     * Gets the idle time of the system, in milliseconds
     *
     * @returns the idle time of the system
     */
    int idleTime();

    /**
     * Attempts to simulate user activity. This implies that after calling this
     * function, the idle time of the system will become 0 and eventually resumingFromIdle
     * will be triggered
     *
     * @see resumingFromIdle
     */
    void simulateUserActivity();

public Q_SLOTS:
    /**
     * Sets a new timeout to catch. When calling this function, after the system will be idle for
     * msec milliseconds, the signal timeoutReached will be triggered. Please note that until you will
     * call stopCatchingIdleTimeout, or you supply a different timeout, the signal will be triggered every
     * time the system will be idle for msec milliseconds
     *
     * @param msec the time, in milliseconds, after which the signal will be triggered
     *
     * @see stopCatchingIdleTimeout
     * @see timeoutReached
     *
     */
    void addIdleTimeout(int msec);
    void removeIdleTimeout(int msec);
    /**
     * Stops catching the previous set timeout (if any). This means that after calling this function, the signal
     * timeoutReached won't be called again until catchIdleTimeout is called again.
     *
     * @see timeoutReached
     * @see catchIdleTimeout
     */
    void removeAllIdleTimeouts();
    /**
     * Catches the next resume from idle event. This means that whenever user activity will be registered, or
     * simulateUserActivity is called, the signal resumingFromIdle will be triggered. Please note that this function
     * will trigger the signal just for the very first resume event: so you need to call it whenever you need to track
     * a single resume event.
     *
     * @see resumingFromIdle
     * @see simulateUserActivity
     *
     */
    void catchNextResumeEvent();

Q_SIGNALS:
    /**
     * Triggered if KIdle is catching resume events and when the system resumes from an idle state
     *
     * @see catchNextResumeEvent
     */
    void resumingFromIdle();
    /**
     * Triggered if a timeout is being catched and when the system is idle for the chosen amount of milliseconds
     *
     * @param msec the time, in milliseconds, the system has been idle for
     */
    void timeoutReached(int msec);

private:
    KIdleTime();

    KIdleTimePrivate * const d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _k_resumingFromIdle())

};

#endif /* KIDLETIME_H */
