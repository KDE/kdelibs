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
#include <QtCore/QVariant>
#include <QtCore/QVector>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QtDebug>

enum CommandType {
    SetStreamSize,
    SetSeekable,
    Write,
    EndOfData,
    SeekDone,
    ResetDone,
    NeedData,
    EnoughData,
    Seek,
    Reset
};

struct Command
{
    inline Command(CommandType c, const QVariant &d = QVariant()) : data(d), command(c) {}
    inline Command() {}
    QVariant data;
    CommandType command;
};

class Thread1 : public QThread, protected LockFreeQueue<Command>::DataReadyHandler
{
    public:
        Thread1(LockFreeQueue<Command> *q1, LockFreeQueue<Command> *q2)
            : m_sendQueue(q1), m_receiveQueue(q2),
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
            m_receiveQueue->setDataReadyHandler(this);
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
                QVector<Command> commands(0xff);
                m_receiveQueue->dequeue(commands);
                foreach (const Command &c, commands) {
                    switch (c.command) {
                    case SetStreamSize:
                        QCOMPARE(c.data.type(), QVariant::Int);
                        if (c.data.toInt() != 0) {
                            m_gotStreamSize = true;
                            if (m_gotSeekable) {
                                m_timer.start(0, this);
                            }
                        }
                        break;
                    case SetSeekable:
                        QCOMPARE(c.data.type(), QVariant::Bool);
                        m_gotSeekable = true;
                        if (m_gotStreamSize) {
                            m_timer.start(0, this);
                        }
                        break;
                    case Write:
                        if (!m_inSeek && !m_inReset) {
                            m_sendQueue->enqueue(EnoughData);
                            ++m_receivedWrites;
                            if (0 == m_receivedWrites % 10000) {
                                qDebug() << "Thread1 received" << m_receivedWrites << "Write commands.";
                            }
                        }
                        break;
                    case EndOfData:
                        QCOMPARE(m_inSeek, 0);
                        QCOMPARE(m_inReset, 0);
                        break;
                    case SeekDone:
                        --m_inSeek;
                        break;
                    case ResetDone:
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
                    m_sendQueue->enqueue(Command(NeedData, 1 << 12));
                    ++m_needDataSent;
                    if (0 == m_needDataSent % 10000) {
                        qDebug() << "Thread1 sent" << m_needDataSent << "NeedData commands.";
                    }
                }
                if (!m_inSeek && 0 == (qrand() & 0x7f)) {
                    ++m_inSeek;
                    m_sendQueue->enqueue(Command(Seek, 0));
                    ++m_seeksSent;
                    if (0 == m_seeksSent % 1000) {
                        qDebug() << "Thread1 sent" << m_seeksSent << "seek commands.";
                    }
                }
                if (!m_inReset && 0 == (qrand() & 0x3ff)) {
                    ++m_inReset;
                    m_sendQueue->enqueue(Command(Reset));
                    ++m_resetsSent;
                    if (0 == m_resetsSent % 100) {
                        qDebug() << "Thread1 sent" << m_resetsSent << "reset commands.";
                    }
                }
            }
        }

    private:
        QBasicTimer m_timer;
        LockFreeQueue<Command> *m_sendQueue;
        LockFreeQueue<Command> *m_receiveQueue;
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

class Thread2 : public QThread, protected LockFreeQueue<Command>::DataReadyHandler
{
    public:
        Thread2(LockFreeQueue<Command> *q1, LockFreeQueue<Command> *q2)
            : m_sendQueue(q1), m_receiveQueue(q2),
            m_inSeek(0), m_inReset(0), m_noEventSent(true),
            m_enoughDataReceived(0),
            m_writesSent(0),
            m_seeksDone(0),
            m_resetsDone(0)
        {
            moveToThread(this);
            m_sendQueue->enqueue(Command(SetStreamSize, int(1 << 30)));
            m_sendQueue->enqueue(Command(SetSeekable, true));
        }

    protected:
        void run()
        {
            m_receiveQueue->setDataReadyHandler(this);
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
                QVector<Command> commands(0xff);
                m_receiveQueue->dequeue(commands);
                foreach (const Command &c, commands) {
                    switch (c.command) {
                    case NeedData:
                        QCOMPARE(c.data.type(), QVariant::Int);
                        m_requestedSize = c.data.toInt();
                        QVERIFY(m_requestedSize > 0);
                        m_timer.start(0, this);
                        break;
                    case EnoughData:
                        m_requestedSize = 0;
                        ++m_enoughDataReceived;
                        if (0 == m_enoughDataReceived % 10000) {
                            qDebug() << "Thread2 received" << m_enoughDataReceived << "EnoughData commands.";
                        }
                        break;
                    case Seek:
                        ++m_inSeek;
                        break;
                    case Reset:
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
                    m_sendQueue->enqueue(SeekDone);
                    ++m_seeksDone;
                    if (0 == m_seeksDone % 1000) {
                        qDebug() << "Thread2 answered" << m_seeksDone << "Seek commands.";
                    }
                } else if (m_inReset && (qrand() & 1)) {
                    --m_inReset;
                    m_sendQueue->enqueue(ResetDone);
                    ++m_resetsDone;
                    if (0 == m_resetsDone % 100) {
                        qDebug() << "Thread2 answered" << m_resetsDone << "Reset commands.";
                    }
                }
                // misbehaving frontend: we sometimes send writes even when in seek/reset
                if (m_requestedSize > 0 && (qrand() & 1)) {
                    m_sendQueue->enqueue(Command(Write, QByteArray(m_requestedSize, '\0')));
                    ++m_writesSent;
                    if (0 == m_writesSent % 10000) {
                        qDebug() << "Thread2 sent" << m_writesSent << "Write commands.";
                    }
                }
            }
        }

    private:
        QBasicTimer m_timer;
        LockFreeQueue<Command> *m_sendQueue;
        LockFreeQueue<Command> *m_receiveQueue;
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
        LockFreeQueue<Command> *m_oneToTwo;
        LockFreeQueue<Command> *m_twoToOne;
        Thread1 *m_backendThread;
        Thread2 *m_frontendThread;
};

void StreamEventQueueTest::initTestCase()
{
    m_oneToTwo = new LockFreeQueue<Command>;
    m_twoToOne = new LockFreeQueue<Command>;
    m_backendThread = new Thread1(m_oneToTwo, m_twoToOne);
    m_frontendThread = new Thread2(m_twoToOne, m_oneToTwo);
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
    delete m_oneToTwo;
    delete m_twoToOne;
}

QTEST_MAIN(StreamEventQueueTest)

#include "streameventqueuetest.moc"
