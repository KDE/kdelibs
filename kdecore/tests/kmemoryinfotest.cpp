/*
   Copyright 2010 Pino Toscano <pino@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kmemoryinfo.h>

#include <qtest_kde.h>

#include <QtCore/QDateTime>
#include <QtCore/QObject>

class KMemoryInfoTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testNull();
    void testUpdate();
    void testDetails();

    void benchmark();
};


void KMemoryInfoTest::testNull()
{
    KMemoryInfo mi;
    QVERIFY(mi.lastUpdate().isNull());
    QCOMPARE(mi.requestedDetails(), KMemoryInfo::MemoryDetails(0));
    QCOMPARE(mi.detail(KMemoryInfo::TotalRam), qint64(-1));
    QCOMPARE(mi.detail(KMemoryInfo::FreeRam), qint64(-1));
    QCOMPARE(mi.detail(KMemoryInfo::SharedRam), qint64(-1));
    QCOMPARE(mi.detail(KMemoryInfo::BufferRam), qint64(-1));
    QCOMPARE(mi.detail(KMemoryInfo::TotalSwap), qint64(-1));
    QCOMPARE(mi.detail(KMemoryInfo::FreeSwap), qint64(-1));
}

void KMemoryInfoTest::testUpdate()
{
    KMemoryInfo mi;
    QDateTime date = QDateTime::currentDateTime();
    QVERIFY(mi.update(KMemoryInfo::TotalRam));
    QVERIFY(mi.lastUpdate() >= date);
    QVERIFY(mi.lastUpdate() <= QDateTime::currentDateTime());
}

void KMemoryInfoTest::testDetails()
{
    KMemoryInfo mi;

    QVERIFY(mi.update(KMemoryInfo::TotalRam | KMemoryInfo::FreeRam));
    QCOMPARE(mi.requestedDetails(), KMemoryInfo::TotalRam | KMemoryInfo::FreeRam);
    QVERIFY(mi.detail(KMemoryInfo::TotalRam) != -1);
    QVERIFY(mi.detail(KMemoryInfo::FreeRam) != -1);
    QVERIFY(mi.detail(KMemoryInfo::SharedRam) == -1);
    QVERIFY(mi.detail(KMemoryInfo::BufferRam) == -1);
    QVERIFY(mi.detail(KMemoryInfo::TotalSwap) == -1);
    QVERIFY(mi.detail(KMemoryInfo::FreeSwap) == -1);

    QVERIFY(mi.update(KMemoryInfo::TotalSwap | KMemoryInfo::FreeSwap));
    QCOMPARE(mi.requestedDetails(), KMemoryInfo::TotalSwap | KMemoryInfo::FreeSwap);
    QVERIFY(mi.detail(KMemoryInfo::TotalRam) == -1);
    QVERIFY(mi.detail(KMemoryInfo::FreeRam) == -1);
    QVERIFY(mi.detail(KMemoryInfo::SharedRam) == -1);
    QVERIFY(mi.detail(KMemoryInfo::BufferRam) == -1);
    QVERIFY(mi.detail(KMemoryInfo::TotalSwap) != -1);
    QVERIFY(mi.detail(KMemoryInfo::FreeSwap) != -1);

}

void KMemoryInfoTest::benchmark()
{
    KMemoryInfo mi;
    const KMemoryInfo::MemoryDetails details =
        KMemoryInfo::TotalRam | KMemoryInfo::FreeRam | KMemoryInfo::SharedRam | KMemoryInfo::BufferRam
        | KMemoryInfo::TotalSwap | KMemoryInfo::FreeSwap;

    QBENCHMARK {
        mi.update(details);
    }
}

QTEST_KDEMAIN_CORE(KMemoryInfoTest)

#include "kmemoryinfotest.moc"
