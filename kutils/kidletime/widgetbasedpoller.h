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

#ifndef WIDGETBASEDPOLLER_H
#define WIDGETBASEDPOLLER_H

#include "abstractsystempoller.h"

class QTimer;
class QEvent;

class WidgetBasedPoller : public AbstractSystemPoller
{
    Q_OBJECT

public:
    WidgetBasedPoller(QWidget *parent = 0);
    virtual ~WidgetBasedPoller();

    bool isAvailable();
    bool setUpPoller();
    void unloadPoller();

protected:
    bool eventFilter(QObject *object, QEvent *event);

public slots:
    void addTimeout(int nextTimeout);
    void removeTimeout(int nextTimeout);
    QList<int> timeouts() const;
    int forcePollRequest();
    virtual void catchIdleEvent();
    virtual void stopCatchingIdleEvents();

private slots:
    int poll();
    virtual int getIdleTime() = 0;
    void detectedActivity();
    void waitForActivity();
    void releaseInputLock();

private:
    virtual bool additionalSetUp() = 0;

private:
    QTimer * m_pollTimer;
    QWidget * m_grabber;
    QList<int> m_timeouts;
};

#endif /* WIDGETBASEDPOLLER_H */
