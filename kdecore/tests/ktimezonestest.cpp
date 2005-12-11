/* This file is part of the KDE libraries
    Copyright (c) 2005 David Jarvie <software@astrojar.org.uk>

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

#include <stdio.h>
#include <qttest_kde.h>
#include <QDateTime>
#include "ktimezones.h"
#include "ktzfiletimezone.h"
#include "ktimezonestest.moc"


QTTEST_KDEMAIN(KTimezonesTest, NoGUI)


///////////////////
// KTimezones tests
///////////////////

void KTimezonesTest::ktimezones()
{
    KTimezones timezones;
    KTimezone *zone1 = new KTimezone("Zone1");
    KTimezone *zone2 = new KTimezone("Zone2");
    QVERIFY(timezones.add(zone1));
    QVERIFY(!timezones.add(zone1));
    QVERIFY(timezones.add(zone2));
    QCOMPARE(timezones.zones().count(), 2);
    const KTimezone* tz = timezones.zone("Zone1");
    QVERIFY((tz == zone1));
    tz = timezones.zone("Zone99");
    QVERIFY(!tz);
    zone1 = timezones.detach(zone1);
    QVERIFY((bool)zone1);
    QCOMPARE(timezones.zones().count(), 1);
    QVERIFY(!timezones.detach(zone1));
    QVERIFY(timezones.add(zone1));
    QVERIFY((bool)timezones.detach("Zone1"));
    QVERIFY(!timezones.detach("Zone1"));
    QVERIFY((bool)timezones.detach("Zone2"));
    zone1 = new KTimezone("Zone10");
    delete zone1;
}

///////////////////
// KTimezones: UTC
///////////////////

Q_DECLARE_METATYPE(QDateTime)

void KTimezonesTest::utc_data()
{
    QTest::addColumn<QDateTime>("test");
//    QTest::addColumn<qint>("result");
//    QTest::newRow("localtime") << QDateTime(QDate(2005,1,1), QTime(), Qt::LocalTime) << 0;
//    QTest::newRow("winter")    << QDateTime(QDate(2005,1,1), QTime(), Qt::UTC) << 0;
//    QTest::newRow("summer")    << QDateTime(QDate(2005,7,1), QTime(), Qt::UTC) << 0;
}

void KTimezonesTest::utc()
{
    KTimezone* utc = KTimezones::utc();
    QVERIFY((bool)utc);
    if (utc)
      QCOMPARE(utc->name(), QString("UTC"));
    QCOMPARE(utc->offsetAtUTC(QDateTime(QDate(2005,1,1), QTime(), Qt::LocalTime)), 0);
    QCOMPARE(utc->offsetAtUTC(QDateTime(QDate(2005,1,1), QTime(), Qt::UTC)), 0);
    QCOMPARE(utc->offsetAtUTC(QDateTime(QDate(2005,7,1), QTime(), Qt::UTC)), 0);

//    QFETCH(QDateTime, test);
//    QFETCH(int, result);
//    QCOMPARE(utc->offsetAtUTC(test), result);
}

/////////////////////////
// KSystemTimezones tests
/////////////////////////

void KTimezonesTest::local()
{
    const KTimezone *local = KSystemTimezones::local();
    QVERIFY((bool)local);
    QString msg = QString("Please manually verify that the local time zone is \"%1\"").arg(local->name());
    QWARN(msg.toLatin1().data());
}

void KTimezonesTest::zone()
{
    const KTimezone *losAngeles = KSystemTimezones::zone("America/Los_Angeles");
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    QVERIFY((bool)losAngeles);
    QVERIFY((bool)london);
}

void KTimezonesTest::zoneinfoDir()
{
    QString zoneinfo = KSystemTimezones::zoneinfoDir();
    QVERIFY(!zoneinfo.isEmpty());
    QString msg = QString("Please manually verify that the zoneinfo directory is \"%1\"").arg(zoneinfo);
    QWARN(msg.toLatin1().data());
}


    ////////////////////////
    // KSystemTimezone tests
    ////////////////////////

void KTimezonesTest::currentOffset()
{
    // Find the current offset of a time zone
    const KTimezone *local = KSystemTimezones::local();
    QVERIFY((bool)local);
    int offset = local->currentOffset(Qt::UTC);
    QString msg = QString("Please manually verify that the current local time zone offset is %1h%2m%3s").arg(offset/3600).arg((offset/60)%60, 2, 10, QChar('0')).arg(offset%3600, 2, 10, QChar('0'));
    QWARN(msg.toLatin1().data());
}

void KTimezonesTest::offsetAtUTC()
{
    // Find some offsets for Europe/London.
    const KTimezone *losAngeles = KSystemTimezones::zone("America/Los_Angeles");
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QCOMPARE(london->offsetAtUTC(winter), 0);
    QCOMPARE(london->offsetAtUTC(summer), 3600);;
    QCOMPARE(losAngeles->offsetAtUTC(winter), -28800);
    QCOMPARE(losAngeles->offsetAtUTC(summer), -25200);;
}

void KTimezonesTest::offsetAtZoneTime()
{
    QDateTime aGmt(QDate(2005,3,27), QTime(0,30,0), Qt::LocalTime);
    QDateTime aInvalid(QDate(2005,3,27), QTime(1,30,0), Qt::LocalTime);
    QDateTime aBst(QDate(2005,3,27), QTime(2,30,0), Qt::LocalTime);
    QDateTime bBst(QDate(2005,10,30), QTime(0,30,0), Qt::LocalTime);
    QDateTime bBstBeforeGmt(QDate(2005,10,30), QTime(1,30,0), Qt::LocalTime);
    QDateTime bGmt(QDate(2005,10,30), QTime(2,30,0), Qt::LocalTime);
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    int offset2;
    QCOMPARE(london->offsetAtZoneTime(aGmt, &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(london->offsetAtZoneTime(aInvalid, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london->offsetAtZoneTime(aBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london->offsetAtZoneTime(bBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london->offsetAtZoneTime(bBstBeforeGmt, &offset2), 3600);
    QCOMPARE(offset2, 0);
    QCOMPARE(london->offsetAtZoneTime(bGmt, &offset2), 0);
    QCOMPARE(offset2, 0);
}

void KTimezonesTest::abbreviation()
{
    // Fetch time zone abbreviations
    const KTimezone *losAngeles = KSystemTimezones::zone("America/Los_Angeles");
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QString sResult = london->abbreviation(winter);
    QCOMPARE(london->abbreviation(winter), QByteArray("GMT"));
    QCOMPARE(london->abbreviation(summer), QByteArray("BST"));
    QCOMPARE(losAngeles->abbreviation(winter), QByteArray("PST"));
    QCOMPARE(losAngeles->abbreviation(summer), QByteArray("PDT"));
}

void KTimezonesTest::toUTC()
{
    // Convert to UTC.
    const KTimezone *losAngeles = KSystemTimezones::zone("America/Los_Angeles");
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QDateTime winterLocal = winter;
    winterLocal.setTimeSpec(Qt::LocalTime);
    QDateTime summerLocal = summer;
    summerLocal.setTimeSpec(Qt::LocalTime);
    QCOMPARE(london->toUTC(winterLocal), winter);
    QCOMPARE(london->toUTC(summerLocal), summer.addSecs(-3600));;
    QCOMPARE(losAngeles->toUTC(winterLocal), winter.addSecs(8*3600));
    QCOMPARE(losAngeles->toUTC(summerLocal), summer.addSecs(7*3600));
}

void KTimezonesTest::toZoneTime()
{
    // Convert from UTC.
    const KTimezone *losAngeles = KSystemTimezones::zone("America/Los_Angeles");
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QDateTime winterLocal = winter;
    winterLocal.setTimeSpec(Qt::LocalTime);
    QDateTime summerLocal = summer;
    summerLocal.setTimeSpec(Qt::LocalTime);
    QCOMPARE(london->toZoneTime(winter), winterLocal);
    QCOMPARE(london->toZoneTime(summer), summerLocal.addSecs(3600));
    QCOMPARE(losAngeles->toZoneTime(winter), winterLocal.addSecs(-8*3600));
    QCOMPARE(losAngeles->toZoneTime(summer), summerLocal.addSecs(-7*3600));
}

void KTimezonesTest::convert()
{
    // Try time zone conversions.
    const KTimezone *losAngeles = KSystemTimezones::zone("America/Los_Angeles");
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    QDateTime bstBeforePdt(QDate(2005,3,28), QTime(0,0,0), Qt::LocalTime);
    QDateTime bstAfterPdt(QDate(2005,5,1), QTime(0,0,0), Qt::LocalTime);
    QDateTime gmtBeforePst(QDate(2005,10,30), QTime(4,0,0), Qt::LocalTime);
    QDateTime gmtAfterPst(QDate(2005,12,1), QTime(0,0,0), Qt::LocalTime);
    QDateTime bstBeforePdtResult(QDate(2005,3,27), QTime(15,0,0), Qt::LocalTime);
    QDateTime bstAfterPdtResult(QDate(2005,4,30), QTime(16,0,0), Qt::LocalTime);
    QDateTime gmtBeforePstResult(QDate(2005,10,29), QTime(21,0,0), Qt::LocalTime);
    QDateTime gmtAfterPstResult(QDate(2005,11,30), QTime(16,0,0), Qt::LocalTime);

    QCOMPARE(london->convert(losAngeles, bstBeforePdt), bstBeforePdtResult);
    QCOMPARE(london->convert(losAngeles, bstAfterPdt), bstAfterPdtResult);
    QCOMPARE(london->convert(losAngeles, gmtBeforePst), gmtBeforePstResult);
    QCOMPARE(london->convert(losAngeles, gmtAfterPst), gmtAfterPstResult);
    QCOMPARE(losAngeles->convert(losAngeles, bstBeforePdtResult), bstBeforePdtResult);
}

////////////////////////
// KTzfileTimezone tests
////////////////////////

void KTimezonesTest::tzfile()
{
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QString zoneinfo = KSystemTimezones::zoneinfoDir();
    QVERIFY(!zoneinfo.isEmpty());
    KTzfileTimezoneSource tzsource(zoneinfo);
    KTimezone *tzcairo = new KTzfileTimezone(&tzsource, "Africa/Cairo");
    delete tzcairo;
    tzcairo = new KTzfileTimezone(&tzsource, "Africa/Cairo");
    QCOMPARE(tzcairo->offsetAtUTC(winter), 7200);
    delete tzcairo;
}

void KTimezonesTest::tzfileOffsetAtZoneTime()
{
    QDateTime aGmt(QDate(2005,3,27), QTime(0,30,0), Qt::LocalTime);
    QDateTime aInvalid(QDate(2005,3,27), QTime(1,30,0), Qt::LocalTime);
    QDateTime aBst(QDate(2005,3,27), QTime(2,30,0), Qt::LocalTime);
    QDateTime bBst(QDate(2005,10,30), QTime(0,30,0), Qt::LocalTime);
    QDateTime bBstBeforeGmt(QDate(2005,10,30), QTime(1,30,0), Qt::LocalTime);
    QDateTime bGmt(QDate(2005,10,30), QTime(2,30,0), Qt::LocalTime);
    KTzfileTimezoneSource tzsource(KSystemTimezones::zoneinfoDir());
    KTimezone *london = new KTzfileTimezone(&tzsource, "Europe/London");
    int offset2;
    QCOMPARE(london->offsetAtZoneTime(aGmt, &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(london->offsetAtZoneTime(aInvalid, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london->offsetAtZoneTime(aBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london->offsetAtZoneTime(bBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london->offsetAtZoneTime(bBstBeforeGmt, &offset2), 3600);
    QCOMPARE(offset2, 0);
    QCOMPARE(london->offsetAtZoneTime(bGmt, &offset2), 0);
    QCOMPARE(offset2, 0);
    delete london;
}

void KTimezonesTest::tzfileUTCOffsets()
{
    KTzfileTimezoneSource tzsource(KSystemTimezones::zoneinfoDir());
    KTimezone *london = new KTzfileTimezone(&tzsource, "Europe/London");
    QList<int> offsets = london->UTCOffsets();
    QCOMPARE(offsets.count(), 3);
    QCOMPARE(offsets[0], 0);    // GMT
    QCOMPARE(offsets[1], 3600); // BST
    QCOMPARE(offsets[2], 7200); // DST
}

#if 0
  if ((argc==2) && (strcmp(argv[1], "list")==0))
  {

    /////////////////////////
    // KSystemTimezones tests
    /////////////////////////
    KTimezones::ZoneMap allZones = KSystemTimezones::zones();
    for ( KTimezones::ZoneMap::const_iterator it = allZones.begin(), end = allZones.end(); it != end; ++it )
      printf( "%s\n", it.key().latin1() );
    return 0;
  }

  printf( "Usage: ktimezonestest [local|list]!\n" );
  return 1;
}
#endif
