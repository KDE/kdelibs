/**
 * Copyright 2009, 2010  Michael Leupold <lemma@confuego.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>
 */

#include "ksystemeventfilter.h"

#include <QObject>
#include <QWidget>
#include <QWeakPointer>
#include <QSet>
#include <QAbstractEventDispatcher>
#include <kglobal.h>

// Our global event-filter which will pass events to all registered
// widget filters.
bool _k_eventFilter(void *message);

/**
 * @internal This class makes forwarding events to QWidgets possible
 */
class KEventHackWidget : public QWidget
{
public:
#if defined(Q_WS_X11)
    bool publicX11Event(XEvent *e)
    {
        return x11Event(e);
    }
#elif defined(Q_WS_MAC)
    bool publicMacEvent(EventHandlerCallRef caller, EventRef event)
    {
        return macEvent(caller, event);
    }
#elif defined(Q_WS_WIN)
    bool publicWinEvent(MSG *message, long *result)
    {
        return winEvent(message, result);
    }
#endif
};

class KSystemEventFilterPrivate : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void filterWidgetDestroyed(QObject *widget)
    {
        KSystemEventFilter::removeEventFilter(qobject_cast<QWidget*>(widget));
    }

public:
    KSystemEventFilterPrivate()
    {
        // install our event-filter. note that this will only happen when this
        // object is constructed (which is when the global static is first
        // accessed.
        m_nextFilter = QAbstractEventDispatcher::instance()->setEventFilter(_k_eventFilter);
    }

    bool filterEvent(void *message);
    
    // the installed event filters
    QList< QWeakPointer<QWidget> > m_filters;
    // if an event filter had already been previously installed, it is
    // stored here so we can call it if none of our event filters consumes
    // the event
    QAbstractEventDispatcher::EventFilter m_nextFilter;
};

K_GLOBAL_STATIC(KSystemEventFilterPrivate, kSystemEventFilter)

bool _k_eventFilter(void *message)
{
    return kSystemEventFilter->filterEvent(message);
}

bool KSystemEventFilterPrivate::filterEvent(void *message)
{
    if (!m_filters.isEmpty()) {
#if defined(Q_WS_X11)
        XEvent *xevt = static_cast<XEvent*>(message);
        // pass the event as long as it's not consumed
        Q_FOREACH (const QWeakPointer<QWidget> &wp, m_filters) {
            if (QWidget *w = wp.data()) {
                if (static_cast<KEventHackWidget*>(w)->publicX11Event(xevt)) {
                    return true;
                }
            }
        }
#elif defined(Q_WS_MAC)
        // FIXME: untested

/*        NSEvent *nsevt = static_cast<NSEvent*>(message);
        // pass the event as long as it's not consumed
        Q_FOREACH (const QWeakPointer<QWidget> &wp, m_filters) {
            if (QWidget *w = wp.data()) {
                if (static_cast<KEventHackWidget*>(w)->publicMacEvent(0, nsevt->eventRef)) {
                    return true;
                }
            }
        }*/
#elif defined(Q_WS_WIN)
        // FIXME: untested
        
/*        MSG *msg = static_cast<MSG*>(message);
        long ret; // widget filter returns are discarded!
        // pass the event as long as it's not consumed
        Q_FOREACH (const QWeakPointer<QWidget> &wp, m_filters) {
            if (QWidget *w = wp.data()) {
                if (static_cast<KEventHackWidget*>(w)->publicWinEvent(msg, &ret)) {
                    return true;
                }
            }
        }*/
#endif
    }

    // call next filter if we have one
    if (m_nextFilter) {
        return m_nextFilter(message);
    } else {
        return false;
    }
}

namespace KSystemEventFilter
{

void installEventFilter(QWidget *filter)
{
    kSystemEventFilter->m_filters.append(filter);
    kSystemEventFilter->connect(filter, SIGNAL(destroyed(QObject*)),
                                SLOT(filterWidgetDestroyed(QObject*)));
}

void removeEventFilter(const QWidget *filter)
{
    QMutableListIterator< QWeakPointer<QWidget> > it(kSystemEventFilter->m_filters);
    while (it.hasNext()) {
        QWidget *w = it.next().data();
        if (w == filter || w == 0) {
            it.remove();
        }
    }
}

}

#include "ksystemeventfilter.moc"
