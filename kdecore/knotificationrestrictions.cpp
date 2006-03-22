/* This file is part of the KDE libraries
    Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"

#include "knotificationrestrictions.h"
#include "kdebug.h"

#include <QTimer>

#ifdef HAVE_XTEST
#include <QX11Info>

#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#endif // HAVE_XTEST

class KNotificationRestrictions::Private
{
    public:
        Private(Services c)
            : control(c)
#ifdef HAVE_XTEST
             ,screensaverTimer(0),
              haveXTest(0),
              XTestKeyCode(0)
#endif // HAVE_XTEST
        {
        }

        Services control;
        QTimer* screensaverTimer;
#ifdef HAVE_XTEST
        int haveXTest;
        int XTestKeyCode;
#endif // HAVE_XTEST
};

KNotificationRestrictions::KNotificationRestrictions(Services control,
                                                     QObject* parent)
    : QObject(parent)
{
    d = new Private(control);

    if (d->control & ScreenSaver)
    {
        startScreenSaverPrevention();
    }
}

KNotificationRestrictions::~KNotificationRestrictions()
{
    if (d->control & ScreenSaver)
    {
        stopScreenSaverPrevention();
    }

    delete d;
}

void KNotificationRestrictions::screensaverFakeKeyEvent()
{
    kDebug() << "KNotificationRestrictions::screensaverFakeKeyEvent()" << endl;
#ifdef HAVE_XTEST
    kDebug() << "---- using XTestFakeKeyEvent" << endl;
    Display* display = QX11Info::display();
    XTestFakeKeyEvent(display, d->XTestKeyCode, true, CurrentTime);
    XTestFakeKeyEvent(display, d->XTestKeyCode, false, CurrentTime);
    XSync(display, false);
#endif // HAVE_XTEST
}

void KNotificationRestrictions::startScreenSaverPrevention()
{
    kDebug() << "KNotificationRestrictions::startScreenSaverPrevention()" << endl;
#ifdef HAVE_XTEST
    if (!d->haveXTest)
    {
        int a,b,c,e;
        d->haveXTest = XTestQueryExtension(QX11Info::display(), &a, &b, &c, &e);

        if (!d->haveXTest)
        {
            kDebug() << "--- No XTEST!" << endl;
            return;
        }
    }

    if (!d->XTestKeyCode)
    {
        d->XTestKeyCode = XKeysymToKeycode(QX11Info::display(), XK_Shift_L);

        if (!d->XTestKeyCode)
        {
            kDebug() << "--- No XKeyCode for XK_Shift_L!" << endl;
            return;
        }
    }

    if (d->screensaverTimer == 0)
    {
        d->screensaverTimer = new QTimer(this);
        connect(d->screensaverTimer, SIGNAL(timeout()),
                this, SLOT(screensaverFakeKeyEvent()));
    }

    kDebug() << "---- using XTest" << endl;
    // send a fake event right away in case this got started after a period of
    // innactivity leading to the screensaver set to activate in <55s
    screensaverFakeKeyEvent();
    d->screensaverTimer->start(55000);
#endif // HAVE_XTEST
}

void KNotificationRestrictions::stopScreenSaverPrevention()
{
#ifdef HAVE_XTEST
    delete d->screensaverTimer;
    d->screensaverTimer = 0;
#endif // HAVE_XTEST
}
#include "knotificationrestrictions.moc"
