/* This file is part of the KDE libraries
   Copyright (C) 2009 Dario Freddi <drf at kde.org>

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

#ifndef MACPOLLER_H
#define MACPOLLER_H

#include "abstractsystempoller.h"

#include <Carbon/Carbon.h>

class MacPoller: public AbstractSystemPoller
{
    Q_OBJECT

public:
    MacPoller(QWidget *parent = 0);
    virtual ~MacPoller();

    bool isAvailable();
    bool setUpPoller();
    void unloadPoller();

    static pascal void IdleTimerAction(EventLoopTimerRef, EventLoopIdleTimerMessage inState, void* inUserData);

public slots:
    void addTimeout(int nextTimeout);
    void removeTimeout(int nextTimeout);
    QList<int> timeouts() const;
    int forcePollRequest();
    void catchIdleEvent();
    void stopCatchingIdleEvents();
    void simulateUserActivity();
    void triggerResume();

private slots:
    int poll();

private:
    QList<int> m_timeouts;
    EventLoopTimerRef m_timerRef;
    int m_secondsIdle;
    bool m_catch;
};

#endif /* MACPOLLER_H */
