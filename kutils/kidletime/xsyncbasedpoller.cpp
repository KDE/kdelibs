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

#ifdef HAVE_XTEST
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#endif // HAVE_XTEST
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
#ifdef HAVE_XSYNC
        , m_display(QX11Info::display())
        , m_idleCounter(X::None)
        , m_resetAlarm(X::None)
#endif
        , m_available(true)
{
    Q_ASSERT(!s_globalXSyncBasedPoller->q);
    s_globalXSyncBasedPoller->q = this;

#ifdef HAVE_XSYNC
    int sync_major, sync_minor;
    int ncounters;

    if (!XSyncQueryExtension(m_display, &m_sync_event, &m_sync_error)) {
        m_available = false;
        return;
    }

    if (!XSyncInitialize(m_display, &sync_major, &sync_minor)) {
        m_available = false;
        return;
    }

    kDebug() << sync_major << sync_minor;

    m_counters = XSyncListSystemCounters(m_display, &ncounters);

    bool idleFound = false;

    for (int i = 0; i < ncounters; ++i) {
        if (!strcmp(m_counters[i].name, "IDLETIME")) {
            idleFound = true;
            break;
        }
    }

    XSyncFreeSystemCounterList(m_counters);

    if (!idleFound) {
        m_available = false;
    }

#else
    m_available = false;
#endif

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
#ifdef HAVE_XSYNC
    int ncounters;

    if (!isAvailable()) {
        return false;
    }

    kDebug() << "XSync Inited";

    m_counters = XSyncListSystemCounters(m_display, &ncounters);

    bool idleFound = false;

    for (int i = 0; i < ncounters && !m_idleCounter; ++i) {
        if (!strcmp(m_counters[i].name, "IDLETIME")) {
            m_idleCounter = m_counters[i].counter;
            idleFound = true;
        }
    }

    if (!idleFound) {
        return false;
    }

    KApplication::kApplication()->installX11EventFilter(this);

    kDebug() << "Supported, init completed";

    return true;
#else
    return false;
#endif
}

void XSyncBasedPoller::unloadPoller()
{
    //XSyncFreeSystemCounterList( m_counters );
}

void XSyncBasedPoller::addTimeout(int nextTimeout)
{
    /* We need to set the counter to the idle time + the value
     * requested for next timeout
     */
#ifdef HAVE_XSYNC
    XSyncValue timeout;
    XSyncAlarm newalarm = X::None;
    /*XSyncValue idleTime;
    XSyncValue result;
    int overflow;*/

//    XSyncQueryCounter(m_display, m_idleCounter, &idleTime);

    XSyncIntToValue(&timeout, nextTimeout);

//    XSyncValueAdd(&result, idleTime, timeout, &overflow);

    setAlarm(m_display, &newalarm, m_idleCounter,
             XSyncPositiveComparison, timeout);

    m_timeoutAlarm[nextTimeout] = newalarm;
#endif
}

int XSyncBasedPoller::forcePollRequest()
{
    return poll();
}

int XSyncBasedPoller::poll()
{
#ifdef HAVE_XSYNC
    XSyncValue idleTime;

    XSyncQueryCounter(m_display, m_idleCounter, &idleTime);

    return XSyncValueLow32(idleTime);
#endif
    return -1;
}

void XSyncBasedPoller::removeTimeout(int timeout)
{
#ifdef HAVE_XSYNC
    if (m_timeoutAlarm.contains(timeout)) {
        XSyncAlarm a = m_timeoutAlarm[timeout];
        m_timeoutAlarm.remove(timeout);
        XSyncDestroyAlarm(m_display, a);
    }
#endif
}

QList<int> XSyncBasedPoller::timeouts() const
{
    return m_timeoutAlarm.keys();
}

void XSyncBasedPoller::stopCatchingIdleEvents()
{
#ifdef HAVE_XSYNC
    XSyncDestroyAlarm(m_display, m_resetAlarm);
    m_resetAlarm = X::None;
#endif
}

void XSyncBasedPoller::catchIdleEvent()
{
#ifdef HAVE_XSYNC
    XSyncValue idleTime;

    XSyncQueryCounter(m_display, m_idleCounter, &idleTime);

    /* Set the reset alarm to fire the next time idleCounter < the
     * current counter value. XSyncNegativeComparison means <= so
     * we have to subtract 1 from the counter value
     */

    int overflow;
    XSyncValue add;
    XSyncValue plusone;
    XSyncIntToValue(&add, -1);
    XSyncValueAdd(&plusone, idleTime, add, &overflow);
    setAlarm(m_display, &m_resetAlarm, m_idleCounter,
             XSyncNegativeComparison, plusone);
#endif

}

void XSyncBasedPoller::reloadAlarms()
{
    XSyncValue timeout;

    foreach(int nextTimeout, m_timeoutAlarm.keys()) {
        XSyncIntToValue(&timeout, nextTimeout);

        setAlarm(m_display, &(m_timeoutAlarm[nextTimeout]), m_idleCounter,
                 XSyncPositiveComparison, timeout);
    }
}

bool XSyncBasedPoller::x11Event(XEvent *event)
{
#ifdef HAVE_XSYNC
    XSyncAlarmNotifyEvent *alarmEvent;

    if (event->type != m_sync_event + XSyncAlarmNotify) {
        return false;
    }

    alarmEvent = (XSyncAlarmNotifyEvent *)event;

    if (alarmEvent->state == XSyncAlarmDestroyed) {
        return false;
    }

    foreach(int timeout, m_timeoutAlarm.keys()) {
        if (alarmEvent->alarm == m_timeoutAlarm[timeout]) {
            /* Bling! Caught! */
            emit timeoutReached(timeout);
            // Update back the alarm to fire back if the system gets inactive for the same time
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
#else
    return false;
#endif
}

#ifdef HAVE_XSYNC
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

    if (*alarm)
        XSyncChangeAlarm(dpy, *alarm, flags, &attr);
    else
        *alarm = XSyncCreateAlarm(dpy, flags, &attr);
}
#endif

void XSyncBasedPoller::simulateUserActivity()
{
#ifdef HAVE_XTEST
    Display* display = QX11Info::display();
    XTestFakeMotionEvent(display, 0, 1, 2, 0);
    XSync(display, false);
#endif // HAVE_XTEST
}

#include "xsyncbasedpoller.moc"


