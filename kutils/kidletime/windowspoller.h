/***************************************************************************
 *   Copyright (C) 2009 by Dario Freddi <drf@kde.org>                      *
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

#ifndef WINDOWSPOLLER_H_
#define WINDOWSPOLLER_H_

#include "abstractsystempoller.h"

class QTimer;
class QEvent;

class WindowsPoller : public AbstractSystemPoller
{
    Q_OBJECT

public:
    WindowsPoller(QObject *parent = 0);
    virtual ~WindowsBasedPoller();

    AbstractSystemPoller::PollingType getPollingType() {
        return AbstractSystemPoller::WidgetBased;
    };

    bool isAvailable();
    bool setUpPoller();
    void unloadPoller();

protected:
    bool eventFilter(QObject * object, QEvent * event);

public slots:
    void addTimeout(int nextTimeout);
    void removeTimeout(int nextTimeout);
    QList<int> timeouts() const;
    int forcePollRequest();
    void catchIdleEvent();
    void stopCatchingIdleEvents();

private slots:
    int poll();
    void detectedActivity();
    void waitForActivity();
    void releaseInputLock();

signals:
    void resumingFromIdle();
    void timeoutReached(int msec);

private:
    QTimer * m_pollTimer;
    QWidget * m_grabber;
    QList<int> m_timeouts;
};

#endif /* WINDOWSPOLLER_H_ */
