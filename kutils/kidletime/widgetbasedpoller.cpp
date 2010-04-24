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

#include "widgetbasedpoller.h"

#include <QWidget>
#include <QTimer>
#include <QEvent>

#ifndef WIN32
#include <fixx11h.h>
#endif

WidgetBasedPoller::WidgetBasedPoller(QWidget *parent)
        : AbstractSystemPoller(parent)
{
}

WidgetBasedPoller::~WidgetBasedPoller()
{
}

bool WidgetBasedPoller::isAvailable()
{
    return true;
}

bool WidgetBasedPoller::setUpPoller()
{
    m_pollTimer = new QTimer(this);

    //setup idle timer, with some smart polling
    connect(m_pollTimer, SIGNAL(timeout()), this, SLOT(poll()));

    // This code was taken from Lithium/KDE4Powersave
    m_grabber = new QWidget(0, Qt::X11BypassWindowManagerHint);
    m_grabber->move(-1000, -1000);
    m_grabber->setMouseTracking(true);
    m_grabber->installEventFilter(this);
    m_grabber->setObjectName("KIdleGrabberWidget");

    return additionalSetUp();
}

void WidgetBasedPoller::unloadPoller()
{
    m_pollTimer->deleteLater();
    m_grabber->deleteLater();
}

QList<int> WidgetBasedPoller::timeouts() const
{
    return m_timeouts;
}

void WidgetBasedPoller::addTimeout(int nextTimeout)
{
    m_timeouts.append(nextTimeout);
    poll();
}

bool WidgetBasedPoller::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_grabber
            && (event->type() == QEvent::MouseMove || event->type() == QEvent::KeyPress)) {
        detectedActivity();
        return true;
    } else if (object != m_grabber) {
        // If it's not the grabber, fallback to default event filter
        return false;
    }

    // Otherwise, simply ignore it
    return false;

}

void WidgetBasedPoller::waitForActivity()
{
    // This code was taken from Lithium/KDE4Powersave

    m_grabber->show();
    m_grabber->grabMouse();
    m_grabber->grabKeyboard();

}

void WidgetBasedPoller::detectedActivity()
{
    stopCatchingIdleEvents();
    emit resumingFromIdle();
}

void WidgetBasedPoller::releaseInputLock()
{
    m_grabber->releaseMouse();
    m_grabber->releaseKeyboard();
    m_grabber->hide();
}

int WidgetBasedPoller::poll()
{
    int idle = getIdleTime();

    // Check if we reached a timeout..
    foreach(int i, m_timeouts) {
        if ((i - idle < 300 && i > idle) || (idle - i < 300 && idle > i)) {
            // Bingo!
            emit timeoutReached(i);
        }
    }

    // Let's check the timer now!
    int mintime = 0;

    foreach(int i, m_timeouts) {
        if (i > idle && (i < mintime || mintime == 0)) {
            mintime = i;
        }
    }

    //qDebug() << "mintime " << mintime << "idle " << idle;

    if (mintime != 0) {
        m_pollTimer->start(mintime - idle);
    } else {
        m_pollTimer->stop();
    }

    return idle;
}

int WidgetBasedPoller::forcePollRequest()
{
    return poll();
}

void WidgetBasedPoller::removeTimeout(int timeout)
{
    m_timeouts.removeOne(timeout);
    poll();
}

void WidgetBasedPoller::catchIdleEvent()
{
    waitForActivity();
}

void WidgetBasedPoller::stopCatchingIdleEvents()
{
    releaseInputLock();
}

#include "widgetbasedpoller.moc"
