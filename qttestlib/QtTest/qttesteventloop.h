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
#ifndef QTESTEVENTLOOP_H
#define QTESTEVENTLOOP_H

#include <QtCore/qcoreapplication.h>
#include <QtCore/qeventloop.h>
#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>

class QtTestEventLoop : public QObject
{
public:
    inline QtTestEventLoop(QObject *parent = 0)
        : QObject(parent), inLoop(false), _timeout(false), timerId(-1), loop(0) {}
    inline void enterLoop(int secs);
    inline void exitLoop();

    inline void changeInterval(int secs)
    { killTimer(timerId); timerId = startTimer(secs * 1000); }

    inline bool timeout() const
    { return _timeout; }

    inline static QtTestEventLoop &instance()
    {
        static QPointer<QtTestEventLoop> testLoop;
        if (testLoop.isNull())
            testLoop = new QtTestEventLoop(QCoreApplication::instance());
        return *static_cast<QtTestEventLoop *>(testLoop);
    }

protected:
    inline void timerEvent(QTimerEvent *e);

private:
    bool inLoop;
    bool _timeout;
    int timerId;

    QEventLoop *loop;
};

inline void QtTestEventLoop::enterLoop(int secs)
{
    Q_ASSERT(!loop);

    QEventLoop l;

    inLoop = true;
    _timeout = false;

    timerId = startTimer(secs * 1000);

    loop = &l;
    l.exec();
    loop = 0;
}

inline void QtTestEventLoop::exitLoop()
{
    Q_ASSERT(loop);

    killTimer(timerId); timerId = -1;

    loop->exit();

    inLoop = false;
}

inline void QtTestEventLoop::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != timerId)
        return;
    _timeout = true;
    exitLoop();
}

#endif

