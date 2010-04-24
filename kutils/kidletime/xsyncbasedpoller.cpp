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

#include "xsyncbasedpoller.h"

#include <QX11Info>

#include <klocalizedstring.h>
#include <kglobal.h>

#include <fixx11h.h>

class XSyncBasedPollerHelper
{
public:
    XSyncBasedPollerHelper() : q(0) {}
    ~XSyncBasedPollerHelper() {
        delete q;
    }
    XSyncBasedPoller *q;
};

K_GLOBAL_STATIC(XSyncBasedPollerHelper, s_globalXSyncBasedPoller)

XSyncBasedPoller *XSyncBasedPoller::instance()
{
    if (!s_globalXSyncBasedPoller->q) {
        new XSyncBasedPoller;
    }

    return s_globalXSyncBasedPoller->q;
}

XSyncBasedPoller::XSyncBasedPoller(QWidget *parent)
        : AbstractSystemPoller(parent)
        , m_display(QX11Info::display())
        , m_idleCounter(X::None)
        , m_resetAlarm(X::None)
        , m_available(true)
{
    Q_ASSERT(!s_globalXSyncBasedPoller->q);
    s_globalXSyncBasedPoller->q = this;

    int sync_major, sync_minor;
    int ncounters;
    XSyncSystemCounter *counters;

    if (!XSyncQueryExtension(m_display, &m_sync_event, &m_sync_error)) {
        m_available = false;
        return;
    }

    if (!XSyncInitialize(m_display, &sync_major, &sync_minor)) {
        m_available = false;
        return;
    }

    kDebug() << sync_major << sync_minor;

    counters = XSyncListSystemCounters(m_display, &ncounters);

    bool idleFound = false;

    for (int i = 0; i < ncounters; ++i) {
        if (!strcmp(counters[i].name, "IDLETIME")) {
            m_idleCounter = counters[i].counter;
            idleFound = true;
            break;
        }
    }

    XSyncFreeSystemCounterList(counters);

    if (!idleFound) {
        m_available = false;
    }

    if (m_available) {
        kDebug() << "XSync seems available and ready";
    } else {
        kDebug() << "XSync seems not available";
    }
}

XSyncBasedPoller::~XSyncBasedPoller()
{
}

bool XSyncBasedPoller::isAvailable()
{
    return m_available;
}

bool XSyncBasedPoller::setUpPoller()
{
    if (!isAvailable()) {
        return false;
    }

    kDebug() << "XSync Inited";

    KApplication::kApplication()->installX11EventFilter(this);

    kDebug() << "Supported, init completed";

    return true;
}

void XSyncBasedPoller::unloadPoller()
{
}

void XSyncBasedPoller::addTimeout(int nextTimeout)
{
    /* We need to set the counter to the idle time + the value
     * requested for next timeout
     */

    // If there's already an alarm for the requested timeout, skip
    if (m_timeoutAlarm.contains(nextTimeout)) {
        return;
    }

    XSyncValue timeout;
    XSyncAlarm newalarm = X::None;

    XSyncIntToValue(&timeout, nextTimeout);

    setAlarm(m_display, &newalarm, m_idleCounter,
             XSyncPositiveComparison, timeout);

    m_timeoutAlarm.insert(nextTimeout, newalarm);
}

int XSyncBasedPoller::forcePollRequest()
{
    return poll();
}

int XSyncBasedPoller::poll()
{
    XSyncValue idleTime;
    XSyncQueryCounter(m_display, m_idleCounter, &idleTime);

    return XSyncValueLow32(idleTime);
}

void XSyncBasedPoller::removeTimeout(int timeout)
{
    if (m_timeoutAlarm.contains(timeout)) {
        XSyncAlarm a = m_timeoutAlarm[timeout];
        XSyncDestroyAlarm(m_display, a);
        m_timeoutAlarm.remove(timeout);
    }
}

QList<int> XSyncBasedPoller::timeouts() const
{
    return m_timeoutAlarm.keys();
}

void XSyncBasedPoller::stopCatchingIdleEvents()
{
    if (m_resetAlarm != X::None) {
        XSyncDestroyAlarm(m_display, m_resetAlarm);
        m_resetAlarm = X::None;
    }
}

void XSyncBasedPoller::catchIdleEvent()
{
    XSyncValue idleTime;

    XSyncQueryCounter(m_display, m_idleCounter, &idleTime);

    /* Set the reset alarm to fire the next time idleCounter < the
     * current counter value. XSyncNegativeComparison means <= so
     * we have to subtract 1 from the counter value
     */

    //NOTE: this must be a int, else compilation might fail
    int overflow;
    XSyncValue add;
    XSyncValue plusone;
    XSyncIntToValue(&add, -1);
    XSyncValueAdd(&plusone, idleTime, add, &overflow);
    setAlarm(m_display, &m_resetAlarm, m_idleCounter,
             XSyncNegativeComparison, plusone);
}

void XSyncBasedPoller::reloadAlarms()
{
    XSyncValue timeout;

    for (QHash<int, XSyncAlarm>::iterator i = m_timeoutAlarm.begin(); i != m_timeoutAlarm.end(); ++i) {
        XSyncIntToValue(&timeout, i.key());

        setAlarm(m_display, &(i.value()), m_idleCounter,
                 XSyncPositiveComparison, timeout);
    }
}

bool XSyncBasedPoller::x11Event(XEvent *event)
{
    XSyncAlarmNotifyEvent *alarmEvent;

    if (event->type != m_sync_event + XSyncAlarmNotify) {
        return false;
    }

    alarmEvent = (XSyncAlarmNotifyEvent *)event;

    if (alarmEvent->state == XSyncAlarmDestroyed) {
        return false;
    }

    for (QHash<int, XSyncAlarm>::const_iterator i = m_timeoutAlarm.constBegin(); i != m_timeoutAlarm.constEnd(); ++i) {
        if (alarmEvent->alarm == i.value()) {
            /* Bling! Caught! */
            emit timeoutReached(i.key());
            // Update the alarm to fire back if the system gets inactive for the same time
            catchIdleEvent();
            return false;
        }
    }

    if (alarmEvent->alarm == m_resetAlarm) {
        /* Resuming from idle here! */
        stopCatchingIdleEvents();
        reloadAlarms();
        emit resumingFromIdle();
    }

    return false;
}

void XSyncBasedPoller::setAlarm(Display *dpy, XSyncAlarm *alarm, XSyncCounter counter,
                                XSyncTestType test, XSyncValue value)
{
    XSyncAlarmAttributes  attr;
    XSyncValue            delta;
    unsigned int          flags;

    XSyncIntToValue(&delta, 0);

    attr.trigger.counter     = counter;
    attr.trigger.value_type  = XSyncAbsolute;
    attr.trigger.test_type   = test;
    attr.trigger.wait_value  = value;
    attr.delta               = delta;

    flags = XSyncCACounter | XSyncCAValueType | XSyncCATestType |
            XSyncCAValue | XSyncCADelta;

    if (*alarm) {
        XSyncChangeAlarm(dpy, *alarm, flags, &attr);
    } else {
        *alarm = XSyncCreateAlarm(dpy, flags, &attr);
    }
}

void XSyncBasedPoller::simulateUserActivity()
{
    XResetScreenSaver(QX11Info::display());
}

#include "xsyncbasedpoller.moc"
