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

#include "abstractsystempoller.h"

//#include <config-X11.h>

#define HAVE_XTEST 1 // the usual hack

#ifdef HAVE_XTEST
#include <QX11Info>

#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <fixx11h.h>
#endif // HAVE_XTEST

AbstractSystemPoller::AbstractSystemPoller(QObject *parent)
        : QWidget(0)
{
    Q_UNUSED(parent)
}

AbstractSystemPoller::~AbstractSystemPoller()
{
}

void AbstractSystemPoller::simulateUserActivity()
{
#ifdef HAVE_XTEST
    Display* display = QX11Info::display();
    XTestFakeMotionEvent(display, 0, 1, 2, 0);
    XSync(display, false);
#endif // HAVE_XTEST
}

#include "abstractsystempoller.moc"
