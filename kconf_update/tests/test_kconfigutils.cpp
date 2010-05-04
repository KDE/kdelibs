/* This file is part of the KDE libraries
   Copyright 2010 Canonical Ltd
   Author: Aurélien Gâteau <aurelien.gateau@canonical.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "test_kconfigutils.h"

// Qt

// KDE

#include <qtest_kde.h>

// Local
#include "../kconfigutils.h"

QTEST_KDEMAIN_CORE(TestKConfigUtils)

void TestKConfigUtils::testParseGroupString_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<bool>("expectedOk");

    QTest::newRow("simple-group")
        << " group  "
        << (QStringList() << "group")
        << true
        ;

    QTest::newRow("sub-group")
        << "[group][sub]"
        << (QStringList() << "group" << "sub")
        << true
        ;

    QTest::newRow("crazy-sub-group")
        << "[a\\ttab\\x5d[and some hex esc\\x61pe]"
        << (QStringList() << "a\ttab" << "and some hex escape")
        << true
        ;

    QTest::newRow("missing-closing-brace")
        << "[group][sub"
        << QStringList()
        << false
        ;
}

void TestKConfigUtils::testParseGroupString()
{
    QFETCH(QString, input);
    QFETCH(QStringList, expected);
    QFETCH(bool, expectedOk);

    bool ok;
    QString error;
    QStringList output = KConfigUtils::parseGroupString(input, &ok, &error);
    QCOMPARE(output, expected);
    QCOMPARE(ok, expectedOk);
    if (ok) {
        QVERIFY(error.isEmpty());
    } else {
        QVERIFY(!error.isEmpty());
        qDebug() << error;
    }
}

void TestKConfigUtils::testUnescapeString_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<bool>("expectedOk");

    QTest::newRow("plain")
        << "Some text"
        << "Some text"
        << true
        ;

    QTest::newRow("single-char-escapes")
        << "01\\s23\\t45\\n67\\r89\\\\"
        << "01 23\t45\n67\r89\\"
        << true
        ;

    QTest::newRow("hex-escapes")
        << "kd\\x65"
        << "kde"
        << true
        ;

    QTest::newRow("unfinished-hex-escape")
        << "kd\\x6"
        << ""
        << false
        ;

    QTest::newRow("invalid-hex-escape")
        << "kd\\xzz"
        << ""
        << false
        ;

    QTest::newRow("invalid-escape-sequence")
        << "Foo\\a"
        << ""
        << false
        ;

    QTest::newRow("unfinished-escape-sequence")
        << "Foo\\"
        << ""
        << false
        ;
}

void TestKConfigUtils::testUnescapeString()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);
    QFETCH(bool, expectedOk);

    bool ok;
    QString error;
    QString output = KConfigUtils::unescapeString(input, &ok, &error);
    QCOMPARE(output, expected);
    QCOMPARE(ok, expectedOk);
    if (ok) {
        QVERIFY(error.isEmpty());
    } else {
        QVERIFY(!error.isEmpty());
        qDebug() << error;
    }
}

#include "test_kconfigutils.moc"
