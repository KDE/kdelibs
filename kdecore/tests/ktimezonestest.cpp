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
#include <qtest_kde.h>
#include <QDateTime>
#include "ktimezones.h"
#include "ktzfiletimezone.h"
#include "ktimezonestest.moc"


QTEST_KDEMAIN(KTimeZonesTest, NoGUI)


///////////////////
// KTimeZones tests
///////////////////

void KTimeZonesTest::ktimezones()
{
    KTimeZones timezones;
    KTimeZone *zone1 = new KTimeZone("Zone1");
    KTimeZone *zone2 = new KTimeZone("Zone2");
    QVERIFY(timezones.add(zone1));
    QVERIFY(!timezones.add(zone1));
    QVERIFY(timezones.add(zone2));
    QCOMPARE(timezones.zones().count(), 2);
    const KTimeZone* tz = timezones.zone("Zone1");
    QVERIFY((tz == zone1));
    tz = timezones.zone("Zone99");
    QVERIFY(!tz);
    zone1 = const_cast<KTimeZone*>(timezones.detach(zone1));
    QVERIFY((bool)zone1);
    QCOMPARE(timezones.zones().count(), 1);
    QVERIFY(!timezones.detach(zone1));
    QVERIFY(timezones.add(zone1));
    QVERIFY((bool)timezones.detach("Zone1"));
    QVERIFY(!timezones.detach("Zone1"));
    QVERIFY((bool)timezones.detach("Zone2"));
    zone1 = new KTimeZone("Zone10");
    delete zone1;
}

///////////////////
// KTimeZones: UTC
///////////////////

void KTimeZonesTest::utc()
{
    const KTimeZone* utc = KTimeZones::utc();
    QVERIFY((bool)utc);
    if (utc)
      QCOMPARE(utc->name(), QString("UTC"));
    QCOMPARE(utc->offsetAtUTC(QDateTime(QDate(2005,1,1), QTime(), Qt::LocalTime)), 0);
    QCOMPARE(utc->offsetAtUTC(QDateTime(QDate(2005,1,1), QTime(), Qt::UTC)), 0);
    QCOMPARE(utc->offsetAtUTC(QDateTime(QDate(2005,7,1), QTime(), Qt::UTC)), 0);
}

/////////////////////////
// KSystemTimeZones tests
/////////////////////////

void KTimeZonesTest::local()
{
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":Europe/Paris", 1);
    ::tzset();

    const KTimeZone *local = KSystemTimeZones::local();
    QVERIFY((bool)local);
    QCOMPARE(local->name(), QString::fromLatin1("Europe/Paris"));

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KTimeZonesTest::zone()
{
    const KTimeZone *losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    const KTimeZone *london = KSystemTimeZones::zone("Europe/London");
    QVERIFY((bool)losAngeles);
    QVERIFY((bool)london);
}

void KTimeZonesTest::zoneinfoDir()
{
    QString zoneinfo = KSystemTimeZones::zoneinfoDir();
    QVERIFY(!zoneinfo.isEmpty());
    QString msg = QString("Please manually verify that the zoneinfo directory is \"%1\"").arg(zoneinfo);
    QWARN(msg.toLatin1().data());
}


    ////////////////////////
    // KSystemTimeZone tests
    ////////////////////////

void KTimeZonesTest::currentOffset()
{
    // Find the current offset of a time zone
    time_t now = time(0);
    tm *tnow = localtime(&now);
    int offset = tnow->tm_gmtoff;
    const KTimeZone *local = KSystemTimeZones::local();
    QVERIFY((bool)local);
    QCOMPARE(local->currentOffset(Qt::UTC), offset);
}

void KTimeZonesTest::offsetAtUTC()
{
    // Find some offsets for Europe/London.
    const KTimeZone *losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    const KTimeZone *london = KSystemTimeZones::zone("Europe/London");
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QCOMPARE(london->offsetAtUTC(winter), 0);
    QCOMPARE(london->offsetAtUTC(summer), 3600);;
    QCOMPARE(losAngeles->offsetAtUTC(winter), -28800);
    QCOMPARE(losAngeles->offsetAtUTC(summer), -25200);;
}

void KTimeZonesTest::offsetAtZoneTime()
{
    QDateTime aGmt(QDate(2005,3,27), QTime(0,30,0), Qt::LocalTime);
    QDateTime aInvalid(QDate(2005,3,27), QTime(1,30,0), Qt::LocalTime);
    QDateTime aBst(QDate(2005,3,27), QTime(2,30,0), Qt::LocalTime);
    QDateTime bBst(QDate(2005,10,30), QTime(0,30,0), Qt::LocalTime);
    QDateTime bBstBeforeGmt(QDate(2005,10,30), QTime(1,30,0), Qt::LocalTime);
    QDateTime bGmt(QDate(2005,10,30), QTime(2,30,0), Qt::LocalTime);
    const KTimeZone *london = KSystemTimeZones::zone("Europe/London");
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

void KTimeZonesTest::abbreviation()
{
    // Fetch time zone abbreviations
    const KTimeZone *losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    const KTimeZone *london = KSystemTimeZones::zone("Europe/London");
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QString sResult = london->abbreviation(winter);
    QCOMPARE(london->abbreviation(winter), QByteArray("GMT"));
    QCOMPARE(london->abbreviation(summer), QByteArray("BST"));
    QCOMPARE(losAngeles->abbreviation(winter), QByteArray("PST"));
    QCOMPARE(losAngeles->abbreviation(summer), QByteArray("PDT"));
}

void KTimeZonesTest::toUTC()
{
    // Convert to UTC.
    const KTimeZone *losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    const KTimeZone *london = KSystemTimeZones::zone("Europe/London");
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

void KTimeZonesTest::toZoneTime()
{
    // Convert from UTC.
    const KTimeZone *losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    const KTimeZone *london = KSystemTimeZones::zone("Europe/London");
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

void KTimeZonesTest::convert()
{
    // Try time zone conversions.
    const KTimeZone *losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    const KTimeZone *london = KSystemTimeZones::zone("Europe/London");
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
// KTzfileTimeZone tests
////////////////////////

void KTimeZonesTest::tzfile()
{
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QString zoneinfo = KSystemTimeZones::zoneinfoDir();
    QVERIFY(!zoneinfo.isEmpty());
    KTzfileTimeZoneSource tzsource(zoneinfo);
    KTimeZone *tzcairo = new KTzfileTimeZone(&tzsource, "Africa/Cairo");
    delete tzcairo;
    tzcairo = new KTzfileTimeZone(&tzsource, "Africa/Cairo");
    QCOMPARE(tzcairo->offsetAtUTC(winter), 7200);
    delete tzcairo;
}

void KTimeZonesTest::tzfileOffsetAtZoneTime()
{
    QDateTime aGmt(QDate(2005,3,27), QTime(0,30,0), Qt::LocalTime);
    QDateTime aInvalid(QDate(2005,3,27), QTime(1,30,0), Qt::LocalTime);
    QDateTime aBst(QDate(2005,3,27), QTime(2,30,0), Qt::LocalTime);
    QDateTime bBst(QDate(2005,10,30), QTime(0,30,0), Qt::LocalTime);
    QDateTime bBstBeforeGmt(QDate(2005,10,30), QTime(1,30,0), Qt::LocalTime);
    QDateTime bGmt(QDate(2005,10,30), QTime(2,30,0), Qt::LocalTime);
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone *london = new KTzfileTimeZone(&tzsource, "Europe/London");
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

void KTimeZonesTest::tzfileUTCOffsets()
{
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone *london = new KTzfileTimeZone(&tzsource, "Europe/London");
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
    // KSystemTimeZones tests
    /////////////////////////
    KTimeZones::ZoneMap allZones = KSystemTimeZones::zones();
    for ( KTimeZones::ZoneMap::const_iterator it = allZones.begin(), end = allZones.end(); it != end; ++it )
      printf( "%s\n", it.key().latin1() );
    return 0;
  }

  printf( "Usage: ktimezonestest [local|list]!\n" );
  return 1;
}
#endif
