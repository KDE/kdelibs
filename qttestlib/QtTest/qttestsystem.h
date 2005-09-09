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

#include <QtTest/qttestcase.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdatetime.h>

namespace QtTest
{
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
