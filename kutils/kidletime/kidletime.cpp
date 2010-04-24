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

#include <QWeakPointer>
#include <QSet>

#include <kglobal.h>

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
    Q_DECLARE_PUBLIC(KIdleTime)
    KIdleTime *q_ptr;
public:
    KIdleTimePrivate() : catchResume(false), currentId(0) {}

    void loadSystem();
    void unloadCurrentSystem();
    void _k_resumingFromIdle();
    void _k_timeoutReached(int msec);

    QWeakPointer<AbstractSystemPoller> poller;
    bool catchResume;

    int currentId;
    QHash<int, int> associations;
};

KIdleTime::KIdleTime()
        : QObject(0)
        , d_ptr(new KIdleTimePrivate())
{
    Q_ASSERT(!s_globalKIdleTime->q);
    s_globalKIdleTime->q = this;

    d_ptr->q_ptr = this;

    Q_D(KIdleTime);
    d->loadSystem();

    connect(d->poller.data(), SIGNAL(resumingFromIdle()), this, SLOT(_k_resumingFromIdle()));
    connect(d->poller.data(), SIGNAL(timeoutReached(int)), this, SLOT(_k_timeoutReached(int)));
}

KIdleTime::~KIdleTime()
{
    Q_D(KIdleTime);
    d->unloadCurrentSystem();
    delete d_ptr;
}

void KIdleTime::catchNextResumeEvent()
{
    Q_D(KIdleTime);

    if (!d->catchResume) {
        d->catchResume = true;
        d->poller.data()->catchIdleEvent();
    }
}

void KIdleTime::stopCatchingResumeEvent()
{
    Q_D(KIdleTime);

    if (d->catchResume) {
        d->catchResume = false;
        d->poller.data()->stopCatchingIdleEvents();
    }
}

int KIdleTime::addIdleTimeout(int msec)
{
    Q_D(KIdleTime);

    d->poller.data()->addTimeout(msec);

    ++d->currentId;
    d->associations[d->currentId] = msec;

    return d->currentId;
}

void KIdleTime::removeIdleTimeout(int identifier)
{
    Q_D(KIdleTime);

    if (!d->associations.contains(identifier)) {
        return;
    }

    int msec = d->associations[identifier];

    d->associations.remove(identifier);

    if (!d->associations.values().contains(msec)) {
        d->poller.data()->removeTimeout(msec);
    }
}

void KIdleTime::removeAllIdleTimeouts()
{
    Q_D(KIdleTime);

    QHash< int, int >::iterator i = d->associations.begin();
    QSet< int > removed;
    removed.reserve(d->associations.size());

    while (i != d->associations.end()) {
        int msec = d->associations[i.key()];

        i = d->associations.erase(i);

        if (!removed.contains(msec)) {
            d->poller.data()->removeTimeout(msec);
            removed.insert(msec);
        }
    }
}

void KIdleTimePrivate::loadSystem()
{
    if (!poller.isNull()) {
        unloadCurrentSystem();
    }

    // Priority order

#ifdef Q_WS_X11
#ifdef HAVE_XSYNC
#ifdef HAVE_XSCREENSAVER
    if (XSyncBasedPoller::instance()->isAvailable()) {
        poller = XSyncBasedPoller::instance();
    } else {
        poller = new XScreensaverBasedPoller();
    }
#else
    poller = XSyncBasedPoller::instance();
#endif
#else
#ifdef HAVE_XSCREENSAVER
    poller = new XScreensaverBasedPoller();
#endif
#endif
#else
#ifdef Q_WS_MAC
    poller = new MacPoller();
#else
    poller = new WindowsPoller();
#endif
#endif

    if (!poller.isNull()) {
        poller.data()->setUpPoller();
    }
}

void KIdleTimePrivate::unloadCurrentSystem()
{
    if (!poller.isNull()) {
        poller.data()->unloadPoller();
#ifdef Q_WS_X11
        if (qobject_cast<XSyncBasedPoller*>(poller.data()) == 0) {
#endif
            poller.data()->deleteLater();
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
        q->stopCatchingResumeEvent();
    }
}

void KIdleTimePrivate::_k_timeoutReached(int msec)
{
    Q_Q(KIdleTime);

    if (associations.values().contains(msec)) {
        foreach (int key, associations.keys(msec)) {
            emit q->timeoutReached(key);
            emit q->timeoutReached(key, msec);
        }
    }
}

void KIdleTime::simulateUserActivity()
{
    Q_D(KIdleTime);

    d->poller.data()->simulateUserActivity();
}

int KIdleTime::idleTime() const
{
    Q_D(const KIdleTime);

    return d->poller.data()->forcePollRequest();
}

QHash<int, int> KIdleTime::idleTimeouts() const
{
    Q_D(const KIdleTime);

    return d->associations;
}

#include "kidletime.moc"
