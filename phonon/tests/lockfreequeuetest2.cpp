/*  This file is part of the KDE project
    Copyright (C) 2008 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "../lockfreequeue_p.h"

#include <QtTest/QTest>

#include <QtCore/QCoreApplication>
#include <QtCore/QtDebug>
#include <QtCore/QSet>
#include <QtCore/QVariant>
#include <QtCore/QThread>
#include <QtCore/QTimer>

//static int MAX_ENQUEUE_TIME = 0;
//static int MAX_DEQUEUE_TIME = 0;

class ProcessThread : public QThread
{
    public:
        ProcessThread(LockFreeQueue<int> *q1, LockFreeQueue<int> *q2, int waitAfter)
            : m_sendQueue(q2), m_receiveQueue(q1),
            m_keeprunning(true),
            m_waitAfter(waitAfter)
        {
        }

        void quit()
        {
            m_keeprunning = false;
        }

    protected:
        void run()
        {
            int id;
            bool gotOne;
            //QTime timer;
            while (m_keeprunning) {
                //timer.start();
                gotOne = m_receiveQueue->dequeue(&id, LockFreeQueue<int>::NeverBlock);
                //MAX_DEQUEUE_TIME = qMax(timer.elapsed(), MAX_DEQUEUE_TIME);
                if (gotOne) {
                    //timer.start();
                    m_sendQueue->enqueue(id);
                    //MAX_ENQUEUE_TIME = qMax(timer.elapsed(), MAX_ENQUEUE_TIME);
                } else {
                    QThread::msleep(100);
                }
            }
        }

    private:
        LockFreeQueue<int> *m_sendQueue;
        LockFreeQueue<int> *m_receiveQueue;
        volatile bool m_keeprunning;
        int m_waitAfter;
};

class StreamEventQueueTest : public QObject
{
    Q_OBJECT
    private slots:
        void initTestCase();
        void sendData();
        void receiveData();
        void cleanupTestCase();

    private:
        LockFreeQueue<int> *m_toProcess;
        LockFreeQueue<int> *m_doneProcessing;
        QList<ProcessThread *> m_processors;
};

static const int NUMBER_OF_IDS = 0x400000;
static const int DEBUG_MASK = ((NUMBER_OF_IDS - 1) >> 4);
static const int NUMBER_OF_THREADS = 3;

void StreamEventQueueTest::initTestCase()
{
    m_toProcess = new LockFreeQueue<int>;
    m_doneProcessing = new LockFreeQueue<int>;
    for (int i = 0; i < NUMBER_OF_THREADS; ++i) {
        ProcessThread *t = new ProcessThread(m_toProcess, m_doneProcessing, i * 10);
        t->start();
        m_processors << t;
    }
    LockFreeQueueBase::KeepNodePoolMemoryManagement::setPoolSize(1000);
}

void StreamEventQueueTest::sendData()
{
    int i = 0;
    while (i < NUMBER_OF_IDS) {
        *m_toProcess << i++;
        if (0 == (i & DEBUG_MASK)) {
            qDebug() << "sending:" << i;
        }
    }
}

void StreamEventQueueTest::receiveData()
{
    uchar received[NUMBER_OF_IDS];
    bool empty = false;
    int count = 0;
    memset(&received[0], false, sizeof(uchar) * NUMBER_OF_IDS);
    QTime timeout;
    QVector<int> data;
    data.reserve(100);
    forever {
        if (!empty && m_toProcess->isEmpty()) {
            empty = true;
            qDebug() << "nothing more to process";
        }
        if (m_doneProcessing->isEmpty()) {
            if (timeout.isNull()) {
                timeout.start();
            } else {
                if (timeout.elapsed() >= 2000) {
                    if (count == NUMBER_OF_IDS) {
                        // SUCCESS
                        break;
                    }
                    qDebug() << count;
                }
                QVERIFY(timeout.elapsed() < 10000);
            }
        }
        while (!m_doneProcessing->isEmpty()) {
            timeout = QTime();
            m_doneProcessing->dequeue(data);
            foreach (int x, data) {
                QVERIFY(x >= 0);
                QVERIFY(x < NUMBER_OF_IDS);
                QCOMPARE(received[x], (uchar)false);
                received[x] = true;
                ++count;
                QVERIFY(count <= NUMBER_OF_IDS);
                if (0 == (count & DEBUG_MASK)) {
                    qDebug() << "received:" << count;
                }
            }
        }
        QTest::qWait(200);
    }
}

void StreamEventQueueTest::cleanupTestCase()
{
    //qDebug() << "maximal enqueue time:" << MAX_ENQUEUE_TIME;
    //qDebug() << "maximal dequeue time:" << MAX_DEQUEUE_TIME;
    LockFreeQueueBase::KeepNodePoolMemoryManagement::clear();
    foreach (ProcessThread *p, m_processors) {
        p->quit();
    }
    foreach (ProcessThread *p, m_processors) {
        p->wait();
        delete p;
    }
    delete m_toProcess;
    delete m_doneProcessing;
}

QTEST_MAIN(StreamEventQueueTest)

#include "lockfreequeuetest2.moc"
