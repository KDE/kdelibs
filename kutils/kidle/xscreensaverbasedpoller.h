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

#ifndef XSCREENSAVERBASEDPOLLER_H
#define XSCREENSAVERBASEDPOLLER_H

#include "widgetbasedpoller.h"

#include "screensaver_interface.h"

class XScreensaverBasedPoller: public WidgetBasedPoller
{
    Q_OBJECT

public:
    XScreensaverBasedPoller(QWidget *parent = 0);
    virtual ~XScreensaverBasedPoller();

public slots:
    void simulateUserActivity();

private:
    bool additionalSetUp();

private slots:
    void screensaverActivated(bool activated);
    int getIdleTime();

private:
    OrgFreedesktopScreenSaverInterface * m_screenSaverIface;
};

#endif /* XSCREENSAVERBASEDPOLLER_H_ */
