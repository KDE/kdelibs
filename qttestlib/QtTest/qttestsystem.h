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
#ifndef QTTESTSYSTEM_H
#define QTTESTSYSTEM_H

#include <QtCore/qcoreapplication.h>
#include <QtCore/qdatetime.h>
#ifdef Q_OS_UNIX
#include <time.h>
#endif
#ifdef Q_OS_WIN32
#include <windows.h> //for Sleep
#endif

namespace QtTest
{
    inline static void sleep(int ms)
    {
#ifdef Q_OS_WIN32
        Sleep(uint(ms));
#else
        struct timespec ts = { 0, 0 };
        // a nanosecond is 1/1000 of a microsecond, a microsecond is 1/1000 of a millisecond
        ts.tv_nsec = ms * 1000 * 1000;
        nanosleep(&ts, NULL);
#endif
    }

    inline static void wait(int ms)
    {
        Q_ASSERT(QCoreApplication::instance());

        QTime timer;
        timer.start();
        do {
            QCoreApplication::processEvents(QEventLoop::AllEvents, ms);
            QtTest::sleep(10);
        } while (timer.elapsed() < ms);
    }
}

#endif
