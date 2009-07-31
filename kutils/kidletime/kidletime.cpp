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

#include "kidletime.h"

#include <config-kidletime.h>

#ifdef Q_WS_X11
#ifdef HAVE_XSCREENSAVER
#include "xscreensaverbasedpoller.h"
#endif
#ifdef HAVE_XSYNC
#include "xsyncbasedpoller.h"
#endif
#else
#ifdef Q_WS_MAC
#include "macpoller.h"
#else
#include "windowspoller.h"
#endif
#endif

#include <kglobal.h>
#include <QPointer>

class KIdleTimeHelper
{
public:
    KIdleTimeHelper() : q(0) {}
    ~KIdleTimeHelper() {
        delete q;
    }
    KIdleTime *q;
};

K_GLOBAL_STATIC(KIdleTimeHelper, s_globalKIdleTime)

KIdleTime *KIdleTime::instance()
{
    if (!s_globalKIdleTime->q) {
        new KIdleTime;
    }

    return s_globalKIdleTime->q;
}

class KIdleTimePrivate
{
public:
    KIdleTimePrivate() : catchResume(false) {};

    Q_DECLARE_PUBLIC(KIdleTime)
    KIdleTime *q_ptr;

    void loadSystem();
    void unloadCurrentSystem();
    void _k_resumingFromIdle();

    QPointer<AbstractSystemPoller> poller;
    bool catchResume;
};

KIdleTime::KIdleTime()
        : QObject(0)
        , d_ptr(new KIdleTimePrivate())
{
    Q_ASSERT(!s_globalKIdleTime->q);
    s_globalKIdleTime->q = this;

    d_ptr->q_ptr = this;
    d_ptr->loadSystem();

    connect(d_ptr->poller, SIGNAL(resumingFromIdle()), this, SLOT(_k_resumingFromIdle()));
    connect(d_ptr->poller, SIGNAL(timeoutReached(int)), this, SIGNAL(timeoutReached(int)));
}

KIdleTime::~KIdleTime()
{
    Q_D(KIdleTime);
    d->unloadCurrentSystem();
}

void KIdleTime::catchNextResumeEvent()
{
    Q_D(KIdleTime);

    if (!d->catchResume) {
        d->catchResume = true;
        d->poller->catchIdleEvent();
    }
}

void KIdleTime::addIdleTimeout(int msec)
{
    Q_D(KIdleTime);

    d->poller->addTimeout(msec);
}

void KIdleTime::removeIdleTimeout(int msec)
{
    Q_D(KIdleTime);

    d->poller->removeTimeout(msec);
}

void KIdleTime::removeAllIdleTimeouts()
{
    Q_D(KIdleTime);

    foreach(int i, d->poller->timeouts()) {
        removeIdleTimeout(i);
    }
}

void KIdleTimePrivate::loadSystem()
{
    if (poller) {
        unloadCurrentSystem();
    }

    // Priority order

#ifdef Q_WS_X11
#ifdef HAVE_XSYNC
#ifdef HAVE_XSCREENSAVER
    if (XSyncBasedPoller::instance()->isAvailable()) {
        XSyncBasedPoller::instance()->setUpPoller();
        poller = XSyncBasedPoller::instance();
    } else {
        poller = new XScreensaverBasedPoller();
        poller->setUpPoller();
    }
#else
    XSyncBasedPoller::instance()->setUpPoller();
    poller = XSyncBasedPoller::instance();
#endif
#else
#ifdef HAVE_XSCREENSAVER
    poller = new XScreensaverBasedPoller();
    poller->setUpPoller();
#endif
#endif
#else
#ifdef Q_WS_MAC
    poller = new MacPoller();
    poller->setUpPoller();
#else
    poller = new WindowsPoller();
    poller->setUpPoller();
#endif
#endif
}

void KIdleTimePrivate::unloadCurrentSystem()
{
    if (poller) {
        poller->unloadPoller();
#ifdef Q_WS_X11
        if (qobject_cast<XSyncBasedPoller*>(poller) == 0) {
#endif
            poller->deleteLater();
#ifdef Q_WS_X11
        }
#endif
    }
}

void KIdleTimePrivate::_k_resumingFromIdle()
{
    Q_Q(KIdleTime);

    if (catchResume) {
        emit q->resumingFromIdle();
        catchResume = false;
    }
}

void KIdleTime::simulateUserActivity()
{
    Q_D(KIdleTime);

    d->poller->simulateUserActivity();
}

int KIdleTime::idleTime() const
{
    Q_D(const KIdleTime);

    return d->poller->forcePollRequest();
}

QList<int> KIdleTime::idleTimeouts() const
{
    Q_D(const KIdleTime);

    return d->poller->timeouts();
}

#include "kidletime.moc"
