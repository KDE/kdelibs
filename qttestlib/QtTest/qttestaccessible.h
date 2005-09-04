/***************************************************************************
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
*/
#ifndef QTESTACCESSIBLE_H
#define QTESTACCESSIBLE_H

#ifndef QT_NO_ACCESSIBILITY

#define QTEST_ACCESSIBILITY

#define VERIFY_EVENT(object, child, event) \
    VERIFY(QtTestAccessibility::verifyEvent(object, child, (int)event))

#include <QtCore/qlist.h>
#include <QtGui/qaccessible.h>
#include <QtGui/qapplication.h>

class QObject;

struct QtTestAccessibilityEvent
{
    QtTestAccessibilityEvent(QObject* o = 0, int c = 0, int e = 0)
        : object(o), child(c), event(e) {}

    bool operator==(const QtTestAccessibilityEvent &o) const
    {
        return o.object == object && o.child == child && o.event == event;
    }

    QObject* object;
    int child;
    int event;
};

typedef QList<QtTestAccessibilityEvent> EventList;

class QtTestAccessibility
{
public:
    static void initialize()
    {
        if (!instance()) {
            instance() = new QtTestAccessibility;
            qAddPostRoutine(cleanup);
        }
    }
    static void cleanup()
    {
        delete instance();
        instance() = 0;
    }
    static void clearEvents() { eventList().clear(); }
    static EventList events() { return eventList(); }
    static bool verifyEvent(const QtTestAccessibilityEvent& ev)
    {
        if (eventList().isEmpty())
            return FALSE;
        return eventList().takeFirst() == ev;
    }

    static bool verifyEvent(QObject *o, int c, int e)
    {
        return verifyEvent(QtTestAccessibilityEvent(o, c, e));
    }

private:
    QtTestAccessibility()
    {
        QAccessible::installUpdateHandler(updateHandler);
        QAccessible::installRootObjectHandler(rootObjectHandler);
    }

    ~QtTestAccessibility()
    {
        QAccessible::installUpdateHandler(0);
        QAccessible::installRootObjectHandler(0);
    }

    static void rootObjectHandler(QObject *object)
    {
        //    qDebug("rootObjectHandler called %p", object);
        if (object) {
            QApplication* app = qobject_cast<QApplication*>(object);
            if ( !app )
                qWarning("QTEST_ACCESSIBILITY: root Object is not a QApplication!");
        } else {
            qWarning("QTEST_ACCESSIBILITY: root Object called with 0 pointer");
        }
    }

    static void updateHandler(QObject *o, int c, QAccessible::Event e)
    {
        //    qDebug("updateHandler called: %p %d %d", o, c, (int)e);
        eventList().append(QtTestAccessibilityEvent(o, c, (int)e));
    }

    static EventList &eventList()
    {
        static EventList list;
        return list;
    }

    static QtTestAccessibility *&instance()
    {
        static QtTestAccessibility *ta = 0;
        return ta;
    }
};

#endif

#endif

