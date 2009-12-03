/* This file is part of the KDE libraries
   Copyright (c) 2005,2006 David Jarvie <software@astrojar.org.uk>

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

#include <cstdlib>
#include <qtest_kde.h>
#include <QtCore/QDir>
#include <kglobal.h>
#include <klocale.h>
#include <kcalendarsystem.h>
#include <ksystemtimezone.h>
#include <kdatetime.h>
#include <kconfiggroup.h>
#include <QtDBus/QtDBus>

#include "kdatetimetest.moc"

//TODO: test new methods

QTEST_KDEMAIN_CORE(KDateTimeTest)

extern KDECORE_EXPORT int KDateTime_utcCacheHit;
extern KDECORE_EXPORT int KDateTime_zoneCacheHit;


void KDateTimeTest::initTestCase()
{
    cleanupTestCase();

    mDataDir = QDir::homePath() + "/.kde-unit-test/kdatetimetest";
    QVERIFY(QDir().mkpath(mDataDir));
    QFile f;
    f.setFileName(mDataDir + QLatin1String("/zone.tab"));
    f.open(QIODevice::WriteOnly);
    QTextStream fStream(&f);
    fStream << "DE      +5230+01322     Europe/Berlin\n"
               "EG	+3003+03115	Africa/Cairo\n"
               "FR	+4852+00220	Europe/Paris\n"
               "GB	+512830-0001845	Europe/London	Great Britain\n"
               "US	+340308-1181434	America/Los_Angeles	Pacific Time\n";
    f.close();
    QDir dir(mDataDir);
    QVERIFY(dir.mkdir("Africa"));
    QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Cairo"), mDataDir + QLatin1String("/Africa/Cairo"));
    QVERIFY(dir.mkdir("America"));
    QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Los_Angeles"), mDataDir + QLatin1String("/America/Los_Angeles"));
    QVERIFY(dir.mkdir("Europe"));
    QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Berlin"), mDataDir + QLatin1String("/Europe/Berlin"));
    QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/London"), mDataDir + QLatin1String("/Europe/London"));
    QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Paris"), mDataDir + QLatin1String("/Europe/Paris"));

    KConfig config("ktimezonedrc");
    KConfigGroup group(&config, "TimeZones");
    group.writeEntry("ZoneinfoDir", mDataDir);
    group.writeEntry("Zonetab", mDataDir + QString::fromLatin1("/zone.tab"));
    group.writeEntry("LocalZone", QString::fromLatin1("America/Los_Angeles"));
    config.sync();
}

void KDateTimeTest::cleanupTestCase()
{
    removeDir(QLatin1String("kdatetimetest/Africa"));
    removeDir(QLatin1String("kdatetimetest/America"));
    removeDir(QLatin1String("kdatetimetest/Europe"));
    removeDir(QLatin1String("kdatetimetest"));
    removeDir(QLatin1String("share/config"));
    QDir().rmpath(QDir::homePath() + "/.kde-unit-test/share");
}

void KDateTimeTest::removeDir(const QString &subdir)
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


////////////////////////////////////////////////////////////////////////
// KDateTime::Spec constructors and basic property information methods,
// and the static convenience instances/methods.
////////////////////////////////////////////////////////////////////////

void KDateTimeTest::specConstructors()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");

    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":Europe/London", 1);
    ::tzset();

    // Ensure that local time is different from UTC and different from 'london'
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();


    // Default constructor
    KDateTime::Spec invalid;
    QVERIFY(!invalid.isValid());
    QCOMPARE(invalid.type(), KDateTime::Invalid);
    QVERIFY(!invalid.isLocalZone());
    QVERIFY(!invalid.isUtc());
    QVERIFY(!invalid.isOffsetFromUtc());
    QVERIFY(!invalid.isClockTime());
    QCOMPARE(invalid.utcOffset(), 0);
    QVERIFY(!invalid.timeZone().isValid());


    // Time zone
    KDateTime::Spec tz(london);
    QVERIFY(tz.isValid());
    QCOMPARE(tz.type(), KDateTime::TimeZone);
    QVERIFY(!tz.isUtc());
    QVERIFY(!tz.isOffsetFromUtc());
    QVERIFY(!tz.isLocalZone());
    QVERIFY(!tz.isClockTime());
    QCOMPARE(tz.utcOffset(), 0);
    QCOMPARE(tz.timeZone(), london);

    KDateTime::Spec tzLocal(losAngeles);
    QVERIFY(tzLocal.isValid());
    QCOMPARE(tzLocal.type(), KDateTime::TimeZone);
    QCOMPARE(tzLocal, KDateTime::Spec(KDateTime::LocalZone));
    QVERIFY(!tzLocal.isUtc());
    QVERIFY(!tzLocal.isOffsetFromUtc());
    QVERIFY(tzLocal.isLocalZone());
    QVERIFY(!tzLocal.isClockTime());
    QCOMPARE(tzLocal.utcOffset(), 0);
    QCOMPARE(tzLocal.timeZone(), losAngeles);

    // ... copy constructor
    KDateTime::Spec tzCopy(tz);
    QVERIFY(tzCopy.isValid());
    QCOMPARE(tzCopy.type(), KDateTime::TimeZone);
    QVERIFY(!tzCopy.isUtc());
    QVERIFY(!tzCopy.isOffsetFromUtc());
    QVERIFY(!tzCopy.isLocalZone());
    QVERIFY(!tzCopy.isClockTime());
    QCOMPARE(tzCopy.utcOffset(), 0);
    QCOMPARE(tzCopy.timeZone(), london);


    // Local time zone
    KDateTime::Spec local(KDateTime::LocalZone);
    QVERIFY(local.isValid());
    QCOMPARE(local.type(), KDateTime::TimeZone);
    QCOMPARE(local, KDateTime::Spec(KDateTime::LocalZone));
    QVERIFY(!local.isUtc());
    QVERIFY(!local.isOffsetFromUtc());
    QVERIFY(local.isLocalZone());
    QVERIFY(!local.isClockTime());
    QCOMPARE(local.utcOffset(), 0);
    QCOMPARE(local.timeZone(), KSystemTimeZones::local());

    KDateTime::Spec localx(KDateTime::Spec(KDateTime::LocalZone, 2*3600));
    QVERIFY(localx.isValid());
    QCOMPARE(localx.type(), KDateTime::TimeZone);
    QCOMPARE(localx, KDateTime::Spec(KDateTime::LocalZone));
    QVERIFY(!localx.isUtc());
    QVERIFY(!localx.isOffsetFromUtc());
    QVERIFY(localx.isLocalZone());
    QVERIFY(!localx.isClockTime());
    QCOMPARE(localx.utcOffset(), 0);
    QCOMPARE(localx.timeZone(), KSystemTimeZones::local());

    KDateTime::Spec local2 = KDateTime::Spec::LocalZone();
    QVERIFY(local2.isValid());
    QCOMPARE(local2.type(), KDateTime::TimeZone);
    QCOMPARE(local2, KDateTime::Spec(KDateTime::LocalZone));
    QVERIFY(!local2.isUtc());
    QVERIFY(!local2.isOffsetFromUtc());
    QVERIFY(local2.isLocalZone());
    QVERIFY(!local2.isClockTime());
    QCOMPARE(local2.utcOffset(), 0);
    QCOMPARE(local2.timeZone(), KSystemTimeZones::local());

    // ... copy constructor
    KDateTime::Spec localCopy(local);
    QVERIFY(localCopy.isValid());
    QCOMPARE(localCopy.type(), KDateTime::TimeZone);
    QCOMPARE(localCopy, KDateTime::Spec(KDateTime::LocalZone));
    QVERIFY(!localCopy.isUtc());
    QVERIFY(!localCopy.isOffsetFromUtc());
    QVERIFY(localCopy.isLocalZone());
    QVERIFY(!localCopy.isClockTime());
    QCOMPARE(localCopy.utcOffset(), 0);
    QCOMPARE(localCopy.timeZone(), losAngeles);


    // UTC
    KDateTime::Spec utc(KDateTime::UTC);
    QVERIFY(utc.isValid());
    QCOMPARE(utc.type(), KDateTime::UTC);
    QVERIFY(utc.isUtc());
    QVERIFY(!utc.isOffsetFromUtc());
    QVERIFY(!utc.isLocalZone());
    QVERIFY(!utc.isClockTime());
    QCOMPARE(utc.utcOffset(), 0);
    QCOMPARE(utc.timeZone(), KTimeZone::utc());

    KDateTime::Spec utcx(KDateTime::UTC, 2*3600);
    QVERIFY(utcx.isValid());
    QCOMPARE(utcx.type(), KDateTime::UTC);
    QVERIFY(utcx.isUtc());
    QVERIFY(!utcx.isOffsetFromUtc());
    QVERIFY(!utcx.isLocalZone());
    QVERIFY(!utcx.isClockTime());
    QCOMPARE(utcx.utcOffset(), 0);
    QCOMPARE(utcx.timeZone(), KTimeZone::utc());

    const KDateTime::Spec& utc2 = KDateTime::Spec::UTC();
    QVERIFY(utc2.isValid());
    QCOMPARE(utc2.type(), KDateTime::UTC);
    QVERIFY(utc2.isUtc());
    QVERIFY(!utc2.isOffsetFromUtc());
    QVERIFY(!utc2.isLocalZone());
    QVERIFY(!utc2.isClockTime());
    QCOMPARE(utc2.utcOffset(), 0);
    QCOMPARE(utc2.timeZone(), KTimeZone::utc());

    // ... copy constructor
    KDateTime::Spec utcCopy(utc);
    QVERIFY(utcCopy.isValid());
    QCOMPARE(utcCopy.type(), KDateTime::UTC);
    QVERIFY(utcCopy.isUtc());
    QVERIFY(!utcCopy.isOffsetFromUtc());
    QVERIFY(!utcCopy.isLocalZone());
    QVERIFY(!utcCopy.isClockTime());
    QCOMPARE(utcCopy.utcOffset(), 0);
    QCOMPARE(utcCopy.timeZone(), KTimeZone::utc());


    // Offset from UTC
    KDateTime::Spec offset0(KDateTime::OffsetFromUTC);
    QVERIFY(offset0.isValid());
    QCOMPARE(offset0.type(), KDateTime::OffsetFromUTC);
    QVERIFY(offset0.isUtc());
    QVERIFY(offset0.isOffsetFromUtc());
    QVERIFY(!offset0.isLocalZone());
    QVERIFY(!offset0.isClockTime());
    QCOMPARE(offset0.utcOffset(), 0);
    QVERIFY(!offset0.timeZone().isValid());

    KDateTime::Spec offset(KDateTime::Spec(KDateTime::OffsetFromUTC, -2*3600));
    QVERIFY(offset.isValid());
    QCOMPARE(offset.type(), KDateTime::OffsetFromUTC);
    QVERIFY(!offset.isUtc());
    QVERIFY(offset.isOffsetFromUtc());
    QVERIFY(!offset.isLocalZone());
    QVERIFY(!offset.isClockTime());
    QCOMPARE(offset.utcOffset(), -2*3600);
    QVERIFY(!offset.timeZone().isValid());

    KDateTime::Spec offset2 = KDateTime::Spec::OffsetFromUTC(2*3600);
    QVERIFY(offset2.isValid());
    QCOMPARE(offset2.type(), KDateTime::OffsetFromUTC);
    QVERIFY(!offset2.isUtc());
    QVERIFY(offset2.isOffsetFromUtc());
    QVERIFY(!offset2.isLocalZone());
    QVERIFY(!offset2.isClockTime());
    QCOMPARE(offset2.utcOffset(), 2*3600);
    QVERIFY(!offset2.timeZone().isValid());

    // ... copy constructor
    KDateTime::Spec offsetCopy(offset);
    QVERIFY(offsetCopy.isValid());
    QCOMPARE(offsetCopy.type(), KDateTime::OffsetFromUTC);
    QVERIFY(!offsetCopy.isUtc());
    QVERIFY(offsetCopy.isOffsetFromUtc());
    QVERIFY(!offsetCopy.isLocalZone());
    QVERIFY(!offsetCopy.isClockTime());
    QCOMPARE(offsetCopy.utcOffset(), -2*3600);
    QVERIFY(!offsetCopy.timeZone().isValid());


    // Local clock time
    KDateTime::Spec clock(KDateTime::ClockTime);
    QVERIFY(clock.isValid());
    QCOMPARE(clock.type(), KDateTime::ClockTime);
    QVERIFY(!clock.isUtc());
    QVERIFY(!clock.isOffsetFromUtc());
    QVERIFY(!clock.isLocalZone());
    QVERIFY(clock.isClockTime());
    QCOMPARE(clock.utcOffset(), 0);
    QVERIFY(!clock.timeZone().isValid());

    KDateTime::Spec clockx(KDateTime::Spec(KDateTime::ClockTime, 2*3600));
    QVERIFY(clockx.isValid());
    QCOMPARE(clockx.type(), KDateTime::ClockTime);
    QVERIFY(!clockx.isUtc());
    QVERIFY(!clockx.isOffsetFromUtc());
    QVERIFY(!clockx.isLocalZone());
    QVERIFY(clockx.isClockTime());
    QCOMPARE(clockx.utcOffset(), 0);
    QVERIFY(!clockx.timeZone().isValid());

    KDateTime::Spec clock2 = KDateTime::Spec::ClockTime();
    QVERIFY(clock2.isValid());
    QCOMPARE(clock2.type(), KDateTime::ClockTime);
    QVERIFY(!clock2.isUtc());
    QVERIFY(!clock2.isOffsetFromUtc());
    QVERIFY(!clock2.isLocalZone());
    QVERIFY(clock2.isClockTime());
    QCOMPARE(clock2.utcOffset(), 0);
    QVERIFY(!clock2.timeZone().isValid());

    // ... copy constructor
    KDateTime::Spec clockCopy(clock);
    QVERIFY(clockCopy.isValid());
    QCOMPARE(clockCopy.type(), KDateTime::ClockTime);
    QVERIFY(!clockCopy.isUtc());
    QVERIFY(!clockCopy.isOffsetFromUtc());
    QVERIFY(!clockCopy.isLocalZone());
    QVERIFY(clockCopy.isClockTime());
    QCOMPARE(clockCopy.utcOffset(), 0);
    QVERIFY(!clockCopy.timeZone().isValid());


    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

////////////////////////////////////////////////////////////////////////
// KDateTime::Spec setType(), operator==(), etc.
////////////////////////////////////////////////////////////////////////

void KDateTimeTest::specSet()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    KDateTime::Spec spec;
    QCOMPARE(spec.type(), KDateTime::Invalid);

    spec.setType(KDateTime::OffsetFromUTC, 7200);
    QCOMPARE(spec.type(), KDateTime::OffsetFromUTC);
    QVERIFY(spec.equivalentTo(KDateTime::Spec::OffsetFromUTC(7200)));
    QVERIFY(!spec.equivalentTo(KDateTime::Spec::OffsetFromUTC(0)));
    QVERIFY(spec == KDateTime::Spec::OffsetFromUTC(7200));
    QVERIFY(!(spec != KDateTime::Spec::OffsetFromUTC(7200)));
    QVERIFY(spec != KDateTime::Spec::OffsetFromUTC(-7200));
    QVERIFY(spec != KDateTime::Spec(london));

    spec.setType(KDateTime::OffsetFromUTC, 0);
    QCOMPARE(spec.type(), KDateTime::OffsetFromUTC);
    QVERIFY(spec.equivalentTo(KDateTime::Spec::OffsetFromUTC(0)));
    QVERIFY(spec.equivalentTo(KDateTime::Spec::UTC()));
    QVERIFY(!spec.equivalentTo(KDateTime::Spec::OffsetFromUTC(7200)));
    QVERIFY(spec == KDateTime::Spec::OffsetFromUTC(0));
    QVERIFY(!(spec != KDateTime::Spec::OffsetFromUTC(0)));
    QVERIFY(spec != KDateTime::Spec::OffsetFromUTC(-7200));
    QVERIFY(spec != KDateTime::Spec(london));

    spec.setType(london);
    QCOMPARE(spec.type(), KDateTime::TimeZone);
    QVERIFY(spec.equivalentTo(KDateTime::Spec(london)));
    QVERIFY(spec == KDateTime::Spec(london));
    QVERIFY(!(spec != KDateTime::Spec(london)));
    QVERIFY(spec != KDateTime::Spec::OffsetFromUTC(0));
    QVERIFY(!spec.equivalentTo(KDateTime::Spec::OffsetFromUTC(0)));

    spec.setType(KDateTime::LocalZone);
    QCOMPARE(spec.type(), KDateTime::TimeZone);
    QVERIFY(spec.equivalentTo(KDateTime::Spec::LocalZone()));
    QVERIFY(spec == KDateTime::Spec::LocalZone());
    QVERIFY(!(spec != KDateTime::Spec::LocalZone()));
    QVERIFY(spec.equivalentTo(KDateTime::Spec(losAngeles)));
    QVERIFY(spec == KDateTime::Spec(losAngeles));
    QVERIFY(spec != KDateTime::Spec(london));
    QVERIFY(!spec.equivalentTo(KDateTime::Spec(london)));

    spec.setType(KDateTime::UTC);
    QCOMPARE(spec.type(), KDateTime::UTC);
    QVERIFY(spec.equivalentTo(KDateTime::Spec::UTC()));
    QVERIFY(spec == KDateTime::Spec::UTC());
    QVERIFY(!(spec != KDateTime::Spec::UTC()));
    QVERIFY(spec != KDateTime::Spec::LocalZone());
    QVERIFY(!spec.equivalentTo(KDateTime::Spec::LocalZone()));
    QVERIFY(spec.equivalentTo(KDateTime::Spec::OffsetFromUTC(0)));

    spec.setType(KDateTime::ClockTime);
    QCOMPARE(spec.type(), KDateTime::ClockTime);
    QVERIFY(spec.equivalentTo(KDateTime::Spec::ClockTime()));
    QVERIFY(spec == KDateTime::Spec::ClockTime());
    QVERIFY(!(spec != KDateTime::Spec::ClockTime()));
    QVERIFY(spec != KDateTime::Spec::UTC());
    QVERIFY(!spec.equivalentTo(KDateTime::Spec::UTC()));

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}


//////////////////////////////////////////////////////
// Constructors and basic property information methods
//////////////////////////////////////////////////////

void KDateTimeTest::constructors()
{
    QDate d(2001,2,13);
    QTime t(3,45,14);
    QDateTime dtLocal(d, t, Qt::LocalTime);
    QDateTime dtUTC(d, t, Qt::UTC);
    KTimeZone london = KSystemTimeZones::zone("Europe/London");

    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":Europe/London", 1);
    ::tzset();
    QDateTime dtUTCtoLondon = dtUTC.toLocalTime();

    // Ensure that local time is different from UTC and different from 'london'
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();


    // Default constructor
    KDateTime deflt;
    QVERIFY(deflt.isNull());
    QVERIFY(!deflt.isValid());


    // No time zone or timeSpec explicitly specified
    KDateTime datetimeL(dtLocal);
    QVERIFY(!datetimeL.isNull());
    QVERIFY(datetimeL.isValid());
    QVERIFY(!datetimeL.isDateOnly());
    QCOMPARE(datetimeL.timeType(), KDateTime::TimeZone);
    QCOMPARE(datetimeL.timeSpec(), KDateTime::Spec::LocalZone());
    QVERIFY(datetimeL.isLocalZone());
    QVERIFY(!datetimeL.isUtc());
    QVERIFY(!datetimeL.isOffsetFromUtc());
    QVERIFY(!datetimeL.isClockTime());
    QCOMPARE(datetimeL.utcOffset(), -8*3600);
    QCOMPARE(datetimeL.timeZone(), KSystemTimeZones::local());
    QCOMPARE(datetimeL.dateTime(), dtLocal);

    KDateTime datetimeU(dtUTC);
    QVERIFY(!datetimeU.isNull());
    QVERIFY(datetimeU.isValid());
    QVERIFY(!datetimeU.isDateOnly());
    QCOMPARE(datetimeU.timeType(), KDateTime::UTC);
    QVERIFY(!datetimeU.isLocalZone());
    QVERIFY(datetimeU.isUtc());
    QVERIFY(!datetimeU.isOffsetFromUtc());
    QVERIFY(!datetimeU.isClockTime());
    QCOMPARE(datetimeU.utcOffset(), 0);
    QCOMPARE(datetimeU.timeZone(), KTimeZone::utc());
    QCOMPARE(datetimeU.dateTime(), dtUTC);


    // Time zone
    KDateTime dateTz(d, london);
    QVERIFY(!dateTz.isNull());
    QVERIFY(dateTz.isValid());
    QVERIFY(dateTz.isDateOnly());
    QCOMPARE(dateTz.timeType(), KDateTime::TimeZone);
    QVERIFY(!dateTz.isUtc());
    QVERIFY(!dateTz.isOffsetFromUtc());
    QVERIFY(!dateTz.isLocalZone());
    QVERIFY(!dateTz.isClockTime());
    QCOMPARE(dateTz.utcOffset(), 0);
    QCOMPARE(dateTz.timeZone(), london);
    QCOMPARE(dateTz.dateTime(), QDateTime(d, QTime(0,0,0), Qt::LocalTime));
    QCOMPARE(dateTz.date(), d);

    KDateTime dateTimeTz(d, QTime(3,45,14), london);
    QVERIFY(!dateTimeTz.isNull());
    QVERIFY(dateTimeTz.isValid());
    QVERIFY(!dateTimeTz.isDateOnly());
    QCOMPARE(dateTimeTz.timeType(), KDateTime::TimeZone);
    QVERIFY(!dateTimeTz.isUtc());
    QVERIFY(!dateTimeTz.isOffsetFromUtc());
    QVERIFY(!dateTimeTz.isLocalZone());
    QVERIFY(!dateTimeTz.isClockTime());
    QCOMPARE(dateTimeTz.utcOffset(), 0);
    QCOMPARE(dateTimeTz.timeZone(), london);
    QCOMPARE(dateTimeTz.dateTime(), QDateTime(d, QTime(3,45,14), Qt::LocalTime));
    QCOMPARE(dateTimeTz.time(), QTime(3,45,14));

    KDateTime datetimeTz(dtLocal, london);
    QVERIFY(!datetimeTz.isNull());
    QVERIFY(datetimeTz.isValid());
    QVERIFY(!dateTimeTz.isDateOnly());
    QCOMPARE(datetimeTz.timeType(), KDateTime::TimeZone);
    QVERIFY(!datetimeTz.isUtc());
    QVERIFY(!datetimeTz.isOffsetFromUtc());
    QVERIFY(!datetimeTz.isLocalZone());
    QVERIFY(!datetimeTz.isClockTime());
    QCOMPARE(datetimeTz.utcOffset(), 0);
    QCOMPARE(datetimeTz.timeZone(), london);
    QCOMPARE(datetimeTz.dateTime(), dtLocal);

    KDateTime datetimeTz2(dtUTC, london);
    QVERIFY(!datetimeTz2.isNull());
    QVERIFY(datetimeTz2.isValid());
    QVERIFY(!dateTimeTz.isDateOnly());
    QCOMPARE(datetimeTz2.timeType(), KDateTime::TimeZone);
    QVERIFY(!datetimeTz2.isUtc());
    QVERIFY(!datetimeTz2.isOffsetFromUtc());
    QVERIFY(!datetimeTz2.isLocalZone());
    QVERIFY(!datetimeTz2.isClockTime());
    QCOMPARE(datetimeTz2.utcOffset(), 0);
    QCOMPARE(datetimeTz2.timeZone(), london);
    QCOMPARE(datetimeTz2.dateTime(), dtUTCtoLondon);

    // ... copy constructor
    KDateTime datetimeTzCopy(datetimeTz);
    QVERIFY(!datetimeTzCopy.isNull());
    QVERIFY(datetimeTzCopy.isValid());
    QVERIFY(!dateTimeTz.isDateOnly());
    QCOMPARE(datetimeTzCopy.timeType(), KDateTime::TimeZone);
    QVERIFY(!datetimeTzCopy.isUtc());
    QVERIFY(!datetimeTzCopy.isOffsetFromUtc());
    QVERIFY(!datetimeTzCopy.isLocalZone());
    QVERIFY(!datetimeTzCopy.isClockTime());
    QCOMPARE(datetimeTzCopy.utcOffset(), 0);
    QCOMPARE(datetimeTzCopy.timeZone(), datetimeTz.timeZone());
    QCOMPARE(datetimeTzCopy.dateTime(), datetimeTz.dateTime());

    // UTC
    KDateTime date_UTC(d, KDateTime::Spec::UTC());
    QVERIFY(!date_UTC.isNull());
    QVERIFY(date_UTC.isValid());
    QVERIFY(date_UTC.isDateOnly());
    QCOMPARE(date_UTC.timeType(), KDateTime::UTC);
    QVERIFY(date_UTC.isUtc());
    QVERIFY(!date_UTC.isOffsetFromUtc());
    QVERIFY(!date_UTC.isLocalZone());
    QVERIFY(!date_UTC.isClockTime());
    QCOMPARE(date_UTC.utcOffset(), 0);
    QCOMPARE(date_UTC.timeZone(), KTimeZone::utc());
    QCOMPARE(date_UTC.dateTime(), QDateTime(d, QTime(0,0,0), Qt::UTC));

    KDateTime dateTime_UTC(d, t, KDateTime::UTC);
    QVERIFY(!dateTime_UTC.isNull());
    QVERIFY(dateTime_UTC.isValid());
    QVERIFY(!dateTime_UTC.isDateOnly());
    QCOMPARE(dateTime_UTC.timeType(), KDateTime::UTC);
    QVERIFY(dateTime_UTC.isUtc());
    QVERIFY(!dateTime_UTC.isOffsetFromUtc());
    QVERIFY(!dateTime_UTC.isLocalZone());
    QVERIFY(!dateTime_UTC.isClockTime());
    QCOMPARE(dateTime_UTC.utcOffset(), 0);
    QCOMPARE(dateTime_UTC.timeZone(), KTimeZone::utc());
    QCOMPARE(dateTime_UTC.dateTime(), QDateTime(d, t, Qt::UTC));

    KDateTime datetime_UTC(dtLocal, KDateTime::UTC);
    QVERIFY(!datetime_UTC.isNull());
    QVERIFY(datetime_UTC.isValid());
    QVERIFY(!datetime_UTC.isDateOnly());
    QCOMPARE(datetime_UTC.timeType(), KDateTime::UTC);
    QVERIFY(datetime_UTC.isUtc());
    QVERIFY(!datetime_UTC.isOffsetFromUtc());
    QVERIFY(!datetime_UTC.isLocalZone());
    QVERIFY(!datetime_UTC.isClockTime());
    QCOMPARE(datetime_UTC.utcOffset(), 0);
    QCOMPARE(datetime_UTC.timeZone(), KTimeZone::utc());
    QCOMPARE(datetime_UTC.dateTime(), dtLocal.toUTC());

    KDateTime datetime_UTC2(dtUTC, KDateTime::UTC);
    QVERIFY(!datetime_UTC2.isNull());
    QVERIFY(datetime_UTC2.isValid());
    QVERIFY(!datetime_UTC2.isDateOnly());
    QCOMPARE(datetime_UTC2.timeType(), KDateTime::UTC);
    QVERIFY(datetime_UTC2.isUtc());
    QVERIFY(!datetime_UTC2.isOffsetFromUtc());
    QVERIFY(!datetime_UTC2.isLocalZone());
    QVERIFY(!datetime_UTC2.isClockTime());
    QCOMPARE(datetime_UTC2.utcOffset(), 0);
    QCOMPARE(datetime_UTC2.timeZone(), KTimeZone::utc());
    QCOMPARE(datetime_UTC2.dateTime(), dtUTC);
    QCOMPARE(datetime_UTC2.date(), dtUTC.date());
    QCOMPARE(datetime_UTC2.time(), dtUTC.time());

    // ... copy constructor
    KDateTime datetime_UTCCopy(datetime_UTC);
    QVERIFY(!datetime_UTCCopy.isNull());
    QVERIFY(datetime_UTCCopy.isValid());
    QVERIFY(!datetime_UTCCopy.isDateOnly());
    QCOMPARE(datetime_UTCCopy.timeType(), KDateTime::UTC);
    QVERIFY(datetime_UTCCopy.isUtc());
    QVERIFY(!datetime_UTCCopy.isOffsetFromUtc());
    QVERIFY(!datetime_UTCCopy.isLocalZone());
    QVERIFY(!datetime_UTCCopy.isClockTime());
    QCOMPARE(datetime_UTCCopy.utcOffset(), 0);
    QCOMPARE(datetime_UTCCopy.timeZone(), datetime_UTC.timeZone());
    QCOMPARE(datetime_UTCCopy.dateTime(), datetime_UTC.dateTime());


    // Offset from UTC
    KDateTime date_OffsetFromUTC(d, KDateTime::Spec::OffsetFromUTC(-2*3600));
    QVERIFY(!date_OffsetFromUTC.isNull());
    QVERIFY(date_OffsetFromUTC.isValid());
    QVERIFY(date_OffsetFromUTC.isDateOnly());
    QCOMPARE(date_OffsetFromUTC.timeType(), KDateTime::OffsetFromUTC);
    QVERIFY(!date_OffsetFromUTC.isUtc());
    QVERIFY(date_OffsetFromUTC.isOffsetFromUtc());
    QVERIFY(!date_OffsetFromUTC.isLocalZone());
    QVERIFY(!date_OffsetFromUTC.isClockTime());
    QCOMPARE(date_OffsetFromUTC.utcOffset(), -2*3600);
    QVERIFY(!date_OffsetFromUTC.timeZone().isValid());
    QCOMPARE(date_OffsetFromUTC.dateTime(), QDateTime(d, QTime(0,0,0), Qt::LocalTime));

    KDateTime dateTime_OffsetFromUTC(d, t, KDateTime::Spec::OffsetFromUTC(2*3600));
    QVERIFY(!dateTime_OffsetFromUTC.isNull());
    QVERIFY(dateTime_OffsetFromUTC.isValid());
    QVERIFY(!dateTime_OffsetFromUTC.isDateOnly());
    QCOMPARE(dateTime_OffsetFromUTC.timeType(), KDateTime::OffsetFromUTC);
    QVERIFY(!dateTime_OffsetFromUTC.isUtc());
    QVERIFY(dateTime_OffsetFromUTC.isOffsetFromUtc());
    QVERIFY(!dateTime_OffsetFromUTC.isLocalZone());
    QVERIFY(!dateTime_OffsetFromUTC.isClockTime());
    QCOMPARE(dateTime_OffsetFromUTC.utcOffset(), 2*3600);
    QVERIFY(!dateTime_OffsetFromUTC.timeZone().isValid());
    QCOMPARE(dateTime_OffsetFromUTC.dateTime(), QDateTime(d, t, Qt::LocalTime));

    KDateTime datetime_OffsetFromUTC(dtLocal, KDateTime::Spec::OffsetFromUTC(-2*3600));
    QVERIFY(!datetime_OffsetFromUTC.isNull());
    QVERIFY(datetime_OffsetFromUTC.isValid());
    QVERIFY(!datetime_OffsetFromUTC.isDateOnly());
    QCOMPARE(datetime_OffsetFromUTC.timeType(), KDateTime::OffsetFromUTC);
    QVERIFY(!datetime_OffsetFromUTC.isUtc());
    QVERIFY(datetime_OffsetFromUTC.isOffsetFromUtc());
    QVERIFY(!datetime_OffsetFromUTC.isLocalZone());
    QVERIFY(!datetime_OffsetFromUTC.isClockTime());
    QCOMPARE(datetime_OffsetFromUTC.utcOffset(), -2*3600);
    QVERIFY(!datetime_OffsetFromUTC.timeZone().isValid());
    QCOMPARE(datetime_OffsetFromUTC.dateTime(), dtLocal);
    QCOMPARE(datetime_OffsetFromUTC.date(), dtLocal.date());
    QCOMPARE(datetime_OffsetFromUTC.time(), dtLocal.time());

    KDateTime datetime_OffsetFromUTC2(dtUTC, KDateTime::Spec::OffsetFromUTC(2*3600));
    QVERIFY(!datetime_OffsetFromUTC2.isNull());
    QVERIFY(datetime_OffsetFromUTC2.isValid());
    QVERIFY(!datetime_OffsetFromUTC2.isDateOnly());
    QCOMPARE(datetime_OffsetFromUTC2.timeType(), KDateTime::OffsetFromUTC);
    QVERIFY(!datetime_OffsetFromUTC2.isUtc());
    QVERIFY(datetime_OffsetFromUTC2.isOffsetFromUtc());
    QVERIFY(!datetime_OffsetFromUTC2.isLocalZone());
    QVERIFY(!datetime_OffsetFromUTC2.isClockTime());
    QCOMPARE(datetime_OffsetFromUTC2.utcOffset(), 2*3600);
    QVERIFY(!datetime_OffsetFromUTC2.timeZone().isValid());
    QDateTime dtof = dtUTC.addSecs(2*3600);
    dtof.setTimeSpec(Qt::LocalTime);
    QCOMPARE(datetime_OffsetFromUTC2.dateTime(), dtof);

    // ... copy constructor
    KDateTime datetime_OffsetFromUTCCopy(datetime_OffsetFromUTC);
    QVERIFY(!datetime_OffsetFromUTCCopy.isNull());
    QVERIFY(datetime_OffsetFromUTCCopy.isValid());
    QVERIFY(!datetime_OffsetFromUTCCopy.isDateOnly());
    QCOMPARE(datetime_OffsetFromUTCCopy.timeType(), KDateTime::OffsetFromUTC);
    QVERIFY(!datetime_OffsetFromUTCCopy.isUtc());
    QVERIFY(datetime_OffsetFromUTCCopy.isOffsetFromUtc());
    QVERIFY(!datetime_OffsetFromUTCCopy.isLocalZone());
    QVERIFY(!datetime_OffsetFromUTCCopy.isClockTime());
    QCOMPARE(datetime_OffsetFromUTCCopy.utcOffset(), -2*3600);
    QVERIFY(!datetime_OffsetFromUTCCopy.timeZone().isValid());
    QCOMPARE(datetime_OffsetFromUTCCopy.dateTime(), datetime_OffsetFromUTC.dateTime());


    // Local time zone
    KDateTime date_LocalZone(d, KDateTime::Spec::LocalZone());
    QVERIFY(!date_LocalZone.isNull());
    QVERIFY(date_LocalZone.isValid());
    QVERIFY(date_LocalZone.isDateOnly());
    QCOMPARE(date_LocalZone.timeType(), KDateTime::TimeZone);
    QCOMPARE(date_LocalZone.timeSpec(), KDateTime::Spec::LocalZone());
    QVERIFY(!date_LocalZone.isUtc());
    QVERIFY(!date_LocalZone.isOffsetFromUtc());
    QVERIFY(date_LocalZone.isLocalZone());
    QVERIFY(!date_LocalZone.isClockTime());
    QCOMPARE(date_LocalZone.utcOffset(), -8*3600);
    QCOMPARE(date_LocalZone.timeZone(), KSystemTimeZones::local());
    QCOMPARE(date_LocalZone.dateTime(), QDateTime(d, QTime(0,0,0), Qt::LocalTime));

    KDateTime dateTime_LocalZone(d, t, KDateTime::LocalZone);
    QVERIFY(!dateTime_LocalZone.isNull());
    QVERIFY(dateTime_LocalZone.isValid());
    QVERIFY(!dateTime_LocalZone.isDateOnly());
    QCOMPARE(dateTime_LocalZone.timeType(), KDateTime::TimeZone);
    QCOMPARE(dateTime_LocalZone.timeSpec(), KDateTime::Spec::LocalZone());
    QVERIFY(!dateTime_LocalZone.isUtc());
    QVERIFY(!dateTime_LocalZone.isOffsetFromUtc());
    QVERIFY(dateTime_LocalZone.isLocalZone());
    QVERIFY(!dateTime_LocalZone.isClockTime());
    QCOMPARE(dateTime_LocalZone.utcOffset(), -8*3600);
    QCOMPARE(dateTime_LocalZone.timeZone(), KSystemTimeZones::local());
    QCOMPARE(dateTime_LocalZone.dateTime(), QDateTime(d, t, Qt::LocalTime));

    KDateTime datetime_LocalZone(dtLocal, KDateTime::LocalZone);
    QVERIFY(!datetime_LocalZone.isNull());
    QVERIFY(datetime_LocalZone.isValid());
    QVERIFY(!datetime_LocalZone.isDateOnly());
    QCOMPARE(datetime_LocalZone.timeType(), KDateTime::TimeZone);
    QCOMPARE(datetime_LocalZone.timeSpec(), KDateTime::Spec::LocalZone());
    QVERIFY(!datetime_LocalZone.isUtc());
    QVERIFY(!datetime_LocalZone.isOffsetFromUtc());
    QVERIFY(datetime_LocalZone.isLocalZone());
    QVERIFY(!datetime_LocalZone.isClockTime());
    QCOMPARE(datetime_LocalZone.utcOffset(), -8*3600);
    QCOMPARE(datetime_LocalZone.timeZone(), KSystemTimeZones::local());
    QCOMPARE(datetime_LocalZone.dateTime(), dtLocal);
    QCOMPARE(datetime_LocalZone.date(), dtLocal.date());
    QCOMPARE(datetime_LocalZone.time(), dtLocal.time());

    KDateTime datetime_LocalZone2(dtUTC, KDateTime::LocalZone);
    QVERIFY(!datetime_LocalZone2.isNull());
    QVERIFY(datetime_LocalZone2.isValid());
    QVERIFY(!datetime_LocalZone2.isDateOnly());
    QCOMPARE(datetime_LocalZone2.timeType(), KDateTime::TimeZone);
    QCOMPARE(datetime_LocalZone2.timeSpec(), KDateTime::Spec::LocalZone());
    QVERIFY(!datetime_LocalZone2.isUtc());
    QVERIFY(!datetime_LocalZone2.isOffsetFromUtc());
    QVERIFY(datetime_LocalZone2.isLocalZone());
    QVERIFY(!datetime_LocalZone2.isClockTime());
    QCOMPARE(datetime_LocalZone2.utcOffset(), -8*3600);
    QCOMPARE(datetime_LocalZone2.timeZone(), KSystemTimeZones::local());
    QCOMPARE(datetime_LocalZone2.dateTime(), dtUTC.toLocalTime());

    // ... copy constructor
    KDateTime datetime_LocalZoneCopy(datetime_LocalZone);
    QVERIFY(!datetime_LocalZoneCopy.isNull());
    QVERIFY(datetime_LocalZoneCopy.isValid());
    QVERIFY(!datetime_LocalZoneCopy.isDateOnly());
    QCOMPARE(datetime_LocalZoneCopy.timeType(), KDateTime::TimeZone);
    QCOMPARE(datetime_LocalZoneCopy.timeSpec(), KDateTime::Spec::LocalZone());
    QVERIFY(!datetime_LocalZoneCopy.isUtc());
    QVERIFY(!datetime_LocalZoneCopy.isOffsetFromUtc());
    QVERIFY(datetime_LocalZoneCopy.isLocalZone());
    QVERIFY(!datetime_LocalZoneCopy.isClockTime());
    QCOMPARE(datetime_LocalZoneCopy.utcOffset(), -8*3600);
    QCOMPARE(datetime_LocalZoneCopy.timeZone(), datetime_LocalZone.timeZone());
    QCOMPARE(datetime_LocalZoneCopy.dateTime(), datetime_LocalZone.dateTime());


    // Local clock time
    KDateTime date_ClockTime(d, KDateTime::Spec::ClockTime());
    QVERIFY(!date_ClockTime.isNull());
    QVERIFY(date_ClockTime.isValid());
    QVERIFY(date_ClockTime.isDateOnly());
    QCOMPARE(date_ClockTime.timeType(), KDateTime::ClockTime);
    QVERIFY(!date_ClockTime.isUtc());
    QVERIFY(!date_ClockTime.isOffsetFromUtc());
    QVERIFY(!date_ClockTime.isLocalZone());
    QVERIFY(date_ClockTime.isClockTime());
    QCOMPARE(date_ClockTime.utcOffset(), 0);
    QVERIFY(!date_ClockTime.timeZone().isValid());
    QCOMPARE(date_ClockTime.dateTime(), QDateTime(d, QTime(0,0,0), Qt::LocalTime));

    KDateTime dateTime_ClockTime(d, t, KDateTime::ClockTime);
    QVERIFY(!dateTime_ClockTime.isNull());
    QVERIFY(dateTime_ClockTime.isValid());
    QVERIFY(!dateTime_ClockTime.isDateOnly());
    QCOMPARE(dateTime_ClockTime.timeType(), KDateTime::ClockTime);
    QVERIFY(!dateTime_ClockTime.isUtc());
    QVERIFY(!dateTime_ClockTime.isOffsetFromUtc());
    QVERIFY(!dateTime_ClockTime.isLocalZone());
    QVERIFY(dateTime_ClockTime.isClockTime());
    QCOMPARE(dateTime_ClockTime.utcOffset(), 0);
    QVERIFY(!dateTime_ClockTime.timeZone().isValid());
    QCOMPARE(dateTime_ClockTime.dateTime(), QDateTime(d, t, Qt::LocalTime));

    KDateTime datetime_ClockTime(dtLocal, KDateTime::ClockTime);
    QVERIFY(!datetime_ClockTime.isNull());
    QVERIFY(datetime_ClockTime.isValid());
    QVERIFY(!datetime_ClockTime.isDateOnly());
    QCOMPARE(datetime_ClockTime.timeType(), KDateTime::ClockTime);
    QVERIFY(!datetime_ClockTime.isUtc());
    QVERIFY(!datetime_ClockTime.isOffsetFromUtc());
    QVERIFY(!datetime_ClockTime.isLocalZone());
    QVERIFY(datetime_ClockTime.isClockTime());
    QCOMPARE(datetime_ClockTime.utcOffset(), 0);
    QVERIFY(!datetime_ClockTime.timeZone().isValid());
    QCOMPARE(datetime_ClockTime.dateTime(), dtLocal);
    QCOMPARE(datetime_ClockTime.date(), dtLocal.date());
    QCOMPARE(datetime_ClockTime.time(), dtLocal.time());

    KDateTime datetime_ClockTime2(dtUTC, KDateTime::ClockTime);
    QVERIFY(!datetime_ClockTime2.isNull());
    QVERIFY(datetime_ClockTime2.isValid());
    QVERIFY(!datetime_ClockTime2.isDateOnly());
    QCOMPARE(datetime_ClockTime2.timeType(), KDateTime::ClockTime);
    QVERIFY(!datetime_ClockTime2.isUtc());
    QVERIFY(!datetime_ClockTime2.isOffsetFromUtc());
    QVERIFY(!datetime_ClockTime2.isLocalZone());
    QVERIFY(datetime_ClockTime2.isClockTime());
    QCOMPARE(datetime_ClockTime2.utcOffset(), 0);
    QVERIFY(!datetime_ClockTime2.timeZone().isValid());
    QCOMPARE(datetime_ClockTime2.dateTime(), dtUTC.toLocalTime());

    // ... copy constructor
    KDateTime datetime_ClockTimeCopy(datetime_ClockTime);
    QVERIFY(!datetime_ClockTimeCopy.isNull());
    QVERIFY(datetime_ClockTimeCopy.isValid());
    QVERIFY(!datetime_ClockTimeCopy.isDateOnly());
    QCOMPARE(datetime_ClockTimeCopy.timeType(), KDateTime::ClockTime);
    QVERIFY(!datetime_ClockTimeCopy.isUtc());
    QVERIFY(!datetime_ClockTimeCopy.isOffsetFromUtc());
    QVERIFY(!datetime_ClockTimeCopy.isLocalZone());
    QVERIFY(datetime_ClockTimeCopy.isClockTime());
    QCOMPARE(datetime_ClockTimeCopy.utcOffset(), 0);
    QVERIFY(!datetime_ClockTimeCopy.timeZone().isValid());
    QCOMPARE(datetime_ClockTimeCopy.dateTime(), datetime_ClockTime.dateTime());


    // Invalid time zone specification for a constructor
    KDateTime date_TimeZone(d, KDateTime::TimeZone);
    QVERIFY(!date_TimeZone.isValid());
    KDateTime dateTime_TimeZone(d, t, KDateTime::TimeZone);
    QVERIFY(!dateTime_TimeZone.isValid());
    KDateTime datetime_TimeZone(dtLocal, KDateTime::TimeZone);
    QVERIFY(!datetime_TimeZone.isValid());
    KDateTime datetime_Invalid(dtLocal, KDateTime::Invalid);
    QVERIFY(!datetime_Invalid.isValid());

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

///////////////////////////////////
// Time conversion and operator==()
///////////////////////////////////

void KDateTimeTest::toUtc()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    // Zone -> UTC
    KDateTime londonWinter(QDate(2005,1,1), QTime(0,0,0), london);
    KDateTime utcWinter = londonWinter.toUtc();
    QVERIFY(utcWinter.isUtc());
    QCOMPARE(utcWinter.dateTime(), QDateTime(QDate(2005,1,1), QTime(0,0,0), Qt::UTC));
    QVERIFY(londonWinter == utcWinter);
    KDateTime londonSummer(QDate(2005,6,1), QTime(0,0,0), london);
    KDateTime utcSummer = londonSummer.toUtc();
    QVERIFY(utcSummer.isUtc());
    QCOMPARE(utcSummer.dateTime(), QDateTime(QDate(2005,5,31), QTime(23,0,0), Qt::UTC));
    QVERIFY(londonSummer == utcSummer);
    QVERIFY(!(londonSummer == utcWinter));
    QVERIFY(!(londonWinter == utcSummer));

    // UTC offset -> UTC
    KDateTime offset(QDate(2005,6,6), QTime(1,2,30), KDateTime::Spec::OffsetFromUTC(-5400));  // -0130
    KDateTime utcOffset = offset.toUtc();
    QVERIFY(utcOffset.isUtc());
    QCOMPARE(utcOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(2,32,30), Qt::UTC));
    QVERIFY(offset == utcOffset);
    QVERIFY(!(offset == utcSummer));

    // Clock time -> UTC
    KDateTime clock(QDate(2005,6,6), QTime(1,2,30), KDateTime::ClockTime);
    KDateTime utcClock = clock.toUtc();
    QVERIFY(utcClock.isUtc());
    QCOMPARE(utcClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(8,2,30), Qt::UTC));
    QVERIFY(clock == utcClock);
    QVERIFY(!(clock == utcOffset));

    // UTC -> UTC
    KDateTime utc(QDate(2005,6,6), QTime(1,2,30), KDateTime::UTC);
    KDateTime utcUtc = utc.toUtc();
    QVERIFY(utcUtc.isUtc());
    QCOMPARE(utcUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(1,2,30), Qt::UTC));
    QVERIFY(utc == utcUtc);
    QVERIFY(!(utc == utcClock));

    // ** Date only ** //

    // Zone -> UTC
    londonSummer.setDateOnly(true);
    utcSummer = londonSummer.toUtc();
    QVERIFY(utcSummer.isDateOnly());
    QCOMPARE(utcSummer.dateTime(), QDateTime(QDate(2005,6,1), QTime(0,0,0), Qt::UTC));
    QVERIFY(utcSummer != londonSummer);
    QVERIFY(!(utcSummer == londonSummer));
    londonWinter.setDateOnly(true);
    utcWinter = londonWinter.toUtc();
    QVERIFY(utcWinter == londonWinter);
    QVERIFY(!(utcWinter != londonWinter));

    // UTC offset -> UTC
    offset.setDateOnly(true);
    utcOffset = offset.toUtc();
    QVERIFY(utcOffset.isDateOnly());
    QCOMPARE(utcOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::UTC));
    QVERIFY(offset != utcOffset);
    QVERIFY(!(offset == utcOffset));
    KDateTime utcOffset1(QDate(2005,6,6), KDateTime::Spec::OffsetFromUTC(0));
    QVERIFY(utcOffset1 == utcOffset1.toUtc());

    // Clock time -> UTC
    clock.setDateOnly(true);
    utcClock = clock.toUtc();
    QVERIFY(utcClock.isDateOnly());
    QCOMPARE(utcClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::UTC));
    QVERIFY(clock != utcClock);
    QVERIFY(!(clock == utcClock));

    // UTC -> UTC
    utc.setDateOnly(true);
    utcUtc = utc.toUtc();
    QVERIFY(utcUtc.isDateOnly());
    QCOMPARE(utcUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::UTC));
    QVERIFY(utc == utcUtc);
    QVERIFY(!(utc != utcUtc));

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::toOffsetFromUtc()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    // ***** toOffsetFromUtc(void) *****

    // Zone -> UTC offset
    KDateTime londonWinter(QDate(2005,1,1), QTime(2,0,0), london);
    KDateTime offsetWinter = londonWinter.toOffsetFromUtc();
    QVERIFY(offsetWinter.isOffsetFromUtc());
    QCOMPARE(offsetWinter.utcOffset(), 0);
    QCOMPARE(offsetWinter.dateTime(), QDateTime(QDate(2005,1,1), QTime(2,0,0), Qt::LocalTime));
    QVERIFY(londonWinter == offsetWinter);
    KDateTime londonSummer(QDate(2005,6,1), QTime(14,0,0), london);
    KDateTime offsetSummer = londonSummer.toOffsetFromUtc();
    QVERIFY(offsetSummer.isOffsetFromUtc());
    QCOMPARE(offsetSummer.utcOffset(), 3600);
    QCOMPARE(offsetSummer.dateTime(), QDateTime(QDate(2005,6,1), QTime(14,0,0), Qt::LocalTime));
    QVERIFY(londonSummer == offsetSummer);
    QVERIFY(!(londonSummer == offsetWinter));
    QVERIFY(!(londonWinter == offsetSummer));

    // UTC offset -> UTC offset
    KDateTime offset(QDate(2005,6,6), QTime(11,2,30), KDateTime::Spec::OffsetFromUTC(-5400));  // -0130
    KDateTime offsetOffset = offset.toOffsetFromUtc();
    QVERIFY(offsetOffset.isOffsetFromUtc());
    QCOMPARE(offsetOffset.utcOffset(), -5400);
    QCOMPARE(offsetOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(11,2,30), Qt::LocalTime));
    QVERIFY(offset == offsetOffset);
    QVERIFY(!(offset == offsetSummer));

    // Clock time -> UTC offset
    KDateTime clock(QDate(2005,6,6), QTime(1,2,30), KDateTime::ClockTime);
    KDateTime offsetClock = clock.toOffsetFromUtc();
    QVERIFY(offsetClock.isOffsetFromUtc());
    QCOMPARE(offsetClock.utcOffset(), -7*3600);
    QCOMPARE(offsetClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(1,2,30), Qt::LocalTime));
    QVERIFY(clock == offsetClock);
    QVERIFY(!(clock == offsetOffset));

    // UTC -> UTC offset
    KDateTime utc(QDate(2005,6,6), QTime(11,2,30), KDateTime::UTC);
    KDateTime offsetUtc = utc.toOffsetFromUtc();
    QVERIFY(offsetUtc.isOffsetFromUtc());
    QCOMPARE(offsetUtc.utcOffset(), 0);
    QCOMPARE(offsetUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(11,2,30), Qt::LocalTime));
    QVERIFY(utc == offsetUtc);
    QVERIFY(!(utc == offsetClock));

    // ** Date only ** //

    // Zone -> UTC offset
    londonSummer.setDateOnly(true);
    offsetSummer = londonSummer.toOffsetFromUtc();
    QVERIFY(offsetSummer.isDateOnly());
    QVERIFY(offsetSummer.isOffsetFromUtc());
    QCOMPARE(offsetSummer.utcOffset(), 3600);
    QCOMPARE(offsetSummer.dateTime(), QDateTime(QDate(2005,6,1), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(offsetSummer == londonSummer);
    QVERIFY(!(offsetSummer != londonSummer));
    londonWinter.setDateOnly(true);
    offsetWinter = londonWinter.toUtc();
    QVERIFY(offsetWinter == londonWinter);
    QVERIFY(!(offsetWinter != londonWinter));

    // UTC offset -> UTC offset
    offset.setDateOnly(true);
    offsetOffset = offset.toOffsetFromUtc();
    QVERIFY(offsetOffset.isDateOnly());
    QVERIFY(offsetOffset.isOffsetFromUtc());
    QCOMPARE(offsetOffset.utcOffset(), -5400);
    QCOMPARE(offsetOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(offset == offsetOffset);
    QVERIFY(!(offset != offsetOffset));

    // Clock time -> UTC offset
    clock.setDateOnly(true);
    offsetClock = clock.toOffsetFromUtc();
    QVERIFY(offsetClock.isDateOnly());
    QVERIFY(offsetClock.isOffsetFromUtc());
    QCOMPARE(offsetClock.utcOffset(), -7*3600);
    QCOMPARE(offsetClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(clock == offsetClock);
    QVERIFY(!(clock != offsetClock));

    // UTC -> UTC offset
    utc.setDateOnly(true);
    offsetUtc = utc.toOffsetFromUtc();
    QVERIFY(offsetUtc.isDateOnly());
    QVERIFY(offsetUtc.isOffsetFromUtc());
    QCOMPARE(offsetUtc.utcOffset(), 0);
    QCOMPARE(offsetUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(utc == offsetUtc);
    QVERIFY(!(utc != offsetUtc));

    // ***** toOffsetFromUtc(int utcOffset) *****

    // Zone -> UTC offset
    KDateTime londonWinter2(QDate(2005,1,1), QTime(2,0,0), london);
    offsetWinter = londonWinter2.toOffsetFromUtc(5400);    // +1H30M
    QVERIFY(offsetWinter.isOffsetFromUtc());
    QCOMPARE(offsetWinter.utcOffset(), 5400);
    QCOMPARE(offsetWinter.dateTime(), QDateTime(QDate(2005,1,1), QTime(3,30,0), Qt::LocalTime));
    QVERIFY(londonWinter2 == offsetWinter);
    KDateTime londonSummer2(QDate(2005,6,1), QTime(14,0,0), london);
    offsetSummer = londonSummer2.toOffsetFromUtc(5400);
    QVERIFY(offsetSummer.isOffsetFromUtc());
    QCOMPARE(offsetSummer.utcOffset(), 5400);
    QCOMPARE(offsetSummer.dateTime(), QDateTime(QDate(2005,6,1), QTime(14,30,0), Qt::LocalTime));
    QVERIFY(londonSummer2 == offsetSummer);
    QVERIFY(!(londonSummer2 == offsetWinter));
    QVERIFY(!(londonWinter2 == offsetSummer));

    // UTC offset -> UTC offset
    KDateTime offset2(QDate(2005,6,6), QTime(11,2,30), KDateTime::Spec::OffsetFromUTC(-5400));  // -0130
    offsetOffset = offset2.toOffsetFromUtc(3600);
    QVERIFY(offsetOffset.isOffsetFromUtc());
    QCOMPARE(offsetOffset.utcOffset(), 3600);
    QCOMPARE(offsetOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(13,32,30), Qt::LocalTime));
    QVERIFY(offset2 == offsetOffset);
    QVERIFY(!(offset2 == offsetSummer));

    // Clock time -> UTC offset
    KDateTime clock2(QDate(2005,6,6), QTime(1,2,30), KDateTime::ClockTime);
    offsetClock = clock2.toOffsetFromUtc(0);
    QVERIFY(offsetClock.isOffsetFromUtc());
    QCOMPARE(offsetClock.utcOffset(), 0);
    QCOMPARE(offsetClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(8,2,30), Qt::LocalTime));
    QVERIFY(clock2 == offsetClock);
    QVERIFY(!(clock2 == offsetOffset));

    // UTC -> UTC offset
    KDateTime utc2(QDate(2005,6,6), QTime(11,2,30), KDateTime::UTC);
    offsetUtc = utc2.toOffsetFromUtc(-3600);
    QVERIFY(offsetUtc.isOffsetFromUtc());
    QCOMPARE(offsetUtc.utcOffset(), -3600);
    QCOMPARE(offsetUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(10,2,30), Qt::LocalTime));
    QVERIFY(utc2 == offsetUtc);
    QVERIFY(!(utc2 == offsetClock));

    // ** Date only ** //

    // Zone -> UTC offset
    londonSummer2.setDateOnly(true);
    offsetSummer = londonSummer2.toOffsetFromUtc(5400);
    QVERIFY(offsetSummer.isDateOnly());
    QVERIFY(offsetSummer.isOffsetFromUtc());
    QCOMPARE(offsetSummer.utcOffset(), 5400);
    QCOMPARE(offsetSummer.dateTime(), QDateTime(QDate(2005,6,1), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(londonSummer2 != offsetSummer);
    QVERIFY(!(londonSummer2 == offsetSummer));
    QVERIFY(londonSummer2 == KDateTime(QDate(2005,6,1), KDateTime::Spec::OffsetFromUTC(3600)));

    // UTC offset -> UTC offset
    offset2.setDateOnly(true);
    offsetOffset = offset2.toOffsetFromUtc(-3600);
    QVERIFY(offsetOffset.isDateOnly());
    QVERIFY(offsetOffset.isOffsetFromUtc());
    QCOMPARE(offsetOffset.utcOffset(), -3600);
    QCOMPARE(offsetOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(offset2 != offsetOffset);
    QVERIFY(!(offset2 == offsetOffset));

    // Clock time -> UTC offset
    clock2.setDateOnly(true);
    offsetClock = clock2.toOffsetFromUtc(6*3600);
    QVERIFY(offsetClock.isDateOnly());
    QVERIFY(offsetClock.isOffsetFromUtc());
    QCOMPARE(offsetClock.utcOffset(), 6*3600);
    QCOMPARE(offsetClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(clock2 != offsetClock);
    QVERIFY(!(clock2 == offsetClock));
    QVERIFY(clock == KDateTime(QDate(2005,6,6), KDateTime::Spec::OffsetFromUTC(-7*3600)));

    // UTC -> UTC offset
    utc2.setDateOnly(true);
    offsetUtc = utc2.toOffsetFromUtc(1800);
    QVERIFY(offsetUtc.isDateOnly());
    QVERIFY(offsetUtc.isOffsetFromUtc());
    QCOMPARE(offsetUtc.utcOffset(), 1800);
    QCOMPARE(offsetUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(utc2 != offsetUtc);
    QVERIFY(!(utc2 == offsetUtc));
    QVERIFY(utc2 == KDateTime(QDate(2005,6,6), KDateTime::Spec::OffsetFromUTC(0)));

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::toLocalZone()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    // Zone -> LocalZone
    KDateTime londonWinter(QDate(2005,1,1), QTime(0,0,0), london);
    KDateTime locWinter = londonWinter.toLocalZone();
    QVERIFY(locWinter.isLocalZone());
    QCOMPARE(locWinter.dateTime(), QDateTime(QDate(2004,12,31), QTime(16,0,0), Qt::LocalTime));
    QVERIFY(londonWinter == locWinter);
    KDateTime londonSummer(QDate(2005,6,1), QTime(0,0,0), london);
    KDateTime locSummer = londonSummer.toLocalZone();
    QVERIFY(locSummer.isLocalZone());
    QCOMPARE(locSummer.dateTime(), QDateTime(QDate(2005,5,31), QTime(16,0,0), Qt::LocalTime));
    QVERIFY(londonSummer == locSummer);
    QVERIFY(!(londonSummer == locWinter));
    QVERIFY(!(londonWinter == locSummer));

    // UTC offset -> LocalZone
    KDateTime offset(QDate(2005,6,6), QTime(11,2,30), KDateTime::Spec::OffsetFromUTC(-5400));  // -0130
    KDateTime locOffset = offset.toLocalZone();
    QVERIFY(locOffset.isLocalZone());
    QCOMPARE(locOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(5,32,30), Qt::LocalTime));
    QVERIFY(offset == locOffset);
    QVERIFY(!(offset == locSummer));

    // Clock time -> LocalZone
    KDateTime clock(QDate(2005,6,6), QTime(1,2,30), KDateTime::ClockTime);
    KDateTime locClock = clock.toLocalZone();
    QVERIFY(locClock.isLocalZone());
    QCOMPARE(locClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(1,2,30), Qt::LocalTime));
    QVERIFY(clock == locClock);
    QVERIFY(!(clock == locOffset));

    // UTC -> LocalZone
    KDateTime utc(QDate(2005,6,6), QTime(11,2,30), KDateTime::UTC);
    KDateTime locUtc = utc.toLocalZone();
    QVERIFY(locUtc.isLocalZone());
    QCOMPARE(locUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(4,2,30), Qt::LocalTime));
    QVERIFY(utc == locUtc);
    QVERIFY(!(utc == locClock));

    // ** Date only ** //

    // Zone -> LocalZone
    londonSummer.setDateOnly(true);
    locSummer = londonSummer.toLocalZone();
    QVERIFY(locSummer.isDateOnly());
    QCOMPARE(locSummer.dateTime(), QDateTime(QDate(2005,6,1), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(londonSummer != locSummer);
    QVERIFY(!(londonSummer == locSummer));

    // UTC offset -> LocalZone
    offset.setDateOnly(true);
    locOffset = offset.toLocalZone();
    QVERIFY(locOffset.isDateOnly());
    QCOMPARE(locOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(offset != locOffset);
    QVERIFY(!(offset == locOffset));
    QVERIFY(locOffset == KDateTime(QDate(2005,6,6), KDateTime::Spec::OffsetFromUTC(-7*3600)));

    // Clock time -> LocalZone
    clock.setDateOnly(true);
    locClock = clock.toLocalZone();
    QVERIFY(locClock.isDateOnly());
    QCOMPARE(locClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(clock == locClock);
    QVERIFY(!(clock != locClock));

    // UTC -> LocalZone
    utc.setDateOnly(true);
    locUtc = utc.toLocalZone();
    QVERIFY(locUtc.isDateOnly());
    QCOMPARE(locUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(utc != locUtc);
    QVERIFY(!(utc == locUtc));

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::toClockTime()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    // Zone -> ClockTime
    KDateTime londonWinter(QDate(2005,1,1), QTime(0,0,0), london);
    KDateTime locWinter = londonWinter.toClockTime();
    QVERIFY(locWinter.isClockTime());
    QCOMPARE(locWinter.dateTime(), QDateTime(QDate(2004,12,31), QTime(16,0,0), Qt::LocalTime));
    QVERIFY(londonWinter == locWinter);
    KDateTime londonSummer(QDate(2005,6,1), QTime(0,0,0), london);
    KDateTime locSummer = londonSummer.toClockTime();
    QVERIFY(locSummer.isClockTime());
    QCOMPARE(locSummer.dateTime(), QDateTime(QDate(2005,5,31), QTime(16,0,0), Qt::LocalTime));
    QVERIFY(londonSummer == locSummer);
    QVERIFY(!(londonSummer == locWinter));
    QVERIFY(!(londonWinter == locSummer));

    // UTC offset -> ClockTime
    KDateTime offset(QDate(2005,6,6), QTime(11,2,30), KDateTime::Spec::OffsetFromUTC(-5400));  // -0130
    KDateTime locOffset = offset.toClockTime();
    QVERIFY(locOffset.isClockTime());
    QCOMPARE(locOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(5,32,30), Qt::LocalTime));
    QVERIFY(offset == locOffset);
    QVERIFY(!(offset == locSummer));

    // Clock time -> ClockTime
    KDateTime clock(QDate(2005,6,6), QTime(1,2,30), KDateTime::ClockTime);
    KDateTime locClock = clock.toClockTime();
    QVERIFY(locClock.isClockTime());
    QCOMPARE(locClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(1,2,30), Qt::LocalTime));
    QVERIFY(clock == locClock);
    QVERIFY(!(clock == locOffset));

    // UTC -> ClockTime
    KDateTime utc(QDate(2005,6,6), QTime(11,2,30), KDateTime::UTC);
    KDateTime locUtc = utc.toClockTime();
    QVERIFY(locUtc.isClockTime());
    QCOMPARE(locUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(4,2,30), Qt::LocalTime));
    QVERIFY(utc == locUtc);
    QVERIFY(!(utc == locClock));

    // ** Date only ** //

    // Zone -> ClockTime
    londonSummer.setDateOnly(true);
    locSummer = londonSummer.toClockTime();
    QVERIFY(locSummer.isDateOnly());
    QCOMPARE(locSummer.dateTime(), QDateTime(QDate(2005,6,1), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(londonSummer != locSummer);
    QVERIFY(!(londonSummer == locSummer));

    // UTC offset -> ClockTime
    offset.setDateOnly(true);
    locOffset = offset.toClockTime();
    QVERIFY(locOffset.isDateOnly());
    QCOMPARE(locOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(offset != locOffset);
    QVERIFY(!(offset == locOffset));
    QVERIFY(locOffset == KDateTime(QDate(2005,6,6), KDateTime::Spec::OffsetFromUTC(-7*3600)));

    // Clock time -> ClockTime
    clock.setDateOnly(true);
    locClock = clock.toClockTime();
    QVERIFY(locClock.isDateOnly());
    QCOMPARE(locClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(clock == locClock);
    QVERIFY(!(clock != locClock));

    // UTC -> ClockTime
    utc.setDateOnly(true);
    locUtc = utc.toClockTime();
    QVERIFY(locUtc.isDateOnly());
    QCOMPARE(locUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(utc != locUtc);
    QVERIFY(!(utc == locUtc));

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::toZone()
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    QSignalSpy timeoutSpy(&timer, SIGNAL(timeout()));

    // This test relies on kded running, and on kdebase/runtime being installed
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kded")) {
        QSKIP("kded not running", SkipSingle);
    }
    QDBusInterface ktimezoned("org.kde.kded", "/modules/ktimezoned", "org.kde.kded.KTimeZoned");
    if (!ktimezoned.isValid())
    {
        // Need to load the KDED time zones module
        QDBusInterface kded("org.kde.kded", "/kded", "org.kde.kded");
        QDBusReply<bool> reply = kded.call("loadModule", "ktimezoned");
        if (!reply.isValid() || !reply)
            QSKIP("Could not load ktimezoned kded module", SkipSingle);
    }

    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");

    // Ensure that local time is different from UTC and different from 'london'
    KConfig config("ktimezonedrc");
    KConfigGroup group(&config, "TimeZones");
    group.writeEntry("LocalZone", QString::fromLatin1("Europe/London"));
    config.sync();
    QDBusMessage message = QDBusMessage::createSignal("/Daemon", "org.kde.KTimeZoned", "configChanged");
    QDBusConnection::sessionBus().send(message);
    timer.start(1000);
    loop.exec();

    // Zone -> Zone
    KDateTime londonWinter(QDate(2005,1,1), QTime(0,0,0), london);
    KDateTime locWinter = londonWinter.toZone(losAngeles);
    QCOMPARE(locWinter.timeZone(), losAngeles);
    QCOMPARE(locWinter.dateTime(), QDateTime(QDate(2004,12,31), QTime(16,0,0), Qt::LocalTime));
    QVERIFY(londonWinter == locWinter);
    KDateTime londonSummer(QDate(2005,6,1), QTime(0,0,0), london);
    KDateTime locSummer = londonSummer.toZone(losAngeles);
    QCOMPARE(locWinter.timeZone(), losAngeles);
    QCOMPARE(locSummer.dateTime(), QDateTime(QDate(2005,5,31), QTime(16,0,0), Qt::LocalTime));
    QVERIFY(londonSummer == locSummer);
    QVERIFY(!(londonSummer == locWinter));
    QVERIFY(!(londonWinter == locSummer));

    // UTC offset -> Zone
    KDateTime offset(QDate(2005,6,6), QTime(11,2,30), KDateTime::Spec::OffsetFromUTC(-5400));  // -0130
    KDateTime locOffset = offset.toZone(losAngeles);
    QCOMPARE(locOffset.timeZone(), losAngeles);
    QCOMPARE(locOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(5,32,30), Qt::LocalTime));
    QVERIFY(offset == locOffset);
    QVERIFY(!(offset == locSummer));

    // Clock time -> Zone
    KDateTime clock(QDate(2005,6,6), QTime(11,2,30), KDateTime::ClockTime);
    KDateTime locClock = clock.toZone(losAngeles);
    QCOMPARE(locClock.timeZone(), losAngeles);
    QCOMPARE(locClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(3,2,30), Qt::LocalTime));
    QVERIFY(clock == locClock);
    QVERIFY(!(clock == locOffset));

    // UTC -> Zone
    KDateTime utc(QDate(2005,6,6), QTime(11,2,30), KDateTime::UTC);
    KDateTime locUtc = utc.toZone(losAngeles);
    QCOMPARE(locUtc.timeZone(), losAngeles);
    QCOMPARE(locUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(4,2,30), Qt::LocalTime));
    QVERIFY(utc == locUtc);
    QVERIFY(!(utc == locClock));

    // ** Date only ** //

    // Zone -> Zone
    londonSummer.setDateOnly(true);
    locSummer = londonSummer.toZone(losAngeles);
    QVERIFY(locSummer.isDateOnly());
    QCOMPARE(locSummer.dateTime(), QDateTime(QDate(2005,6,1), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(londonSummer != locSummer);
    QVERIFY(!(londonSummer == locSummer));

    // UTC offset -> Zone
    offset.setDateOnly(true);
    locOffset = offset.toZone(losAngeles);
    QVERIFY(locOffset.isDateOnly());
    QCOMPARE(locOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(offset != locOffset);
    QVERIFY(!(offset == locOffset));

    // Clock time -> Zone
    clock.setDateOnly(true);
    locClock = clock.toZone(losAngeles);
    QVERIFY(locClock.isDateOnly());
    QCOMPARE(locClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(clock != locClock);
    QVERIFY(!(clock == locClock));

    // UTC -> Zone
    utc.setDateOnly(true);
    locUtc = utc.toZone(losAngeles);
    QVERIFY(locUtc.isDateOnly());
    QCOMPARE(locUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));
    QVERIFY(utc != locUtc);
    QVERIFY(!(utc == locUtc));

    // Restore the original local time zone
    group.writeEntry("LocalZone", QString::fromLatin1("America/Los_Angeles"));
    config.sync();
    QDBusConnection::sessionBus().send(message);
    timer.start(1000);
    loop.exec();
}

void KDateTimeTest::toTimeSpec()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    KTimeZone cairo  = KSystemTimeZones::zone("Africa/Cairo");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    KDateTime::Spec utcSpec(KDateTime::UTC);
    KDateTime::Spec cairoSpec(cairo);
    KDateTime::Spec offset1200Spec(KDateTime::OffsetFromUTC, 1200);
    KDateTime::Spec clockSpec(KDateTime::ClockTime);

    KDateTime utc1(QDate(2004,3,1), QTime(3,45,2), KDateTime::UTC);
    KDateTime zone1(QDate(2004,3,1), QTime(3,45,2), cairo);
    KDateTime offset1(QDate(2004,3,1), QTime(3,45,2), KDateTime::Spec::OffsetFromUTC(1200));    // +00:20
    KDateTime clock1(QDate(2004,3,1), QTime(3,45,2), KDateTime::ClockTime);

    KDateTime utc(QDate(2005,6,6), QTime(1,2,30), KDateTime::UTC);
    KDateTime zone(QDate(2005,7,1), QTime(2,0,0), london);
    KDateTime offset(QDate(2005,6,6), QTime(1,2,30), KDateTime::Spec::OffsetFromUTC(-5400));  // -01:30
    KDateTime clock(QDate(2005,6,6), QTime(1,2,30), KDateTime::ClockTime);

    // To UTC
    KDateTime utcZone = zone.toTimeSpec(utcSpec);
    QVERIFY(utcZone.isUtc());
    QVERIFY(utcZone == KDateTime(QDate(2005,7,1), QTime(1,0,0), KDateTime::UTC));
    QVERIFY(zone.timeSpec() != utcSpec);
    QVERIFY(utcZone.timeSpec() == utcSpec);

    KDateTime utcOffset = offset.toTimeSpec(utcSpec);
    QVERIFY(utcOffset.isUtc());
    QVERIFY(utcOffset == KDateTime(QDate(2005,6,6), QTime(2,32,30), KDateTime::UTC));
    QVERIFY(offset.timeSpec() != utcSpec);
    QVERIFY(utcOffset.timeSpec() == utcSpec);

    KDateTime utcClock = clock.toTimeSpec(utcSpec);
    QVERIFY(utcClock.isUtc());
    QVERIFY(utcClock == KDateTime(QDate(2005,6,6), QTime(8,2,30), KDateTime::UTC));
    QVERIFY(clock.timeSpec() != utcSpec);
    QVERIFY(utcZone.timeSpec() == utcSpec);

    KDateTime utcUtc = utc.toTimeSpec(utcSpec);
    QVERIFY(utcUtc.isUtc());
    QVERIFY(utcUtc == KDateTime(QDate(2005,6,6), QTime(1,2,30), KDateTime::UTC));
    QVERIFY(utc.timeSpec() == utcSpec);
    QVERIFY(utcUtc.timeSpec() == utcSpec);

    // To Zone
    KDateTime zoneZone = zone.toTimeSpec(cairoSpec);
    QCOMPARE(zoneZone.timeZone(), cairo);
    QVERIFY(zoneZone == KDateTime(QDate(2005,7,1), QTime(4,0,0), cairo));
    QVERIFY(zone.timeSpec() != cairoSpec);
    QVERIFY(zoneZone.timeSpec() == cairoSpec);

    KDateTime zoneOffset = offset.toTimeSpec(cairoSpec);
    QCOMPARE(zoneOffset.timeZone(), cairo);
    QVERIFY(zoneOffset == KDateTime(QDate(2005,6,6), QTime(5,32,30), cairo));
    QVERIFY(offset.timeSpec() != cairoSpec);
    QVERIFY(zoneOffset.timeSpec() == cairoSpec);

    KDateTime zoneClock = clock.toTimeSpec(cairoSpec);
    QCOMPARE(zoneClock.timeZone(), cairo);
    QVERIFY(zoneClock == KDateTime(QDate(2005,6,6), QTime(11,2,30), cairo));
    QVERIFY(clock.timeSpec() != cairoSpec);
    QVERIFY(zoneClock.timeSpec() == cairoSpec);

    KDateTime zoneUtc = utc.toTimeSpec(cairoSpec);
    QCOMPARE(zoneUtc.timeZone(), cairo);
    QVERIFY(zoneUtc == KDateTime(QDate(2005,6,6), QTime(4,2,30), cairo));
    QVERIFY(utc.timeSpec() != cairoSpec);
    QVERIFY(zoneUtc.timeSpec() == cairoSpec);

    // To UTC offset
    KDateTime offsetZone = zone.toTimeSpec(offset1200Spec);
    QVERIFY(offsetZone.isOffsetFromUtc());
    QCOMPARE(offsetZone.utcOffset(), 1200);
    QVERIFY(offsetZone == KDateTime(QDate(2005,7,1), QTime(1,20,0), KDateTime::Spec::OffsetFromUTC(1200)));
    QVERIFY(zone.timeSpec() != offset1200Spec);
    QVERIFY(offsetZone.timeSpec() == offset1200Spec);

    KDateTime offsetOffset = offset.toTimeSpec(offset1200Spec);
    QVERIFY(offsetOffset.isOffsetFromUtc());
    QCOMPARE(offsetZone.utcOffset(), 1200);
    QVERIFY(offsetOffset == KDateTime(QDate(2005,6,6), QTime(2,52,30), KDateTime::Spec::OffsetFromUTC(1200)));
    QVERIFY(offset.timeSpec() != offset1200Spec);
    QVERIFY(offsetOffset.timeSpec() == offset1200Spec);

    KDateTime offsetClock = clock.toTimeSpec(offset1200Spec);
    QVERIFY(offsetClock.isOffsetFromUtc());
    QCOMPARE(offsetZone.utcOffset(), 1200);
    QVERIFY(offsetClock == KDateTime(QDate(2005,6,6), QTime(8,22,30), KDateTime::Spec::OffsetFromUTC(1200)));
    QVERIFY(clock.timeSpec() != offset1200Spec);
    QVERIFY(offsetClock.timeSpec() == offset1200Spec);

    KDateTime offsetUtc = utc.toTimeSpec(offset1200Spec);
    QVERIFY(offsetUtc.isOffsetFromUtc());
    QCOMPARE(offsetZone.utcOffset(), 1200);
    QVERIFY(offsetUtc == KDateTime(QDate(2005,6,6), QTime(1,22,30), KDateTime::Spec::OffsetFromUTC(1200)));
    QVERIFY(utc.timeSpec() != offset1200Spec);
    QVERIFY(offsetUtc.timeSpec() == offset1200Spec);

    // To Clock time
    KDateTime clockZone = zone.toTimeSpec(clockSpec);
    QVERIFY(clockZone.isClockTime());
    QVERIFY(clockZone == KDateTime(QDate(2005,6,30), QTime(18,0,0), KDateTime::ClockTime));
    QVERIFY(zone.timeSpec() != clockSpec);
    QVERIFY(clockZone.timeSpec() == clockSpec);

    KDateTime clockOffset = offset.toTimeSpec(clockSpec);
    QVERIFY(clockOffset.isClockTime());
    QVERIFY(clockOffset == KDateTime(QDate(2005,6,5), QTime(19,32,30), KDateTime::ClockTime));
    QVERIFY(offset.timeSpec() != clockSpec);
    QVERIFY(clockOffset.timeSpec() == clockSpec);

    KDateTime clockClock = clock.toTimeSpec(clockSpec);
    QVERIFY(clockClock.isClockTime());
    QVERIFY(clockClock == KDateTime(QDate(2005,6,6), QTime(1,2,30), KDateTime::ClockTime));
    QVERIFY(clock.timeSpec() == clockSpec);
    QVERIFY(clockClock.timeSpec() == clockSpec);

    KDateTime clockUtc = utc.toTimeSpec(clockSpec);
    QVERIFY(clockUtc.isClockTime());
    QVERIFY(clockUtc == KDateTime(QDate(2005,6,5), QTime(18,2,30), KDateTime::ClockTime));
    QVERIFY(utc.timeSpec() != clockSpec);
    QVERIFY(clockUtc.timeSpec() == clockSpec);


    // ** Date only ** //

    KDateTime zoned(QDate(2005,7,1), london);
    KDateTime offsetd(QDate(2005,6,6), KDateTime::Spec::OffsetFromUTC(-5400));  // -01:30
    KDateTime clockd(QDate(2005,6,6), KDateTime::ClockTime);
    KDateTime utcd(QDate(2005,6,6), KDateTime::UTC);

    // To UTC
    utcZone = zoned.toTimeSpec(utcSpec);
    QVERIFY(utcZone.isUtc());
    QVERIFY(utcZone.isDateOnly());
    QVERIFY(utcZone == KDateTime(QDate(2005,7,1), KDateTime::UTC));
    QVERIFY(utcZone != zoned);

    utcOffset = offsetd.toTimeSpec(utcSpec);
    QVERIFY(utcOffset.isUtc());
    QVERIFY(utcOffset.isDateOnly());
    QVERIFY(utcOffset == KDateTime(QDate(2005,6,6), KDateTime::UTC));
    QVERIFY(utcOffset != offsetd);

    utcClock = clockd.toTimeSpec(utcSpec);
    QVERIFY(utcClock.isUtc());
    QVERIFY(utcClock.isDateOnly());
    QVERIFY(utcClock == KDateTime(QDate(2005,6,6), KDateTime::UTC));
    QVERIFY(utcClock != clockd);

    utcUtc = utcd.toTimeSpec(utcSpec);
    QVERIFY(utcUtc.isUtc());
    QVERIFY(utcUtc.isDateOnly());
    QVERIFY(utcUtc == KDateTime(QDate(2005,6,6), KDateTime::UTC));
    QVERIFY(utcUtc == utcd);

    // To Zone
    zoneZone = zoned.toTimeSpec(cairoSpec);
    QVERIFY(zoneZone.isDateOnly());
    QCOMPARE(zoneZone.timeZone(), cairo);
    QVERIFY(zoneZone == KDateTime(QDate(2005,7,1), cairo));
    QVERIFY(zoneZone != zoned);

    zoneOffset = offsetd.toTimeSpec(cairoSpec);
    QVERIFY(zoneOffset.isDateOnly());
    QCOMPARE(zoneOffset.timeZone(), cairo);
    QVERIFY(zoneOffset == KDateTime(QDate(2005,6,6), cairo));
    QVERIFY(zoneOffset != offsetd);

    zoneClock = clockd.toTimeSpec(cairoSpec);
    QVERIFY(zoneClock.isDateOnly());
    QCOMPARE(zoneClock.timeZone(), cairo);
    QVERIFY(zoneClock == KDateTime(QDate(2005,6,6), cairo));
    QVERIFY(zoneClock != clockd);

    zoneUtc = utcd.toTimeSpec(cairoSpec);
    QVERIFY(zoneUtc.isDateOnly());
    QCOMPARE(zoneUtc.timeZone(), cairo);
    QVERIFY(zoneUtc == KDateTime(QDate(2005,6,6), cairo));
    QVERIFY(zoneUtc != utcd);

    // To UTC offset
    offsetZone = zoned.toTimeSpec(offset1200Spec);
    QVERIFY(offsetZone.isOffsetFromUtc());
    QVERIFY(offsetZone.isDateOnly());
    QCOMPARE(offsetZone.utcOffset(), 1200);
    QVERIFY(offsetZone == KDateTime(QDate(2005,7,1), KDateTime::Spec::OffsetFromUTC(1200)));
    QVERIFY(offsetZone != zoned);

    offsetOffset = offsetd.toTimeSpec(offset1200Spec);
    QVERIFY(offsetOffset.isOffsetFromUtc());
    QVERIFY(offsetOffset.isDateOnly());
    QCOMPARE(offsetZone.utcOffset(), 1200);
    QVERIFY(offsetOffset == KDateTime(QDate(2005,6,6), KDateTime::Spec::OffsetFromUTC(1200)));
    QVERIFY(offsetOffset != offsetd);

    offsetClock = clockd.toTimeSpec(offset1200Spec);
    QVERIFY(offsetClock.isOffsetFromUtc());
    QVERIFY(offsetClock.isDateOnly());
    QCOMPARE(offsetZone.utcOffset(), 1200);
    QVERIFY(offsetClock == KDateTime(QDate(2005,6,6), KDateTime::Spec::OffsetFromUTC(1200)));
    QVERIFY(offsetClock != clockd);

    offsetUtc = utcd.toTimeSpec(offset1200Spec);
    QVERIFY(offsetUtc.isOffsetFromUtc());
    QVERIFY(offsetUtc.isDateOnly());
    QCOMPARE(offsetZone.utcOffset(), 1200);
    QVERIFY(offsetUtc == KDateTime(QDate(2005,6,6), KDateTime::Spec::OffsetFromUTC(1200)));
    QVERIFY(offsetUtc != utcd);

    // To Clock time
    clockZone = zoned.toTimeSpec(clockSpec);
    QVERIFY(clockZone.isClockTime());
    QVERIFY(clockZone.isDateOnly());
    QVERIFY(clockZone == KDateTime(QDate(2005,7,1), KDateTime::ClockTime));
    QVERIFY(clockZone != zoned);

    clockOffset = offsetd.toTimeSpec(clockSpec);
    QVERIFY(clockOffset.isClockTime());
    QVERIFY(clockOffset.isDateOnly());
    QVERIFY(clockOffset == KDateTime(QDate(2005,6,6), KDateTime::ClockTime));
    QVERIFY(clockOffset != offsetd);

    clockClock = clockd.toTimeSpec(clockSpec);
    QVERIFY(clockClock.isClockTime());
    QVERIFY(clockClock.isDateOnly());
    QVERIFY(clockClock == KDateTime(QDate(2005,6,6), KDateTime::ClockTime));
    QVERIFY(clockClock == clockd);

    clockUtc = utcd.toTimeSpec(clockSpec);
    QVERIFY(clockUtc.isClockTime());
    QVERIFY(clockUtc.isDateOnly());
    QVERIFY(clockUtc == KDateTime(QDate(2005,6,6), KDateTime::ClockTime));
    QVERIFY(clockUtc != utcd);


    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}


////////////////////////////////////////////////////////////////////////
// Set methods: setDate(), setTime(), setDateTime(), setTimeSpec()
////////////////////////////////////////////////////////////////////////

void KDateTimeTest::set()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    // Zone
    KDateTime zoned(QDate(2005,6,1), london);
    zoned.setDate(QDate(2004,5,2));
    QVERIFY(zoned.isDateOnly());
    QCOMPARE(zoned.dateTime(), QDateTime(QDate(2004,5,2), QTime(0,0,0), Qt::LocalTime));
    zoned.setTime(QTime(12,13,14));
    QVERIFY(!zoned.isDateOnly());
    QCOMPARE(zoned.dateTime(), QDateTime(QDate(2004,5,2), QTime(12,13,14), Qt::LocalTime));
    zoned.setDate(QDate(2004,5,4));
    QVERIFY(!zoned.isDateOnly());

    zoned.setDateOnly(false);
    QVERIFY(!zoned.isDateOnly());
    QCOMPARE(zoned.dateTime(), QDateTime(QDate(2004,5,4), QTime(12,13,14), Qt::LocalTime));
    zoned.setDateOnly(true);
    QVERIFY(zoned.isDateOnly());
    QCOMPARE(zoned.dateTime(), QDateTime(QDate(2004,5,4), QTime(0,0,0), Qt::LocalTime));
    zoned.setDateOnly(false);
    QVERIFY(!zoned.isDateOnly());
    QCOMPARE(zoned.dateTime(), QDateTime(QDate(2004,5,4), QTime(0,0,0), Qt::LocalTime));

    KDateTime zone(QDate(2005,6,1), QTime(3,40,0), london);
    zone.setDate(QDate(2004,5,2));
    QCOMPARE(zone.dateTime(), QDateTime(QDate(2004,5,2), QTime(3,40,0), Qt::LocalTime));
    zone.setTime(QTime(12,13,14));
    QCOMPARE(zone.dateTime(), QDateTime(QDate(2004,5,2), QTime(12,13,14), Qt::LocalTime));
    zone.setDateTime(QDateTime(QDate(2003,6,10), QTime(5,6,7)));
    QCOMPARE(zone.dateTime(), QDateTime(QDate(2003,6,10), QTime(5,6,7), Qt::LocalTime));
    QCOMPARE(zone.utcOffset(), 3600);
    QCOMPARE(zone.toUtc().dateTime(), QDateTime(QDate(2003,6,10), QTime(4,6,7), Qt::UTC));

    // UTC offset
    KDateTime offsetd(QDate(2005,6,6), KDateTime::Spec::OffsetFromUTC(-5400));  // -0130
    offsetd.setDate(QDate(2004,5,2));
    QVERIFY(offsetd.isDateOnly());
    QCOMPARE(offsetd.dateTime(), QDateTime(QDate(2004,5,2), QTime(0,0,0), Qt::LocalTime));
    offsetd.setTime(QTime(12,13,14));
    QVERIFY(!offsetd.isDateOnly());
    QCOMPARE(offsetd.dateTime(), QDateTime(QDate(2004,5,2), QTime(12,13,14), Qt::LocalTime));
    offsetd.setDate(QDate(2004,5,4));
    QVERIFY(!offsetd.isDateOnly());

    offsetd.setDateOnly(false);
    QVERIFY(!offsetd.isDateOnly());
    QCOMPARE(offsetd.dateTime(), QDateTime(QDate(2004,5,4), QTime(12,13,14), Qt::LocalTime));
    offsetd.setDateOnly(true);
    QVERIFY(offsetd.isDateOnly());
    QCOMPARE(offsetd.dateTime(), QDateTime(QDate(2004,5,4), QTime(0,0,0), Qt::LocalTime));
    offsetd.setDateOnly(false);
    QVERIFY(!offsetd.isDateOnly());
    QCOMPARE(offsetd.dateTime(), QDateTime(QDate(2004,5,4), QTime(0,0,0), Qt::LocalTime));

    KDateTime offset(QDate(2005,6,6), QTime(1,2,30), KDateTime::Spec::OffsetFromUTC(-5400));  // -0130
    offset.setDate(QDate(2004,5,2));
    QCOMPARE(offset.dateTime(), QDateTime(QDate(2004,5,2), QTime(1,2,30), Qt::LocalTime));
    offset.setTime(QTime(12,13,14));
    QCOMPARE(offset.dateTime(), QDateTime(QDate(2004,5,2), QTime(12,13,14), Qt::LocalTime));
    offset.setDateTime(QDateTime(QDate(2003,12,10), QTime(5,6,7)));
    QCOMPARE(offset.dateTime(), QDateTime(QDate(2003,12,10), QTime(5,6,7), Qt::LocalTime));
    QCOMPARE(offset.utcOffset(), -5400);
    QCOMPARE(offset.toUtc().dateTime(), QDateTime(QDate(2003,12,10), QTime(6,36,7), Qt::UTC));

    // Clock time
    KDateTime clockd(QDate(2005,6,6), KDateTime::ClockTime);
    clockd.setDate(QDate(2004,5,2));
    QVERIFY(clockd.isDateOnly());
    QCOMPARE(clockd.dateTime(), QDateTime(QDate(2004,5,2), QTime(0,0,0), Qt::LocalTime));
    clockd.setTime(QTime(12,13,14));
    QVERIFY(!clockd.isDateOnly());
    QCOMPARE(clockd.dateTime(), QDateTime(QDate(2004,5,2), QTime(12,13,14), Qt::LocalTime));
    clockd.setDate(QDate(2004,5,4));
    QVERIFY(!clockd.isDateOnly());

    clockd.setDateOnly(false);
    QVERIFY(!clockd.isDateOnly());
    QCOMPARE(clockd.dateTime(), QDateTime(QDate(2004,5,4), QTime(12,13,14), Qt::LocalTime));
    clockd.setDateOnly(true);
    QVERIFY(clockd.isDateOnly());
    QCOMPARE(clockd.dateTime(), QDateTime(QDate(2004,5,4), QTime(0,0,0), Qt::LocalTime));
    clockd.setDateOnly(false);
    QVERIFY(!clockd.isDateOnly());
    QCOMPARE(clockd.dateTime(), QDateTime(QDate(2004,5,4), QTime(0,0,0), Qt::LocalTime));

    KDateTime clock(QDate(2005,6,6), QTime(1,2,30), KDateTime::ClockTime);
    clock.setDate(QDate(2004,5,2));
    QCOMPARE(clock.dateTime(), QDateTime(QDate(2004,5,2), QTime(1,2,30), Qt::LocalTime));
    clock.setTime(QTime(12,13,14));
    QCOMPARE(clock.dateTime(), QDateTime(QDate(2004,5,2), QTime(12,13,14), Qt::LocalTime));
    clock.setDateTime(QDateTime(QDate(2003,12,10), QTime(5,6,7)));
    QCOMPARE(clock.dateTime(), QDateTime(QDate(2003,12,10), QTime(5,6,7), Qt::LocalTime));
    QCOMPARE(clock.utcOffset(), 0);
    QCOMPARE(clock.toUtc().dateTime(), QDateTime(QDate(2003,12,10), QTime(13,6,7), Qt::UTC));

    // UTC
    KDateTime utcd(QDate(2005,6,6), KDateTime::UTC);
    utcd.setDate(QDate(2004,5,2));
    QVERIFY(utcd.isDateOnly());
    QCOMPARE(utcd.dateTime(), QDateTime(QDate(2004,5,2), QTime(0,0,0), Qt::UTC));
    utcd.setTime(QTime(12,13,14));
    QVERIFY(!utcd.isDateOnly());
    QCOMPARE(utcd.dateTime(), QDateTime(QDate(2004,5,2), QTime(12,13,14), Qt::UTC));
    utcd.setDate(QDate(2004,5,4));
    QVERIFY(!utcd.isDateOnly());

    utcd.setDateOnly(false);
    QVERIFY(!utcd.isDateOnly());
    QCOMPARE(utcd.dateTime(), QDateTime(QDate(2004,5,4), QTime(12,13,14), Qt::UTC));
    utcd.setDateOnly(true);
    QVERIFY(utcd.isDateOnly());
    QCOMPARE(utcd.dateTime(), QDateTime(QDate(2004,5,4), QTime(0,0,0), Qt::UTC));
    utcd.setDateOnly(false);
    QVERIFY(!utcd.isDateOnly());
    QCOMPARE(utcd.dateTime(), QDateTime(QDate(2004,5,4), QTime(0,0,0), Qt::UTC));

    KDateTime utc(QDate(2005,6,6), QTime(1,2,30), KDateTime::UTC);
    utc.setDate(QDate(2004,5,2));
    QCOMPARE(utc.dateTime(), QDateTime(QDate(2004,5,2), QTime(1,2,30), Qt::UTC));
    utc.setTime(QTime(12,13,14));
    QCOMPARE(utc.dateTime(), QDateTime(QDate(2004,5,2), QTime(12,13,14), Qt::UTC));
    utc.setDateTime(QDateTime(QDate(2003,12,10), QTime(5,6,7), Qt::UTC));
    QCOMPARE(utc.utcOffset(), 0);
    QCOMPARE(utc.dateTime(), QDateTime(QDate(2003,12,10), QTime(5,6,7), Qt::UTC));

    // setTimeSpec(SpecType)
    QCOMPARE(zone.dateTime(), QDateTime(QDate(2003,6,10), QTime(5,6,7), Qt::LocalTime));
    zone.setTimeSpec(KDateTime::Spec::OffsetFromUTC(7200));
    QVERIFY(zone.isOffsetFromUtc());
    QCOMPARE(zone.utcOffset(), 7200);
    QCOMPARE(zone.toUtc().dateTime(), QDateTime(QDate(2003,6,10), QTime(3,6,7), Qt::UTC));
    zone.setTimeSpec(KDateTime::LocalZone);
    QVERIFY(zone.isLocalZone());
    QCOMPARE(zone.utcOffset(), -7*3600);
    QCOMPARE(zone.toUtc().dateTime(), QDateTime(QDate(2003,6,10), QTime(12,6,7), Qt::UTC));
    zone.setTimeSpec(KDateTime::UTC);
    QVERIFY(zone.isUtc());
    QCOMPARE(zone.utcOffset(), 0);
    QCOMPARE(zone.dateTime(), QDateTime(QDate(2003,6,10), QTime(5,6,7), Qt::UTC));
    zone.setTimeSpec(KDateTime::ClockTime);
    QVERIFY(zone.isClockTime());
    QCOMPARE(zone.utcOffset(), 0);
    QCOMPARE(zone.toUtc().dateTime(), QDateTime(QDate(2003,6,10), QTime(12,6,7), Qt::UTC));

    // setTimeSpec(KDateTime::Spec)
    QCOMPARE(zone.dateTime(), QDateTime(QDate(2003,6,10), QTime(5,6,7), Qt::LocalTime));
    zone.setTimeSpec(offset.timeSpec());
    QVERIFY(zone.isOffsetFromUtc());
    QCOMPARE(zone.toUtc().dateTime(), QDateTime(QDate(2003,6,10), QTime(6,36,7), Qt::UTC));
    QVERIFY(zone.timeSpec() == offset.timeSpec());
    zone.setTimeSpec(KDateTime::LocalZone);
    QVERIFY(zone.isLocalZone());
    QCOMPARE(zone.toUtc().dateTime(), QDateTime(QDate(2003,6,10), QTime(12,6,7), Qt::UTC));
    zone.setTimeSpec(utc.timeSpec());
    QVERIFY(zone.isUtc());
    QCOMPARE(zone.dateTime(), QDateTime(QDate(2003,6,10), QTime(5,6,7), Qt::UTC));
    zone.setTimeSpec(clock.timeSpec());
    QVERIFY(zone.isClockTime());
    QCOMPARE(zone.toUtc().dateTime(), QDateTime(QDate(2003,6,10), QTime(12,6,7), Qt::UTC));
    zone.setTimeSpec(london);
    QCOMPARE(zone.timeZone(), london);
    QCOMPARE(zone.utcOffset(), 3600);
    QCOMPARE(zone.toUtc().dateTime(), QDateTime(QDate(2003,6,10), QTime(4,6,7), Qt::UTC));

    // time_t
    utcd = KDateTime(QDate(2005,6,6), QTime(12,15,20), KDateTime::UTC);
    QDateTime qtt = utcd.dateTime();
    uint secs = qtt.toTime_t();
    KDateTime tt;
    tt.setTime_t(static_cast<qint64>(secs));
    QVERIFY(tt.isUtc());
    QCOMPARE(tt.dateTime(), utcd.dateTime());
    QCOMPARE(tt.toTime_t(), secs);

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}


/////////////////////////////////////////////////////////
// compare()
/////////////////////////////////////////////////////////

void KDateTimeTest::compare()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    KTimeZone cairo  = KSystemTimeZones::zone("Africa/Cairo");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    // Date/time values
    QCOMPARE(KDateTime(QDate(2004,3,1), QTime(3,45,2), cairo).compare(KDateTime(QDate(2004,3,1), QTime(3,45,3), cairo)), KDateTime::Before);
    QCOMPARE(KDateTime(QDate(2004,3,1), QTime(3,45,2), cairo).compare(KDateTime(QDate(2004,3,1), QTime(3,45,2), KDateTime::UTC)), KDateTime::Before);
    QCOMPARE(KDateTime(QDate(2004,3,1), QTime(3,45,2), KDateTime::UTC).compare(KDateTime(QDate(2004,3,1), QTime(3,45,2), cairo)), KDateTime::After);
    QCOMPARE(KDateTime(QDate(2004,3,1), QTime(3,45,3), cairo).compare(KDateTime(QDate(2004,3,1), QTime(3,45,2), cairo)), KDateTime::After);
    QCOMPARE(KDateTime(QDate(2004,3,2), QTime(3,45,2), cairo).compare(KDateTime(QDate(2004,3,1), QTime(17,45,2), KDateTime::LocalZone)), KDateTime::Equal);
    QCOMPARE(KDateTime(QDate(2004,3,2), QTime(3,45,2), cairo).compare(KDateTime(QDate(2004,3,2), QTime(3,45,2), cairo)), KDateTime::Equal);

    // Date/time : date-only
    QCOMPARE(KDateTime(QDate(2004,3,1), QTime(0,0,0), cairo).compare(KDateTime(QDate(2004,3,1), cairo)), KDateTime::AtStart);
    QCOMPARE(KDateTime(QDate(2004,3,1), QTime(3,45,2), cairo).compare(KDateTime(QDate(2004,3,1), cairo)), KDateTime::Inside);
    QCOMPARE(KDateTime(QDate(2004,3,1), QTime(23,59,59,999), cairo).compare(KDateTime(QDate(2004,3,1), cairo)), KDateTime::AtEnd);
    QCOMPARE(KDateTime(QDate(2004,3,1), QTime(23,59,59,999), cairo).compare(KDateTime(QDate(2004,3,2), cairo)), KDateTime::Before);
    QCOMPARE(KDateTime(QDate(2004,3,3), QTime(0,0,0), cairo).compare(KDateTime(QDate(2004,3,2), cairo)), KDateTime::After);

    QCOMPARE(KDateTime(QDate(2004,3,2), QTime(9,59,59,999), cairo).compare(KDateTime(QDate(2004,3,2), KDateTime::LocalZone)), KDateTime::Before);
    QCOMPARE(KDateTime(QDate(2004,3,2), QTime(10,0,0), cairo).compare(KDateTime(QDate(2004,3,2), KDateTime::LocalZone)), KDateTime::AtStart);
    QCOMPARE(KDateTime(QDate(2004,3,2), QTime(10,0,1), cairo).compare(KDateTime(QDate(2004,3,2), KDateTime::LocalZone)), KDateTime::Inside);
    QCOMPARE(KDateTime(QDate(2004,3,3), QTime(9,59,59,999), cairo).compare(KDateTime(QDate(2004,3,2), KDateTime::LocalZone)), KDateTime::AtEnd);
    QCOMPARE(KDateTime(QDate(2004,3,3), QTime(10,0,0), cairo).compare(KDateTime(QDate(2004,3,2), KDateTime::LocalZone)), KDateTime::After);

    // Date-only : date/time
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,1), QTime(0,0,0), cairo)), KDateTime::StartsAt);
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,1), QTime(3,45,2), cairo)), KDateTime::Outside);
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,1), QTime(23,59,59,999), cairo)), KDateTime::EndsAt);
    QCOMPARE(KDateTime(QDate(2004,3,2), cairo).compare(KDateTime(QDate(2004,3,1), QTime(23,59,59,999), cairo)), KDateTime::After);
    QCOMPARE(KDateTime(QDate(2004,3,2), cairo).compare(KDateTime(QDate(2004,3,3), QTime(0,0,0), cairo)), KDateTime::Before);

    QCOMPARE(KDateTime(QDate(2004,3,2), KDateTime::LocalZone).compare(KDateTime(QDate(2004,3,2), QTime(9,59,59,999), cairo)), KDateTime::After);
    QCOMPARE(KDateTime(QDate(2004,3,2), KDateTime::LocalZone).compare(KDateTime(QDate(2004,3,2), QTime(10,0,0), cairo)), KDateTime::StartsAt);
    QCOMPARE(KDateTime(QDate(2004,3,2), KDateTime::LocalZone).compare(KDateTime(QDate(2004,3,3), QTime(9,59,59,999), cairo)), KDateTime::EndsAt);
    QCOMPARE(KDateTime(QDate(2004,3,2), KDateTime::LocalZone).compare(KDateTime(QDate(2004,3,3), QTime(10,0,0), cairo)), KDateTime::Before);

    // Date-only values
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,2), cairo)), KDateTime::Before);
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,2), KDateTime::Spec::OffsetFromUTC(2*3600))), KDateTime::Before);
    QCOMPARE(KDateTime(QDate(2004,3,1), london).compare(KDateTime(QDate(2004,3,2), cairo)), static_cast<KDateTime::Comparison>(KDateTime::Before|KDateTime::AtStart|KDateTime::Inside));
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,2), KDateTime::Spec::OffsetFromUTC(3*3600))), static_cast<KDateTime::Comparison>(KDateTime::Before|KDateTime::AtStart|KDateTime::Inside));
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,1), cairo)), KDateTime::Equal);
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,1), KDateTime::Spec::OffsetFromUTC(2*3600))), KDateTime::Equal);
    QCOMPARE(KDateTime(QDate(2004,3,2), cairo).compare(KDateTime(QDate(2004,3,1), london)), static_cast<KDateTime::Comparison>(KDateTime::Inside|KDateTime::AtEnd|KDateTime::After));
    QCOMPARE(KDateTime(QDate(2004,3,2), KDateTime::Spec::OffsetFromUTC(3*3600)).compare(KDateTime(QDate(2004,3,1), cairo)), static_cast<KDateTime::Comparison>(KDateTime::Inside|KDateTime::AtEnd|KDateTime::After));
    QCOMPARE(KDateTime(QDate(2004,3,2), cairo).compare(KDateTime(QDate(2004,3,1), cairo)), KDateTime::After);
    QCOMPARE(KDateTime(QDate(2004,3,2), KDateTime::Spec::OffsetFromUTC(2*3600)).compare(KDateTime(QDate(2004,3,1), cairo)), KDateTime::After);
    // Compare days when daylight savings changes occur
    QCOMPARE(KDateTime(QDate(2005,3,27), london).compare(KDateTime(QDate(2005,3,27), KDateTime::Spec::OffsetFromUTC(0))), static_cast<KDateTime::Comparison>(KDateTime::AtStart|KDateTime::Inside));
    QCOMPARE(KDateTime(QDate(2005,3,27), KDateTime::Spec::OffsetFromUTC(0)).compare(KDateTime(QDate(2005,3,27), london)), KDateTime::StartsAt);
    QCOMPARE(KDateTime(QDate(2005,10,30), london).compare(KDateTime(QDate(2005,10,30), KDateTime::UTC)), KDateTime::EndsAt);
    QCOMPARE(KDateTime(QDate(2005,10,30), KDateTime::UTC).compare(KDateTime(QDate(2005,10,30), london)), static_cast<KDateTime::Comparison>(KDateTime::Inside|KDateTime::AtEnd));

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}


/////////////////////////////////////////////////////////
// Addition and subtraction methods, and operator<() etc.
/////////////////////////////////////////////////////////

void KDateTimeTest::addSubtract()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    // UTC
    KDateTime utc1(QDate(2005,7,6), QTime(3,40,0), KDateTime::UTC);
    KDateTime utc2 = utc1.addSecs(184 * 86400);
    QVERIFY(utc2.isUtc());
    QCOMPARE(utc2.dateTime(), QDateTime(QDate(2006,1,6), QTime(3,40,0), Qt::UTC));
    KDateTime utc3 = utc1.addDays(184);
    QVERIFY(utc3.isUtc());
    QCOMPARE(utc2.dateTime(), utc3.dateTime());
    KDateTime utc4 = utc1.addMonths(6);
    QVERIFY(utc4.isUtc());
    QCOMPARE(utc2.dateTime(), utc4.dateTime());
    KDateTime utc5 = utc1.addYears(4);
    QVERIFY(utc5.isUtc());
    QCOMPARE(utc5.dateTime(), QDateTime(QDate(2009,7,6), QTime(3,40,0), Qt::UTC));
    QCOMPARE(utc1.secsTo(utc2), 184 * 86400);
    QCOMPARE(utc1.secsTo(utc3), 184 * 86400);
    QCOMPARE(utc1.daysTo(utc2), 184);
    QVERIFY(utc1 < utc2);
    QVERIFY(!(utc2 < utc1));
    QVERIFY(utc2 == utc3);

    // UTC offset
    KDateTime offset1(QDate(2005,7,6), QTime(3,40,0), KDateTime::Spec::OffsetFromUTC(-5400));  // -0130
    KDateTime offset2 = offset1.addSecs(184 * 86400);
    QVERIFY(offset2.isOffsetFromUtc());
    QCOMPARE(offset2.utcOffset(), -5400);
    QCOMPARE(offset2.dateTime(), QDateTime(QDate(2006,1,6), QTime(3,40,0), Qt::LocalTime));
    KDateTime offset3 = offset1.addDays(184);
    QVERIFY(offset3.isOffsetFromUtc());
    QCOMPARE(offset3.utcOffset(), -5400);
    QCOMPARE(offset2.dateTime(), offset3.dateTime());
    KDateTime offset4 = offset1.addMonths(6);
    QVERIFY(offset4.isOffsetFromUtc());
    QCOMPARE(offset4.utcOffset(), -5400);
    QCOMPARE(offset2.dateTime(), offset4.dateTime());
    KDateTime offset5 = offset1.addYears(4);
    QVERIFY(offset5.isOffsetFromUtc());
    QCOMPARE(offset5.utcOffset(), -5400);
    QCOMPARE(offset5.dateTime(), QDateTime(QDate(2009,7,6), QTime(3,40,0), Qt::LocalTime));
    QCOMPARE(offset1.secsTo(offset2), 184 * 86400);
    QCOMPARE(offset1.secsTo(offset3), 184 * 86400);
    QCOMPARE(offset1.daysTo(offset2), 184);
    QVERIFY(offset1 < offset2);
    QVERIFY(!(offset2 < offset1));
    QVERIFY(offset2 == offset3);

    // Zone
    KDateTime zone1(QDate(2005,7,6), QTime(3,40,0), london);
    KDateTime zone2 = zone1.addSecs(184 * 86400);
    QCOMPARE(zone2.timeZone(), london);
    QCOMPARE(zone2.dateTime(), QDateTime(QDate(2006,1,6), QTime(2,40,0), Qt::LocalTime));
    KDateTime zone3 = zone1.addDays(184);
    QCOMPARE(zone3.timeZone(), london);
    QCOMPARE(zone3.dateTime(), QDateTime(QDate(2006,1,6), QTime(3,40,0), Qt::LocalTime));
    KDateTime zone4 = zone1.addMonths(6);
    QCOMPARE(zone4.timeZone(), london);
    QCOMPARE(zone4.dateTime(), zone3.dateTime());
    KDateTime zone5 = zone1.addYears(4);
    QCOMPARE(zone5.timeZone(), london);
    QCOMPARE(zone5.dateTime(), QDateTime(QDate(2009,7,6), QTime(3,40,0), Qt::LocalTime));
    QCOMPARE(zone1.secsTo(zone2), 184 * 86400);
    QCOMPARE(zone1.secsTo(zone3), 184 * 86400 + 3600);
    QCOMPARE(zone1.daysTo(zone2), 184);
    QCOMPARE(zone1.daysTo(zone3), 184);
    QVERIFY(zone1 < zone2);
    QVERIFY(!(zone2 < zone1));
    QVERIFY(!(zone2 == zone3));

    // Local zone
    KDateTime local1(QDate(2005,7,6), QTime(3,40,0), KDateTime::LocalZone);
    KDateTime local2 = local1.addSecs(184 * 86400);
    QVERIFY(local2.isLocalZone());
    QCOMPARE(local2.timeZone(), losAngeles);
    QCOMPARE(local2.dateTime(), QDateTime(QDate(2006,1,6), QTime(2,40,0), Qt::LocalTime));
    KDateTime local3 = local1.addDays(184);
    QVERIFY(local3.isLocalZone());
    QCOMPARE(local3.dateTime(), QDateTime(QDate(2006,1,6), QTime(3,40,0), Qt::LocalTime));
    KDateTime local4 = local1.addMonths(6);
    QVERIFY(local4.isLocalZone());
    QCOMPARE(local4.dateTime(), local3.dateTime());
    KDateTime local5 = local1.addYears(4);
    QVERIFY(local5.isLocalZone());
    QCOMPARE(local5.dateTime(), QDateTime(QDate(2009,7,6), QTime(3,40,0), Qt::LocalTime));
    QCOMPARE(local1.secsTo(local2), 184 * 86400);
    QCOMPARE(local1.secsTo(local3), 184 * 86400 + 3600);
    QCOMPARE(local1.daysTo(local2), 184);
    QCOMPARE(local1.daysTo(local3), 184);
    QVERIFY(local1 < local2);
    QVERIFY(!(local2 < local1));
    QVERIFY(!(local2 == local3));

    // Clock time
    KDateTime clock1(QDate(2005,7,6), QTime(3,40,0), KDateTime::ClockTime);
    KDateTime clock2 = clock1.addSecs(184 * 86400);
    QVERIFY(clock2.isClockTime());
    QCOMPARE(clock2.dateTime(), QDateTime(QDate(2006,1,6), QTime(3,40,0), Qt::LocalTime));
    KDateTime clock3 = clock1.addDays(184);
    QVERIFY(clock3.isClockTime());
    QCOMPARE(clock2.dateTime(), clock3.dateTime());
    KDateTime clock4 = clock1.addMonths(6);
    QVERIFY(clock4.isClockTime());
    QCOMPARE(clock2.dateTime(), clock4.dateTime());
    KDateTime clock5 = clock1.addYears(4);
    QVERIFY(clock5.isClockTime());
    QCOMPARE(clock5.dateTime(), QDateTime(QDate(2009,7,6), QTime(3,40,0), Qt::LocalTime));
    QCOMPARE(clock1.secsTo(clock2), 184 * 86400);
    QCOMPARE(clock1.secsTo(clock3), 184 * 86400);
    QCOMPARE(clock1.daysTo(clock2), 184);
    QCOMPARE(clock1.daysTo(clock3), 184);
    QVERIFY(clock1 < clock2);
    QVERIFY(!(clock2 < clock1));
    QVERIFY(clock2 == clock3);

    // Mixed timeSpecs
    QCOMPARE(utc1.secsTo(offset1), 5400);
    QCOMPARE(utc1.secsTo(offset2), 184 * 86400 + 5400);
    QCOMPARE(offset2.secsTo(utc1), -(184 * 86400 + 5400));
    QVERIFY(utc1 < offset1);
    QVERIFY(utc1 <= offset1);
    QVERIFY(!(offset1 < utc1));
    QVERIFY(!(offset1 <= utc1));
    QCOMPARE(utc1.secsTo(zone1), -3600);
    QCOMPARE(utc1.secsTo(zone2), 184 * 86400 - 3600);
    QCOMPARE(zone2.secsTo(utc1), -(184 * 86400 - 3600));
    QVERIFY(utc1 > zone1);
    QVERIFY(utc1 >= zone1);
    QVERIFY(!(zone1 > utc1));
    QVERIFY(!(zone1 >= utc1));
    QCOMPARE(utc1.secsTo(local1), 7 * 3600);
    QCOMPARE(utc1.secsTo(local2), 184 * 86400 + 7 * 3600);
    QCOMPARE(local2.secsTo(utc1), -(184 * 86400 + 7 * 3600));
    QVERIFY(utc1 < local1);
    QVERIFY(utc1 <= local1);
    QVERIFY(!(local1 < utc1));
    QVERIFY(!(local1 <= utc1));
    QCOMPARE(utc1.secsTo(clock1), 7 * 3600);
    QCOMPARE(utc1.secsTo(clock2), 184 * 86400 + 8 * 3600);
    QCOMPARE(clock2.secsTo(utc1), -(184 * 86400 + 8 * 3600));
    QVERIFY(utc1 < clock1);
    QVERIFY(utc1 <= clock1);
    QVERIFY(!(clock1 < utc1));
    QVERIFY(!(clock1 <= utc1));

    QCOMPARE(offset1.secsTo(zone1), -9000);
    QCOMPARE(offset1.secsTo(zone2), 184 * 86400 -9000);
    QCOMPARE(zone2.secsTo(offset1), -(184 * 86400 - 9000));
    QVERIFY(offset1 > zone1);
    QVERIFY(offset1 >= zone1);
    QVERIFY(!(zone1 > offset1));
    QVERIFY(!(zone1 >= offset1));
    QCOMPARE(offset1.secsTo(local1), 7*3600 - 5400);
    QCOMPARE(offset1.secsTo(local2), 184 * 86400 + 7*3600 - 5400);
    QCOMPARE(local2.secsTo(offset1), -(184 * 86400 + 7*3600 - 5400));
    QVERIFY(offset1 < local1);
    QVERIFY(offset1 <= local1);
    QVERIFY(!(local1 < offset1));
    QVERIFY(!(local1 <= offset1));
    QCOMPARE(offset1.secsTo(clock1), 7*3600 - 5400);
    QCOMPARE(offset1.secsTo(clock2), 184 * 86400 + 8*3600 - 5400);
    QCOMPARE(clock2.secsTo(offset1), -(184 * 86400 + 8*3600 - 5400));
    QVERIFY(offset1 < clock1);
    QVERIFY(offset1 <= clock1);
    QVERIFY(!(clock1 < offset1));
    QVERIFY(!(clock1 <= offset1));

    QCOMPARE(zone1.secsTo(local1), 8*3600);
    QCOMPARE(zone1.secsTo(local2), 184 * 86400 + 8*3600);
    QCOMPARE(local2.secsTo(zone1), -(184 * 86400 + 8*3600));
    QVERIFY(zone1 < local1);
    QVERIFY(zone1 <= local1);
    QVERIFY(!(local1 < zone1));
    QVERIFY(!(local1 <= zone1));
    QCOMPARE(zone1.secsTo(clock1), 8*3600);
    QCOMPARE(zone1.secsTo(clock2), 184 * 86400 + 9*3600);
    QCOMPARE(clock2.secsTo(zone1), -(184 * 86400 + 9*3600));
    QVERIFY(zone1 < clock1);
    QVERIFY(zone1 <= clock1);
    QVERIFY(!(clock1 < zone1));
    QVERIFY(!(clock1 <= zone1));

    QCOMPARE(local1.secsTo(clock1), 0);
    QCOMPARE(local1.secsTo(clock2), 184 * 86400 + 3600);
    QCOMPARE(clock2.secsTo(local1), -(184 * 86400 + 3600));
    QVERIFY(local1 == clock1);
    QVERIFY(local1 <= clock1);
    QVERIFY(local1 >= clock1);
    QVERIFY(!(local1 < clock1));
    QVERIFY(!(local1 > clock1));

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::addMSecs()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    // UTC
    KDateTime utc1(QDate(2005,7,6), QTime(23,59,0,100), KDateTime::UTC);
    KDateTime utc2 = utc1.addMSecs(59899);
    QVERIFY(utc2.isUtc());
    QCOMPARE(utc2.dateTime(), QDateTime(QDate(2005,7,6), QTime(23,59,59,999), Qt::UTC));
    utc2 = utc1.addMSecs(59900);
    QVERIFY(utc2.isUtc());
    QCOMPARE(utc2.dateTime(), QDateTime(QDate(2005,7,7), QTime(0,0,0,0), Qt::UTC));
    KDateTime utc1a(QDate(2005,7,6), QTime(0,0,5,100), KDateTime::UTC);
    utc2 = utc1a.addMSecs(-5100);
    QVERIFY(utc2.isUtc());
    QCOMPARE(utc2.dateTime(), QDateTime(QDate(2005,7,6), QTime(0,0,0,0), Qt::UTC));
    utc2 = utc1a.addMSecs(-5101);
    QVERIFY(utc2.isUtc());
    QCOMPARE(utc2.dateTime(), QDateTime(QDate(2005,7,5), QTime(23,59,59,999), Qt::UTC));

    // UTC offset
    KDateTime offset1(QDate(2005,7,6), QTime(3,40,0,100), KDateTime::Spec::OffsetFromUTC(-5400));  // -0130
    KDateTime offset2 = offset1.addMSecs(5899);
    QVERIFY(offset2.isOffsetFromUtc());
    QCOMPARE(offset2.utcOffset(), -5400);
    QCOMPARE(offset2.dateTime(), QDateTime(QDate(2005,7,6), QTime(3,40,5,999), Qt::LocalTime));
    offset2 = offset1.addMSecs(5900);
    QVERIFY(offset2.isOffsetFromUtc());
    QCOMPARE(offset2.utcOffset(), -5400);
    QCOMPARE(offset2.dateTime(), QDateTime(QDate(2005,7,6), QTime(3,40,6,0), Qt::LocalTime));
    offset2 = offset1.addMSecs(-5100);
    QVERIFY(offset2.isOffsetFromUtc());
    QCOMPARE(offset2.utcOffset(), -5400);
    QCOMPARE(offset2.dateTime(), QDateTime(QDate(2005,7,6), QTime(3,39,55,0), Qt::LocalTime));
    offset2 = offset1.addMSecs(-5101);
    QVERIFY(offset2.isOffsetFromUtc());
    QCOMPARE(offset2.utcOffset(), -5400);
    QCOMPARE(offset2.dateTime(), QDateTime(QDate(2005,7,6), QTime(3,39,54,999), Qt::LocalTime));

    // Zone
    KDateTime zone1(QDate(2002,3,31), QTime(0,40,0,100), london);   // time changes at 01:00 UTC
    KDateTime zone2 = zone1.addMSecs(3600*1000+899);
    QCOMPARE(zone2.timeZone(), london);
    QCOMPARE(zone2.dateTime(), QDateTime(QDate(2002,3,31), QTime(2,40,0,999), Qt::LocalTime));
    zone2 = zone1.addMSecs(3600*1000+900);
    QCOMPARE(zone2.timeZone(), london);
    QCOMPARE(zone2.dateTime(), QDateTime(QDate(2002,3,31), QTime(2,40,1,0), Qt::LocalTime));
    KDateTime zone1a(QDate(2002,3,31), QTime(2,40,0,100), london);   // time changes at 01:00 UTC
    zone2 = zone1a.addMSecs(-(3600*1000+100));
    QCOMPARE(zone2.timeZone(), london);
    QCOMPARE(zone2.dateTime(), QDateTime(QDate(2002,3,31), QTime(0,40,0,0), Qt::LocalTime));
    zone2 = zone1a.addMSecs(-(3600*1000+101));
    QCOMPARE(zone2.timeZone(), london);
    QCOMPARE(zone2.dateTime(), QDateTime(QDate(2002,3,31), QTime(0,39,59,999), Qt::LocalTime));

    // Local zone
    KDateTime local1(QDate(2002,4,7), QTime(1,59,0,100), KDateTime::LocalZone);   // time changes at 02:00 local
    KDateTime local2 = local1.addMSecs(59899);
    QVERIFY(local2.isLocalZone());
    QCOMPARE(local2.timeZone(), losAngeles);
    QCOMPARE(local2.dateTime(), QDateTime(QDate(2002,4,7), QTime(1,59,59,999), Qt::LocalTime));
    local2 = local1.addMSecs(59900);
    QVERIFY(local2.isLocalZone());
    QCOMPARE(local2.timeZone(), losAngeles);
    QCOMPARE(local2.dateTime(), QDateTime(QDate(2002,4,7), QTime(3,0,0,0), Qt::LocalTime));
    KDateTime local1a(QDate(2002,4,7), QTime(3,0,0,100), KDateTime::LocalZone);   // time changes at 02:00 local
    local2 = local1a.addMSecs(-100);
    QVERIFY(local2.isLocalZone());
    QCOMPARE(local2.timeZone(), losAngeles);
    QCOMPARE(local2.dateTime(), QDateTime(QDate(2002,4,7), QTime(3,0,0,0), Qt::LocalTime));
    local2 = local1a.addMSecs(-101);
    QVERIFY(local2.isLocalZone());
    QCOMPARE(local2.timeZone(), losAngeles);
    QCOMPARE(local2.dateTime(), QDateTime(QDate(2002,4,7), QTime(1,59,59,999), Qt::LocalTime));

    // Clock time
    KDateTime clock1(QDate(2002,4,7), QTime(1,59,0,100), KDateTime::ClockTime);
    KDateTime clock2 = clock1.addMSecs(59899);
    QVERIFY(clock2.isClockTime());
    QCOMPARE(clock2.dateTime(), QDateTime(QDate(2002,4,7), QTime(1,59,59,999), Qt::LocalTime));
    clock2 = clock1.addMSecs(59900);
    QVERIFY(clock2.isClockTime());
    QCOMPARE(clock2.dateTime(), QDateTime(QDate(2002,4,7), QTime(2,0,0,0), Qt::LocalTime));
    KDateTime clock1a(QDate(2002,4,7), QTime(0,0,0,100), KDateTime::ClockTime);
    clock2 = clock1a.addMSecs(-100);
    QVERIFY(clock2.isClockTime());
    QCOMPARE(clock2.dateTime(), QDateTime(QDate(2002,4,7), QTime(0,0,0,0), Qt::LocalTime));
    clock2 = clock1a.addMSecs(-101);
    QVERIFY(clock2.isClockTime());
    QCOMPARE(clock2.dateTime(), QDateTime(QDate(2002,4,6), QTime(23,59,59,999), Qt::LocalTime));

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::addSubtractDate()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    // UTC
    KDateTime utc1(QDate(2005,7,6), KDateTime::UTC);
    KDateTime utc2 = utc1.addSecs(184 * 86400 + 100);
    QVERIFY(utc2.isUtc());
    QVERIFY(utc2.isDateOnly());
    QCOMPARE(utc2.dateTime(), QDateTime(QDate(2006,1,6), QTime(0,0,0), Qt::UTC));
    KDateTime utc3 = utc1.addDays(184);
    QVERIFY(utc3.isUtc());
    QVERIFY(utc3.isDateOnly());
    QCOMPARE(utc2.dateTime(), utc3.dateTime());
    KDateTime utc4 = utc1.addMonths(6);
    QVERIFY(utc4.isUtc());
    QVERIFY(utc4.isDateOnly());
    QCOMPARE(utc2.dateTime(), utc4.dateTime());
    KDateTime utc5 = utc1.addYears(4);
    QVERIFY(utc5.isUtc());
    QVERIFY(utc5.isDateOnly());
    QCOMPARE(utc5.dateTime(), QDateTime(QDate(2009,7,6), QTime(0,0,0), Qt::UTC));
    QCOMPARE(utc1.secsTo(utc2), 184 * 86400);
    QCOMPARE(utc1.secsTo(utc3), 184 * 86400);
    QCOMPARE(utc1.daysTo(utc2), 184);
    QVERIFY(utc1 < utc2);
    QVERIFY(!(utc2 < utc1));
    QVERIFY(utc2 == utc3);

    // UTC offset
    KDateTime offset1(QDate(2005,7,6), KDateTime::Spec::OffsetFromUTC(-5400));  // -0130
    KDateTime offset2 = offset1.addSecs(184 * 86400);
    QVERIFY(offset2.isDateOnly());
    QVERIFY(offset2.isOffsetFromUtc());
    QCOMPARE(offset2.utcOffset(), -5400);
    QCOMPARE(offset2.dateTime(), QDateTime(QDate(2006,1,6), QTime(0,0,0), Qt::LocalTime));
    KDateTime offset3 = offset1.addDays(184);
    QVERIFY(offset3.isDateOnly());
    QVERIFY(offset3.isOffsetFromUtc());
    QCOMPARE(offset3.utcOffset(), -5400);
    QCOMPARE(offset2.dateTime(), offset3.dateTime());
    KDateTime offset4 = offset1.addMonths(6);
    QVERIFY(offset4.isDateOnly());
    QVERIFY(offset4.isOffsetFromUtc());
    QCOMPARE(offset4.utcOffset(), -5400);
    QCOMPARE(offset2.dateTime(), offset4.dateTime());
    KDateTime offset5 = offset1.addYears(4);
    QVERIFY(offset5.isDateOnly());
    QVERIFY(offset5.isOffsetFromUtc());
    QCOMPARE(offset5.utcOffset(), -5400);
    QCOMPARE(offset5.dateTime(), QDateTime(QDate(2009,7,6), QTime(0,0,0), Qt::LocalTime));
    QCOMPARE(offset1.secsTo(offset2), 184 * 86400);
    QCOMPARE(offset1.secsTo(offset3), 184 * 86400);
    QCOMPARE(offset1.daysTo(offset2), 184);
    QVERIFY(offset1 < offset2);
    QVERIFY(!(offset2 < offset1));
    QVERIFY(offset2 == offset3);

    // Zone
    KDateTime zone1(QDate(2005,7,6), london);
    KDateTime zone2 = zone1.addSecs(184 * 86400);
    QVERIFY(zone2.isDateOnly());
    QCOMPARE(zone2.timeZone(), london);
    QCOMPARE(zone2.dateTime(), QDateTime(QDate(2006,1,6), QTime(0,0,0), Qt::LocalTime));
    KDateTime zone3 = zone1.addDays(184);
    QVERIFY(zone3.isDateOnly());
    QCOMPARE(zone3.timeZone(), london);
    QCOMPARE(zone3.dateTime(), QDateTime(QDate(2006,1,6), QTime(0,0,0), Qt::LocalTime));
    KDateTime zone4 = zone1.addMonths(6);
    QVERIFY(zone4.isDateOnly());
    QCOMPARE(zone4.timeZone(), london);
    QCOMPARE(zone4.dateTime(), zone3.dateTime());
    KDateTime zone5 = zone1.addYears(4);
    QVERIFY(zone5.isDateOnly());
    QCOMPARE(zone5.timeZone(), london);
    QCOMPARE(zone5.dateTime(), QDateTime(QDate(2009,7,6), QTime(0,0,0), Qt::LocalTime));
    QCOMPARE(zone1.secsTo(zone2), 184 * 86400);
    QCOMPARE(zone1.secsTo(zone3), 184 * 86400);
    QCOMPARE(zone1.daysTo(zone2), 184);
    QCOMPARE(zone1.daysTo(zone3), 184);
    QVERIFY(zone1 < zone2);
    QVERIFY(!(zone2 < zone1));
    QVERIFY(zone2 == zone3);

    // Local zone
    KDateTime local1(QDate(2005,7,6), KDateTime::LocalZone);
    KDateTime local2 = local1.addSecs(184 * 86400);
    QVERIFY(local2.isDateOnly());
    QVERIFY(local2.isLocalZone());
    QCOMPARE(local2.timeZone(), losAngeles);
    QCOMPARE(local2.dateTime(), QDateTime(QDate(2006,1,6), QTime(0,0,0), Qt::LocalTime));
    KDateTime local3 = local1.addDays(184);
    QVERIFY(local3.isDateOnly());
    QVERIFY(local3.isLocalZone());
    QCOMPARE(local3.dateTime(), QDateTime(QDate(2006,1,6), QTime(0,0,0), Qt::LocalTime));
    KDateTime local4 = local1.addMonths(6);
    QVERIFY(local4.isDateOnly());
    QVERIFY(local4.isLocalZone());
    QCOMPARE(local4.dateTime(), local3.dateTime());
    KDateTime local5 = local1.addYears(4);
    QVERIFY(local5.isDateOnly());
    QVERIFY(local5.isLocalZone());
    QCOMPARE(local5.dateTime(), QDateTime(QDate(2009,7,6), QTime(0,0,0), Qt::LocalTime));
    QCOMPARE(local1.secsTo(local2), 184 * 86400);
    QCOMPARE(local1.secsTo(local3), 184 * 86400);
    QCOMPARE(local1.daysTo(local2), 184);
    QCOMPARE(local1.daysTo(local3), 184);
    QVERIFY(local1 < local2);
    QVERIFY(!(local2 < local1));
    QVERIFY(local2 == local3);

    // Clock time
    KDateTime clock1(QDate(2005,7,6), KDateTime::ClockTime);
    KDateTime clock2 = clock1.addSecs(184 * 86400);
    QVERIFY(clock2.isDateOnly());
    QVERIFY(clock2.isClockTime());
    QCOMPARE(clock2.dateTime(), QDateTime(QDate(2006,1,6), QTime(0,0,0), Qt::LocalTime));
    KDateTime clock3 = clock1.addDays(184);
    QVERIFY(clock3.isDateOnly());
    QVERIFY(clock3.isClockTime());
    QCOMPARE(clock2.dateTime(), clock3.dateTime());
    KDateTime clock4 = clock1.addMonths(6);
    QVERIFY(clock4.isDateOnly());
    QVERIFY(clock4.isClockTime());
    QCOMPARE(clock2.dateTime(), clock4.dateTime());
    KDateTime clock5 = clock1.addYears(4);
    QVERIFY(clock5.isDateOnly());
    QVERIFY(clock5.isClockTime());
    QCOMPARE(clock5.dateTime(), QDateTime(QDate(2009,7,6), QTime(0,0,0), Qt::LocalTime));
    QCOMPARE(clock1.secsTo(clock2), 184 * 86400);
    QCOMPARE(clock1.secsTo(clock3), 184 * 86400);
    QCOMPARE(clock1.daysTo(clock2), 184);
    QCOMPARE(clock1.daysTo(clock3), 184);
    QVERIFY(clock1 < clock2);
    QVERIFY(!(clock2 < clock1));
    QVERIFY(clock2 == clock3);

    // Mixed timeSpecs
    QCOMPARE(utc1.secsTo(offset1), 0);
    QCOMPARE(utc1.secsTo(offset2), 184 * 86400);
    QCOMPARE(offset2.secsTo(utc1), -(184 * 86400));
    QVERIFY(!(utc1 < offset1));
    QVERIFY(utc1 <= offset1);
    QVERIFY(!(offset1 < utc1));
    QVERIFY(offset1 <= utc1);
    QCOMPARE(utc1.secsTo(zone1), 0);
    QCOMPARE(utc1.secsTo(zone2), 184 * 86400);
    QCOMPARE(zone2.secsTo(utc1), -(184 * 86400));
    QVERIFY(!(utc1 > zone1));
    QVERIFY(utc1 >= zone1);
    QVERIFY(!(zone1 > utc1));
    QVERIFY(zone1 >= utc1);
    QCOMPARE(utc1.secsTo(local1), 0);
    QCOMPARE(utc1.secsTo(local2), 184 * 86400);
    QCOMPARE(local2.secsTo(utc1), -(184 * 86400));
    QVERIFY(!(utc1 < local1));
    QVERIFY(utc1 <= local1);
    QVERIFY(!(local1 < utc1));
    QVERIFY(local1 <= utc1);
    QCOMPARE(utc1.secsTo(clock1), 0);
    QCOMPARE(utc1.secsTo(clock2), 184 * 86400);
    QCOMPARE(clock2.secsTo(utc1), -(184 * 86400));
    QVERIFY(!(utc1 < clock1));
    QVERIFY(utc1 <= clock1);
    QVERIFY(!(clock1 < utc1));
    QVERIFY(clock1 <= utc1);

    QCOMPARE(offset1.secsTo(zone1), 0);
    QCOMPARE(offset1.secsTo(zone2), 184 * 86400);
    QCOMPARE(zone2.secsTo(offset1), -(184 * 86400));
    QVERIFY(!(offset1 > zone1));
    QVERIFY(offset1 >= zone1);
    QVERIFY(!(zone1 > offset1));
    QVERIFY(zone1 >= offset1);
    QCOMPARE(offset1.secsTo(local1), 0);
    QCOMPARE(offset1.secsTo(local2), 184 * 86400);
    QCOMPARE(local2.secsTo(offset1), -(184 * 86400));
    QVERIFY(!(offset1 < local1));
    QVERIFY(offset1 <= local1);
    QVERIFY(!(local1 < offset1));
    QVERIFY(local1 <= offset1);
    QCOMPARE(offset1.secsTo(clock1), 0);
    QCOMPARE(offset1.secsTo(clock2), 184 * 86400);
    QCOMPARE(clock2.secsTo(offset1), -(184 * 86400));
    QVERIFY(!(offset1 < clock1));
    QVERIFY(offset1 <= clock1);
    QVERIFY(!(clock1 < offset1));
    QVERIFY(clock1 <= offset1);

    QCOMPARE(zone1.secsTo(local1), 0);
    QCOMPARE(zone1.secsTo(local2), 184 * 86400);
    QCOMPARE(local2.secsTo(zone1), -(184 * 86400));
    QVERIFY(!(zone1 < local1));
    QVERIFY(zone1 <= local1);
    QVERIFY(!(local1 < zone1));
    QVERIFY(local1 <= zone1);
    QCOMPARE(zone1.secsTo(clock1), 0);
    QCOMPARE(zone1.secsTo(clock2), 184 * 86400);
    QCOMPARE(clock2.secsTo(zone1), -(184 * 86400));
    QVERIFY(!(zone1 < clock1));
    QVERIFY(zone1 <= clock1);
    QVERIFY(!(clock1 < zone1));
    QVERIFY(clock1 <= zone1);

    QCOMPARE(local1.secsTo(clock1), 0);
    QCOMPARE(local1.secsTo(clock2), 184 * 86400);
    QCOMPARE(clock2.secsTo(local1), -(184 * 86400));
    QVERIFY(local1 == clock1);
    QVERIFY(local1 <= clock1);
    QVERIFY(local1 >= clock1);
    QVERIFY(!(local1 < clock1));
    QVERIFY(!(local1 > clock1));


    // Mixed date/time and date-only

    // UTC
    utc3.setTime(QTime(13,14,15));
    QVERIFY(!utc3.isDateOnly());
    QCOMPARE(utc3.time(), QTime(13,14,15));
    QCOMPARE(utc1.secsTo(utc3), 184 * 86400);

    KDateTime utc1t(QDate(2005,7,6), QTime(3,40,0), KDateTime::UTC);
    QCOMPARE(utc1t.secsTo(utc2), 184 * 86400);

    // UTC offset
    offset3.setTime(QTime(13,14,15));
    QVERIFY(!offset3.isDateOnly());
    QCOMPARE(offset3.time(), QTime(13,14,15));
    QCOMPARE(offset1.secsTo(offset3), 184 * 86400);

    KDateTime offset1t(QDate(2005,7,6), QTime(3,40,0), KDateTime::Spec::OffsetFromUTC(-5400));  // -0130
    QCOMPARE(offset1t.secsTo(offset2), 184 * 86400);

    KDateTime offset2t(QDate(2005,7,6), QTime(0,40,0), KDateTime::Spec::OffsetFromUTC(5400));  // +0130

    // Zone
    zone3.setTime(QTime(13,14,15));
    QVERIFY(!zone3.isDateOnly());
    QCOMPARE(zone3.time(), QTime(13,14,15));
    QCOMPARE(zone1.secsTo(zone3), 184 * 86400);

    KDateTime zone1t(QDate(2005,7,6), QTime(3,40,0), london);
    QCOMPARE(zone1t.secsTo(zone2), 184 * 86400);

    // Local zone
    local3.setTime(QTime(13,14,15));
    QVERIFY(!local3.isDateOnly());
    QCOMPARE(local3.time(), QTime(13,14,15));
    QCOMPARE(local1.secsTo(local3), 184 * 86400);

    KDateTime local1t(QDate(2005,7,6), QTime(3,40,0), KDateTime::LocalZone);
    QCOMPARE(local1t.secsTo(local2), 184 * 86400);

    KDateTime local2t(QDate(2005,7,5), QTime(23,40,0), KDateTime::LocalZone);

    // Clock time
    clock3.setTime(QTime(13,14,15));
    QVERIFY(!clock3.isDateOnly());
    QCOMPARE(clock3.time(), QTime(13,14,15));
    QCOMPARE(clock1.secsTo(clock3), 184 * 86400);

    KDateTime clock1t(QDate(2005,7,6), QTime(3,40,0), KDateTime::ClockTime);
    QCOMPARE(clock1t.secsTo(clock2), 184 * 86400);

    // Mixed timeSpecs
    QCOMPARE(utc1t.secsTo(offset1), 0);
    QVERIFY(utc1t != offset1);
    QVERIFY(offset1 != utc1t);
    QVERIFY(!(utc1t < offset1));
    QVERIFY(utc1t <= offset1);
    QVERIFY(!(offset1 < utc1t));
    QVERIFY(offset1 <= utc1t);
    QCOMPARE(utc1.secsTo(offset2t), -86400);
    QCOMPARE(offset2t.secsTo(utc1), 86400);
    QVERIFY(utc1 != offset2t);
    QVERIFY(offset2t != utc1);
    QVERIFY(utc1 > offset2t);
    QVERIFY(utc1 >= offset2t);
    QVERIFY(offset2t < utc1);
    QVERIFY(offset2t <= utc1);
    QCOMPARE(utc1t.secsTo(offset2), 184 * 86400);
    QCOMPARE(offset2.secsTo(utc1t), -(184 * 86400));
    QCOMPARE(utc1t.secsTo(zone1), 0);
    QVERIFY(utc1t != zone1);
    QVERIFY(zone1 != utc1t);
    QVERIFY(!(utc1t < zone1));
    QVERIFY(!(utc1t > zone1));
    QVERIFY(!(zone1 < utc1t));
    QVERIFY(!(zone1 > utc1t));
    QCOMPARE(utc1t.secsTo(zone2), 184 * 86400);
    QCOMPARE(zone2.secsTo(utc1t), -(184 * 86400));
    QVERIFY(utc1t != zone2);
    QVERIFY(zone2 != utc1t);
    QVERIFY(utc1t < zone2);
    QVERIFY(utc1t <= zone2);
    QVERIFY(!(zone2 < utc1t));
    QVERIFY(!(zone2 <= utc1t));
    QCOMPARE(utc1t.secsTo(local1), 86400);
    QCOMPARE(utc1t.secsTo(local2), 185 * 86400);
    QCOMPARE(local2.secsTo(utc1t), -(185 * 86400));
    QVERIFY(utc1t != local1);
    QVERIFY(local1 != utc1t);
    QVERIFY(utc1t < local1);
    QVERIFY(utc1t <= local1);
    QVERIFY(!(local1 < utc1t));
    QVERIFY(!(local1 <= utc1t));
    QCOMPARE(utc1.secsTo(local2t), 0);
    QCOMPARE(local2t.secsTo(utc1), 0);
    QVERIFY(utc1 != local2t);
    QVERIFY(local2t != utc1);
    QVERIFY(!(utc1 < local2t));
    QVERIFY(utc1 <= local2t);
    QVERIFY(!(local2t < utc1));
    QVERIFY(local2t <= utc1);
    QCOMPARE(utc1t.secsTo(clock1), 86400);
    QCOMPARE(utc1t.secsTo(clock2), 185 * 86400);
    QCOMPARE(clock2.secsTo(utc1t), -(185 * 86400));
    QVERIFY(utc1t != clock1);
    QVERIFY(clock1 != utc1t);
    QVERIFY(utc1t < clock1);
    QVERIFY(utc1t <= clock1);
    QVERIFY(!(clock1 < utc1t));
    QVERIFY(!(clock1 <= utc1t));

    QCOMPARE(offset1t.secsTo(zone1), 0);
    QCOMPARE(offset1t.secsTo(zone2), 184 * 86400);
    QCOMPARE(zone2.secsTo(offset1t), -(184 * 86400));
    QVERIFY(offset1t != zone1);
    QVERIFY(zone1 != offset1t);
    QVERIFY(!(offset1t > zone1));
    QVERIFY(offset1t >= zone1);
    QVERIFY(!(zone1 > offset1t));
    QVERIFY(zone1 >= offset1t);
    QCOMPARE(offset1t.secsTo(local1), 86400);
    QCOMPARE(offset1t.secsTo(local2), 185 * 86400);
    QCOMPARE(local2.secsTo(offset1t), -(185 * 86400));
    QVERIFY(offset1t != local1);
    QVERIFY(local1 != offset1t);
    QVERIFY(offset1t < local1);
    QVERIFY(offset1t <= local1);
    QVERIFY(!(local1 < offset1t));
    QVERIFY(!(local1 <= offset1t));
    QCOMPARE(offset1t.secsTo(clock1), 86400);
    QCOMPARE(offset1t.secsTo(clock2), 185 * 86400);
    QCOMPARE(clock2.secsTo(offset1t), -(185 * 86400));
    QVERIFY(offset1t != clock1);
    QVERIFY(clock1 != offset1t);
    QVERIFY(offset1t < clock1);
    QVERIFY(offset1t <= clock1);
    QVERIFY(!(clock1 < offset1t));
    QVERIFY(!(clock1 <= offset1t));

    QCOMPARE(zone1t.secsTo(local1), 86400);
    QCOMPARE(zone1t.secsTo(local2), 185 * 86400);
    QCOMPARE(local2.secsTo(zone1t), -(185 * 86400));
    QVERIFY(zone1t != local1);
    QVERIFY(local1 != zone1t);
    QVERIFY(zone1t < local1);
    QVERIFY(zone1t <= local1);
    QVERIFY(!(local1 < zone1t));
    QVERIFY(!(local1 <= zone1t));
    QCOMPARE(zone1t.secsTo(clock1), 86400);
    QCOMPARE(zone1t.secsTo(clock2), 185 * 86400);
    QCOMPARE(clock2.secsTo(zone1t), -(185 * 86400));
    QVERIFY(zone1t != clock1);
    QVERIFY(clock1 != zone1t);
    QVERIFY(zone1t < clock1);
    QVERIFY(zone1t <= clock1);
    QVERIFY(!(clock1 < zone1t));
    QVERIFY(!(clock1 <= zone1t));

    QCOMPARE(local1t.secsTo(clock1), 0);
    QCOMPARE(local1t.secsTo(clock2), 184 * 86400);
    QCOMPARE(clock2.secsTo(local1t), -(184 * 86400));
    QVERIFY(local1t != clock1);
    QVERIFY(local1t != clock1);
    QVERIFY(!(local1t < clock1));
    QVERIFY(local1t <= clock1);
    QVERIFY(!(local1t < clock1));
    QVERIFY(local1t <= clock1);

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}


///////////////////////////////////////////
// Tests around daylight saving time shifts
///////////////////////////////////////////

void KDateTimeTest::dstShifts()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    // Shift from DST to standard time for the UK in 2005 was at 2005-10-30 01:00 UTC.
    QDateTime qdt(QDate(2005,10,29), QTime(23,59,59), Qt::UTC);
    KDateTime dt(qdt, london);
    QVERIFY(!dt.isSecondOccurrence());
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,10,30), QTime(0,59,59), Qt::LocalTime));
    dt = KDateTime(QDateTime(QDate(2005,10,30), QTime(0,0,0), Qt::UTC), london);
    QVERIFY(!dt.isSecondOccurrence());
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime));
    dt = KDateTime(QDateTime(QDate(2005,10,30), QTime(0,59,59), Qt::UTC), london);
    QVERIFY(!dt.isSecondOccurrence());
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime));
    dt = KDateTime(QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::UTC), london);
    QVERIFY(dt.isSecondOccurrence());
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime));
    dt = KDateTime(QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::UTC), london);
    QVERIFY(dt.isSecondOccurrence());
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime));
    dt = KDateTime(QDateTime(QDate(2005,10,30), QTime(2,0,0), Qt::UTC), london);
    QVERIFY(!dt.isSecondOccurrence());
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,10,30), QTime(2,0,0), Qt::LocalTime));

    dt = KDateTime(QDateTime(QDate(2005,10,30), QTime(0,59,59), Qt::LocalTime), london);
    dt.setSecondOccurrence(true);   // this has no effect
    QCOMPARE(dt.toUtc().dateTime(), QDateTime(QDate(2005,10,29), QTime(23,59,59), Qt::UTC));
    dt = KDateTime(QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime), london);
    QVERIFY(!dt.isSecondOccurrence());
    QCOMPARE(dt.toUtc().dateTime(), QDateTime(QDate(2005,10,30), QTime(0,0,0), Qt::UTC));
    dt = KDateTime(QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime), london);
    QVERIFY(!dt.isSecondOccurrence());
    QCOMPARE(dt.toUtc().dateTime(), QDateTime(QDate(2005,10,30), QTime(0,59,59), Qt::UTC));
    dt = KDateTime(QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::LocalTime), london);
    dt.setSecondOccurrence(true);
    QCOMPARE(dt.toUtc().dateTime(), QDateTime(QDate(2005,10,30), QTime(1,0,0), Qt::UTC));
    QVERIFY(dt.isSecondOccurrence());
    dt = KDateTime(QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::LocalTime), london);
    dt.setSecondOccurrence(true);
    QCOMPARE(dt.toUtc().dateTime(), QDateTime(QDate(2005,10,30), QTime(1,59,59), Qt::UTC));
    QVERIFY(dt.isSecondOccurrence());
    dt = KDateTime(QDateTime(QDate(2005,10,30), QTime(2,0,0), Qt::LocalTime), london);
    dt.setSecondOccurrence(true);   // this has no effect
    QVERIFY(!dt.isSecondOccurrence());
    QCOMPARE(dt.toUtc().dateTime(), QDateTime(QDate(2005,10,30), QTime(2,0,0), Qt::UTC));

    dt = KDateTime(QDateTime(QDate(2005,10,30), QTime(0,59,59), Qt::LocalTime), london);
    KDateTime dt1 = dt.addSecs(1);   // local time 01:00:00
    QVERIFY(!dt1.isSecondOccurrence());
    dt1 = dt.addSecs(3600);   // local time 01:59:59
    QVERIFY(!dt1.isSecondOccurrence());
    dt1 = dt.addSecs(3601);   // local time 01:00:00
    QVERIFY(dt1.isSecondOccurrence());
    dt1 = dt.addSecs(7200);   // local time 01:59:59
    QVERIFY(dt1.isSecondOccurrence());
    dt1 = dt.addSecs(7201);   // local time 02:00:00
    QVERIFY(!dt1.isSecondOccurrence());

    QVERIFY(KDateTime(QDate(2005,10,29), london) == KDateTime(QDate(2005,10,29), KDateTime::Spec::OffsetFromUTC(3600)));
    QVERIFY(KDateTime(QDate(2005,10,30), london) != KDateTime(QDate(2005,10,30), KDateTime::Spec::OffsetFromUTC(3600)));
    QVERIFY(KDateTime(QDate(2005,10,30), london) != KDateTime(QDate(2005,10,30), KDateTime::Spec::OffsetFromUTC(0)));
    QVERIFY(KDateTime(QDate(2005,10,31), london) == KDateTime(QDate(2005,10,31), KDateTime::Spec::OffsetFromUTC(0)));

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}


////////////////////
// String conversion
////////////////////

void KDateTimeTest::strings_iso8601()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    bool decpt = KGlobal::locale()->decimalSymbol() == QLatin1String(".");   // whether this locale uses '.' as decimal symbol

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    KDateTime dtlocal(QDate(1999,12,11), QTime(3,45,06,12), KDateTime::LocalZone);
    QString s = dtlocal.toString(KDateTime::ISODate);
    if (decpt)
        QCOMPARE(s, QString("1999-12-11T03:45:06.012-08:00"));
    else
        QCOMPARE(s, QString("1999-12-11T03:45:06,012-08:00"));
    KDateTime dtlocal1 = KDateTime::fromString(s, KDateTime::ISODate);
    QCOMPARE(dtlocal1.dateTime().toUTC(), dtlocal.dateTime().toUTC());
    QCOMPARE(dtlocal1.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtlocal1.utcOffset(), -8*3600);
    QVERIFY(dtlocal1 == dtlocal);
    dtlocal.setDateOnly(true);
    s = dtlocal.toString(KDateTime::ISODate);
    QCOMPARE(s, QString("1999-12-11T00:00:00-08:00"));

    KDateTime dtzone(QDate(1999,6,11), QTime(3,45,06,12), london);
    s = dtzone.toString(KDateTime::ISODate);
    if (decpt)
        QCOMPARE(s, QString("1999-06-11T03:45:06.012+01:00"));
    else
        QCOMPARE(s, QString("1999-06-11T03:45:06,012+01:00"));
    KDateTime dtzone1 = KDateTime::fromString(s, KDateTime::ISODate);
    QCOMPARE(dtzone1.dateTime().toUTC(), dtzone.dateTime().toUTC());
    QCOMPARE(dtzone1.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtzone1.utcOffset(), 3600);
    QVERIFY(dtzone1 == dtzone);
    dtzone.setDateOnly(true);
    s = dtzone.toString(KDateTime::ISODate);
    QCOMPARE(s, QString("1999-06-11T00:00:00+01:00"));

    KDateTime dtclock(QDate(1999,12,11), QTime(3,45,06), KDateTime::ClockTime);
    s = dtclock.toString(KDateTime::ISODate);
    QCOMPARE(s, QString("1999-12-11T03:45:06"));
    KDateTime dtclock1 = KDateTime::fromString(s, KDateTime::ISODate);
    QCOMPARE(dtclock1.dateTime(), dtclock.dateTime());
    QCOMPARE(dtclock1.timeType(), KDateTime::ClockTime);
    QVERIFY(dtclock1 == dtclock);
    dtclock.setDateOnly(true);
    s = dtclock.toString(KDateTime::ISODate);
    QCOMPARE(s, QString("1999-12-11"));
    dtclock1 = KDateTime::fromString(s, KDateTime::ISODate);
    QVERIFY(dtclock1.isDateOnly());
    QCOMPARE(dtclock1.timeType(), KDateTime::ClockTime);
    QCOMPARE(dtclock1.date(), QDate(1999,12,11));

    KDateTime dtutc(QDate(1999,12,11), QTime(3,45,00), KDateTime::UTC);
    s = dtutc.toString(KDateTime::ISODate);
    QCOMPARE(s, QString("1999-12-11T03:45:00Z"));
    KDateTime dtutc1 = KDateTime::fromString(s, KDateTime::ISODate);
    QCOMPARE(dtutc1.dateTime(), dtutc.dateTime());
    QCOMPARE(dtutc1.timeType(), KDateTime::UTC);
    QVERIFY(dtutc1 == dtutc);
    dtutc.setDateOnly(true);
    s = dtutc.toString(KDateTime::ISODate);
    QCOMPARE(s, QString("1999-12-11T00:00:00Z"));

    // Check signed years
    KDateTime dtneg(QDate(-1999,12,11), QTime(3,45,06), KDateTime::ClockTime);
    s = dtneg.toString(KDateTime::ISODate);
    QCOMPARE(s, QString("-1999-12-11T03:45:06"));
    KDateTime dtneg1 = KDateTime::fromString(s, KDateTime::ISODate);
    QCOMPARE(dtneg1.dateTime(), dtneg.dateTime());
    QCOMPARE(dtneg1.timeType(), KDateTime::ClockTime);
    QVERIFY(dtneg1 == dtneg);
    KDateTime dtneg2 = KDateTime::fromString(QString("-19991211T034506"), KDateTime::ISODate);
    QVERIFY(dtneg2 == dtneg);

    dtneg.setDateOnly(true);
    s = dtneg.toString(KDateTime::ISODate);
    QCOMPARE(s, QString("-1999-12-11"));
    dtneg1 = KDateTime::fromString(s, KDateTime::ISODate);
    QVERIFY(dtneg1.isDateOnly());
    QCOMPARE(dtneg1.timeType(), KDateTime::ClockTime);
    QCOMPARE(dtneg1.date(), QDate(-1999,12,11));
    dtneg2 = KDateTime::fromString(QString("-19991211"), KDateTime::ISODate);
    QVERIFY(dtneg2 == dtneg1);

    s = QString("+1999-12-11T03:45:06");
    KDateTime dtpos = KDateTime::fromString(s, KDateTime::ISODate);
    QCOMPARE(dtpos.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,06)));
    QCOMPARE(dtpos.timeType(), KDateTime::ClockTime);
    KDateTime dtpos2 = KDateTime::fromString(QString("+19991211T034506"), KDateTime::ISODate);
    QVERIFY(dtpos2 == dtpos);

    dtpos.setDateOnly(true);
    s = QString("+1999-12-11");
    dtpos = KDateTime::fromString(s, KDateTime::ISODate);
    QVERIFY(dtpos.isDateOnly());
    QCOMPARE(dtpos.timeType(), KDateTime::ClockTime);
    QCOMPARE(dtpos.date(), QDate(1999,12,11));
    dtpos2 = KDateTime::fromString(QString("+19991211"), KDateTime::ISODate);
    QVERIFY(dtpos2 == dtpos);

    // Check years with >4 digits
    KDateTime dtbig(QDate(123456,12,11), QTime(3,45,06), KDateTime::ClockTime);
    s = dtbig.toString(KDateTime::ISODate);
    QCOMPARE(s, QString("123456-12-11T03:45:06"));
    KDateTime dtbig1 = KDateTime::fromString(s, KDateTime::ISODate);
    QCOMPARE(dtbig1.dateTime(), dtbig.dateTime());
    QCOMPARE(dtbig1.timeType(), KDateTime::ClockTime);
    QVERIFY(dtbig1 == dtbig);
    KDateTime dtbig2 = KDateTime::fromString(QString("1234561211T034506"), KDateTime::ISODate);
    QVERIFY(dtbig2 == dtbig);

    dtbig.setDateOnly(true);
    s = dtbig.toString(KDateTime::ISODate);
    QCOMPARE(s, QString("123456-12-11"));
    dtbig1 = KDateTime::fromString(s, KDateTime::ISODate);
    QVERIFY(dtbig1.isDateOnly());
    QCOMPARE(dtbig1.timeType(), KDateTime::ClockTime);
    QCOMPARE(dtbig1.date(), QDate(123456,12,11));
    dtbig2 = KDateTime::fromString(QString("1234561211"), KDateTime::ISODate);
    QVERIFY(dtbig2 == dtbig1);

    // Check basic format strings
    bool negZero = true;
    KDateTime dt = KDateTime::fromString(QString("20000301T1213"), KDateTime::ISODate, &negZero);
    QVERIFY(dt.timeType() == KDateTime::ClockTime);
    QVERIFY(!dt.isDateOnly());
    QVERIFY(!negZero);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2000,3,1), QTime(12,13,0), Qt::LocalTime));
    dt = KDateTime::fromString(QString("20000301"), KDateTime::ISODate, &negZero);
    QVERIFY(dt.timeType() == KDateTime::ClockTime);
    QVERIFY(dt.isDateOnly());
    QVERIFY(!negZero);
    QCOMPARE(dt.date(), QDate(2000,3,1));
    KDateTime::setFromStringDefault(KDateTime::UTC);
    dt = KDateTime::fromString(QString("20000301T1213"), KDateTime::ISODate);
    QVERIFY(dt.timeType() == KDateTime::UTC);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2000,3,1), QTime(12,13,0), Qt::UTC));
    KDateTime::setFromStringDefault(KDateTime::LocalZone);
    dt = KDateTime::fromString(QString("20000301T1213"), KDateTime::ISODate);
    QVERIFY(dt.timeSpec() == KDateTime::Spec::LocalZone());
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2000,3,1), QTime(12,13,0), Qt::LocalTime));
    KDateTime::setFromStringDefault(london);
    dt = KDateTime::fromString(QString("20000301T1213"), KDateTime::ISODate);
    QVERIFY(dt.timeType() == KDateTime::TimeZone);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2000,3,1), QTime(12,13,0), Qt::LocalTime));
    KDateTime::setFromStringDefault(KDateTime::Spec::OffsetFromUTC(5000));  // = +01:23:20
    dt = KDateTime::fromString(QString("20000601T1213"), KDateTime::ISODate);
    QVERIFY(dt.timeType() == KDateTime::OffsetFromUTC);
    QCOMPARE(dt.utcOffset(), 5000);
    QCOMPARE(dt.toUtc().dateTime(), QDateTime(QDate(2000,6,1), QTime(10,49,40), Qt::UTC));
    KDateTime::setFromStringDefault(KDateTime::ClockTime);
    dt = KDateTime::fromString(QString("6543210301T1213"), KDateTime::ISODate, &negZero);
    QVERIFY(dt.timeType() == KDateTime::ClockTime);
    QVERIFY(!dt.isDateOnly());
    QVERIFY(!negZero);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(654321,3,1), QTime(12,13,0), Qt::LocalTime));
    dt = KDateTime::fromString(QString("6543210301"), KDateTime::ISODate, &negZero);
    QVERIFY(dt.isDateOnly());
    QVERIFY(!negZero);
    QCOMPARE(dt.date(), QDate(654321,3,1));
    dt = KDateTime::fromString(QString("-47120301T1213"), KDateTime::ISODate, &negZero);
    QVERIFY(dt.timeType() == KDateTime::ClockTime);
    QVERIFY(!dt.isDateOnly());
    QVERIFY(!negZero);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(-4712,3,1), QTime(12,13,0), Qt::LocalTime));
    dt = KDateTime::fromString(QString("-47120301"), KDateTime::ISODate, &negZero);
    QVERIFY(dt.isDateOnly());
    QVERIFY(!negZero);
    QCOMPARE(dt.date(), QDate(-4712,3,1));

    // Check strings containing day-of-the-year
    dt = KDateTime::fromString(QString("1999-060T19:20:21.06-11:20"), KDateTime::ISODate);
    QVERIFY(dt.timeType() == KDateTime::OffsetFromUTC);
    QCOMPARE(dt.utcOffset(), -11*3600 - 20*60);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(1999,3,1), QTime(19,20,21,60), Qt::LocalTime));
    dt = KDateTime::fromString(QString("1999-060T19:20:21,06-11:20"), KDateTime::ISODate);
    QVERIFY(dt.timeType() == KDateTime::OffsetFromUTC);
    QCOMPARE(dt.utcOffset(), -11*3600 - 20*60);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(1999,3,1), QTime(19,20,21,60), Qt::LocalTime));
    dt = KDateTime::fromString(QString("1999060T192021.06-1120"), KDateTime::ISODate);
    QVERIFY(dt.timeType() == KDateTime::OffsetFromUTC);
    QCOMPARE(dt.utcOffset(), -11*3600 - 20*60);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(1999,3,1), QTime(19,20,21,60), Qt::LocalTime));
    dt = KDateTime::fromString(QString("1999-060"), KDateTime::ISODate);
    QVERIFY(dt.timeType() == KDateTime::ClockTime);
    QVERIFY(dt.isDateOnly());
    QCOMPARE(dt.date(), QDate(1999,3,1));

    // Check 24:00:00
    dt = KDateTime::fromString(QString("1999-06-11T24:00:00+03:00"), KDateTime::ISODate);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(1999,6,12), QTime(0,0,0), Qt::LocalTime));
    dt = KDateTime::fromString(QString("1999-06-11T24:00:01+03:00"), KDateTime::ISODate);
    QVERIFY(!dt.isValid());

    // Check leap second
    dt = KDateTime::fromString(QString("1999-06-11T23:59:60Z"), KDateTime::ISODate);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(1999,6,11), QTime(23,59,59), Qt::UTC));
    dt = KDateTime::fromString(QString("1999-06-11T13:59:60Z"), KDateTime::ISODate);
    QVERIFY(!dt.isValid());
    dt = KDateTime::fromString(QString("1999-06-11T13:59:60-10:00"), KDateTime::ISODate);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(1999,6,11), QTime(13,59,59), Qt::LocalTime));
    dt = KDateTime::fromString(QString("1999-06-11T23:59:60-10:00"), KDateTime::ISODate);
    QVERIFY(!dt.isValid());

    // Check negZero
    dt = KDateTime::fromString(QString("1999-060T19:20:21.06-00:00"), KDateTime::ISODate, &negZero);
    QVERIFY(negZero);
    dt = KDateTime::fromString(QString("1999-060T19:20:21.06+00:00"), KDateTime::ISODate, &negZero);
    QVERIFY(!negZero);

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::strings_rfc2822()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    bool negZero = true;
    KDateTime dtlocal(QDate(1999,12,11), QTime(3,45,06), KDateTime::LocalZone);
    QString s = dtlocal.toString(KDateTime::RFCDate);
    QCOMPARE(s, QString("11 Dec 1999 03:45:06 -0800"));
    KDateTime dtlocal1 = KDateTime::fromString(s, KDateTime::RFCDate, &negZero);
    QCOMPARE(dtlocal1.dateTime().toUTC(), dtlocal.dateTime().toUTC());
    QCOMPARE(dtlocal1.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtlocal1.utcOffset(), -8*3600);
    QVERIFY(dtlocal1 == dtlocal);
    QVERIFY(!negZero);
    KDateTime dtlocal2 = KDateTime::fromString(s, KDateTime::RFCDateDay);
    QVERIFY(!dtlocal2.isValid());
    s = dtlocal.toString(KDateTime::RFCDateDay);
    QCOMPARE(s, QString("Sat, 11 Dec 1999 03:45:06 -0800"));
    dtlocal2 = KDateTime::fromString(s, KDateTime::RFCDate);
    QVERIFY(dtlocal1 == dtlocal2);
    QCOMPARE(dtlocal1.dateTime(), dtlocal2.dateTime());
    dtlocal2 = KDateTime::fromString(s, KDateTime::RFCDateDay);
    QVERIFY(dtlocal1 == dtlocal2);
    dtlocal2 = KDateTime::fromString(QString("Saturday, 11-Dec-99 03:45:06 -0800"), KDateTime::RFCDate);
    QVERIFY(dtlocal1 == dtlocal2);
    dtlocal2 = KDateTime::fromString(QString("11 Dec 1999 03:45:06 PST"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtlocal1 == dtlocal2);
    QVERIFY(!negZero);
    dtlocal.setDateOnly(true);
    s = dtlocal.toString(KDateTime::RFCDate);
    QCOMPARE(s, QString("11 Dec 1999 00:00 -0800"));
    s = dtlocal.toString(KDateTime::RFCDateDay);
    QCOMPARE(s, QString("Sat, 11 Dec 1999 00:00 -0800"));

    KDateTime dtzone(QDate(1999,6,11), QTime(3,45,06), london);
    s = dtzone.toString(KDateTime::RFCDate);
    QCOMPARE(s, QString("11 Jun 1999 03:45:06 +0100"));
    KDateTime dtzone1 = KDateTime::fromString(s, KDateTime::RFCDate);
    QCOMPARE(dtzone1.dateTime().toUTC(), dtzone.dateTime().toUTC());
    QCOMPARE(dtzone1.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtzone1.utcOffset(), 3600);
    QVERIFY(dtzone1 == dtzone);
    KDateTime dtzone2 = KDateTime::fromString(s, KDateTime::RFCDateDay);
    QVERIFY(!dtzone2.isValid());
    s = dtzone.toString(KDateTime::RFCDateDay);
    QCOMPARE(s, QString("Fri, 11 Jun 1999 03:45:06 +0100"));
    dtzone2 = KDateTime::fromString(s, KDateTime::RFCDate);
    QVERIFY(dtzone1 == dtzone2);
    QCOMPARE(dtzone1.dateTime(), dtzone2.dateTime());
    dtzone2 = KDateTime::fromString(s, KDateTime::RFCDateDay, &negZero);
    QVERIFY(dtzone1 == dtzone2);
    QVERIFY(!negZero);
    dtzone2 = KDateTime::fromString(QString("Friday, 11-Jun-99 03:45:06 +0100"), KDateTime::RFCDateDay);
    QVERIFY(dtzone1 == dtzone2);
    dtzone.setDateOnly(true);
    s = dtzone.toString(KDateTime::RFCDate);
    QCOMPARE(s, QString("11 Jun 1999 00:00 +0100"));
    s = dtzone.toString(KDateTime::RFCDateDay);
    QCOMPARE(s, QString("Fri, 11 Jun 1999 00:00 +0100"));

    KDateTime dtclock(QDate(1999,12,11), QTime(3,45,06), KDateTime::ClockTime);
    s = dtclock.toString(KDateTime::RFCDate);
    QCOMPARE(s, QString("11 Dec 1999 03:45:06 -0800"));
    KDateTime dtclock1 = KDateTime::fromString(s, KDateTime::RFCDate, &negZero);
    QCOMPARE(dtclock1.dateTime(), dtclock.dateTime());
    QCOMPARE(dtclock1.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtclock1.utcOffset(), -8*3600);
    QVERIFY(dtclock1 == dtclock);
    QVERIFY(!negZero);
    KDateTime dtclock2 = KDateTime::fromString(s, KDateTime::RFCDateDay);
    QVERIFY(!dtclock2.isValid());
    s = dtclock.toString(KDateTime::RFCDateDay);
    QCOMPARE(s, QString("Sat, 11 Dec 1999 03:45:06 -0800"));
    dtclock2 = KDateTime::fromString(s, KDateTime::RFCDate);
    QVERIFY(dtclock1 == dtclock2);
    QCOMPARE(dtclock1.dateTime(), dtclock2.dateTime());
    dtclock2 = KDateTime::fromString(s, KDateTime::RFCDateDay);
    QVERIFY(dtclock1 == dtclock2);
    dtclock2 = KDateTime::fromString(QString("Saturday, 11-Dec-99 03:45:06 -0800"), KDateTime::RFCDateDay);
    QVERIFY(dtclock1 == dtclock2);
    dtclock.setDateOnly(true);
    s = dtclock.toString(KDateTime::RFCDate);
    QCOMPARE(s, QString("11 Dec 1999 00:00 -0800"));
    s = dtclock.toString(KDateTime::RFCDateDay);
    QCOMPARE(s, QString("Sat, 11 Dec 1999 00:00 -0800"));

    KDateTime dtutc(QDate(1999,12,11), QTime(3,45,00), KDateTime::UTC);
    s = dtutc.toString(KDateTime::RFCDate);
    QCOMPARE(s, QString("11 Dec 1999 03:45 +0000"));
    KDateTime dtutc1 = KDateTime::fromString(s, KDateTime::RFCDate, &negZero);
    QCOMPARE(dtutc1.dateTime(), dtutc.dateTime());
    QCOMPARE(dtutc1.timeType(), KDateTime::UTC);
    QVERIFY(dtutc1 == dtutc);
    QVERIFY(!negZero);
    KDateTime dtutc2 = KDateTime::fromString(s, KDateTime::RFCDateDay);
    QVERIFY(!dtutc2.isValid());
    s = dtutc.toString(KDateTime::RFCDateDay);
    QCOMPARE(s, QString("Sat, 11 Dec 1999 03:45 +0000"));
    dtutc2 = KDateTime::fromString(s, KDateTime::RFCDate);
    QVERIFY(dtutc1 == dtutc2);
    QCOMPARE(dtutc1.dateTime(), dtutc2.dateTime());
    dtutc2 = KDateTime::fromString(s, KDateTime::RFCDateDay);
    QVERIFY(dtutc1 == dtutc2);
    dtutc2 = KDateTime::fromString(QString("Saturday, 11-Dec-99 03:45 +0000"), KDateTime::RFCDate);
    QVERIFY(dtutc1 == dtutc2);
    dtutc.setDateOnly(true);
    s = dtutc.toString(KDateTime::RFCDate);
    QCOMPARE(s, QString("11 Dec 1999 00:00 +0000"));
    s = dtutc.toString(KDateTime::RFCDateDay);
    QCOMPARE(s, QString("Sat, 11 Dec 1999 00:00 +0000"));

    // Check '-0000' and unknown/invalid time zone names
    dtutc2 = KDateTime::fromString(QString("11 Dec 1999 03:45 -0000"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtutc1 == dtutc2);
    QVERIFY(negZero);
    dtutc2 = KDateTime::fromString(QString("11 Dec 1999 03:45 B"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtutc1 == dtutc2);
    QVERIFY(negZero);
    dtutc2 = KDateTime::fromString(QString("11 Dec 1999 03:45 BCDE"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtutc1 == dtutc2);
    QVERIFY(negZero);

    // Check named time offsets
    KDateTime dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 UT"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeType() == KDateTime::UTC);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::UTC));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 GMT"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeType() == KDateTime::UTC);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::UTC));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 EDT"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeType() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.utcOffset(), -4*3600);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::LocalTime));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 EST"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeType() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.utcOffset(), -5*3600);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::LocalTime));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 CDT"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeType() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.utcOffset(), -5*3600);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::LocalTime));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 CST"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeType() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.utcOffset(), -6*3600);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::LocalTime));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 MDT"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeType() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.utcOffset(), -6*3600);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::LocalTime));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 MST"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeType() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.utcOffset(), -7*3600);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::LocalTime));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 PDT"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeType() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.utcOffset(), -7*3600);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::LocalTime));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 PST"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeType() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.utcOffset(), -8*3600);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::LocalTime));
    QVERIFY(!negZero);

    // Check leap second
    KDateTime dt = KDateTime::fromString(QString("11 Dec 1999 23:59:60 -0000"), KDateTime::RFCDate);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(1999,12,11), QTime(23,59,59), Qt::UTC));
    dt = KDateTime::fromString(QString("11 Dec 1999 13:59:60 -0000"), KDateTime::RFCDate);
    QVERIFY(!dt.isValid());
    dt = KDateTime::fromString(QString("11 Jun 1999 13:59:60 -1000"), KDateTime::RFCDate);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(1999,6,11), QTime(13,59,59), Qt::LocalTime));
    dt = KDateTime::fromString(QString("11 Dec 1999 23:59:60 -1000"), KDateTime::RFCDate);
    QVERIFY(!dt.isValid());

    // Check erroneous strings:
    dtutc2 = KDateTime::fromString(QString("11 Dec 1999 23:59:60 -00:00"), KDateTime::RFCDate);
    QVERIFY(!dtutc2.isValid());     // colon in UTC offset
    dtutc2 = KDateTime::fromString(QString("Sun, 11 Dec 1999 03:45 +0000"), KDateTime::RFCDate);
    QVERIFY(!dtutc2.isValid());     // wrong weekday
    dtutc2 = KDateTime::fromString(QString("Satu, 11 Dec 1999 03:45 +0000"), KDateTime::RFCDate);
    QVERIFY(!dtutc2.isValid());     // bad weekday
    dtutc2 = KDateTime::fromString(QString("11 Dece 1999 03:45 +0000"), KDateTime::RFCDate);
    QVERIFY(!dtutc2.isValid());     // bad month
    dtutc2 = KDateTime::fromString(QString("11-Dec 1999 03:45 +0000"), KDateTime::RFCDate);
    QVERIFY(!dtutc2.isValid());     // only one hyphen in date

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::strings_qttextdate()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    bool negZero = true;
    KDateTime dtlocal(QDate(1999,12,11), QTime(3,45,06), KDateTime::LocalZone);
    QString s = dtlocal.toString(KDateTime::QtTextDate);
    QCOMPARE(s, QString("Sat Dec 11 03:45:06 1999 -0800"));
    KDateTime dtlocal1 = KDateTime::fromString(s, KDateTime::QtTextDate, &negZero);
    QCOMPARE(dtlocal1.dateTime().toUTC(), dtlocal.dateTime().toUTC());
    QCOMPARE(dtlocal1.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtlocal1.utcOffset(), -8*3600);
    QVERIFY(dtlocal1 == dtlocal);
    QVERIFY(!dtlocal1.isDateOnly());
    QVERIFY(!negZero);
    dtlocal.setDateOnly(true);
    s = dtlocal.toString(KDateTime::QtTextDate);
    QCOMPARE(s, QString("Sat Dec 11 1999 -0800"));
    dtlocal1 = KDateTime::fromString(s, KDateTime::QtTextDate, &negZero);
    QVERIFY(dtlocal1.isDateOnly());
    QCOMPARE(dtlocal1.date(), QDate(1999,12,11));
    QCOMPARE(dtlocal1.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtlocal1.utcOffset(), -8*3600);

    KDateTime dtzone(QDate(1999,6,11), QTime(3,45,06), london);
    s = dtzone.toString(KDateTime::QtTextDate);
    QCOMPARE(s, QString("Fri Jun 11 03:45:06 1999 +0100"));
    KDateTime dtzone1 = KDateTime::fromString(s, KDateTime::QtTextDate);
    QCOMPARE(dtzone1.dateTime().toUTC(), dtzone.dateTime().toUTC());
    QCOMPARE(dtzone1.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtzone1.utcOffset(), 3600);
    QVERIFY(!dtzone1.isDateOnly());
    QVERIFY(dtzone1 == dtzone);
    KDateTime dtzone2 = KDateTime::fromString(s, KDateTime::QtTextDate, &negZero);
    QVERIFY(dtzone1 == dtzone2);
    QVERIFY(!negZero);
    dtzone.setDateOnly(true);
    s = dtzone.toString(KDateTime::QtTextDate);
    QCOMPARE(s, QString("Fri Jun 11 1999 +0100"));
    dtzone1 = KDateTime::fromString(s, KDateTime::QtTextDate, &negZero);
    QVERIFY(dtzone1.isDateOnly());
    QCOMPARE(dtzone1.date(), QDate(1999,6,11));
    QCOMPARE(dtzone1.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtzone1.utcOffset(), 3600);

    KDateTime dtclock(QDate(1999,12,11), QTime(3,45,06), KDateTime::ClockTime);
    s = dtclock.toString(KDateTime::QtTextDate);
    QCOMPARE(s, QString("Sat Dec 11 03:45:06 1999"));
    KDateTime dtclock1 = KDateTime::fromString(s, KDateTime::QtTextDate, &negZero);
    QCOMPARE(dtclock1.dateTime(), dtclock.dateTime());
    QCOMPARE(dtclock1.timeType(), KDateTime::ClockTime);
    QVERIFY(dtclock1 == dtclock);
    QVERIFY(!dtclock1.isDateOnly());
    QVERIFY(!negZero);
    dtclock.setDateOnly(true);
    s = dtclock.toString(KDateTime::QtTextDate);
    QCOMPARE(s, QString("Sat Dec 11 1999"));
    dtclock1 = KDateTime::fromString(s, KDateTime::QtTextDate, &negZero);
    QVERIFY(dtclock1.isDateOnly());
    QCOMPARE(dtclock1.date(), QDate(1999,12,11));
    QCOMPARE(dtclock1.timeType(), KDateTime::ClockTime);

    KDateTime dtutc(QDate(1999,12,11), QTime(3,45,00), KDateTime::UTC);
    s = dtutc.toString(KDateTime::QtTextDate);
    QCOMPARE(s, QString("Sat Dec 11 03:45:00 1999 +0000"));
    KDateTime dtutc1 = KDateTime::fromString(s, KDateTime::QtTextDate, &negZero);
    QCOMPARE(dtutc1.dateTime(), dtutc.dateTime());
    QCOMPARE(dtutc1.timeType(), KDateTime::UTC);
    QVERIFY(dtutc1 == dtutc);
    QVERIFY(!dtutc1.isDateOnly());
    QVERIFY(!negZero);
    dtutc.setDateOnly(true);
    s = dtutc.toString(KDateTime::QtTextDate);
    QCOMPARE(s, QString("Sat Dec 11 1999 +0000"));
    dtutc1 = KDateTime::fromString(s, KDateTime::QtTextDate, &negZero);
    QVERIFY(dtutc1.isDateOnly());
    QCOMPARE(dtutc1.date(), QDate(1999,12,11));
    QCOMPARE(dtutc1.timeType(), KDateTime::UTC);

    // Check '-0000'
    KDateTime dtutc2 = KDateTime::fromString(QString("Sat Dec 11 03:45:00 1999 -0000"), KDateTime::QtTextDate, &negZero);
    QVERIFY(dtutc1 != dtutc2);
    QVERIFY(negZero);

    // Check erroneous strings
    dtutc2 = KDateTime::fromString(QString("Sat Dec 11 03:45:00 1999 GMT"), KDateTime::QtTextDate, &negZero);
    QVERIFY(!dtutc2.isValid());
    dtutc2 = KDateTime::fromString(QString("Sun Dec 11 03:45:00 1999 +0000"), KDateTime::QtTextDate);
    QVERIFY(dtutc2.isValid());     // wrong weekday: accepted by Qt!!
    dtutc2 = KDateTime::fromString(QString("Satu, Dec 11 03:45:00 1999 +0000"), KDateTime::QtTextDate);
    QVERIFY(dtutc2.isValid());     // bad weekday, accepted by Qt (since 4.3)
    dtutc2 = KDateTime::fromString(QString("Sat Dece 11 03:45:00 1999 +0000"), KDateTime::QtTextDate);
    QVERIFY(!dtutc2.isValid());     // bad month, not accepted by Qt anymore (since 4.3)

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::strings_format()
{
    KCalendarSystem* calendar = KCalendarSystem::create(QString("gregorian"), KGlobal::locale());
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    KTimeZone paris  = KSystemTimeZones::zone("Europe/Paris");
    KTimeZone berlin = KSystemTimeZones::zone("Europe/Berlin");
    KTimeZone cairo  = KSystemTimeZones::zone("Africa/Cairo");
    KTimeZones zones;
    zones.add(london);
    zones.add(paris);
    zones.add(berlin);
    zones.add(cairo);

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    // toString()
    QString all = QLatin1String("%Y.%y.%m.%:m.%B.%b.%d.%e.%A.%a-%H.%k.%I.%l.%M.%S?%:s?%P.%p.%:u.%z.%Z.%:Z.%:A.%:a.%:B.%:b/%:S.%:z.%%.");
    KDateTime dt(QDate(1999,2,3), QTime(6,5,0), KDateTime::LocalZone);
    QString s = dt.toString(all);
    QCOMPARE(s, QString::fromLatin1("1999.99.02.2.%1.%2.03.3.%3.%4-06.6.06.6.05.00?000?am.AM.-08.-0800.PST.America/Los_Angeles.Wednesday.Wed.February.Feb/.-08:00.%.")
                                   .arg(calendar->monthName(2,1999,KCalendarSystem::LongName))
                                   .arg(calendar->monthName(2,1999,KCalendarSystem::ShortName))
                                   .arg(calendar->weekDayName(3,KCalendarSystem::LongDayName))
                                   .arg(calendar->weekDayName(3,KCalendarSystem::ShortDayName)));

    KDateTime dtzone(QDate(1970,4,30), QTime(12,45,16,25), london);
    s = dtzone.toString(all);
    QCOMPARE(s, QString::fromLatin1("1970.70.04.4.%1.%2.30.30.%3.%4-12.12.12.12.45.16?025?pm.PM.+01.+0100.BST.Europe/London.Thursday.Thu.April.Apr/:16.+01:00.%.")
                                   .arg(calendar->monthName(4,2000,KCalendarSystem::LongName))
                                   .arg(calendar->monthName(4,2000,KCalendarSystem::ShortName))
                                   .arg(calendar->weekDayName(4,KCalendarSystem::LongDayName))
                                   .arg(calendar->weekDayName(4,KCalendarSystem::ShortDayName)));

    KDateTime dtclock(QDate(2005,9,5), QTime(0,0,06,1), KDateTime::ClockTime);
    s = dtclock.toString(all);
    QCOMPARE(s, QString::fromLatin1("2005.05.09.9.%1.%2.05.5.%3.%4-00.0.12.12.00.06?001?am.AM.....Monday.Mon.September.Sep/:06..%.")
                                   .arg(calendar->monthName(9,2000,KCalendarSystem::LongName))
                                   .arg(calendar->monthName(9,2000,KCalendarSystem::ShortName))
                                   .arg(calendar->weekDayName(1,KCalendarSystem::LongDayName))
                                   .arg(calendar->weekDayName(1,KCalendarSystem::ShortDayName)));

    KDateTime dtutc(QDate(2000,12,31), QTime(13,45,16,100), KDateTime::UTC);
    s = dtutc.toString(all);
    QCOMPARE(s, QString::fromLatin1("2000.00.12.12.%1.%2.31.31.%3.%4-13.13.01.1.45.16?100?pm.PM.+00.+0000.UTC.UTC.Sunday.Sun.December.Dec/:16.+00:00.%.")
                                   .arg(calendar->monthName(12,2000,KCalendarSystem::LongName))
                                   .arg(calendar->monthName(12,2000,KCalendarSystem::ShortName))
                                   .arg(calendar->weekDayName(7,KCalendarSystem::LongDayName))
                                   .arg(calendar->weekDayName(7,KCalendarSystem::ShortDayName)));

    // fromString() without KTimeZones parameter
    dt = KDateTime::fromString(QLatin1String("2005/9/05/20:2,03"), QLatin1String("%Y/%:m/%d/%S:%k,%M"));
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(2,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeType(), KDateTime::ClockTime);

    dt = KDateTime::fromString(QString::fromLatin1("%1pm05ab%2t/052/20:2,03+10")
                                   .arg(calendar->weekDayName(1,KCalendarSystem::LongDayName))
                                   .arg(calendar->monthName(9,1999,KCalendarSystem::LongName)),
                               QLatin1String("%a%p%yab%Bt/%e2/%S:%l,%M %z"));
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dt.utcOffset(), 10*3600);
    dt = KDateTime::fromString(QString::fromLatin1("%1pm05ab%2t/052/20:2,03+10")
                                   .arg(calendar->weekDayName(1,KCalendarSystem::ShortDayName))
                                   .arg(calendar->monthName(9,1999,KCalendarSystem::ShortName)),
                               QLatin1String("%a%p%yab%Bt/%d2/%s:%l,%:M %z"));
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dt.utcOffset(), 10*3600);
    dt = KDateTime::fromString(QString::fromLatin1("monpm05absEpt/052/20:2,03+10"), QLatin1String("%a%p%yab%Bt/%d2/%S:%l,%M %z"));
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dt.utcOffset(), 10*3600);
    dt = KDateTime::fromString(QString::fromLatin1("monDAYpm05absEptemBert/052/20:2,03+10"), QLatin1String("%a%p%yab%Bt/%e2/%S:%l,%M %z"));
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dt.utcOffset(), 10*3600);
    dt = KDateTime::fromString(QString::fromLatin1("monDAYpm05abmzatemer/052/20:2,03+10"), QLatin1String("%a%p%yab%B/%e2/%S:%l,%M %z"));
    QVERIFY(!dt.isValid());    // invalid month name
    dt = KDateTime::fromString(QString::fromLatin1("monDApm05absep/052/20:2,03+10"), QLatin1String("%a%p%yab%B/%e2/%S:%l,%M %z"));
    QVERIFY(!dt.isValid());    // invalid day name
    dt = KDateTime::fromString(QLatin1String("mONdAYPM2005absEpt/052/20:02,03+1000"), QLatin1String("%:A%:p%Yab%Bt/%d2/%S:%I,%M %:u"));
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,3,20), Qt::LocalTime));
    QCOMPARE(dt.utcOffset(), 10*3600);
    QCOMPARE(dt.timeType(), KDateTime::OffsetFromUTC);
    dtclock = KDateTime::fromString(QLatin1String("mONdAYPM2005abSept/052/20:02,03+100"), QLatin1String("%:A%:p%Yab%Bt/%e2/%S:%l,%M %:u"));
    QVERIFY(!dtclock.isValid());    // wrong number of digits in UTC offset
    dtclock = KDateTime::fromString(QLatin1String("mONdAYPM2005abSept/052/20:02,03+1"), QLatin1String("%:A%:p%Yab%Bt/%d2/%S:%I,%M %z"));
    QVERIFY(!dtclock.isValid());    // wrong number of digits in UTC offset
    dtclock = KDateTime::fromString(QLatin1String("mONdAYPM2005absEpt/052/20:13,03+1000"), QLatin1String("%:A%:p%Yab%Bt/%d2/%S:%I,%M %:u"));
    QVERIFY(!dtclock.isValid());    // hours out of range for am/pm
    dtclock = KDateTime::fromString(QLatin1String("mONdAYPM2005absEpt/052/20:00,03+1000"), QLatin1String("%:A%:p%Yab%Bt/%d2/%S:%I,%M %:u"));
    QVERIFY(!dtclock.isValid());    // hours out of range for am/pm

    // fromString() with KTimeZones parameter
    dt = KDateTime::fromString(QLatin1String("mon 2005/9/05/20:2,03"), QLatin1String("%:a %Y/%:m/%e/%S:%k,%M"), &zones);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(2,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeType(), KDateTime::ClockTime);
    dt = KDateTime::fromString(QLatin1String("tue 2005/9/05/20:2,03"), QLatin1String("%:a %Y/%:m/%d/%S:%k,%M"), &zones);
    QVERIFY(!dt.isValid());    // wrong day-of-week

    dt = KDateTime::fromString(QLatin1String("pm2005absEpt/05monday/20:2,03+03:00"), QLatin1String("%p%Yab%Bt/%e%:A/%S:%l,%M %:z"), &zones);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeType(), KDateTime::TimeZone);
    QCOMPARE(dt.utcOffset(), 3*3600);
    QCOMPARE(dt.timeZone(), cairo);
    dt = KDateTime::fromString(QLatin1String("pm2005absEpt/05sunday/20:2,03+03:00"), QLatin1String("%p%Yab%Bt/%d%A/%S:%l,%M %:z"), &zones);
    QVERIFY(!dt.isValid());    // wrong day-of-week

    dt = KDateTime::fromString(QLatin1String("200509051430:01.3+0100"), QLatin1String("%Y%m%d%H%M%:S%:s%z"), &zones, true);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,30,01,300), Qt::LocalTime));
    QCOMPARE(dt.timeType(), KDateTime::TimeZone);
    QCOMPARE(dt.timeZone(), london);
    QCOMPARE(dt.utcOffset(), 3600);

    dt = KDateTime::fromString(QLatin1String("200509051430:01.3+0500"), QLatin1String("%Y%m%d%H%M%:S%:s%z"), &zones, false);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,30,01,300), Qt::LocalTime));
    QCOMPARE(dt.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dt.utcOffset(), 5*3600);

    dt = KDateTime::fromString(QLatin1String("200509051430:01.3+0200"), QLatin1String("%Y%m%d%H%M%:S%:s%z"), &zones, true);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,30,01,300), Qt::LocalTime));
    QCOMPARE(dt.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dt.utcOffset(), 2*3600);
    dt = KDateTime::fromString(QLatin1String("200509051430:01.3+0200"), QLatin1String("%Y%m%d%H%M%:S%:s%z"), &zones, false);
    QVERIFY(!dt.isValid());    // matches paris and berlin

    dt = KDateTime::fromString(QLatin1String("2005September051430 CEST"), QLatin1String("%Y%:B%d%H%M%:S %Z"), &zones, true);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,30,0), Qt::LocalTime));
    QCOMPARE(dt.timeType(), KDateTime::OffsetFromUTC);
    QCOMPARE(dt.utcOffset(), 2*3600);
    dt = KDateTime::fromString(QLatin1String("2005September051430 CEST"), QLatin1String("%Y%:B%d%H%M%:S %Z"), &zones, false);
    QVERIFY(!dt.isValid());    // matches paris and berlin

    dt = KDateTime::fromString(QLatin1String("pm05absEptembeRt/   052/   20:12,03+0100"), QLatin1String("%:P%yab%:bt/  %e2/%t%S:%l,%M %z"), &zones);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(12,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeType(), KDateTime::TimeZone);
    QCOMPARE(dt.utcOffset(), 3600);
    QCOMPARE(dt.timeZone(), london);

    dt = KDateTime::fromString(QLatin1String("2005absEpt/042sun/20.0123456:12Am,3Africa/Cairo%"), QLatin1String("%Yab%bt/%e2%a/%S%:s:%I%P,%:M %:Z%%"), &zones);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,4), QTime(0,3,20,12), Qt::LocalTime));
    QCOMPARE(dt.timeType(), KDateTime::TimeZone);
    QCOMPARE(dt.timeZone(), cairo);
    QCOMPARE(dt.utcOffset(), 3*3600);

    // Test large and minimum date values
    dt = KDateTime(QDate(-2005,9,5), QTime(0,0,06,1), KDateTime::ClockTime);
    s = dt.toString(QLatin1String("%Y"));
    QCOMPARE(s, QString::fromLatin1("-2005"));

    dt = KDateTime(QDate(-15,9,5), QTime(0,0,06,1), KDateTime::ClockTime);
    s = dt.toString(QLatin1String("%Y"));
    QCOMPARE(s, QString::fromLatin1("-0015"));

    dt = KDateTime::fromString(QLatin1String("-471209051430:01.3+0500"), QLatin1String("%Y%m%d%H%M%:S%:s%z"));
    QCOMPARE(dt.dateTime(), QDateTime(QDate(-4712,9,5), QTime(14,30,1,300), Qt::LocalTime));
    QCOMPARE(dt.utcOffset(), 5*3600);
    QVERIFY(dt.isValid());
    QVERIFY(!dt.outOfRange());

    dt = KDateTime::fromString(QLatin1String("999909051430:01.3+0500"), QLatin1String("%Y%m%d%H%M%:S%:s%z"));
    QCOMPARE(dt.dateTime(), QDateTime(QDate(9999,9,5), QTime(14,30,1,300), Qt::LocalTime));
    QCOMPARE(dt.utcOffset(), 5*3600);
    QVERIFY(dt.isValid());
    QVERIFY(!dt.outOfRange());

    dt = KDateTime::fromString(QLatin1String("123456.09051430:01.3+0500"), QLatin1String("%:Y.%m%d%H%M%:S%:s%z"));
    QCOMPARE(dt.dateTime(), QDateTime(QDate(123456,9,5), QTime(14,30,1,300), Qt::LocalTime));
    QCOMPARE(dt.utcOffset(), 5*3600);
    QVERIFY(dt.isValid());
    QVERIFY(!dt.outOfRange());
    s = dt.toString(QLatin1String("%Y"));
    QCOMPARE(s, QString::fromLatin1("123456"));

    dt = KDateTime::fromString(QLatin1String("-471412311430:01.3+0500"), QLatin1String("%Y%m%d%H%M%:S%:s%z"));
    QVERIFY(!dt.isValid());    // too early
    QVERIFY(dt.outOfRange());


    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::cache()
{
    KTimeZone london = KSystemTimeZones::zone("Europe/London");
    KTimeZone losAngeles = KSystemTimeZones::zone("America/Los_Angeles");
    KTimeZone cairo = KSystemTimeZones::zone("Africa/Cairo");

    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":Europe/London", 1);
    ::tzset();

    // Ensure that local time is different from UTC and different from 'london'
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    int utcHit  = KDateTime_utcCacheHit;
    int zoneHit = KDateTime_zoneCacheHit;
    KDateTime local(QDate(2005,6,1), QTime(12,0,0), KDateTime::LocalZone);
    QCOMPARE(KDateTime_utcCacheHit, utcHit);
    QCOMPARE(KDateTime_zoneCacheHit, zoneHit);
    KDateTime dt1 = local.toZone(london);
    QCOMPARE(KDateTime_utcCacheHit, utcHit);
    QCOMPARE(KDateTime_zoneCacheHit, zoneHit);
    KDateTime cai = local.toZone(cairo);
    ++utcHit;
    QCOMPARE(KDateTime_utcCacheHit, utcHit);
    QCOMPARE(KDateTime_zoneCacheHit, zoneHit);
    KDateTime dt2a = local.toZone(london);
    ++utcHit;
    QCOMPARE(KDateTime_utcCacheHit, utcHit);
    QCOMPARE(KDateTime_zoneCacheHit, zoneHit);
    KDateTime dt2 = local.toZone(london);
    ++zoneHit;
    QCOMPARE(KDateTime_utcCacheHit, utcHit);
    QCOMPARE(KDateTime_zoneCacheHit, zoneHit);
    KDateTime dt3 = dt2;
    QCOMPARE(KDateTime_utcCacheHit, utcHit);
    QCOMPARE(KDateTime_zoneCacheHit, zoneHit);
    KDateTime dt4 = dt2.toZone(losAngeles);
    ++zoneHit;
    QCOMPARE(KDateTime_utcCacheHit, utcHit);
    QCOMPARE(KDateTime_zoneCacheHit, zoneHit);
    KDateTime dt4a = dt3.toZone(losAngeles);
    ++zoneHit;
    QCOMPARE(KDateTime_utcCacheHit, utcHit);
    QCOMPARE(KDateTime_zoneCacheHit, zoneHit);
    KDateTime dt5 = dt2.toZone(losAngeles);
    ++zoneHit;
    QCOMPARE(KDateTime_utcCacheHit, utcHit);
    QCOMPARE(KDateTime_zoneCacheHit, zoneHit);
    KDateTime dt5a = dt3.toZone(losAngeles);
    ++zoneHit;
    QCOMPARE(KDateTime_utcCacheHit, utcHit);
    QCOMPARE(KDateTime_zoneCacheHit, zoneHit);
    KDateTime dt6 = dt2.toZone(cairo);
    ++utcHit;
    QCOMPARE(KDateTime_utcCacheHit, utcHit);
    QCOMPARE(KDateTime_zoneCacheHit, zoneHit);
    KDateTime dt6a = dt3.toZone(cairo);
    ++zoneHit;
    QCOMPARE(KDateTime_utcCacheHit, utcHit);
    QCOMPARE(KDateTime_zoneCacheHit, zoneHit);
    dt3.detach();
    KDateTime dt7 = dt2.toZone(london);
    QCOMPARE(KDateTime_utcCacheHit, utcHit);
    QCOMPARE(KDateTime_zoneCacheHit, zoneHit);
    KDateTime dt7a = dt3.toZone(london);
    QCOMPARE(KDateTime_utcCacheHit, utcHit);
    QCOMPARE(KDateTime_zoneCacheHit, zoneHit);

    // Check that cached time zone conversions are cleared correctly
    KDateTime utc1(QDate(2005,7,6), QTime(3,40,0), KDateTime::UTC);
    KDateTime la1 = utc1.toTimeSpec(losAngeles);
    KDateTime utc2 = utc1.addDays(1);
    KDateTime la2 = utc2.toTimeSpec(losAngeles);
    QVERIFY(la1 != la2);
    QCOMPARE(la1.secsTo(la2), 86400);

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::misc()
{
    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    KDateTime local = KDateTime::currentLocalDateTime();
    KDateTime utc = KDateTime::currentUtcDateTime();
    QDateTime qcurrent = QDateTime::currentDateTime();
    // Because 3 calls to fetch the current time were made, they will differ slightly
    KDateTime localUtc = local.toUtc();
    int diff = localUtc.secsTo(utc);
    if (diff > 1  ||  diff < 0)
        QCOMPARE(local.toUtc().dateTime(), utc.dateTime());
    diff = local.dateTime().secsTo(qcurrent);
    if (diff > 1  ||  diff < 0)
        QCOMPARE(local.dateTime(), qcurrent);

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}
