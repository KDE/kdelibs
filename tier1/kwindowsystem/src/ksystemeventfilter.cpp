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

#include "ksystemeventfilter_p.h"

#include <QObject>
#include <QWidget>
#include <QWeakPointer>
#include <QSet>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QAbstractEventDispatcher>
#else
#include <QAbstractNativeEventFilter>
#include <QCoreApplication>
#endif

#include <config-kwindowsystem.h>

// Our global event-filter which will pass events to all registered
// widget filters.
bool _k_eventFilter(void *message);

/**
 * @internal This class makes forwarding events to QWidgets possible
 */
class KEventHackWidget : public QWidget
{
public:
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    bool publicNativeEvent(const QByteArray &eventType, void *message, long *result)
    {
        return nativeEvent(eventType, message, result);
    }
#endif
};

class KSystemEventFilterPrivate : public QObject
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                                                ,  public QAbstractNativeEventFilter
#endif
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        QCoreApplication::instance()->installNativeEventFilter(this);
#else
        m_nextFilter = QAbstractEventDispatcher::instance()->setEventFilter(_k_eventFilter);
#endif
    }

    bool nativeEventFilter(const QByteArray&, void *message, long *);

    // the installed event filters
    QList< QWeakPointer<QWidget> > m_filters;
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    // if an event filter had already been previously installed, it is
    // stored here so we can call it if none of our event filters consumes
    // the event
    QAbstractEventDispatcher::EventFilter m_nextFilter;
#endif
};

Q_GLOBAL_STATIC(KSystemEventFilterPrivate, kSystemEventFilter)

bool _k_eventFilter(void *message)
{
    return kSystemEventFilter()->nativeEventFilter("", message, 0);
}

bool KSystemEventFilterPrivate::nativeEventFilter(const QByteArray& eventType, void *message, long *result)
{
    if (!m_filters.isEmpty()) {
        // pass the event as long as it's not consumed
        Q_FOREACH (const QWeakPointer<QWidget> &wp, m_filters) {
            if (QWidget *w = wp.data()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                if (static_cast<KEventHackWidget*>(w)->publicNativeEvent(eventType, message, result)) {
                    return true;
                }
#endif
            }
        }
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    // call next filter if we have one
    if (m_nextFilter)
        return m_nextFilter(message);
#endif

    return false;
}

namespace KSystemEventFilter
{

void installEventFilter(QWidget *filter)
{
    kSystemEventFilter()->m_filters.append(filter);
    kSystemEventFilter()->connect(filter, SIGNAL(destroyed(QObject*)),
                                SLOT(filterWidgetDestroyed(QObject*)));
}

void removeEventFilter(const QWidget *filter)
{
    QMutableListIterator< QWeakPointer<QWidget> > it(kSystemEventFilter()->m_filters);
    while (it.hasNext()) {
        QWidget *w = it.next().data();
        if (w == filter || w == 0) {
            it.remove();
        }
    }
}

}

#include "ksystemeventfilter.moc"
