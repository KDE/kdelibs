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

#include "../kidletime.h"

KIdleTest::KIdleTest()
{
    connect(KIdleTime::instance(), SIGNAL(resumingFromIdle()), this, SLOT(resumeEvent()));
    connect(KIdleTime::instance(), SIGNAL(timeoutReached(int)), this, SLOT(timeoutReached(int)));

    KIdleTime::instance()->catchNextResumeEvent();

    printf("Your idle time is %d\n", KIdleTime::instance()->idleTime());
    printf("Welcome!! Move your mouse or do something to start...\n");
}

KIdleTest::~KIdleTest()
{
}

void KIdleTest::resumeEvent()
{
    KIdleTime::instance()->removeAllIdleTimeouts();

    printf("Great! Now stay idle for 5 seconds to get a nice message. From 10 seconds on, you can move your mouse to get back here.\n");
    printf("If you will stay idle for too long, I will simulate your activity after 25 seconds, and make everything start back\n");

    KIdleTime::instance()->addIdleTimeout(5000);
    KIdleTime::instance()->addIdleTimeout(10000);
    KIdleTime::instance()->addIdleTimeout(25000);
}

void KIdleTest::timeoutReached(int timeout)
{
    if (timeout == 5000) {
        printf("5 seconds passed, stay still some more...\n");
    } else if (timeout == 10000) {
        KIdleTime::instance()->catchNextResumeEvent();
        printf("Cool. You can move your mouse to start over\n");
    } else if (timeout == 25000) {
        printf("Uff, you're annoying me. Let's start again. I'm simulating your activity now\n");
        KIdleTime::instance()->simulateUserActivity();
    } else {
        qDebug() << "OUCH";
    }
}
