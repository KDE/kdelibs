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

#include "../streameventqueue_p.h"

#include <qtest_kde.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>
#include <QtCore/QTimer>

using namespace Phonon;

class BackendThread : public QThread, protected LockFreeQueueBase::DataReadyHandler
{
    public:
        BackendThread(StreamEventQueue *q)
            : m_eventQueue(q),
            m_gotStreamSize(false), m_gotSeekable(false),
            m_inSeek(0), m_inReset(0), m_noEventSent(true),
            m_receivedWrites(0),
            m_needDataSent(0),
            m_resetsSent(0),
            m_seeksSent(0)
        {
            moveToThread(this);
        }

    protected:
        void run()
        {
            m_eventQueue->setBackendCommandHandler(this);
            dataReady();
            exec();
        }

        void dataReady()
        {
            if (m_noEventSent) {
                m_noEventSent = false;
                QCoreApplication::postEvent(this, new QEvent(QEvent::User));
            }
        }

        void customEvent(QEvent *e)
        {
            if (e->type() == QEvent::User) {
                m_noEventSent = true;
                StreamEventQueue::Command c;
                while (m_eventQueue->nextCommandForBackend(&c)) {
                    switch (c.command) {
                    case StreamEventQueue::SetStreamSize:
                        QCOMPARE(c.data.type(), QVariant::Int);
                        if (c.data.toInt() != 0) {
                            m_gotStreamSize = true;
                            if (m_gotSeekable) {
                                m_timer.start(0, this);
                            }
                        }
                        break;
                    case StreamEventQueue::SetSeekable:
                        QCOMPARE(c.data.type(), QVariant::Bool);
                        m_gotSeekable = true;
                        if (m_gotStreamSize) {
                            m_timer.start(0, this);
                        }
                        break;
                    case StreamEventQueue::Write:
                        QCOMPARE(m_inSeek, 0);
                        QCOMPARE(m_inReset, 0);
                        m_eventQueue->sendToFrontend(StreamEventQueue::EnoughData);
                        ++m_receivedWrites;
                        if (0 == m_receivedWrites % 10000) {
                            qDebug() << "BackendThread  received" << m_receivedWrites << "Write commands.";
                        }
                        break;
                    case StreamEventQueue::EndOfData:
                        QCOMPARE(m_inSeek, 0);
                        QCOMPARE(m_inReset, 0);
                        break;
                    case StreamEventQueue::SeekDone:
                        --m_inSeek;
                        break;
                    case StreamEventQueue::ResetDone:
                        --m_inReset;
                        break;
                    default:
                        qFatal("invalid command %d sent to backend thread", c.command);
                    }
                }
            }
        }

        void timerEvent(QTimerEvent *e)
        {
            if (e->timerId() == m_timer.timerId()) {
                if (0 == (qrand() & 3)) {
                    m_eventQueue->sendToFrontend(StreamEventQueue::NeedData, 1 << 12);
                    ++m_needDataSent;
                    if (0 == m_needDataSent % 10000) {
                        qDebug() << "BackendThread  sent" << m_needDataSent << "NeedData commands.";
                    }
                }
                if (!m_inSeek && 0 == (qrand() & 0x7f)) {
                    ++m_inSeek;
                    m_eventQueue->sendToFrontend(StreamEventQueue::Seek, 0);
                    ++m_seeksSent;
                    if (0 == m_seeksSent % 1000) {
                        qDebug() << "BackendThread  sent" << m_seeksSent << "seek commands.";
                    }
                }
                if (!m_inReset && 0 == (qrand() & 0x3ff)) {
                    ++m_inReset;
                    m_eventQueue->sendToFrontend(StreamEventQueue::Reset);
                    ++m_resetsSent;
                    if (0 == m_resetsSent % 100) {
                        qDebug() << "BackendThread  sent" << m_resetsSent << "reset commands.";
                    }
                }
            }
        }

    private:
        QBasicTimer m_timer;
        StreamEventQueue *m_eventQueue;
        bool m_gotStreamSize;
        bool m_gotSeekable;
        int m_inSeek;
        int m_inReset;
        volatile bool m_noEventSent;

        int m_receivedWrites;
        int m_needDataSent;
        int m_resetsSent;
        int m_seeksSent;
};

class FrontendThread : public QThread, protected LockFreeQueueBase::DataReadyHandler
{
    public:
        FrontendThread(StreamEventQueue *q)
            : m_eventQueue(q), m_inSeek(0), m_inReset(0), m_noEventSent(true),
            m_enoughDataReceived(0),
            m_writesSent(0),
            m_seeksDone(0),
            m_resetsDone(0)
        {
            moveToThread(this);
            m_eventQueue->sendToBackend(StreamEventQueue::SetStreamSize, int(1 << 30));
            m_eventQueue->sendToBackend(StreamEventQueue::SetSeekable, true);
        }

    protected:
        void run()
        {
            m_eventQueue->setFrontendCommandHandler(this);
            dataReady();
            exec();
        }

        void dataReady()
        {
            if (m_noEventSent) {
                m_noEventSent = false;
                QCoreApplication::postEvent(this, new QEvent(QEvent::User));
            }
        }

        void customEvent(QEvent *e)
        {
            if (e->type() == QEvent::User) {
                m_noEventSent = true;
                StreamEventQueue::Command c;
                while (m_eventQueue->nextCommandForFrontend(&c)) {
                    switch (c.command) {
                    case StreamEventQueue::NeedData:
                        QCOMPARE(c.data.type(), QVariant::Int);
                        m_requestedSize = c.data.toInt();
                        QVERIFY(m_requestedSize > 0);
                        m_timer.start(0, this);
                        break;
                    case StreamEventQueue::EnoughData:
                        m_requestedSize = 0;
                        ++m_enoughDataReceived;
                        if (0 == m_enoughDataReceived % 10000) {
                            qDebug() << "FrontendThread received" << m_enoughDataReceived << "EnoughData commands.";
                        }
                        break;
                    case StreamEventQueue::Seek:
                        ++m_inSeek;
                        break;
                    case StreamEventQueue::Reset:
                        ++m_inReset;
                        break;
                    default:
                        qFatal("invalid command %d sent to frontend thread", c.command);
                    }
                }
            }
        }

        void timerEvent(QTimerEvent *e)
        {
            if (e->timerId() == m_timer.timerId()) {
                if (m_inSeek && (qrand() & 1)) {
                    --m_inSeek;
                    m_eventQueue->sendToBackend(StreamEventQueue::SeekDone);
                    ++m_seeksDone;
                    if (0 == m_seeksDone % 1000) {
                        qDebug() << "FrontendThread answered" << m_seeksDone << "Seek commands.";
                    }
                } else if (m_inReset && (qrand() & 1)) {
                    --m_inReset;
                    m_eventQueue->sendToBackend(StreamEventQueue::ResetDone);
                    ++m_resetsDone;
                    if (0 == m_resetsDone % 100) {
                        qDebug() << "FrontendThread answered" << m_resetsDone << "Reset commands.";
                    }
                }
                // misbehaving frontend: we sometimes send writes even when in seek/reset
                if (m_requestedSize > 0 && (qrand() & 1)) {
                    m_eventQueue->sendToBackend(StreamEventQueue::Write, QByteArray(m_requestedSize, '\0'));
                    ++m_writesSent;
                    if (0 == m_writesSent % 10000) {
                        qDebug() << "FrontendThread sent" << m_writesSent << "Write commands.";
                    }
                }
            }
        }

    private:
        QBasicTimer m_timer;
        StreamEventQueue *m_eventQueue;
        int m_requestedSize;
        int m_inSeek;
        int m_inReset;
        volatile bool m_noEventSent;

        int m_enoughDataReceived;
        int m_writesSent;
        int m_seeksDone;
        int m_resetsDone;
};

class StreamEventQueueTest : public QObject
{
    Q_OBJECT
    private slots:
        void initTestCase();
        void wait();
        void cleanupTestCase();

    private:
        StreamEventQueue *m_eventQueue;
        BackendThread *m_backendThread;
        FrontendThread *m_frontendThread;
};

void StreamEventQueueTest::initTestCase()
{
    m_eventQueue = new StreamEventQueue;
    m_backendThread = new BackendThread(m_eventQueue);
    m_frontendThread = new FrontendThread(m_eventQueue);
    m_backendThread->start();
    m_frontendThread->start();
}

void StreamEventQueueTest::wait()
{
    qDebug() << "wait ten minutes";
    QTest::qWait(600000);
}

void StreamEventQueueTest::cleanupTestCase()
{
    m_backendThread->quit();
    m_frontendThread->quit();
    m_backendThread->wait();
    m_frontendThread->wait();
    delete m_backendThread;
    delete m_frontendThread;
    delete m_eventQueue;
}

QTEST_MAIN(StreamEventQueueTest)

#include "streameventqueuetest.moc"
