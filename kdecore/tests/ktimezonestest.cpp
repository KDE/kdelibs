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

#include <QtTest/qttest_kde.h>
#include <QDateTime>
#include "ktimezones.h"
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
    VERIFY(timezones.add(zone1));
    VERIFY(!timezones.add(zone1));
    VERIFY(timezones.add(zone2));
    COMPARE(timezones.zones().count(), 2);
    const KTimezone* tz = timezones.zone("Zone1");
    VERIFY((tz == zone1));
    tz = timezones.zone("Zone99");
    VERIFY(!tz);
    zone1 = timezones.detach(zone1);
    VERIFY((bool)zone1);
    COMPARE(timezones.zones().count(), 1);
    VERIFY(!timezones.detach(zone1));
    VERIFY(timezones.add(zone1));
    VERIFY((bool)timezones.detach("Zone1"));
    VERIFY(!timezones.detach("Zone1"));
    VERIFY((bool)timezones.detach("Zone2"));
    zone1 = new KTimezone("Zone10");
    delete zone1;
}

///////////////////
// KTimezones: UTC
///////////////////

void KTimezonesTest::utc_data(QtTestTable& t)
{
    t.defineElement("QDateTime", "test");
//    t.defineElement("qint", "result");
//    *t.newData("localtime") << QDateTime(QDate(2005,1,1), QTime(), Qt::LocalTime) << 0;
//    *t.newData("winter")    << QDateTime(QDate(2005,1,1), QTime(), Qt::UTC) << 0;
//    *t.newData("summer")    << QDateTime(QDate(2005,7,1), QTime(), Qt::UTC) << 0;
}

void KTimezonesTest::utc()
{
    KTimezone* utc = KTimezones::utc();
    VERIFY((bool)utc);
    if (utc)
      COMPARE(utc->name(), QString("UTC"));
    COMPARE(utc->offsetAtUTC(QDateTime(QDate(2005,1,1), QTime(), Qt::LocalTime)), 0);
    COMPARE(utc->offsetAtUTC(QDateTime(QDate(2005,1,1), QTime(), Qt::UTC)), 0);
    COMPARE(utc->offsetAtUTC(QDateTime(QDate(2005,7,1), QTime(), Qt::UTC)), 0);

//    FETCH(QDateTime, test);
//    FETCH(int, result);
//    COMPARE(utc->offsetAtUTC(test), result);
}

/////////////////////////
// KSystemTimezones tests
/////////////////////////

void KTimezonesTest::local()
{
    const KTimezone *local = KSystemTimezones::local();
    VERIFY((bool)local);
    QString msg = QString("Please manually verify that the local time zone is \"%1\"").arg(local->name());
    WARN(msg.toLatin1().data());
}

void KTimezonesTest::zone()
{
    const KTimezone *losAngeles = KSystemTimezones::zone("America/Los_Angeles");
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    VERIFY((bool)losAngeles);
    VERIFY((bool)london);
}

void KTimezonesTest::zoneinfoDir()
{
    QString zoneinfo = KSystemTimezones::zoneinfoDir();
    VERIFY(!zoneinfo.isEmpty());
    QString msg = QString("Please manually verify that the zoneinfo directory is \"%1\"").arg(zoneinfo);
    WARN(msg.toLatin1().data());
}


    ////////////////////////
    // KSystemTimezone tests
    ////////////////////////

void KTimezonesTest::currentOffset()
{
    // Find the current offset of a time zone
    const KTimezone *local = KSystemTimezones::local();
    VERIFY((bool)local);
    int offset = local->currentOffset(Qt::UTC);
    QString msg = QString("Please manually verify that the current local time zone offset is %1h:%2m:%3s").arg(offset/3600).arg((offset/60)%60, 2, QChar('0')).arg(offset%3600, 2, QChar('0'));
    WARN(msg.toLatin1().data());
}

void KTimezonesTest::offsetAtUTC()
{
    // Find some offsets for Europe/London.
    const KTimezone *losAngeles = KSystemTimezones::zone("America/Los_Angeles");
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    COMPARE(london->offsetAtUTC(winter), 0);
    COMPARE(london->offsetAtUTC(summer), 3600);;
    COMPARE(losAngeles->offsetAtUTC(winter), -28800);
    COMPARE(losAngeles->offsetAtUTC(summer), -25200);;
}

void KTimezonesTest::abbreviation()
{
    // Fetch time zone abbreviations
    const KTimezone *losAngeles = KSystemTimezones::zone("America/Los_Angeles");
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QString sResult = london->abbreviation(winter);
    COMPARE(london->abbreviation(winter), QByteArray("GMT"));
    COMPARE(london->abbreviation(summer), QByteArray("BST"));
    COMPARE(losAngeles->abbreviation(winter), QByteArray("PST"));
    COMPARE(losAngeles->abbreviation(summer), QByteArray("PDT"));
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
    COMPARE(london->toUTC(winterLocal), winter);
    COMPARE(london->toUTC(summerLocal), summer.addSecs(-3600));;
    COMPARE(losAngeles->toUTC(winterLocal), winter.addSecs(8*3600));
    COMPARE(losAngeles->toUTC(summerLocal), summer.addSecs(7*3600));
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
    COMPARE(london->toZoneTime(winter), winterLocal);
    COMPARE(london->toZoneTime(summer), summerLocal.addSecs(3600));
    COMPARE(losAngeles->toZoneTime(winter), winterLocal.addSecs(-8*3600));
    COMPARE(losAngeles->toZoneTime(summer), summerLocal.addSecs(-7*3600));
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

    COMPARE(london->convert(losAngeles, bstBeforePdt), bstBeforePdtResult);
    COMPARE(london->convert(losAngeles, bstAfterPdt), bstAfterPdtResult);
    COMPARE(london->convert(losAngeles, gmtBeforePst), gmtBeforePstResult);
    COMPARE(london->convert(losAngeles, gmtAfterPst), gmtAfterPstResult);
}

////////////////////////
// KTzfileTimezone tests
////////////////////////

void KTimezonesTest::tzfile()
{
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QString zoneinfo = KSystemTimezones::zoneinfoDir();
    VERIFY(!zoneinfo.isEmpty());
    KTzfileTimezoneSource tzsource(zoneinfo);
    KTimezone *tzcairo = new KTzfileTimezone(&tzsource, "Africa/Cairo");
    delete tzcairo;
    tzcairo = new KTzfileTimezone(&tzsource, "Africa/Cairo");
    COMPARE(tzcairo->offsetAtUTC(winter), 7200);
    delete tzcairo;
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
