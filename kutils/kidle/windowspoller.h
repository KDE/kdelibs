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

#ifndef WINDOWSPOLLER_H
#define WINDOWSPOLLER_H

#include "widgetbasedpoller.h"

class QTimer;

class WindowsPoller : public WidgetBasedPoller
{
    Q_OBJECT

public:
    WindowsPoller(QWidget *parent = 0);
    virtual ~WindowsPoller();

public slots:
    void simulateUserActivity();
    void catchIdleEvent();
    void stopCatchingIdleEvents();

private:
    bool additionalSetUp();

private slots:
    int getIdleTime();
    void checkForIdle();

private:
    QTimer * m_idleTimer;
};

#endif /* WINDOWSPOLLER_H */
