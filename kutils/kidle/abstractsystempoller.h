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

#ifndef ABSTRACTSYSTEMPOLLER_H
#define ABSTRACTSYSTEMPOLLER_H

#include <QWidget>

class AbstractSystemPoller : public QWidget
{
    Q_OBJECT

public:

    AbstractSystemPoller(QWidget *parent = 0);
    virtual ~AbstractSystemPoller();

    virtual bool isAvailable() = 0;
    virtual bool setUpPoller() = 0;
    virtual void unloadPoller() = 0;

public slots:
    virtual void addTimeout(int nextTimeout) = 0;
    virtual void removeTimeout(int nextTimeout) = 0;
    virtual QList<int> timeouts() const = 0;
    virtual int forcePollRequest() = 0;
    virtual void catchIdleEvent() = 0;
    virtual void stopCatchingIdleEvents() = 0;
    virtual void simulateUserActivity() = 0;

signals:
    void resumingFromIdle();
    void timeoutReached(int msec);

};

#endif /* ABSTRACTSYSTEMPOLLER_H */
