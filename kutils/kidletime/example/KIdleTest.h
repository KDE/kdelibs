/*
 * KIdleTest.h
 *
 *  Created on: 05/lug/2009
 *      Author: drf
 */

#ifndef KIDLETEST_H_
#define KIDLETEST_H_

#include <QObject>

class KIdleTest : public QObject
{
    Q_OBJECT

    public:
        KIdleTest();
        virtual ~KIdleTest();

    public slots:
        void timeoutReached(int timeout);
        void resumeEvent();
};

#endif /* KIDLETEST_H_ */
