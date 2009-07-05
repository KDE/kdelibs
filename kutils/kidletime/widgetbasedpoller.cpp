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

#include "widgetbasedpoller.h"

#include <QWidget>
#include <QTimer>
#include <QX11Info>
#include <QEvent>

#include <klocalizedstring.h>

WidgetBasedPoller::WidgetBasedPoller(QObject *parent)
        : AbstractSystemPoller(parent)
{
}

WidgetBasedPoller::~WidgetBasedPoller()
{
}

bool WidgetBasedPoller::isAvailable()
{
#ifdef HAVE_XSCREENSAVER
    return true;
#else
    return false;
#endif
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

    m_screenSaverIface = new OrgFreedesktopScreenSaverInterface("org.freedesktop.ScreenSaver", "/ScreenSaver",
            QDBusConnection::sessionBus(), this);

    connect(m_screenSaverIface, SIGNAL(ActiveChanged(bool)), SLOT(screensaverActivated(bool)));

    return true;
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

void WidgetBasedPoller::screensaverActivated(bool activated)
{
    // We care only if it has been disactivated

    if (!activated) {
        m_screenSaverIface->SimulateUserActivity();
        emit resumingFromIdle();
    }
}

bool WidgetBasedPoller::eventFilter(QObject * object, QEvent * event)
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
    emit resumingFromIdle();
}

void WidgetBasedPoller::releaseInputLock()
{
    m_grabber->releaseMouse();
    m_grabber->releaseKeyboard();
    m_grabber->hide();
}

#ifdef HAVE_XSCREENSAVER
#include <X11/extensions/scrnsaver.h>
#endif

int WidgetBasedPoller::poll()
{
    /* Hack! Since KRunner still doesn't behave properly, the
     * correct way to go doesn't work (yet), and it's this one:
        ------------------------------------------------------------
        int idle = m_screenSaverIface->GetSessionIdleTime();
        ------------------------------------------------------------
     */
    /// In the meanwhile, this X11 hackish way gets its job done.
    //----------------------------------------------------------

    int idle = 0;

#ifdef HAVE_XSCREENSAVER
    XScreenSaverInfo * mitInfo = 0;
    mitInfo = XScreenSaverAllocInfo();
    XScreenSaverQueryInfo(QX11Info::display(), DefaultRootWindow(QX11Info::display()), mitInfo);
    idle = mitInfo->idle;
    //----------------------------------------------------------
#endif

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
