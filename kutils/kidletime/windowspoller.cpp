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

#include "windowspoller.h"

#define _WIN32_WINNT 0x0501
#include <windows.h>

#include <QTimer>

WindowsPoller::WindowsPoller(QWidget *parent)
        : WidgetBasedPoller(parent)
{
}

WindowsPoller::~WindowsPoller()
{
}

int WindowsPoller::getIdleTime()
{
#ifndef _WIN32_WCE
    int idle = 0;

    LASTINPUTINFO lii;
    memset(&lii, 0, sizeof(lii));

    lii.cbSize = sizeof(lii);

    BOOL ok = GetLastInputInfo(&lii);
    if (ok) {
        idle = GetTickCount() - lii.dwTime;
    }

    return idle;
#else
    return GetIdleTime();
#endif
}

bool WindowsPoller::additionalSetUp()
{
    m_idleTimer = new QTimer(this);
    connect(m_idleTimer, SIGNAL(timeout()), this, SLOT(checkForIdle()));
    return true;
}

void WindowsPoller::simulateUserActivity()
{
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    int x = (int)100 * 65536 / width;
    int y = (int)100 * 65536 / height;

    mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, x, y, NULL, NULL);
}

void WindowsPoller::catchIdleEvent()
{
    m_idleTimer->start(800);
}

void WindowsPoller::stopCatchingIdleEvents()
{
    m_idleTimer->stop();
}

void WindowsPoller::checkForIdle()
{
    if (getIdleTime() < 1000) {
        stopCatchingIdleEvents();
        emit resumingFromIdle();
    }
}

#include "windowspoller.moc"
