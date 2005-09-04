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
#ifndef QTESTKEYBOARD_H
#define QTESTKEYBOARD_H

#include "QtTest/qttestassert.h"
#include "QtTest/qttest_global.h"
#include "QtTest/qttestsystem.h"

#include <QtCore/qpointer.h>
#include <QtGui/qapplication.h>
#include <QtGui/qevent.h>
#include <QtGui/qwidget.h>

#if defined Q_CC_HPACC
// aCC 3.37 complains about the template syntax in this
// file. It doesn't complain in Qt itself, though. (?)
#define QT_NO_PARTIAL_TEMPLATE_SPECIALIZATION
#endif

namespace QtTest
{
    int Q_TESTLIB_EXPORT defaultKeyDelay();

#ifndef QT_NO_PARTIAL_TEMPLATE_SPECIALIZATION
    template <int>
    class QEventSizeOfChecker
    {
    private:
        QEventSizeOfChecker() {}
    };
    template <>
    class QEventSizeOfChecker<sizeof(QEvent)>
    {
    public:
        QEventSizeOfChecker() {}
    };
#endif

    enum KeyAction { Press, Release, Click };

    class QSpontaneKeyEvent
    {
    public:
        void setSpontaneous() { spont = 1; };
        bool spontaneous() { return spont; };
        virtual void dummyFunc() {};
        virtual ~QSpontaneKeyEvent() {}

    protected:
        void *d;
        ushort t;

    private:
        ushort posted : 1;
        ushort spont : 1;
        ushort m_accept : 1;
        ushort reserved : 13;
    };

    static void simulateEvent(QWidget *widget, bool press, int code,
                              Qt::KeyboardModifiers modifier, QString text, bool repeat, int delay=-1)
    {
        QTEST_ASSERT(widget);
#ifndef QT_NO_PARTIAL_TEMPLATE_SPECIALIZATION
        // this is a compile time assert in case the signature of QEvent changed.
        QEventSizeOfChecker<sizeof(QSpontaneKeyEvent)> someone_changed_qevent_please_fix_testlib;
        Q_UNUSED(someone_changed_qevent_please_fix_testlib);
#endif
        // and the same paranoia for runtime for compiler that don't understand partial templates
        QTEST_ASSERT(sizeof(QEvent) == sizeof(QSpontaneKeyEvent));

        if (delay == -1 || delay < defaultKeyDelay())
            delay = defaultKeyDelay();
        if(delay > 0)
            QtTest::wait(delay);

        QKeyEvent a(press ? QEvent::KeyPress : QEvent::KeyRelease, code, modifier, text, repeat);
        // this is should be safe because of the paranoia methods above.
        reinterpret_cast<QSpontaneKeyEvent *>(&a)->setSpontaneous();
        qApp->notify(widget, &a);
    }

    static void sendKeyEvent(KeyAction action, QWidget *widget, Qt::Key code,
                             QString text, Qt::KeyboardModifiers modifier, int delay=-1)
    {
        QTEST_ASSERT(qApp);

        if (!widget)
            widget = QWidget::keyboardGrabber();
        if (!widget)
            widget = qApp->focusWidget();
        if (!widget)
            widget = qApp->activePopupWidget();
        if (!widget)
            widget = qApp->activeWindow();

        QTEST_ASSERT(widget);

        if (action == Click) {
            QPointer<QWidget> ptr(widget);
            sendKeyEvent(Press, widget, code, text, modifier, delay);
            if (!ptr) {
                // if we send key-events to embedded widgets, they might be destroyed
                // when the user presses Return
                return;
            }
            sendKeyEvent(Release, widget, code, text, modifier, delay);
            return;
        }

        bool repeat = false;

        if (action == Press) {
            if (modifier & Qt::ShiftModifier)
                simulateEvent(widget, true, Qt::Key_Shift, 0, QString(), false, delay);

            if (modifier & Qt::ControlModifier)
                simulateEvent(widget, true, Qt::Key_Control, modifier & Qt::ShiftModifier, QString(), false, delay);

            if (modifier & Qt::AltModifier)
                simulateEvent(widget, true, Qt::Key_Alt,
                              modifier & (Qt::ShiftModifier | Qt::ControlModifier), QString(), false, delay);
            if (modifier & Qt::MetaModifier)
                simulateEvent(widget, true, Qt::Key_Meta, modifier & (Qt::ShiftModifier
                                                                      | Qt::ControlModifier | Qt::AltModifier), QString(), false, delay);
            simulateEvent(widget, true, code, modifier, text, repeat, delay);
        } else if (action == Release) {
            simulateEvent(widget, false, code, modifier, text, repeat, delay);

            if (modifier & Qt::MetaModifier)
                simulateEvent(widget, false, Qt::Key_Meta, modifier, QString(), false, delay);
            if (modifier & Qt::AltModifier)
                simulateEvent(widget, false, Qt::Key_Alt, modifier &
                              (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier), QString(), false, delay);

            if (modifier & Qt::ControlModifier)
                simulateEvent(widget, false, Qt::Key_Control,
                              modifier & (Qt::ShiftModifier | Qt::ControlModifier), QString(), false, delay);

            if (modifier & Qt::ShiftModifier)
                simulateEvent(widget, false, Qt::Key_Shift, modifier & Qt::ShiftModifier, QString(), false, delay);
        }
    }

    // Convenience function
    static void sendKeyEvent(KeyAction action, QWidget *widget, Qt::Key code,
                             char ascii, Qt::KeyboardModifiers modifier, int delay=-1)
    {
        QString text;
        if (ascii)
            text = QString(QChar::fromLatin1(ascii));
        sendKeyEvent(action, widget, code, text, modifier, delay);
    }

    inline static void keyEvent(KeyAction action, QWidget *widget, char ascii,
                                Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)
    { sendKeyEvent(action, widget, asciiToKey(ascii), ascii, modifier, delay); }
    inline static void keyEvent(KeyAction action, QWidget *widget, Qt::Key key,
                                Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)
    { sendKeyEvent(action, widget, key, keyToAscii(key), modifier, delay); }

    inline static void keyClicks(QWidget *widget, const QString &sequence,
                                 Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)
    {
        for (int i=0; i < sequence.length(); i++)
            keyEvent(Click, widget, sequence.at(i).toLatin1(), modifier, delay);
    }

    inline static void keyPress(QWidget *widget, char key, Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)
    { keyEvent(Press, widget, key, modifier, delay); }
    inline static void keyRelease(QWidget *widget, char key, Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)
    { keyEvent(Release, widget, key, modifier, delay); }
    inline static void keyClick(QWidget *widget, char key, Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)
    { keyEvent(Click, widget, key, modifier, delay); }
    inline static void keyPress(QWidget *widget, Qt::Key key, Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)
    { keyEvent(Press, widget, key, modifier, delay); }
    inline static void keyRelease(QWidget *widget, Qt::Key key, Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)
    { keyEvent(Release, widget, key, modifier, delay); }
    inline static void keyClick(QWidget *widget, Qt::Key key, Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)
    { keyEvent(Click, widget, key, modifier, delay); }

}

#endif //QTESTKEYBOARD_H

