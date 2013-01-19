/* This file is part of the KDE libraries

    Copyright 2012 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kxmessages.h>
#include <QSignalSpy>
#include <qtest_widgets.h>

class KXMessages_UnitTest : public QObject
{
    Q_OBJECT
public:
    KXMessages_UnitTest()
        : m_msgs() {
    }

private Q_SLOTS:
    void testStart();

private:
    KXMessages m_msgs;
};

void KXMessages_UnitTest::testStart()
{
    const QByteArray type = "kxmessage_unittest";
    KXMessages receiver(type);

    // Check that all message sizes work, i.e. no bug when exactly 20 or 40 bytes,
    // despite the internal splitting.
    QString message;
    for (int i = 1 ; i < 50 ; ++i) {
        QSignalSpy spy(&receiver, SIGNAL(gotMessage(QString)));
        message += "a";
        m_msgs.broadcastMessage(type, message);

        QVERIFY(spy.wait());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toString(), message);
    }
}

QTEST_MAIN(KXMessages_UnitTest)

#include "kxmessages_unittest.moc"
