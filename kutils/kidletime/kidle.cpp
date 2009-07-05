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

#include "kidle.h"

#include "widgetbasedpoller.h"
#include "xsyncbasedpoller.h"

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
        : d_ptr(new KIdleTimePrivate())
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

    if (XSyncBasedPoller::instance()->isAvailable()) {
        XSyncBasedPoller::instance()->setUpPoller();
        poller = XSyncBasedPoller::instance();
    } else {
        poller = new WidgetBasedPoller();
        poller->setUpPoller();
    }
}

void KIdleTimePrivate::unloadCurrentSystem()
{
    if (poller) {
        poller->unloadPoller();

        if (poller->getPollingType() != AbstractSystemPoller::XSyncBased) {
            poller->deleteLater();
        }
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
    d->poller->stopCatchingIdleEvents();
}

int KIdleTime::idleTime()
{
    Q_D(KIdleTime);

    return d->poller->forcePollRequest();
}

#include "kidle.moc"
