/***************************************************************************
 *   Copyright (C) 2009 by Dario Freddi <drf@kde.org>                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "KIdleTest.h"
#include <QDebug>
#include <QTimer>

#include "../kidle.h"

KIdleTest::KIdleTest()
{
    connect(KIdleTime::instance(), SIGNAL(resumingFromIdle()), this, SLOT(resumeEvent()));
    connect(KIdleTime::instance(), SIGNAL(timeoutReached(int)), this, SLOT(timeoutReached(int)));

    KIdleTime::instance()->catchNextResumeEvent();

    qDebug() << "Your idle time is " << KIdleTime::instance()->idleTime();
    qDebug() << "Welcome!! Move your mouse or do something to start...";

}

KIdleTest::~KIdleTest()
{
}

void KIdleTest::resumeEvent()
{
    KIdleTime::instance()->removeAllIdleTimeouts();

    qDebug() << "Great! Now stay idle for 5 seconds to get a nice message. From 10 seconds on, you can move your mouse to get back here.";
    qDebug() << "If you will stay idle for too long, I will simulate your activity after 25 seconds, and make everything start back";

    KIdleTime::instance()->addIdleTimeout(5000);
    KIdleTime::instance()->addIdleTimeout(10000);
    KIdleTime::instance()->addIdleTimeout(25000);
}

void KIdleTest::timeoutReached(int timeout)
{
    if (timeout == 5000) {
        qDebug() << "5 seconds passed, stay still some more...";
    } else if (timeout == 10000) {
        KIdleTime::instance()->catchNextResumeEvent();
        qDebug() << "Cool. You can move your mouse to start over";
    } else if (timeout == 25000) {
        qDebug() << "Uff, you're annoying me. Let's start again. I'm simulating your activity now";
        KIdleTime::instance()->simulateUserActivity();
    } else {
        qDebug() << "OUCH";
    }
}
