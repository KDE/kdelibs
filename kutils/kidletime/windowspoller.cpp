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

#include "windowspoller.h"

#define _WIN32_WINNT 0x0501
#include <windows.h>

WindowsPoller::WindowsPoller(QObject *parent)
        : AbstractSystemPoller(parent)
{
}

WindowsPoller::~WindowsPoller()
{
}

bool WindowsPoller::isAvailable()
{

}

bool WindowsPoller::setUpPoller()
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

    return true;
}

void WindowsPoller::unloadPoller()
{
    m_pollTimer->deleteLater();
    m_grabber->deleteLater();
}

QList<int> WindowsPoller::timeouts() const
{
    return m_timeouts;
}

void WindowsPoller::addTimeout(int nextTimeout)
{
    m_timeouts.append(nextTimeout);
    poll();
}

bool WindowsPoller::eventFilter(QObject * object, QEvent * event)
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

void WindowsPoller::waitForActivity()
{
    // This code was taken from Lithium/KDE4Powersave

    m_grabber->show();
    m_grabber->grabMouse();
    m_grabber->grabKeyboard();

}

void WindowsPoller::detectedActivity()
{
    releaseInputLock();
    emit resumingFromIdle();
}

void WindowsPoller::releaseInputLock()
{
    m_grabber->releaseMouse();
    m_grabber->releaseKeyboard();
    m_grabber->hide();
}

int WindowsPoller::poll()
{
    int idle = 0;

    LASTINPUTINFO lii;
    memset(&lii, 0, sizeof(lii));

    lii.cbSize = sizeof(lii);

    BOOL ok = GetLastInputInfo(&li);
    if (ok) {
        idle = GetTickCount() - li.dwTime;
    }

    // Check if we reached a timeout..
    foreach(int i, m_timeouts) {
        if (i - idle < 1000 || idle - i < 1000) {
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

    if (mintime != 0) {
        m_pollTimer->start(mintime - idle);
    } else {
        m_pollTimer->stop();
    }

    return idle;
}

int WindowsPoller::forcePollRequest()
{
    return poll();
}

void WindowsPoller::removeTimeout(int timeout)
{
    m_timeouts.removeOne(timeout);
    poll();
}

void WindowsPoller::catchIdleEvent()
{
    waitForActivity();
}

void WindowsPoller::stopCatchingIdleEvents()
{
    releaseInputLock();
}

#include "windowspoller.moc"
