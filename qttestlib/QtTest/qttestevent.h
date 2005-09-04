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
#ifndef QTESTEVENT_H
#define QTESTEVENT_H

#include "QtTest/qttest_global.h"
#include "QtTest/qttestkeyboard.h"
#include "QtTest/qttestmouse.h"
#include "QtTest/qttestsystem.h"

#include <QtCore/qlist.h>

#include <stdlib.h>

class QtTestEvent
{
public:
    virtual void simulate(QWidget *w) = 0;
    virtual QtTestEvent *clone() const = 0;

    virtual ~QtTestEvent() {}
};

class QtTestKeyEvent: public QtTestEvent
{
public:
    inline QtTestKeyEvent(QtTest::KeyAction action, Qt::Key key, Qt::KeyboardModifiers modifiers, int delay)
        : _action(action), _delay(delay), _modifiers(modifiers), _ascii(0), _key(key) {}
    inline QtTestKeyEvent(QtTest::KeyAction action, char ascii, Qt::KeyboardModifiers modifiers, int delay)
        : _action(action), _delay(delay), _modifiers(modifiers),
          _ascii(ascii), _key(Qt::Key_unknown) {}
    inline QtTestEvent *clone() const { return new QtTestKeyEvent(*this); }

    inline void simulate(QWidget *w)
    {
        if (_ascii == 0)
            QtTest::keyEvent(_action, w, _key, _modifiers, _delay);
        else
            QtTest::keyEvent(_action, w, _ascii, _modifiers, _delay);
    }

protected:
    QtTest::KeyAction _action;
    int _delay;
    Qt::KeyboardModifiers _modifiers;
    char _ascii;
    Qt::Key _key;
};

class QtTestKeyClicksEvent: public QtTestEvent
{
public:
    inline QtTestKeyClicksEvent(const QString &keys, Qt::KeyboardModifiers modifiers, int delay)
        : _keys(keys), _modifiers(modifiers), _delay(delay) {}
    inline QtTestEvent *clone() const { return new QtTestKeyClicksEvent(*this); }

    inline void simulate(QWidget *w)
    {
        QtTest::keyClicks(w, _keys, _modifiers, _delay);
    }

private:
    QString _keys;
    Qt::KeyboardModifiers _modifiers;
    int _delay;
};

class QtTestMouseEvent: public QtTestEvent
{
public:
    inline QtTestMouseEvent(QtTest::MouseAction action, Qt::MouseButton button,
            Qt::KeyboardModifiers modifiers, QPoint position, int delay)
        : _action(action), _button(button), _modifiers(modifiers), _pos(position), _delay(delay) {}
    inline QtTestEvent *clone() const { return new QtTestMouseEvent(*this); }

    inline void simulate(QWidget *w)
    {
        QtTest::mouseEvent(_action, w, _button, _modifiers, _pos, _delay);
    }

private:
    QtTest::MouseAction _action;
    Qt::MouseButton _button;
    Qt::KeyboardModifiers _modifiers;
    QPoint _pos;
    int _delay;
};

class QtTestDelayEvent: public QtTestEvent
{
public:
    inline QtTestDelayEvent(int msecs): _delay(msecs) {}
    inline QtTestEvent *clone() const { return new QtTestDelayEvent(*this); }

    inline void simulate(QWidget * /*w*/) { QtTest::wait(_delay); }

private:
    int _delay;
};

typedef QList<QtTestEvent *> QTEventList;

class QtTestEventList: public QTEventList
{
public:
    inline QtTestEventList() {}
    inline QtTestEventList(const QtTestEventList &other): QTEventList()
    { for (int i = 0; i < other.count(); ++i) append(other.at(i)->clone()); }
    inline ~QtTestEventList()
    { clear(); }
    inline void clear()
    { qDeleteAll(*this); QList<QtTestEvent *>::clear(); }

    inline void addKeyClick(Qt::Key qtKey, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { addKeyEvent(QtTest::Click, qtKey, modifiers, msecs); }
    inline void addKeyPress(Qt::Key qtKey, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { addKeyEvent(QtTest::Press, qtKey, modifiers, msecs); }
    inline void addKeyRelease(Qt::Key qtKey, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { addKeyEvent(QtTest::Release, qtKey, modifiers, msecs); }
    inline void addKeyEvent(QtTest::KeyAction action, Qt::Key qtKey,
                            Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { append(new QtTestKeyEvent(action, qtKey, modifiers, msecs)); }

    inline void addKeyClick(char ascii, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { addKeyEvent(QtTest::Click, ascii, modifiers, msecs); }
    inline void addKeyPress(char ascii, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { addKeyEvent(QtTest::Press, ascii, modifiers, msecs); }
    inline void addKeyRelease(char ascii, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { addKeyEvent(QtTest::Release, ascii, modifiers, msecs); }
    inline void addKeyClicks(const QString &keys, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { append(new QtTestKeyClicksEvent(keys, modifiers, msecs)); }
    inline void addKeyEvent(QtTest::KeyAction action, char ascii, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { append(new QtTestKeyEvent(action, ascii, modifiers, msecs)); }

    inline void addMousePress(Qt::MouseButton button, Qt::KeyboardModifiers stateKey = 0,
                              QPoint pos = QPoint(), int delay=-1)
    { append(new QtTestMouseEvent(QtTest::MousePress, button, stateKey, pos, delay)); }
    inline void addMouseRelease(Qt::MouseButton button, Qt::KeyboardModifiers stateKey = 0,
                                QPoint pos = QPoint(), int delay=-1)
    { append(new QtTestMouseEvent(QtTest::MouseRelease, button, stateKey, pos, delay)); }
    inline void addMouseClick(Qt::MouseButton button, Qt::KeyboardModifiers stateKey = 0,
                              QPoint pos = QPoint(), int delay=-1)
    { append(new QtTestMouseEvent(QtTest::MouseClick, button, stateKey, pos, delay)); }
    inline void addMouseDClick(Qt::MouseButton button, Qt::KeyboardModifiers stateKey = 0,
                            QPoint pos = QPoint(), int delay=-1)
    { append(new QtTestMouseEvent(QtTest::MousePress, button, stateKey, pos, delay)); }
    inline void addMouseMove(QPoint pos = QPoint(), int delay=-1)
    { append(new QtTestMouseEvent(QtTest::MouseMove, Qt::NoButton, 0, pos, delay)); }

    inline void addDelay(int msecs)
    { append(new QtTestDelayEvent(msecs)); }

    inline void simulate(QWidget *w)
    {
        for (int i = 0; i < count(); ++i)
            at(i)->simulate(w);
    }
};

Q_DECLARE_METATYPE(QtTestEventList)

#endif

