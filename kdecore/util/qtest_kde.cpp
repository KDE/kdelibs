/* This file is part of the KDE libraries
   Copyright (C) 2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "qtest_kde.h"

// A signal spy which exits the event loop when the signal is called,
// and remembers that the signal was emitted.
class KDESignalSpy : public QObject
{
    Q_OBJECT
public:
    KDESignalSpy(QObject *obj, const char *signal, int timeout)
        : QObject(0), m_obj(obj), m_emitted(false)
    {
        connect(obj, signal, this, SLOT(slotSignalEmitted()));
        if (timeout > 0) {
            QObject::connect(&m_timer, SIGNAL(timeout()), &m_loop, SLOT(quit()));
            m_timer.setSingleShot(true);
            m_timer.start(timeout);
        }
        m_loop.exec();
    }
    bool signalEmitted() const { return m_emitted; }

private Q_SLOTS:
    void slotSignalEmitted()
    {
        m_emitted = true;
        disconnect(m_obj, 0, this, 0);
        m_timer.stop();
        m_loop.quit();
    }
private:
    QObject* m_obj;
    bool m_emitted;
    QEventLoop m_loop;
    QTimer m_timer;
};

// Unit test for this code: tests/kglobaltest.cpp

bool QTest::kWaitForSignal(QObject *obj, const char *signal, int timeout )
{
    KDESignalSpy spy(obj, signal, timeout);
    return spy.signalEmitted();
}

#include "qtest_kde.moc"
