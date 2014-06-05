/* This file is part of the KDE libraries
   Copyright (c) 2005-2007,2011-2012 David Jarvie <djarvie@kde.org>

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

#include "ktimezonestest_p.h"

#include <stdio.h>
#include <qtest_kde.h>
#include <QtCore/QDir>
#include <QtCore/QDate>
#include <QtDBus/QtDBus>
#include <config-date.h> // for HAVE_TM_GMTOFF
#include "ksystemtimezone.h"
#include "ktzfiletimezone.h"
#include "ktimezonestest.moc"

QTEST_KDEMAIN_CORE(KTimeZonesTest)

TimeZoneTestData s_testData;

void KTimeZonesTest::initTestCase()
{
    s_testData.setupTimeZoneTest(); // see ktimezonestest_p.h
    mDataDir = s_testData.dataDir();
}

void KTimeZonesTest::cleanupTestCase()
{
    s_testData.cleanupTimeZoneTest();
}

///////////////////
// KTimeZones tests
///////////////////

void KTimeZonesTest::ktimezones()
{
    KTimeZones timezones;
    KTimeZone zone1("Zone1");
    QVERIFY(zone1.isValid());
    KTimeZone zone2("Zone2");
    QVERIFY(zone2.isValid());
    QVERIFY(timezones.add(zone1));
    QVERIFY(!timezones.add(zone1));
    QVERIFY(timezones.add(zone2));
    QCOMPARE(timezones.zones().count(), 2);
    KTimeZone tz = timezones.zone("Zone1");
    QVERIFY((tz == zone1));
    tz = timezones.zone("Zone99");
    QVERIFY(!tz.isValid());
    zone1 = timezones.remove(zone1);
    QVERIFY(zone1.isValid());
    QCOMPARE(timezones.zones().count(), 1);
    QVERIFY(!timezones.remove(zone1).isValid());
    QVERIFY(timezones.add(zone1));
    QVERIFY(timezones.remove("Zone1").isValid());
    QVERIFY(!timezones.remove("Zone1").isValid());
    QVERIFY(timezones.remove("Zone2").isValid());
    zone1 = KTimeZone("Zone10");
    QVERIFY(timezones.add(zone1));
    QCOMPARE(timezones.zones().count(), 1);
    timezones.clear();
    QCOMPARE(timezones.zones().count(), 0);
}

//////////////////////////
// KTimeZone: ref counting
//////////////////////////

void KTimeZonesTest::refcount()
{
    KTimeZone *zone1 = new KTimeZone("Zone1");
    QCOMPARE(zone1->name(), QString("Zone1"));
    KTimeZones timezones;
    timezones.add(*zone1);
    delete zone1;
    zone1 = 0;
    KTimeZone tz = timezones.zone("Zone1");
    QVERIFY(tz.isValid());
    QCOMPARE(tz.name(), QString("Zone1"));
    QCOMPARE(tz.type(), QByteArray("KTimeZone"));
}

///////////////////
// KTimeZone: UTC
///////////////////

void KTimeZonesTest::utc()
{
    KTimeZone utc = KTimeZone::utc();
    QVERIFY(utc.isValid());
    QCOMPARE(utc.name(), QString("UTC"));
    QCOMPARE(utc.offsetAtUtc(QDateTime(QDate(2005,1,1), QTime(), Qt::LocalTime)), 0);
    QCOMPARE(utc.offsetAtUtc(QDateTime(QDate(2005,1,1), QTime(), Qt::UTC)), 0);
    QCOMPARE(utc.offsetAtUtc(QDateTime(QDate(2005,2,1), QTime(), Qt::UTC)), 0);
    QCOMPARE(utc.offsetAtUtc(QDateTime(QDate(2005,7,1), QTime(), Qt::UTC)), 0);
}

/////////////////////////
// KSystemTimeZones tests
/////////////////////////

void KTimeZonesTest::local()
{
    KTimeZone local = KSystemTimeZones::local();
    QVERIFY(local.isValid());
    QCOMPARE(local.name(), QString::fromLatin1("Europe/Paris"));
    QCOMPARE(local.type(), QByteArray("KTzfileTimeZone"));
}

void KTimeZonesTest::zone()
{
    KTimeZone utc = KSystemTimeZones::zone("UTC");
    QVERIFY(utc.isValid());
    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY(losAngeles.isValid());
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY(london.isValid());
    QCOMPARE(london.countryCode(), QString("GB"));
    QCOMPARE(london.latitude(), float(51*3600 + 28*60 + 30)/3600.0f);
    QCOMPARE(london.longitude(), -float(0*3600 + 18*60 + 45)/3600.0f);
    QCOMPARE(london.comment(), QString("Great Britain"));
    QCOMPARE(losAngeles.longitude(), -float(118*3600 + 14*60 + 34)/3600.0f);
}

void KTimeZonesTest::zoneinfoDir()
{
    QString zoneinfo = KSystemTimeZones::zoneinfoDir();
    QCOMPARE(zoneinfo, mDataDir);
}

void KTimeZonesTest::zonetabChange()
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    QSignalSpy timeoutSpy(&timer, SIGNAL(timeout()));

    QCOMPARE(KSystemTimeZones::zones().count(), 5);
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY(london.isValid());
    QCOMPARE(london.countryCode(), QString("GB"));
    QCOMPARE(london.latitude(), float(51*3600 + 28*60 + 30)/3600.0f);
    QCOMPARE(london.longitude(), -float(0*3600 + 18*60 + 45)/3600.0f);
    QCOMPARE(london.comment(), QString("Great Britain"));
    QVERIFY(!KSystemTimeZones::zone("Europe/Berlin").isValid());
    QVERIFY(KSystemTimeZones::zone("Europe/Paris").isValid());
    QVERIFY(KSystemTimeZones::zone("Europe/London").isValid());
    QVERIFY(KSystemTimeZones::zone("Africa/Cairo").isValid());
    QVERIFY(!KSystemTimeZones::zone("Asia/Dili").isValid());
    QVERIFY(KSystemTimeZones::zone("America/Los_Angeles").isValid());

    // Check that 'london' is automatically updated with the new zone.tab
    // contents, and that the new zones are added to KSystemTimeZones.
    s_testData.writeZoneTab(true);
    QDBusMessage message = QDBusMessage::createSignal("/Daemon", "org.kde.KTimeZoned", "zonetabChanged");
    QList<QVariant> args;
    args += QString(mDataDir + QLatin1String("/zone.tab"));
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
    timer.start(1000);
    loop.exec();
    QCOMPARE(KSystemTimeZones::zones().count(), 7);
    QVERIFY(london.isValid());
    QCOMPARE(london.countryCode(), QString("XX"));
    QCOMPARE(london.latitude(), -float(51*3600 + 28*60 + 30)/3600.0f);
    QCOMPARE(london.longitude(), float(0*3600 + 18*60 + 45)/3600.0f);
    QCOMPARE(london.comment(), QString("Greater Britain"));
    QCOMPARE(KSystemTimeZones::zone("Europe/London"), london);
    QVERIFY(KSystemTimeZones::zone("Europe/Berlin").isValid());
    QVERIFY(KSystemTimeZones::zone("Europe/Paris").isValid());
    QVERIFY(KSystemTimeZones::zone("Europe/London").isValid());
    QVERIFY(KSystemTimeZones::zone("Africa/Cairo").isValid());
    QVERIFY(KSystemTimeZones::zone("Africa/Johannesburg").isValid());
    QVERIFY(KSystemTimeZones::zone("Asia/Dili").isValid());
    QVERIFY(KSystemTimeZones::zone("America/Los_Angeles").isValid());

    // Check that 'london' is automatically updated with the new zone.tab
    // contents, and that the removed zones are deleted from KSystemTimeZones.
    s_testData.writeZoneTab(false);
    QDBusConnection::sessionBus().send(message);
    timer.start(1000);
    loop.exec();
    QCOMPARE(KSystemTimeZones::zones().count(), 5);
    QVERIFY(london.isValid());
    QCOMPARE(london.countryCode(), QString("GB"));
    QCOMPARE(london.latitude(), float(51*3600 + 28*60 + 30)/3600.0f);
    QCOMPARE(london.longitude(), -float(0*3600 + 18*60 + 45)/3600.0f);
    QCOMPARE(london.comment(), QString("Great Britain"));
    QCOMPARE(KSystemTimeZones::zone("Europe/London"), london);
    QVERIFY(!KSystemTimeZones::zone("Europe/Berlin").isValid());
    QVERIFY(KSystemTimeZones::zone("Europe/Paris").isValid());
    QVERIFY(KSystemTimeZones::zone("Europe/London").isValid());
    QVERIFY(KSystemTimeZones::zone("Africa/Cairo").isValid());
    QVERIFY(!KSystemTimeZones::zone("Asia/Dili").isValid());
    QVERIFY(KSystemTimeZones::zone("America/Los_Angeles").isValid());
}


////////////////////////
// KSystemTimeZone tests
////////////////////////

void KTimeZonesTest::currentOffset()
{
    QString tzfile = ':' + mDataDir + "/Europe/Paris";
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", tzfile.toLatin1().data(), 1);
    ::tzset();

    // Find the current offset of a time zone
    time_t now = time(0);
    tm *tnow = localtime(&now);
#if defined(HAVE_TM_GMTOFF)
    int offset = tnow->tm_gmtoff;
#else
    int offset = 0;
#endif
    KTimeZone local = KSystemTimeZones::local();
    QVERIFY(local.isValid());
    QCOMPARE(local.currentOffset(Qt::UTC), offset);


    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KTimeZonesTest::offsetAtUtc()
{
    // Find some offsets for Europe/London.
    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY(losAngeles.isValid());
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY(london.isValid());
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QCOMPARE(london.offsetAtUtc(winter), 0);
    QCOMPARE(london.offsetAtUtc(summer), 3600);;
    QCOMPARE(losAngeles.offsetAtUtc(winter), -28800);
    QCOMPARE(losAngeles.offsetAtUtc(summer), -25200);;
}

void KTimeZonesTest::offsetAtZoneTime()
{
    QDateTime Gmt1(QDate(2005,3,27), QTime(0,30,0), Qt::LocalTime);
    QDateTime GmtToBst1(QDate(2005,3,27), QTime(0,59,59), Qt::LocalTime);
    QDateTime GmtToBst2(QDate(2005,3,27), QTime(1,0,0), Qt::LocalTime);
    QDateTime GmtToBst3(QDate(2005,3,27), QTime(1,59,59), Qt::LocalTime);
    QDateTime GmtToBst4(QDate(2005,3,27), QTime(2,0,0), Qt::LocalTime);
    QDateTime Bst1(QDate(2005,4,27), QTime(2,30,0), Qt::LocalTime);
    QDateTime Bst2(QDate(2005,10,30), QTime(0,30,0), Qt::LocalTime);
    QDateTime BstToGmt1(QDate(2005,10,30), QTime(0,59,59), Qt::LocalTime);
    QDateTime BstToGmt2(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime);
    QDateTime BstToGmt3(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime);
    QDateTime BstToGmt4(QDate(2005,10,30), QTime(2,0,0), Qt::LocalTime);
    QDateTime Gmt2(QDate(2005,10,30), QTime(2,30,0), Qt::LocalTime);
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY(london.isValid());
    int offset2;
    QCOMPARE(london.offsetAtZoneTime(Gmt1, &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(london.offsetAtZoneTime(GmtToBst1, &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(london.offsetAtZoneTime(GmtToBst2, &offset2), KTimeZone::InvalidOffset);
    QCOMPARE(offset2, KTimeZone::InvalidOffset);
    QCOMPARE(london.offsetAtZoneTime(GmtToBst3, &offset2), KTimeZone::InvalidOffset);
    QCOMPARE(offset2, KTimeZone::InvalidOffset);
    QCOMPARE(london.offsetAtZoneTime(GmtToBst4, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london.offsetAtZoneTime(Bst1, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london.offsetAtZoneTime(Bst2, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london.offsetAtZoneTime(BstToGmt1, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london.offsetAtZoneTime(BstToGmt2, &offset2), 3600);
    QCOMPARE(offset2, 0);
    QCOMPARE(london.offsetAtZoneTime(BstToGmt3, &offset2), 3600);
    QCOMPARE(offset2, 0);
    QCOMPARE(london.offsetAtZoneTime(BstToGmt4, &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(london.offsetAtZoneTime(Gmt2, &offset2), 0);
    QCOMPARE(offset2, 0);

    KTimeZone johannesburg = KSystemTimeZones::zone("Africa/Johannesburg");
    QVERIFY(johannesburg.isValid());
    QDateTime recent(QDate(2013,5,10), QTime(13,0,0), Qt::LocalTime);
    QCOMPARE(johannesburg.offsetAtZoneTime(recent, &offset2), 7200);
    QCOMPARE(offset2, 7200);
}

void KTimeZonesTest::abbreviation()
{
    // Fetch time zone abbreviations
    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY(losAngeles.isValid());
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY(london.isValid());
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QString sResult = london.abbreviation(winter);
    QCOMPARE(london.abbreviation(winter), QByteArray("GMT"));
    QCOMPARE(london.abbreviation(summer), QByteArray("BST"));
    QCOMPARE(losAngeles.abbreviation(winter), QByteArray("PST"));
    QCOMPARE(losAngeles.abbreviation(summer), QByteArray("PDT"));
}

void KTimeZonesTest::timet()
{
    QDateTime t1(QDate(1970,1,2), QTime(1,30,5), Qt::UTC);
    QDateTime t2(QDate(1969,12,30), QTime(22,29,55), Qt::UTC);
    time_t t1t = KTimeZone::toTime_t(t1);
    time_t t2t = KTimeZone::toTime_t(t2);
    QCOMPARE((int)t1t, 86400 + 3600 + 30*60 + 5);
    QCOMPARE((int)t2t, -(86400 + 3600 + 30*60 + 5));
    QCOMPARE(KTimeZone::fromTime_t(t1t), t1);
    QCOMPARE(KTimeZone::fromTime_t(t2t), t2);
}

void KTimeZonesTest::toUtc()
{
    // Convert to UTC.
    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY(losAngeles.isValid());
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY(london.isValid());
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QDateTime winterLocal = winter;
    winterLocal.setTimeSpec(Qt::LocalTime);
    QDateTime summerLocal = summer;
    summerLocal.setTimeSpec(Qt::LocalTime);
    QCOMPARE(london.toUtc(winterLocal), winter);
    QCOMPARE(london.toUtc(summerLocal), summer.addSecs(-3600));;
    QCOMPARE(losAngeles.toUtc(winterLocal), winter.addSecs(8*3600));
    QCOMPARE(losAngeles.toUtc(summerLocal), summer.addSecs(7*3600));
}

void KTimeZonesTest::toZoneTime()
{
    // Convert from UTC.
    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY(losAngeles.isValid());
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY(london.isValid());
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QDateTime winterLocal = winter;
    winterLocal.setTimeSpec(Qt::LocalTime);
    QDateTime summerLocal = summer;
    summerLocal.setTimeSpec(Qt::LocalTime);
    QCOMPARE(london.toZoneTime(winter), winterLocal);
    QCOMPARE(london.toZoneTime(summer), summerLocal.addSecs(3600));
    QCOMPARE(losAngeles.toZoneTime(winter), winterLocal.addSecs(-8*3600));
    QCOMPARE(losAngeles.toZoneTime(summer), summerLocal.addSecs(-7*3600));

    QDateTime prepre(QDate(2005,10,29), QTime(23,59,59), Qt::UTC);  // before time shift (local time not repeated)
    QDateTime pre(QDate(2005,10,30), QTime(0,0,0), Qt::UTC);  // before time shift (local time repeated afterwards)
    QDateTime before(QDate(2005,10,30), QTime(0,59,59), Qt::UTC);  // before time shift (local time repeated afterwards)
    QDateTime at(QDate(2005,10,30), QTime(1,0,0), Qt::UTC);   // at time shift (second occurrence of local time)
    QDateTime last(QDate(2005,10,30), QTime(1,59,59), Qt::UTC);  // after time shift (second occurrence of local time)
    QDateTime after(QDate(2005,10,30), QTime(2,0,0), Qt::UTC);   // after time shift (local time not repeated)
    bool second;
    QCOMPARE(london.toZoneTime(prepre, &second), QDateTime(QDate(2005,10,30), QTime(0,59,59), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(london.toZoneTime(pre, &second), QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(london.toZoneTime(before, &second), QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(london.toZoneTime(at, &second), QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime));
    QVERIFY(second);
    QCOMPARE(london.toZoneTime(last, &second), QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime));
    QVERIFY(second);
    QCOMPARE(london.toZoneTime(after, &second), QDateTime(QDate(2005,10,30), QTime(2,0,0), Qt::LocalTime));
    QVERIFY(!second);
}

void KTimeZonesTest::convert()
{
    // Try time zone conversions.
    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY(losAngeles.isValid());
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY(london.isValid());
    QDateTime bstBeforePdt(QDate(2005,3,28), QTime(0,0,0), Qt::LocalTime);
    QDateTime bstAfterPdt(QDate(2005,5,1), QTime(0,0,0), Qt::LocalTime);
    QDateTime gmtBeforePst(QDate(2005,10,30), QTime(4,0,0), Qt::LocalTime);
    QDateTime gmtAfterPst(QDate(2005,12,1), QTime(0,0,0), Qt::LocalTime);
    QDateTime bstBeforePdtResult(QDate(2005,3,27), QTime(15,0,0), Qt::LocalTime);
    QDateTime bstAfterPdtResult(QDate(2005,4,30), QTime(16,0,0), Qt::LocalTime);
    QDateTime gmtBeforePstResult(QDate(2005,10,29), QTime(21,0,0), Qt::LocalTime);
    QDateTime gmtAfterPstResult(QDate(2005,11,30), QTime(16,0,0), Qt::LocalTime);

    QCOMPARE(london.convert(losAngeles, bstBeforePdt), bstBeforePdtResult);
    QCOMPARE(london.convert(losAngeles, bstAfterPdt), bstAfterPdtResult);
    QCOMPARE(london.convert(losAngeles, gmtBeforePst), gmtBeforePstResult);
    QCOMPARE(london.convert(losAngeles, gmtAfterPst), gmtAfterPstResult);
    QCOMPARE(losAngeles.convert(losAngeles, bstBeforePdtResult), bstBeforePdtResult);
}

////////////////////////
// KTzfileTimeZone tests
// Plus KSystemTimeZones::readZone() tests
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
    QCOMPARE(tzcairo->offsetAtUtc(winter), 7200);
    delete tzcairo;
    KTimeZone *johannesburg = new KTzfileTimeZone(&tzsource, "Africa/Johannesburg");
    delete johannesburg;
    johannesburg = new KTzfileTimeZone(&tzsource, "Africa/Johannesburg");
    QCOMPARE(johannesburg->offsetAtUtc(winter), 7200);
    delete johannesburg;
}

void KTimeZonesTest::tzfileDstShifts()
{
    // Check time zone conversions against zdump output for zone
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone berlin = KTzfileTimeZone(&tzsource, "Europe/Berlin");
    KTimeZone losAngeles = KTzfileTimeZone(&tzsource, "America/Los_Angeles");
    KTimeZone tz = berlin;
    for (int n = 0; n < 2; ++n, tz = losAngeles)
    {
        qDebug() << tz.name();
        QVERIFY(tz.isValid());
        QCOMPARE(tz.type(), QByteArray("KTzfileTimeZone"));
        QFile file(QString::fromLatin1(KDESRCDIR) + tz.name().remove(QRegExp("^.+/")) + QLatin1String(".zdump"));
        QVERIFY(file.open(QIODevice::ReadOnly));
        QTextStream in(&file);
        QString line;
        while (!(line = in.readLine()).isNull())
        {
            QStringList parts = line.split(" ", QString::SkipEmptyParts);
            QCOMPARE(parts[6], QString::fromLatin1("UTC"));
            QCOMPARE(parts[7], QString::fromLatin1("="));
            QCOMPARE(parts[14].left(6), QString::fromLatin1("isdst="));
            QCOMPARE(parts[15].left(7), QString::fromLatin1("gmtoff="));
            QDateTime utc = QDateTime::fromString(static_cast<QStringList>(parts.mid(1, 5)).join(" "));
            utc.setTimeSpec(Qt::UTC);
            QDateTime local = QDateTime::fromString(static_cast<QStringList>(parts.mid(8, 5)).join(" "));
            local.setTimeSpec(Qt::LocalTime);
            QCOMPARE(tz.toZoneTime(utc), local);
            QCOMPARE(parts[13], QString::fromLatin1(tz.abbreviation(utc)));
            bool dst = (parts[14].right(1) != "0");
            QCOMPARE(tz.isDstAtUtc(utc), dst);
            QCOMPARE(parts[15].mid(7).toInt(), tz.offsetAtUtc(utc));
        }
        file.close();
    }
}

void KTimeZonesTest::tzfileToZoneTime()
{
    // Convert from UTC.
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone london = KTzfileTimeZone(&tzsource, "Europe/London");
    QVERIFY(london.isValid());
    QCOMPARE(london.type(), QByteArray("KTzfileTimeZone"));
    QDateTime prepre(QDate(2005,10,29), QTime(23,59,59), Qt::UTC);  // before time shift (local time not repeated)
    QDateTime pre(QDate(2005,10,30), QTime(0,0,0), Qt::UTC);  // before time shift (local time repeated afterwards)
    QDateTime before(QDate(2005,10,30), QTime(0,59,59), Qt::UTC);  // before time shift (local time repeated afterwards)
    QDateTime at(QDate(2005,10,30), QTime(1,0,0), Qt::UTC);    // at time shift (second occurrence of local time)
    QDateTime last(QDate(2005,10,30), QTime(1,59,59), Qt::UTC);  // after time shift (second occurrence of local time)
    QDateTime after(QDate(2005,10,30), QTime(2,0,0), Qt::UTC);    // after time shift (local time not repeated)
    bool second;
    QCOMPARE(london.toZoneTime(prepre, &second), QDateTime(QDate(2005,10,30), QTime(0,59,59), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(london.toZoneTime(pre, &second), QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(london.toZoneTime(before, &second), QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(london.toZoneTime(at, &second), QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime));
    QVERIFY(second);
    QCOMPARE(london.toZoneTime(last, &second), QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime));
    QVERIFY(second);
    QCOMPARE(london.toZoneTime(after, &second), QDateTime(QDate(2005,10,30), QTime(2,0,0), Qt::LocalTime));
    QVERIFY(!second);

    KTimeZone sysLondon = KSystemTimeZones::readZone("Europe/London");
    QVERIFY(sysLondon.isValid());
    QCOMPARE(sysLondon.toZoneTime(prepre, &second), QDateTime(QDate(2005,10,30), QTime(0,59,59), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(sysLondon.toZoneTime(pre, &second), QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(sysLondon.toZoneTime(before, &second), QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime));
    QVERIFY(!second);
    QCOMPARE(sysLondon.toZoneTime(at, &second), QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime));
    QVERIFY(second);
    QCOMPARE(sysLondon.toZoneTime(last, &second), QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime));
    QVERIFY(second);
    QCOMPARE(sysLondon.toZoneTime(after, &second), QDateTime(QDate(2005,10,30), QTime(2,0,0), Qt::LocalTime));
    QVERIFY(!second);
}

void KTimeZonesTest::tzfileOffsetAtUtc()
{
    QDateTime a3Gmt(QDate(2004,12,27), QTime(0,30,0), Qt::UTC);
    QDateTime a2Gmt(QDate(2005,2,27), QTime(0,30,0), Qt::UTC);
    QDateTime aGmt(QDate(2005,3,27), QTime(0,30,0), Qt::UTC);
    QDateTime aBst(QDate(2005,3,27), QTime(1,30,0), Qt::UTC);
    QDateTime a2Bst(QDate(2005,7,27), QTime(2,30,0), Qt::UTC);
    QDateTime bBst(QDate(2005,10,29), QTime(23,30,0), Qt::UTC);
    QDateTime bBstBeforeGmt(QDate(2005,10,30), QTime(0,30,0), Qt::UTC);
    QDateTime bGmt(QDate(2005,10,30), QTime(2,30,0), Qt::UTC);
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone london = KTzfileTimeZone(&tzsource, "Europe/London");
    QVERIFY(london.isValid());
    QCOMPARE(london.type(), QByteArray("KTzfileTimeZone"));
    QCOMPARE(london.offsetAtUtc(a3Gmt), 0);
    QCOMPARE(london.offsetAtUtc(a2Gmt), 0);             // uses cache
    QCOMPARE(london.offsetAtUtc(aGmt), 0);              // uses cache
    QCOMPARE(london.offsetAtUtc(aBst), 3600);
    QCOMPARE(london.offsetAtUtc(a2Bst), 3600);          // uses cache
    QCOMPARE(london.offsetAtUtc(bBst), 3600);           // uses cache
    QCOMPARE(london.offsetAtUtc(bBstBeforeGmt), 3600);  // uses cache
    QCOMPARE(london.offsetAtUtc(bGmt), 0);

    QDateTime recent(QDate(2013,5,10), QTime(13,0,0), Qt::UTC);
    KTimeZone johannesburg = KTzfileTimeZone(&tzsource, "Africa/Johannesburg");
    QVERIFY(johannesburg.isValid());
    QCOMPARE(johannesburg.type(), QByteArray("KTzfileTimeZone"));
    QCOMPARE(johannesburg.offsetAtUtc(recent), 7200);
}

void KTimeZonesTest::tzfileOffsetAtZoneTime()
{
    QDateTime Gmt0(QDate(2005,2,27), QTime(0,30,0), Qt::LocalTime);
    QDateTime Gmt1(QDate(2005,3,27), QTime(0,30,0), Qt::LocalTime);
    QDateTime GmtToBst1(QDate(2005,3,27), QTime(0,59,59), Qt::LocalTime);
    QDateTime GmtToBst2(QDate(2005,3,27), QTime(1,0,0), Qt::LocalTime);
    QDateTime GmtToBst3(QDate(2005,3,27), QTime(1,59,59), Qt::LocalTime);
    QDateTime GmtToBst4(QDate(2005,3,27), QTime(2,0,0), Qt::LocalTime);
    QDateTime Bst1(QDate(2005,4,27), QTime(2,30,0), Qt::LocalTime);
    QDateTime Bst2(QDate(2005,10,30), QTime(0,30,0), Qt::LocalTime);
    QDateTime BstToGmt1(QDate(2005,10,30), QTime(0,59,59), Qt::LocalTime);
    QDateTime BstToGmt2(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime);
    QDateTime BstToGmt3(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime);
    QDateTime BstToGmt4(QDate(2005,10,30), QTime(2,0,0), Qt::LocalTime);
    QDateTime Gmt2(QDate(2005,10,30), QTime(2,30,0), Qt::LocalTime);
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone london = KTzfileTimeZone(&tzsource, "Europe/London");
    QVERIFY(london.isValid());
    int offset2;
    QCOMPARE(london.offsetAtZoneTime(Gmt0, &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(london.offsetAtZoneTime(Gmt1, &offset2), 0);          // uses cache
    QCOMPARE(offset2, 0);
    QCOMPARE(london.offsetAtZoneTime(GmtToBst1, &offset2), 0);     // uses cache
    QCOMPARE(offset2, 0);
    QCOMPARE(london.offsetAtZoneTime(GmtToBst2, &offset2), KTimeZone::InvalidOffset);
    QCOMPARE(offset2, KTimeZone::InvalidOffset);
    QCOMPARE(london.offsetAtZoneTime(GmtToBst3, &offset2), KTimeZone::InvalidOffset);
    QCOMPARE(offset2, KTimeZone::InvalidOffset);
    QCOMPARE(london.offsetAtZoneTime(GmtToBst4, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london.offsetAtZoneTime(Bst1, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london.offsetAtZoneTime(Bst2, &offset2), 3600);       // uses cache
    QCOMPARE(offset2, 3600);
    QCOMPARE(london.offsetAtZoneTime(BstToGmt1, &offset2), 3600);  // uses cache
    QCOMPARE(offset2, 3600);
    QCOMPARE(london.offsetAtZoneTime(BstToGmt2, &offset2), 3600);
    QCOMPARE(offset2, 0);
    QCOMPARE(london.offsetAtZoneTime(BstToGmt3, &offset2), 3600);
    QCOMPARE(offset2, 0);
    QCOMPARE(london.offsetAtZoneTime(BstToGmt4, &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(london.offsetAtZoneTime(Gmt2, &offset2), 0);          // uses cache
    QCOMPARE(offset2, 0);

    KTimeZone sysLondon = KSystemTimeZones::readZone("Europe/London");
    QVERIFY(sysLondon.isValid());
    QCOMPARE(sysLondon.offsetAtZoneTime(Gmt0, &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(sysLondon.offsetAtZoneTime(Gmt1, &offset2), 0);          // uses cache
    QCOMPARE(offset2, 0);
    QCOMPARE(sysLondon.offsetAtZoneTime(GmtToBst1, &offset2), 0);     // uses cache
    QCOMPARE(offset2, 0);
    QCOMPARE(sysLondon.offsetAtZoneTime(GmtToBst2, &offset2), KTimeZone::InvalidOffset);
    QCOMPARE(offset2, KTimeZone::InvalidOffset);
    QCOMPARE(sysLondon.offsetAtZoneTime(GmtToBst3, &offset2), KTimeZone::InvalidOffset);
    QCOMPARE(offset2, KTimeZone::InvalidOffset);
    QCOMPARE(sysLondon.offsetAtZoneTime(GmtToBst4, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(sysLondon.offsetAtZoneTime(Bst1, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(sysLondon.offsetAtZoneTime(Bst2, &offset2), 3600);       // uses cache
    QCOMPARE(offset2, 3600);
    QCOMPARE(sysLondon.offsetAtZoneTime(BstToGmt1, &offset2), 3600);  // uses cache
    QCOMPARE(offset2, 3600);
    QCOMPARE(sysLondon.offsetAtZoneTime(BstToGmt2, &offset2), 3600);
    QCOMPARE(offset2, 0);
    QCOMPARE(sysLondon.offsetAtZoneTime(BstToGmt3, &offset2), 3600);
    QCOMPARE(offset2, 0);
    QCOMPARE(sysLondon.offsetAtZoneTime(BstToGmt4, &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(sysLondon.offsetAtZoneTime(Gmt2, &offset2), 0);          // uses cache
    QCOMPARE(offset2, 0);

    KTimeZone johannesburg = KTzfileTimeZone(&tzsource, "Africa/Johannesburg");
    QVERIFY(johannesburg.isValid());
    QCOMPARE(johannesburg.type(), QByteArray("KTzfileTimeZone"));
    QDateTime recent(QDate(2013,5,10), QTime(13,0,0), Qt::LocalTime);
    QCOMPARE(johannesburg.offsetAtZoneTime(recent, &offset2), 7200);
    QCOMPARE(offset2, 7200);
}

void KTimeZonesTest::tzfileUtcOffsets()
{
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone london = KTzfileTimeZone(&tzsource, "Europe/London");
    QVERIFY(london.isValid());
    QList<int> offsets = london.utcOffsets();
    QCOMPARE(offsets.count(), 3);
    QCOMPARE(offsets[0], 0);    // GMT
    QCOMPARE(offsets[1], 3600); // BST
    QCOMPARE(offsets[2], 7200); // DST

    KTimeZone sysLondon = KSystemTimeZones::readZone("Europe/London");
    QVERIFY(sysLondon.isValid());
    offsets = sysLondon.utcOffsets();
    QCOMPARE(offsets.count(), 3);
    QCOMPARE(offsets[0], 0);    // GMT
    QCOMPARE(offsets[1], 3600); // BST
    QCOMPARE(offsets[2], 7200); // DST
}

void KTimeZonesTest::tzfileAbbreviation()
{
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone london = KTzfileTimeZone(&tzsource, "Europe/London");
    QVERIFY(london.isValid());
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QDateTime standard(QDate(1970,4,30), QTime(12,45,16,25), Qt::UTC);
    QString sResult = london.abbreviation(winter);
    QCOMPARE(london.abbreviation(winter), QByteArray("GMT"));
    QCOMPARE(london.abbreviation(summer), QByteArray("BST"));
    QCOMPARE(london.abbreviation(standard), QByteArray("BST"));

    KTimeZone sysLondon = KSystemTimeZones::readZone("Europe/London");
    QVERIFY(sysLondon.isValid());
    sResult = sysLondon.abbreviation(winter);
    QCOMPARE(sysLondon.abbreviation(winter), QByteArray("GMT"));
    QCOMPARE(sysLondon.abbreviation(summer), QByteArray("BST"));
    QCOMPARE(sysLondon.abbreviation(standard), QByteArray("BST"));
}

void KTimeZonesTest::tzfileTransitions()
{
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone london = KTzfileTimeZone(&tzsource, "Europe/London");
    QVERIFY(london.isValid());
    QList<KTimeZone::Transition> all = london.transitions();
    QVERIFY(!all.isEmpty());
    QDateTime jan2003(QDate(2003,1,1),QTime(0,0,0),Qt::UTC);
    QDateTime jan2006(QDate(2006,1,1),QTime(0,0,0),Qt::UTC);
    int index2006 = london.transitionIndex(jan2006);
    if (index2006 >= 0)
    {
        QVERIFY(all[index2006].time() <= jan2006);
        QList<KTimeZone::Transition> some = london.transitions(QDateTime(), jan2006);
        QList<KTimeZone::Transition> check = all.mid(0, index2006+1);
        QCOMPARE(some.count(), check.count());
        for (int i = 0;  i < some.count();  ++i)
        {
            QCOMPARE(some[i].time(), check[i].time());
            QCOMPARE(some[i].phase(), check[i].phase());
        }
        if (all[index2006].time() < jan2006  &&  ++index2006 < all.count())
            QVERIFY(all[index2006].time() > jan2006);
        some = london.transitions(jan2006);
        check = all.mid(index2006);
        for (int i = 0;  i < some.count();  ++i)
        {
            QCOMPARE(some[i].time(), check[i].time());
            QCOMPARE(some[i].phase(), check[i].phase());
        }
    }
    index2006 = london.transitionIndex(jan2006);
    int index2003 = london.transitionIndex(jan2003);
    if (index2003 >= 0)
    {
        QVERIFY(all[index2003].time() <= jan2003);
        if (all[index2003].time() < jan2003  &&  ++index2003 < all.count())
            QVERIFY(all[index2003].time() > jan2003);
        QList<KTimeZone::Transition> some = london.transitions(jan2003, jan2006);
        QList<KTimeZone::Transition> check = all.mid(index2003, index2006-index2003+1);
        QCOMPARE(some.count(), check.count());
        for (int i = 0;  i < some.count();  ++i)
        {

            QCOMPARE(some[i].time(), check[i].time());
            QCOMPARE(some[i].phase(), check[i].phase());
        }
    }
}

