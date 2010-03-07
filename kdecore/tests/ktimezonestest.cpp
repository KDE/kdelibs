/* This file is part of the KDE libraries
   Copyright (c) 2005-2007 David Jarvie <software@astrojar.org.uk>

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

#include <stdio.h>
#include <qtest_kde.h>
#include <QtCore/QDir>
#include <QtCore/QDate>
#include <QtDBus/QtDBus>
#include "config.h" // for HAVE_TM_GMTOFF
#include "ksystemtimezone.h"
#include "ktzfiletimezone.h"
#include "ktimezonestest.moc"
#include <kconfiggroup.h>

QTEST_KDEMAIN_CORE(KTimeZonesTest)

void KTimeZonesTest::initTestCase()
{
    cleanupTestCase();

    mDataDir = QDir::homePath() + "/.kde-unit-test/ktimezonestest";
    QVERIFY(QDir().mkpath(mDataDir));
    writeZoneTab(false);
    QDir dir(mDataDir);
    QVERIFY(dir.mkdir("Africa"));
    QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Cairo"), mDataDir + QLatin1String("/Africa/Cairo"));
    QVERIFY(dir.mkdir("America"));
    QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Los_Angeles"), mDataDir + QLatin1String("/America/Los_Angeles"));
    QVERIFY(dir.mkdir("Asia"));
    QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Dili"), mDataDir + QLatin1String("/Asia/Dili"));
    QVERIFY(dir.mkdir("Europe"));
    QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Berlin"), mDataDir + QLatin1String("/Europe/Berlin"));
    QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/London"), mDataDir + QLatin1String("/Europe/London"));
    QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Paris"), mDataDir + QLatin1String("/Europe/Paris"));

    KConfig config("ktimezonedrc");
    KConfigGroup group(&config, "TimeZones");
    group.writeEntry("ZoneinfoDir", mDataDir);
    group.writeEntry("Zonetab", mDataDir + QString::fromLatin1("/zone.tab"));
    group.writeEntry("LocalZone", QString::fromLatin1("Europe/Paris"));
    config.sync();
}

void KTimeZonesTest::cleanupTestCase()
{
    removeDir(QLatin1String("ktimezonestest/Africa"));
    removeDir(QLatin1String("ktimezonestest/America"));
    removeDir(QLatin1String("ktimezonestest/Asia"));
    removeDir(QLatin1String("ktimezonestest/Europe"));
    removeDir(QLatin1String("ktimezonestest"));
    removeDir(QLatin1String("share/config"));
    QDir().rmpath(QDir::homePath() + "/.kde-unit-test/share");
}

void KTimeZonesTest::writeZoneTab(bool testcase)
{
    QFile f;
    f.setFileName(mDataDir + QLatin1String("/zone.tab"));
    f.open(QIODevice::WriteOnly);
    QTextStream fStream(&f);
    if (testcase)
        fStream << "DE	+5230+01322	Europe/Berlin\n"
                   "EG	+3003+03115	Africa/Cairo\n"
                   "FR	+4852+00220	Europe/Paris\n"
                   "XX	-512830+0001845	Europe/London	Greater Britain\n"
                   "TL	-0833+12535	Asia/Dili\n"
                   "US	+340308-1181434	America/Los_Angeles	Pacific Time\n";
    else
        fStream << "EG	+3003+03115	Africa/Cairo\n"
                   "FR	+4852+00220	Europe/Paris\n"
                   "GB	+512830-0001845	Europe/London	Great Britain\n"
                   "US	+340308-1181434	America/Los_Angeles	Pacific Time\n";
    f.close();
}

void KTimeZonesTest::removeDir(const QString &subdir)
{
    QDir local = QDir::homePath() + QLatin1String("/.kde-unit-test/") + subdir;
    foreach(const QString &file, local.entryList(QDir::Files))
        if(!local.remove(file))
            qWarning("%s: removing failed", qPrintable( file ));
    QCOMPARE((int)local.entryList(QDir::Files).count(), 0);
    local.cdUp();
    QString subd = subdir;
    subd.remove(QRegExp("^.*/"));
    local.rmpath(subd);
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
    QCOMPARE(local.type(), QByteArray("KSystemTimeZone"));
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

    QCOMPARE(KSystemTimeZones::zones().count(), 4);
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
    writeZoneTab(true);
    QDBusMessage message = QDBusMessage::createSignal("/Daemon", "org.kde.KTimeZoned", "zonetabChanged");
    QList<QVariant> args;
    args += mDataDir + QLatin1String("/zone.tab");
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
    timer.start(1000);
    loop.exec();
    QCOMPARE(KSystemTimeZones::zones().count(), 6);
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
    QVERIFY(KSystemTimeZones::zone("Asia/Dili").isValid());
    QVERIFY(KSystemTimeZones::zone("America/Los_Angeles").isValid());

    // Check that 'london' is automatically updated with the new zone.tab
    // contents, and that the removed zones are deleted from KSystemTimeZones.
    writeZoneTab(false);
    QDBusConnection::sessionBus().send(message);
    timer.start(1000);
    loop.exec();
    QCOMPARE(KSystemTimeZones::zones().count(), 4);
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
    QString tzfile = ":" + mDataDir + "/Europe/Paris";
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
    QVERIFY( losAngeles.isValid() );
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY( london.isValid() );
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QCOMPARE(london.offsetAtUtc(winter), 0);
    QCOMPARE(london.offsetAtUtc(summer), 3600);;
    QCOMPARE(losAngeles.offsetAtUtc(winter), -28800);
    QCOMPARE(losAngeles.offsetAtUtc(summer), -25200);;
}

void KTimeZonesTest::offsetAtZoneTime()
{
    QDateTime aGmt(QDate(2005,3,27), QTime(0,30,0), Qt::LocalTime);
    QDateTime aInvalid(QDate(2005,3,27), QTime(1,30,0), Qt::LocalTime);
    QDateTime aBst(QDate(2005,3,27), QTime(2,30,0), Qt::LocalTime);
    QDateTime bBst(QDate(2005,10,30), QTime(0,30,0), Qt::LocalTime);
    QDateTime bBstBeforeGmt(QDate(2005,10,30), QTime(1,30,0), Qt::LocalTime);
    QDateTime bGmt(QDate(2005,10,30), QTime(2,30,0), Qt::LocalTime);
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY( london.isValid() );
    int offset2;
    QCOMPARE(london.offsetAtZoneTime(aGmt, &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(london.offsetAtZoneTime(aInvalid, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london.offsetAtZoneTime(aBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london.offsetAtZoneTime(bBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london.offsetAtZoneTime(bBstBeforeGmt, &offset2), 3600);
    QCOMPARE(offset2, 0);
    QCOMPARE(london.offsetAtZoneTime(bGmt, &offset2), 0);
    QCOMPARE(offset2, 0);
}

void KTimeZonesTest::abbreviation()
{
    // Fetch time zone abbreviations
    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    QVERIFY( losAngeles.isValid() );
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY( london.isValid() );
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
    QVERIFY( losAngeles.isValid() );
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY( london.isValid() );
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
    QVERIFY( losAngeles.isValid() );
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY( london.isValid() );
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
    QVERIFY( losAngeles.isValid() );
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    QVERIFY( london.isValid() );
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
}

void KTimeZonesTest::tzfileToZoneTime()
{
    // Convert from UTC.
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone london = KTzfileTimeZone(&tzsource, "Europe/London");
    QVERIFY( london.isValid() );
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
    QVERIFY( sysLondon.isValid() );
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

void KTimeZonesTest::tzfileOffsetAtZoneTime()
{
    QDateTime aGmt(QDate(2005,3,27), QTime(0,30,0), Qt::LocalTime);
    QDateTime aInvalid(QDate(2005,3,27), QTime(1,30,0), Qt::LocalTime);
    QDateTime aBst(QDate(2005,3,27), QTime(2,30,0), Qt::LocalTime);
    QDateTime bBst(QDate(2005,10,30), QTime(0,30,0), Qt::LocalTime);
    QDateTime bBstBeforeGmt(QDate(2005,10,30), QTime(1,30,0), Qt::LocalTime);
    QDateTime bGmt(QDate(2005,10,30), QTime(2,30,0), Qt::LocalTime);
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone london = KTzfileTimeZone(&tzsource, "Europe/London");
    QVERIFY( london.isValid() );
    int offset2;
    QCOMPARE(london.offsetAtZoneTime(aGmt, &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(london.offsetAtZoneTime(aInvalid, &offset2), KTimeZone::InvalidOffset);
    QCOMPARE(offset2, KTimeZone::InvalidOffset);
    QCOMPARE(london.offsetAtZoneTime(aBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london.offsetAtZoneTime(bBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(london.offsetAtZoneTime(bBstBeforeGmt, &offset2), 3600);
    QCOMPARE(offset2, 0);
    QCOMPARE(london.offsetAtZoneTime(bGmt, &offset2), 0);
    QCOMPARE(offset2, 0);

    KTimeZone sysLondon = KSystemTimeZones::readZone("Europe/London");
    QVERIFY( sysLondon.isValid() );
    QCOMPARE(sysLondon.offsetAtZoneTime(aGmt, &offset2), 0);
    QCOMPARE(offset2, 0);
    QCOMPARE(sysLondon.offsetAtZoneTime(aInvalid, &offset2), KTimeZone::InvalidOffset);
    QCOMPARE(offset2, KTimeZone::InvalidOffset);
    QCOMPARE(sysLondon.offsetAtZoneTime(aBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(sysLondon.offsetAtZoneTime(bBst, &offset2), 3600);
    QCOMPARE(offset2, 3600);
    QCOMPARE(sysLondon.offsetAtZoneTime(bBstBeforeGmt, &offset2), 3600);
    QCOMPARE(offset2, 0);
    QCOMPARE(sysLondon.offsetAtZoneTime(bGmt, &offset2), 0);
    QCOMPARE(offset2, 0);
}

void KTimeZonesTest::tzfileUtcOffsets()
{
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone london = KTzfileTimeZone(&tzsource, "Europe/London");
    QVERIFY( london.isValid() );
    QList<int> offsets = london.utcOffsets();
    QCOMPARE(offsets.count(), 3);
    QCOMPARE(offsets[0], 0);    // GMT
    QCOMPARE(offsets[1], 3600); // BST
    QCOMPARE(offsets[2], 7200); // DST

    KTimeZone sysLondon = KSystemTimeZones::readZone("Europe/London");
    QVERIFY( sysLondon.isValid() );
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
    QVERIFY( london.isValid() );
    QDateTime winter(QDate(2005,1,1), QTime(0,0,0), Qt::UTC);
    QDateTime summer(QDate(2005,6,1), QTime(0,0,0), Qt::UTC);
    QDateTime standard(QDate(1970,4,30), QTime(12,45,16,25), Qt::UTC);
    QString sResult = london.abbreviation(winter);
    QCOMPARE(london.abbreviation(winter), QByteArray("GMT"));
    QCOMPARE(london.abbreviation(summer), QByteArray("BST"));
    QCOMPARE(london.abbreviation(standard), QByteArray("BST"));

    KTimeZone sysLondon = KSystemTimeZones::readZone("Europe/London");
    QVERIFY( sysLondon.isValid() );
    sResult = sysLondon.abbreviation(winter);
    QCOMPARE(sysLondon.abbreviation(winter), QByteArray("GMT"));
    QCOMPARE(sysLondon.abbreviation(summer), QByteArray("BST"));
    QCOMPARE(sysLondon.abbreviation(standard), QByteArray("BST"));
}

void KTimeZonesTest::tzfileTransitions()
{
    KTzfileTimeZoneSource tzsource(KSystemTimeZones::zoneinfoDir());
    KTimeZone london = KTzfileTimeZone(&tzsource, "Europe/London");
    QVERIFY( london.isValid() );
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
