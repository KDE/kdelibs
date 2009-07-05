/*
 * KIdleTest.cpp
 *
 *  Created on: 05/lug/2009
 *      Author: drf
 */

#include "KIdleTest.h"
#include <QDebug>

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
    qDebug() << "Great! Now stay still for 5 seconds. From 10 seconds on, you can move your mouse to get back here.";
    KIdleTime::instance()->addIdleTimeout(5000);
    KIdleTime::instance()->addIdleTimeout(10000);
}

void KIdleTest::timeoutReached(int timeout)
{
    if (timeout == 5000) {
        qDebug() << "5 seconds passed, stay still some more...";
    } else if (timeout == 10000) {
        KIdleTime::instance()->catchNextResumeEvent();
        qDebug() << "Cool. You can move your mouse to start over";
    }
}
