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
#include <QPointer>
#include <QSet>
#include <QAbstractNativeEventFilter>
#include <QCoreApplication>

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
    bool publicNativeEvent(const QByteArray &eventType, void *message, long *result)
    {
        return nativeEvent(eventType, message, result);
    }
};

class KSystemEventFilterPrivate : public QObject,  public QAbstractNativeEventFilter
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
        QCoreApplication::instance()->installNativeEventFilter(this);
    }

    bool nativeEventFilter(const QByteArray&, void *message, long *);

    // the installed event filters
    QList< QPointer<QWidget> > m_filters;
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
        Q_FOREACH (const QPointer<QWidget> &wp, m_filters) {
            if (QWidget *w = wp.data()) {
                if (static_cast<KEventHackWidget*>(w)->publicNativeEvent(eventType, message, result)) {
                    return true;
                }
            }
        }
    }


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
    QMutableListIterator< QPointer<QWidget> > it(kSystemEventFilter()->m_filters);
    while (it.hasNext()) {
        QWidget *w = it.next().data();
        if (w == filter || w == 0) {
            it.remove();
        }
    }
}

}

#include "ksystemeventfilter.moc"
