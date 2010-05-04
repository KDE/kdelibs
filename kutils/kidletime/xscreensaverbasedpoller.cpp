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

#include "xscreensaverbasedpoller.h"

#include <config-kidletime.h>

#include <QX11Info>

#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>

XScreensaverBasedPoller::XScreensaverBasedPoller(QWidget *parent)
        : WidgetBasedPoller(parent)
{

}

XScreensaverBasedPoller::~XScreensaverBasedPoller()
{
}

bool XScreensaverBasedPoller::additionalSetUp()
{
    m_screenSaverIface = new OrgFreedesktopScreenSaverInterface("org.freedesktop.ScreenSaver", "/ScreenSaver",
            QDBusConnection::sessionBus(), this);

    connect(m_screenSaverIface, SIGNAL(ActiveChanged(bool)), SLOT(screensaverActivated(bool)));

    return true;
}

void XScreensaverBasedPoller::screensaverActivated(bool activated)
{
    // We care only if it has been disactivated

    if (!activated) {
        m_screenSaverIface->SimulateUserActivity();
        emit resumingFromIdle();
    }
}

int XScreensaverBasedPoller::getIdleTime()
{
    XScreenSaverInfo * mitInfo = 0;
    mitInfo = XScreenSaverAllocInfo();
    XScreenSaverQueryInfo(QX11Info::display(), DefaultRootWindow(QX11Info::display()), mitInfo);
    int ret = mitInfo->idle;
    XFree( mitInfo );
    return ret;
}

void XScreensaverBasedPoller::simulateUserActivity()
{
    stopCatchingIdleEvents();
    XResetScreenSaver(QX11Info::display());
    emit resumingFromIdle();
}

#include "xscreensaverbasedpoller.moc"
