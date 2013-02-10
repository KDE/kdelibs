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

#include "abstractsystempoller.h"

#if HAVE_XSCREENSAVER
#include "xscreensaverbasedpoller.h"
#endif

#if HAVE_XSYNC
#include "xsyncbasedpoller.h"
#endif

#ifdef Q_OS_MAC
#include "macpoller.h"
#endif

#ifdef Q_OS_WIN
#include "windowspoller.h"
#endif

#include <QPointer>
#include <QSet>


class KIdleTimeHelper
{
public:
    KIdleTimeHelper() : q(0) {}
    ~KIdleTimeHelper() {
        delete q;
    }
    KIdleTime *q;
};

Q_GLOBAL_STATIC(KIdleTimeHelper, s_globalKIdleTime)

KIdleTime *KIdleTime::instance()
{
    if (!s_globalKIdleTime()->q) {
        new KIdleTime;
    }

    return s_globalKIdleTime()->q;
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

    QPointer<AbstractSystemPoller> poller;
    bool catchResume;

    int currentId;
    QHash<int, int> associations;
};

KIdleTime::KIdleTime()
        : QObject(0)
        , d_ptr(new KIdleTimePrivate())
{
    Q_ASSERT(!s_globalKIdleTime()->q);
    s_globalKIdleTime()->q = this;

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

#if HAVE_XSYNC
#if HAVE_XSCREENSAVER
    if (XSyncBasedPoller::instance()->isAvailable()) {
        poller = XSyncBasedPoller::instance();
    } else {
        poller = new XScreensaverBasedPoller();
    }
#else
    poller = XSyncBasedPoller::instance();
#endif
#else
#if HAVE_XSCREENSAVER
    poller = new XScreensaverBasedPoller();
#endif
#endif

#ifdef Q_OS_MAC
    poller = new MacPoller();
#endif

#ifdef Q_OS_WIN
    poller = new WindowsPoller();
#endif

    if (!poller.isNull()) {
        poller.data()->setUpPoller();
    }
}

void KIdleTimePrivate::unloadCurrentSystem()
{
    if (!poller.isNull()) {
        poller.data()->unloadPoller();
#if HAVE_XSYNC
        if (qobject_cast<XSyncBasedPoller*>(poller.data()) == 0) {
#endif
            poller.data()->deleteLater();
#if HAVE_XSYNC
        }
#endif
    }
}

void KIdleTimePrivate::_k_resumingFromIdle()
{
    Q_Q(KIdleTime);

    if (catchResume) {
        Q_EMIT q->resumingFromIdle();
        q->stopCatchingResumeEvent();
    }
}

void KIdleTimePrivate::_k_timeoutReached(int msec)
{
    Q_Q(KIdleTime);

    if (associations.values().contains(msec)) {
        Q_FOREACH (int key, associations.keys(msec)) {
            Q_EMIT q->timeoutReached(key);
            Q_EMIT q->timeoutReached(key, msec);
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

#include "moc_kidletime.cpp"
