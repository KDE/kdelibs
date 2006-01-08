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

#include <cstdlib>
#include <qtest_kde.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcalendarsystemgregorian.h>
#include "kdatetime.h"
#include "kdatetimetest.moc"


QTEST_KDEMAIN(KDateTimeTest, NoGUI)


//////////////////////////////////////////////////////
// Constructors and basic property information methods
//////////////////////////////////////////////////////

void KDateTimeTest::constructors()
{
    QDate d(2001,2,13);
    QTime t(3,45,14);
    QDateTime dtLocal(d, t, Qt::LocalTime);
    QDateTime dtUTC(d, t, Qt::UTC);
    const KTimezone *london = KSystemTimezones::zone("Europe/London");

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
    QCOMPARE(datetimeL.timeSpec(), KDateTime::LocalZone);
    QVERIFY(datetimeL.isLocalZone());
    QVERIFY(!datetimeL.isUTC());
    QVERIFY(!datetimeL.isOffsetFromUTC());
    QVERIFY(!datetimeL.isClockTime());
    QCOMPARE(datetimeL.UTCOffset(), -8*3600);
    QCOMPARE(datetimeL.timeZone(), KSystemTimezones::local());
    QCOMPARE(datetimeL.dateTime(), dtLocal);

    KDateTime datetimeU(dtUTC);
    QVERIFY(!datetimeU.isNull());
    QVERIFY(datetimeU.isValid());
    QVERIFY(!datetimeU.isDateOnly());
    QCOMPARE(datetimeU.timeSpec(), KDateTime::UTC);
    QVERIFY(!datetimeU.isLocalZone());
    QVERIFY(datetimeU.isUTC());
    QVERIFY(!datetimeU.isOffsetFromUTC());
    QVERIFY(!datetimeU.isClockTime());
    QCOMPARE(datetimeU.UTCOffset(), 0);
    QCOMPARE(datetimeU.timeZone(), KTimezones::utc());
    QCOMPARE(datetimeU.dateTime(), dtUTC);


    // Time zone
    KDateTime dateTz(d, london);
    QVERIFY(!dateTz.isNull());
    QVERIFY(dateTz.isValid());
    QVERIFY(dateTz.isDateOnly());
    QCOMPARE(dateTz.timeSpec(), KDateTime::TimeZone);
    QVERIFY(!dateTz.isUTC());
    QVERIFY(!dateTz.isOffsetFromUTC());
    QVERIFY(!dateTz.isLocalZone());
    QVERIFY(!dateTz.isClockTime());
    QCOMPARE(dateTz.UTCOffset(), 0);
    QCOMPARE(dateTz.timeZone(), london);
    QCOMPARE(dateTz.dateTime(), QDateTime(d, QTime(0,0,0), Qt::LocalTime));
    QCOMPARE(dateTz.date(), d);

    KDateTime dateTimeTz(d, QTime(3,45,14), london);
    QVERIFY(!dateTimeTz.isNull());
    QVERIFY(dateTimeTz.isValid());
    QVERIFY(!dateTimeTz.isDateOnly());
    QCOMPARE(dateTimeTz.timeSpec(), KDateTime::TimeZone);
    QVERIFY(!dateTimeTz.isUTC());
    QVERIFY(!dateTimeTz.isOffsetFromUTC());
    QVERIFY(!dateTimeTz.isLocalZone());
    QVERIFY(!dateTimeTz.isClockTime());
    QCOMPARE(dateTimeTz.UTCOffset(), 0);
    QCOMPARE(dateTimeTz.timeZone(), london);
    QCOMPARE(dateTimeTz.dateTime(), QDateTime(d, QTime(3,45,14), Qt::LocalTime));
    QCOMPARE(dateTimeTz.time(), QTime(3,45,14));

    KDateTime datetimeTz(dtLocal, london);
    QVERIFY(!datetimeTz.isNull());
    QVERIFY(datetimeTz.isValid());
    QVERIFY(!dateTimeTz.isDateOnly());
    QCOMPARE(datetimeTz.timeSpec(), KDateTime::TimeZone);
    QVERIFY(!datetimeTz.isUTC());
    QVERIFY(!datetimeTz.isOffsetFromUTC());
    QVERIFY(!datetimeTz.isLocalZone());
    QVERIFY(!datetimeTz.isClockTime());
    QCOMPARE(datetimeTz.UTCOffset(), 0);
    QCOMPARE(datetimeTz.timeZone(), london);
    QCOMPARE(datetimeTz.dateTime(), dtLocal);

    KDateTime datetimeTz2(dtUTC, london);
    QVERIFY(!datetimeTz2.isNull());
    QVERIFY(datetimeTz2.isValid());
    QVERIFY(!dateTimeTz.isDateOnly());
    QCOMPARE(datetimeTz2.timeSpec(), KDateTime::TimeZone);
    QVERIFY(!datetimeTz2.isUTC());
    QVERIFY(!datetimeTz2.isOffsetFromUTC());
    QVERIFY(!datetimeTz2.isLocalZone());
    QVERIFY(!datetimeTz2.isClockTime());
    QCOMPARE(datetimeTz2.UTCOffset(), 0);
    QCOMPARE(datetimeTz2.timeZone(), london);
    QCOMPARE(datetimeTz2.dateTime(), dtUTCtoLondon);

    // ... copy constructor
    KDateTime datetimeTzCopy(datetimeTz);
    QVERIFY(!datetimeTzCopy.isNull());
    QVERIFY(datetimeTzCopy.isValid());
    QVERIFY(!dateTimeTz.isDateOnly());
    QCOMPARE(datetimeTzCopy.timeSpec(), KDateTime::TimeZone);
    QVERIFY(!datetimeTzCopy.isUTC());
    QVERIFY(!datetimeTzCopy.isOffsetFromUTC());
    QVERIFY(!datetimeTzCopy.isLocalZone());
    QVERIFY(!datetimeTzCopy.isClockTime());
    QCOMPARE(datetimeTzCopy.UTCOffset(), 0);
    QCOMPARE(datetimeTzCopy.timeZone(), datetimeTz.timeZone());
    QCOMPARE(datetimeTzCopy.dateTime(), datetimeTz.dateTime());


    // UTC
    KDateTime date_UTC(d, KDateTime::UTC, 2*3600);
    QVERIFY(!date_UTC.isNull());
    QVERIFY(date_UTC.isValid());
    QVERIFY(date_UTC.isDateOnly());
    QCOMPARE(date_UTC.timeSpec(), KDateTime::UTC);
    QVERIFY(date_UTC.isUTC());
    QVERIFY(!date_UTC.isOffsetFromUTC());
    QVERIFY(!date_UTC.isLocalZone());
    QVERIFY(!date_UTC.isClockTime());
    QCOMPARE(date_UTC.UTCOffset(), 0);
    QCOMPARE(date_UTC.timeZone(), KTimezones::utc());
    QCOMPARE(date_UTC.dateTime(), QDateTime(d, QTime(0,0,0), Qt::UTC));

    KDateTime dateTime_UTC(d, t, KDateTime::UTC, 2*3600);
    QVERIFY(!dateTime_UTC.isNull());
    QVERIFY(dateTime_UTC.isValid());
    QVERIFY(!dateTime_UTC.isDateOnly());
    QCOMPARE(dateTime_UTC.timeSpec(), KDateTime::UTC);
    QVERIFY(dateTime_UTC.isUTC());
    QVERIFY(!dateTime_UTC.isOffsetFromUTC());
    QVERIFY(!dateTime_UTC.isLocalZone());
    QVERIFY(!dateTime_UTC.isClockTime());
    QCOMPARE(dateTime_UTC.UTCOffset(), 0);
    QCOMPARE(dateTime_UTC.timeZone(), KTimezones::utc());
    QCOMPARE(dateTime_UTC.dateTime(), QDateTime(d, t, Qt::UTC));

    KDateTime datetime_UTC(dtLocal, KDateTime::UTC, 2*3600);
    QVERIFY(!datetime_UTC.isNull());
    QVERIFY(datetime_UTC.isValid());
    QVERIFY(!datetime_UTC.isDateOnly());
    QCOMPARE(datetime_UTC.timeSpec(), KDateTime::UTC);
    QVERIFY(datetime_UTC.isUTC());
    QVERIFY(!datetime_UTC.isOffsetFromUTC());
    QVERIFY(!datetime_UTC.isLocalZone());
    QVERIFY(!datetime_UTC.isClockTime());
    QCOMPARE(datetime_UTC.UTCOffset(), 0);
    QCOMPARE(datetime_UTC.timeZone(), KTimezones::utc());
    QCOMPARE(datetime_UTC.dateTime(), dtLocal.toUTC());

    KDateTime datetime_UTC2(dtUTC, KDateTime::UTC, 2*3600);
    QVERIFY(!datetime_UTC2.isNull());
    QVERIFY(datetime_UTC2.isValid());
    QVERIFY(!datetime_UTC2.isDateOnly());
    QCOMPARE(datetime_UTC2.timeSpec(), KDateTime::UTC);
    QVERIFY(datetime_UTC2.isUTC());
    QVERIFY(!datetime_UTC2.isOffsetFromUTC());
    QVERIFY(!datetime_UTC2.isLocalZone());
    QVERIFY(!datetime_UTC2.isClockTime());
    QCOMPARE(datetime_UTC2.UTCOffset(), 0);
    QCOMPARE(datetime_UTC2.timeZone(), KTimezones::utc());
    QCOMPARE(datetime_UTC2.dateTime(), dtUTC);
    QCOMPARE(datetime_UTC2.date(), dtUTC.date());
    QCOMPARE(datetime_UTC2.time(), dtUTC.time());

    // ... copy constructor
    KDateTime datetime_UTCCopy(datetime_UTC);
    QVERIFY(!datetime_UTCCopy.isNull());
    QVERIFY(datetime_UTCCopy.isValid());
    QVERIFY(!datetime_UTCCopy.isDateOnly());
    QCOMPARE(datetime_UTCCopy.timeSpec(), KDateTime::UTC);
    QVERIFY(datetime_UTCCopy.isUTC());
    QVERIFY(!datetime_UTCCopy.isOffsetFromUTC());
    QVERIFY(!datetime_UTCCopy.isLocalZone());
    QVERIFY(!datetime_UTCCopy.isClockTime());
    QCOMPARE(datetime_UTCCopy.UTCOffset(), 0);
    QCOMPARE(datetime_UTCCopy.timeZone(), datetime_UTC.timeZone());
    QCOMPARE(datetime_UTCCopy.dateTime(), datetime_UTC.dateTime());


    // Offset from UTC
    KDateTime date_OffsetFromUTC(d, KDateTime::OffsetFromUTC, -2*3600);
    QVERIFY(!date_OffsetFromUTC.isNull());
    QVERIFY(date_OffsetFromUTC.isValid());
    QVERIFY(date_OffsetFromUTC.isDateOnly());
    QCOMPARE(date_OffsetFromUTC.timeSpec(), KDateTime::OffsetFromUTC);
    QVERIFY(!date_OffsetFromUTC.isUTC());
    QVERIFY(date_OffsetFromUTC.isOffsetFromUTC());
    QVERIFY(!date_OffsetFromUTC.isLocalZone());
    QVERIFY(!date_OffsetFromUTC.isClockTime());
    QCOMPARE(date_OffsetFromUTC.UTCOffset(), -2*3600);
    QVERIFY(!date_OffsetFromUTC.timeZone());
    QCOMPARE(date_OffsetFromUTC.dateTime(), QDateTime(d, QTime(0,0,0), Qt::LocalTime));

    KDateTime dateTime_OffsetFromUTC(d, t, KDateTime::OffsetFromUTC, 2*3600);
    QVERIFY(!dateTime_OffsetFromUTC.isNull());
    QVERIFY(dateTime_OffsetFromUTC.isValid());
    QVERIFY(!dateTime_OffsetFromUTC.isDateOnly());
    QCOMPARE(dateTime_OffsetFromUTC.timeSpec(), KDateTime::OffsetFromUTC);
    QVERIFY(!dateTime_OffsetFromUTC.isUTC());
    QVERIFY(dateTime_OffsetFromUTC.isOffsetFromUTC());
    QVERIFY(!dateTime_OffsetFromUTC.isLocalZone());
    QVERIFY(!dateTime_OffsetFromUTC.isClockTime());
    QCOMPARE(dateTime_OffsetFromUTC.UTCOffset(), 2*3600);
    QVERIFY(!dateTime_OffsetFromUTC.timeZone());
    QCOMPARE(dateTime_OffsetFromUTC.dateTime(), QDateTime(d, t, Qt::LocalTime));

    KDateTime datetime_OffsetFromUTC(dtLocal, KDateTime::OffsetFromUTC, -2*3600);
    QVERIFY(!datetime_OffsetFromUTC.isNull());
    QVERIFY(datetime_OffsetFromUTC.isValid());
    QVERIFY(!datetime_OffsetFromUTC.isDateOnly());
    QCOMPARE(datetime_OffsetFromUTC.timeSpec(), KDateTime::OffsetFromUTC);
    QVERIFY(!datetime_OffsetFromUTC.isUTC());
    QVERIFY(datetime_OffsetFromUTC.isOffsetFromUTC());
    QVERIFY(!datetime_OffsetFromUTC.isLocalZone());
    QVERIFY(!datetime_OffsetFromUTC.isClockTime());
    QCOMPARE(datetime_OffsetFromUTC.UTCOffset(), -2*3600);
    QVERIFY(!datetime_OffsetFromUTC.timeZone());
    QCOMPARE(datetime_OffsetFromUTC.dateTime(), dtLocal);
    QCOMPARE(datetime_OffsetFromUTC.date(), dtLocal.date());
    QCOMPARE(datetime_OffsetFromUTC.time(), dtLocal.time());

    KDateTime datetime_OffsetFromUTC2(dtUTC, KDateTime::OffsetFromUTC, 2*3600);
    QVERIFY(!datetime_OffsetFromUTC2.isNull());
    QVERIFY(datetime_OffsetFromUTC2.isValid());
    QVERIFY(!datetime_OffsetFromUTC2.isDateOnly());
    QCOMPARE(datetime_OffsetFromUTC2.timeSpec(), KDateTime::OffsetFromUTC);
    QVERIFY(!datetime_OffsetFromUTC2.isUTC());
    QVERIFY(datetime_OffsetFromUTC2.isOffsetFromUTC());
    QVERIFY(!datetime_OffsetFromUTC2.isLocalZone());
    QVERIFY(!datetime_OffsetFromUTC2.isClockTime());
    QCOMPARE(datetime_OffsetFromUTC2.UTCOffset(), 2*3600);
    QVERIFY(!datetime_OffsetFromUTC2.timeZone());
    QDateTime dtof = dtUTC.addSecs(2*3600);
    dtof.setTimeSpec(Qt::LocalTime);
    QCOMPARE(datetime_OffsetFromUTC2.dateTime(), dtof);

    // ... copy constructor
    KDateTime datetime_OffsetFromUTCCopy(datetime_OffsetFromUTC);
    QVERIFY(!datetime_OffsetFromUTCCopy.isNull());
    QVERIFY(datetime_OffsetFromUTCCopy.isValid());
    QVERIFY(!datetime_OffsetFromUTCCopy.isDateOnly());
    QCOMPARE(datetime_OffsetFromUTCCopy.timeSpec(), KDateTime::OffsetFromUTC);
    QVERIFY(!datetime_OffsetFromUTCCopy.isUTC());
    QVERIFY(datetime_OffsetFromUTCCopy.isOffsetFromUTC());
    QVERIFY(!datetime_OffsetFromUTCCopy.isLocalZone());
    QVERIFY(!datetime_OffsetFromUTCCopy.isClockTime());
    QCOMPARE(datetime_OffsetFromUTCCopy.UTCOffset(), -2*3600);
    QVERIFY(!datetime_OffsetFromUTCCopy.timeZone());
    QCOMPARE(datetime_OffsetFromUTCCopy.dateTime(), datetime_OffsetFromUTC.dateTime());


    // Local time zone
    KDateTime date_LocalZone(d, KDateTime::LocalZone, 2*3600);
    QVERIFY(!date_LocalZone.isNull());
    QVERIFY(date_LocalZone.isValid());
    QVERIFY(date_LocalZone.isDateOnly());
    QCOMPARE(date_LocalZone.timeSpec(), KDateTime::LocalZone);
    QVERIFY(!date_LocalZone.isUTC());
    QVERIFY(!date_LocalZone.isOffsetFromUTC());
    QVERIFY(date_LocalZone.isLocalZone());
    QVERIFY(!date_LocalZone.isClockTime());
    QCOMPARE(date_LocalZone.UTCOffset(), -8*3600);
    QCOMPARE(date_LocalZone.timeZone(), KSystemTimezones::local());
    QCOMPARE(date_LocalZone.dateTime(), QDateTime(d, QTime(0,0,0), Qt::LocalTime));

    KDateTime dateTime_LocalZone(d, t, KDateTime::LocalZone, 2*3600);
    QVERIFY(!dateTime_LocalZone.isNull());
    QVERIFY(dateTime_LocalZone.isValid());
    QVERIFY(!dateTime_LocalZone.isDateOnly());
    QCOMPARE(dateTime_LocalZone.timeSpec(), KDateTime::LocalZone);
    QVERIFY(!dateTime_LocalZone.isUTC());
    QVERIFY(!dateTime_LocalZone.isOffsetFromUTC());
    QVERIFY(dateTime_LocalZone.isLocalZone());
    QVERIFY(!dateTime_LocalZone.isClockTime());
    QCOMPARE(dateTime_LocalZone.UTCOffset(), -8*3600);
    QCOMPARE(dateTime_LocalZone.timeZone(), KSystemTimezones::local());
    QCOMPARE(dateTime_LocalZone.dateTime(), QDateTime(d, t, Qt::LocalTime));

    KDateTime datetime_LocalZone(dtLocal, KDateTime::LocalZone, 2*3600);
    QVERIFY(!datetime_LocalZone.isNull());
    QVERIFY(datetime_LocalZone.isValid());
    QVERIFY(!datetime_LocalZone.isDateOnly());
    QCOMPARE(datetime_LocalZone.timeSpec(), KDateTime::LocalZone);
    QVERIFY(!datetime_LocalZone.isUTC());
    QVERIFY(!datetime_LocalZone.isOffsetFromUTC());
    QVERIFY(datetime_LocalZone.isLocalZone());
    QVERIFY(!datetime_LocalZone.isClockTime());
    QCOMPARE(datetime_LocalZone.UTCOffset(), -8*3600);
    QCOMPARE(datetime_LocalZone.timeZone(), KSystemTimezones::local());
    QCOMPARE(datetime_LocalZone.dateTime(), dtLocal);
    QCOMPARE(datetime_LocalZone.date(), dtLocal.date());
    QCOMPARE(datetime_LocalZone.time(), dtLocal.time());

    KDateTime datetime_LocalZone2(dtUTC, KDateTime::LocalZone, 2*3600);
    QVERIFY(!datetime_LocalZone2.isNull());
    QVERIFY(datetime_LocalZone2.isValid());
    QVERIFY(!datetime_LocalZone2.isDateOnly());
    QCOMPARE(datetime_LocalZone2.timeSpec(), KDateTime::LocalZone);
    QVERIFY(!datetime_LocalZone2.isUTC());
    QVERIFY(!datetime_LocalZone2.isOffsetFromUTC());
    QVERIFY(datetime_LocalZone2.isLocalZone());
    QVERIFY(!datetime_LocalZone2.isClockTime());
    QCOMPARE(datetime_LocalZone2.UTCOffset(), -8*3600);
    QCOMPARE(datetime_LocalZone2.timeZone(), KSystemTimezones::local());
    QCOMPARE(datetime_LocalZone2.dateTime(), dtUTC.toLocalTime());

    // ... copy constructor
    KDateTime datetime_LocalZoneCopy(datetime_LocalZone);
    QVERIFY(!datetime_LocalZoneCopy.isNull());
    QVERIFY(datetime_LocalZoneCopy.isValid());
    QVERIFY(!datetime_LocalZoneCopy.isDateOnly());
    QCOMPARE(datetime_LocalZoneCopy.timeSpec(), KDateTime::LocalZone);
    QVERIFY(!datetime_LocalZoneCopy.isUTC());
    QVERIFY(!datetime_LocalZoneCopy.isOffsetFromUTC());
    QVERIFY(datetime_LocalZoneCopy.isLocalZone());
    QVERIFY(!datetime_LocalZoneCopy.isClockTime());
    QCOMPARE(datetime_LocalZoneCopy.UTCOffset(), -8*3600);
    QCOMPARE(datetime_LocalZoneCopy.timeZone(), datetime_LocalZone.timeZone());
    QCOMPARE(datetime_LocalZoneCopy.dateTime(), datetime_LocalZone.dateTime());


    // Local clock time
    KDateTime date_ClockTime(d, KDateTime::ClockTime, 2*3600);
    QVERIFY(!date_ClockTime.isNull());
    QVERIFY(date_ClockTime.isValid());
    QVERIFY(date_ClockTime.isDateOnly());
    QCOMPARE(date_ClockTime.timeSpec(), KDateTime::ClockTime);
    QVERIFY(!date_ClockTime.isUTC());
    QVERIFY(!date_ClockTime.isOffsetFromUTC());
    QVERIFY(!date_ClockTime.isLocalZone());
    QVERIFY(date_ClockTime.isClockTime());
    QCOMPARE(date_ClockTime.UTCOffset(), 0);
    QVERIFY(!date_ClockTime.timeZone());
    QCOMPARE(date_ClockTime.dateTime(), QDateTime(d, QTime(0,0,0), Qt::LocalTime));

    KDateTime dateTime_ClockTime(d, t, KDateTime::ClockTime, 2*3600);
    QVERIFY(!dateTime_ClockTime.isNull());
    QVERIFY(dateTime_ClockTime.isValid());
    QVERIFY(!dateTime_ClockTime.isDateOnly());
    QCOMPARE(dateTime_ClockTime.timeSpec(), KDateTime::ClockTime);
    QVERIFY(!dateTime_ClockTime.isUTC());
    QVERIFY(!dateTime_ClockTime.isOffsetFromUTC());
    QVERIFY(!dateTime_ClockTime.isLocalZone());
    QVERIFY(dateTime_ClockTime.isClockTime());
    QCOMPARE(dateTime_ClockTime.UTCOffset(), 0);
    QVERIFY(!dateTime_ClockTime.timeZone());
    QCOMPARE(dateTime_ClockTime.dateTime(), QDateTime(d, t, Qt::LocalTime));

    KDateTime datetime_ClockTime(dtLocal, KDateTime::ClockTime, 2*3600);
    QVERIFY(!datetime_ClockTime.isNull());
    QVERIFY(datetime_ClockTime.isValid());
    QVERIFY(!datetime_ClockTime.isDateOnly());
    QCOMPARE(datetime_ClockTime.timeSpec(), KDateTime::ClockTime);
    QVERIFY(!datetime_ClockTime.isUTC());
    QVERIFY(!datetime_ClockTime.isOffsetFromUTC());
    QVERIFY(!datetime_ClockTime.isLocalZone());
    QVERIFY(datetime_ClockTime.isClockTime());
    QCOMPARE(datetime_ClockTime.UTCOffset(), 0);
    QVERIFY(!datetime_ClockTime.timeZone());
    QCOMPARE(datetime_ClockTime.dateTime(), dtLocal);
    QCOMPARE(datetime_ClockTime.date(), dtLocal.date());
    QCOMPARE(datetime_ClockTime.time(), dtLocal.time());

    KDateTime datetime_ClockTime2(dtUTC, KDateTime::ClockTime, 2*3600);
    QVERIFY(!datetime_ClockTime2.isNull());
    QVERIFY(datetime_ClockTime2.isValid());
    QVERIFY(!datetime_ClockTime2.isDateOnly());
    QCOMPARE(datetime_ClockTime2.timeSpec(), KDateTime::ClockTime);
    QVERIFY(!datetime_ClockTime2.isUTC());
    QVERIFY(!datetime_ClockTime2.isOffsetFromUTC());
    QVERIFY(!datetime_ClockTime2.isLocalZone());
    QVERIFY(datetime_ClockTime2.isClockTime());
    QCOMPARE(datetime_ClockTime2.UTCOffset(), 0);
    QVERIFY(!datetime_ClockTime2.timeZone());
    QCOMPARE(datetime_ClockTime2.dateTime(), dtUTC.toLocalTime());

    // ... copy constructor
    KDateTime datetime_ClockTimeCopy(datetime_ClockTime);
    QVERIFY(!datetime_ClockTimeCopy.isNull());
    QVERIFY(datetime_ClockTimeCopy.isValid());
    QVERIFY(!datetime_ClockTimeCopy.isDateOnly());
    QCOMPARE(datetime_ClockTimeCopy.timeSpec(), KDateTime::ClockTime);
    QVERIFY(!datetime_ClockTimeCopy.isUTC());
    QVERIFY(!datetime_ClockTimeCopy.isOffsetFromUTC());
    QVERIFY(!datetime_ClockTimeCopy.isLocalZone());
    QVERIFY(datetime_ClockTimeCopy.isClockTime());
    QCOMPARE(datetime_ClockTimeCopy.UTCOffset(), 0);
    QVERIFY(!datetime_ClockTimeCopy.timeZone());
    QCOMPARE(datetime_ClockTimeCopy.dateTime(), datetime_ClockTime.dateTime());


    // Invalid time zone specification for a constructor
    KDateTime date_TimeZone(d, KDateTime::TimeZone);
    QVERIFY(!date_TimeZone.isValid());
    KDateTime dateTime_TimeZone(d, t, KDateTime::TimeZone);
    QVERIFY(!dateTime_TimeZone.isValid());
    KDateTime datetime_TimeZone(dtLocal, KDateTime::TimeZone);
    QVERIFY(!datetime_TimeZone.isValid());

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

void KDateTimeTest::toUTC()
{
    const KTimezone *london = KSystemTimezones::zone("Europe/London");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    // Zone -> UTC
    KDateTime londonWinter(QDate(2005,1,1), QTime(0,0,0), london);
    KDateTime utcWinter = londonWinter.toUTC();
    QVERIFY(utcWinter.isUTC());
    QCOMPARE(utcWinter.dateTime(), QDateTime(QDate(2005,1,1), QTime(0,0,0), Qt::UTC));
    QVERIFY(londonWinter == utcWinter);
    KDateTime londonSummer(QDate(2005,6,1), QTime(0,0,0), london);
    KDateTime utcSummer = londonSummer.toUTC();
    QVERIFY(utcSummer.isUTC());
    QCOMPARE(utcSummer.dateTime(), QDateTime(QDate(2005,5,31), QTime(23,0,0), Qt::UTC));
    QVERIFY(londonSummer == utcSummer);
    QVERIFY(!(londonSummer == utcWinter));
    QVERIFY(!(londonWinter == utcSummer));

    // UTC offset -> UTC
    KDateTime offset(QDate(2005,6,6), QTime(1,2,30), KDateTime::OffsetFromUTC, -5400);  // -0130
    KDateTime utcOffset = offset.toUTC();
    QVERIFY(utcOffset.isUTC());
    QCOMPARE(utcOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(2,32,30), Qt::UTC));
    QVERIFY(offset == utcOffset);
    QVERIFY(!(offset == utcSummer));

    // Clock time -> UTC
    KDateTime clock(QDate(2005,6,6), QTime(1,2,30), KDateTime::ClockTime);
    KDateTime utcClock = clock.toUTC();
    QVERIFY(utcClock.isUTC());
    QCOMPARE(utcClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(8,2,30), Qt::UTC));
    QVERIFY(clock == utcClock);
    QVERIFY(!(clock == utcOffset));

    // UTC -> UTC
    KDateTime utc(QDate(2005,6,6), QTime(1,2,30), KDateTime::UTC);
    KDateTime utcUtc = utc.toUTC();
    QVERIFY(utcUtc.isUTC());
    QCOMPARE(utcUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(1,2,30), Qt::UTC));
    QVERIFY(utc == utcUtc);
    QVERIFY(!(utc == utcClock));

    // ** Date only ** //

    // Zone -> UTC
    londonSummer.setDateOnly(true);
    utcSummer = londonSummer.toUTC();
    QVERIFY(utcSummer.isDateOnly());
    QCOMPARE(utcSummer.dateTime(), QDateTime(QDate(2005,6,1), QTime(0,0,0), Qt::UTC));

    // UTC offset -> UTC
    offset.setDateOnly(true);
    utcOffset = offset.toUTC();
    QVERIFY(utcOffset.isDateOnly());
    QCOMPARE(utcOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::UTC));

    // Clock time -> UTC
    clock.setDateOnly(true);
    utcClock = clock.toUTC();
    QVERIFY(utcClock.isDateOnly());
    QCOMPARE(utcClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::UTC));

    // UTC -> UTC
    utc.setDateOnly(true);
    utcUtc = utc.toUTC();
    QVERIFY(utcUtc.isDateOnly());
    QCOMPARE(utcUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::UTC));

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::toLocalZone()
{
    const KTimezone *london = KSystemTimezones::zone("Europe/London");

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
    KDateTime offset(QDate(2005,6,6), QTime(11,2,30), KDateTime::OffsetFromUTC, -5400);  // -0130
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

    // UTC offset -> LocalZone
    offset.setDateOnly(true);
    locOffset = offset.toLocalZone();
    QVERIFY(locOffset.isDateOnly());
    QCOMPARE(locOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));

    // Clock time -> LocalZone
    clock.setDateOnly(true);
    locClock = clock.toLocalZone();
    QVERIFY(locClock.isDateOnly());
    QCOMPARE(locClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));

    // UTC -> LocalZone
    utc.setDateOnly(true);
    locUtc = utc.toLocalZone();
    QVERIFY(locUtc.isDateOnly());
    QCOMPARE(locUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::toClockTime()
{
    const KTimezone *london = KSystemTimezones::zone("Europe/London");

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
    KDateTime offset(QDate(2005,6,6), QTime(11,2,30), KDateTime::OffsetFromUTC, -5400);  // -0130
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

    // UTC offset -> ClockTime
    offset.setDateOnly(true);
    locOffset = offset.toClockTime();
    QVERIFY(locOffset.isDateOnly());
    QCOMPARE(locOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));

    // Clock time -> ClockTime
    clock.setDateOnly(true);
    locClock = clock.toClockTime();
    QVERIFY(locClock.isDateOnly());
    QCOMPARE(locClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));

    // UTC -> ClockTime
    utc.setDateOnly(true);
    locUtc = utc.toClockTime();
    QVERIFY(locUtc.isDateOnly());
    QCOMPARE(locUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::toZone()
{
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    const KTimezone *losAngeles = KSystemTimezones::zone("America/Los_Angeles");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":Europe/London", 1);
    ::tzset();

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
    KDateTime offset(QDate(2005,6,6), QTime(11,2,30), KDateTime::OffsetFromUTC, -5400);  // -0130
    KDateTime locOffset = offset.toZone(losAngeles);
    QCOMPARE(locWinter.timeZone(), losAngeles);
    QCOMPARE(locOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(5,32,30), Qt::LocalTime));
    QVERIFY(offset == locOffset);
    QVERIFY(!(offset == locSummer));

    // Clock time -> Zone
    KDateTime clock(QDate(2005,6,6), QTime(11,2,30), KDateTime::ClockTime);
    KDateTime locClock = clock.toZone(losAngeles);
    QCOMPARE(locWinter.timeZone(), losAngeles);
    QCOMPARE(locClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(3,2,30), Qt::LocalTime));
    QVERIFY(clock == locClock);
    QVERIFY(!(clock == locOffset));

    // UTC -> Zone
    KDateTime utc(QDate(2005,6,6), QTime(11,2,30), KDateTime::UTC);
    KDateTime locUtc = utc.toZone(losAngeles);
    QCOMPARE(locWinter.timeZone(), losAngeles);
    QCOMPARE(locUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(4,2,30), Qt::LocalTime));
    QVERIFY(utc == locUtc);
    QVERIFY(!(utc == locClock));

    // ** Date only ** //

    // Zone -> Zone
    londonSummer.setDateOnly(true);
    locSummer = londonSummer.toZone(losAngeles);
    QVERIFY(locSummer.isDateOnly());
    QCOMPARE(locSummer.dateTime(), QDateTime(QDate(2005,6,1), QTime(0,0,0), Qt::LocalTime));

    // UTC offset -> Zone
    offset.setDateOnly(true);
    locOffset = offset.toZone(losAngeles);
    QVERIFY(locOffset.isDateOnly());
    QCOMPARE(locOffset.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));

    // Clock time -> Zone
    clock.setDateOnly(true);
    locClock = clock.toZone(losAngeles);
    QVERIFY(locClock.isDateOnly());
    QCOMPARE(locClock.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));

    // UTC -> Zone
    utc.setDateOnly(true);
    locUtc = utc.toZone(losAngeles);
    QVERIFY(locUtc.isDateOnly());
    QCOMPARE(locUtc.dateTime(), QDateTime(QDate(2005,6,6), QTime(0,0,0), Qt::LocalTime));

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::toTimeSpec()
{
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    const KTimezone *cairo  = KSystemTimezones::zone("Africa/Cairo");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    KDateTime utc1(QDate(2004,3,1), QTime(3,45,2), KDateTime::UTC);
    KDateTime zone1(QDate(2004,3,1), QTime(3,45,2), cairo);
    KDateTime offset1(QDate(2004,3,1), QTime(3,45,2), KDateTime::OffsetFromUTC, 1200);    // +00:20
    KDateTime clock1(QDate(2004,3,1), QTime(3,45,2), KDateTime::ClockTime);

    KDateTime utc(QDate(2005,6,6), QTime(1,2,30), KDateTime::UTC);
    KDateTime zone(QDate(2005,7,1), QTime(2,0,0), london);
    KDateTime offset(QDate(2005,6,6), QTime(1,2,30), KDateTime::OffsetFromUTC, -5400);  // -01:30
    KDateTime clock(QDate(2005,6,6), QTime(1,2,30), KDateTime::ClockTime);

    // To UTC
    KDateTime utcZone = zone.toTimeSpec(utc1);
    QVERIFY(utcZone.isUTC());
    QVERIFY(utcZone == KDateTime(QDate(2005,7,1), QTime(1,0,0), KDateTime::UTC));
    QVERIFY(!zone.compareTimeSpec(utc1));
    QVERIFY(utcZone.compareTimeSpec(utc1));

    KDateTime utcOffset = offset.toTimeSpec(utc1);
    QVERIFY(utcOffset.isUTC());
    QVERIFY(utcOffset == KDateTime(QDate(2005,6,6), QTime(2,32,30), KDateTime::UTC));
    QVERIFY(!offset.compareTimeSpec(utc1));
    QVERIFY(utcOffset.compareTimeSpec(utc1));

    KDateTime utcClock = clock.toTimeSpec(utc1);
    QVERIFY(utcClock.isUTC());
    QVERIFY(utcClock == KDateTime(QDate(2005,6,6), QTime(8,2,30), KDateTime::UTC));
    QVERIFY(!clock.compareTimeSpec(utc1));
    QVERIFY(utcZone.compareTimeSpec(utc1));

    KDateTime utcUtc = utc.toTimeSpec(utc1);
    QVERIFY(utcUtc.isUTC());
    QVERIFY(utcUtc == KDateTime(QDate(2005,6,6), QTime(1,2,30), KDateTime::UTC));
    QVERIFY(utc.compareTimeSpec(utc1));
    QVERIFY(utcUtc.compareTimeSpec(utc1));

    // To Zone
    KDateTime zoneZone = zone.toTimeSpec(zone1);
    QCOMPARE(zoneZone.timeZone(), cairo);
    QVERIFY(zoneZone == KDateTime(QDate(2005,7,1), QTime(4,0,0), cairo));
    QVERIFY(!zone.compareTimeSpec(zone1));
    QVERIFY(zoneZone.compareTimeSpec(zone1));

    KDateTime zoneOffset = offset.toTimeSpec(zone1);
    QCOMPARE(zoneOffset.timeZone(), cairo);
    QVERIFY(zoneOffset == KDateTime(QDate(2005,6,6), QTime(5,32,30), cairo));
    QVERIFY(!offset.compareTimeSpec(zone1));
    QVERIFY(zoneOffset.compareTimeSpec(zone1));

    KDateTime zoneClock = clock.toTimeSpec(zone1);
    QCOMPARE(zoneClock.timeZone(), cairo);
    QVERIFY(zoneClock == KDateTime(QDate(2005,6,6), QTime(11,2,30), cairo));
    QVERIFY(!clock.compareTimeSpec(zone1));
    QVERIFY(zoneClock.compareTimeSpec(zone1));

    KDateTime zoneUtc = utc.toTimeSpec(zone1);
    QCOMPARE(zoneUtc.timeZone(), cairo);
    QVERIFY(zoneUtc == KDateTime(QDate(2005,6,6), QTime(4,2,30), cairo));
    QVERIFY(!utc.compareTimeSpec(zone1));
    QVERIFY(zoneUtc.compareTimeSpec(zone1));

    // To UTC offset
    KDateTime offsetZone = zone.toTimeSpec(offset1);
    QVERIFY(offsetZone.isOffsetFromUTC());
    QCOMPARE(offsetZone.UTCOffset(), 1200);
    QVERIFY(offsetZone == KDateTime(QDate(2005,7,1), QTime(1,20,0), KDateTime::OffsetFromUTC, 1200));
    QVERIFY(!zone.compareTimeSpec(offset1));
    QVERIFY(offsetZone.compareTimeSpec(offset1));

    KDateTime offsetOffset = offset.toTimeSpec(offset1);
    QVERIFY(offsetOffset.isOffsetFromUTC());
    QCOMPARE(offsetZone.UTCOffset(), 1200);
    QVERIFY(offsetOffset == KDateTime(QDate(2005,6,6), QTime(2,52,30), KDateTime::OffsetFromUTC, 1200));
    QVERIFY(!offset.compareTimeSpec(offset1));
    QVERIFY(offsetOffset.compareTimeSpec(offset1));

    KDateTime offsetClock = clock.toTimeSpec(offset1);
    QVERIFY(offsetClock.isOffsetFromUTC());
    QCOMPARE(offsetZone.UTCOffset(), 1200);
    QVERIFY(offsetClock == KDateTime(QDate(2005,6,6), QTime(8,22,30), KDateTime::OffsetFromUTC, 1200));
    QVERIFY(!clock.compareTimeSpec(offset1));
    QVERIFY(offsetClock.compareTimeSpec(offset1));

    KDateTime offsetUtc = utc.toTimeSpec(offset1);
    QVERIFY(offsetUtc.isOffsetFromUTC());
    QCOMPARE(offsetZone.UTCOffset(), 1200);
    QVERIFY(offsetUtc == KDateTime(QDate(2005,6,6), QTime(1,22,30), KDateTime::OffsetFromUTC, 1200));
    QVERIFY(!utc.compareTimeSpec(offset1));
    QVERIFY(offsetUtc.compareTimeSpec(offset1));

    // To Clock time
    KDateTime clockZone = zone.toTimeSpec(clock1);
    QVERIFY(clockZone.isClockTime());
    QVERIFY(clockZone == KDateTime(QDate(2005,6,30), QTime(18,0,0), KDateTime::ClockTime));
    QVERIFY(!zone.compareTimeSpec(clock1));
    QVERIFY(clockZone.compareTimeSpec(clock1));

    KDateTime clockOffset = offset.toTimeSpec(clock1);
    QVERIFY(clockOffset.isClockTime());
    QVERIFY(clockOffset == KDateTime(QDate(2005,6,5), QTime(19,32,30), KDateTime::ClockTime));
    QVERIFY(!offset.compareTimeSpec(clock1));
    QVERIFY(clockOffset.compareTimeSpec(clock1));

    KDateTime clockClock = clock.toTimeSpec(clock1);
    QVERIFY(clockClock.isClockTime());
    QVERIFY(clockClock == KDateTime(QDate(2005,6,6), QTime(1,2,30), KDateTime::ClockTime));
    QVERIFY(clock.compareTimeSpec(clock1));
    QVERIFY(clockClock.compareTimeSpec(clock1));

    KDateTime clockUtc = utc.toTimeSpec(clock1);
    QVERIFY(clockUtc.isClockTime());
    QVERIFY(clockUtc == KDateTime(QDate(2005,6,5), QTime(18,2,30), KDateTime::ClockTime));
    QVERIFY(!utc.compareTimeSpec(clock1));
    QVERIFY(clockUtc.compareTimeSpec(clock1));


    // ** Date only ** //

    KDateTime zoned(QDate(2005,7,1), london);
    KDateTime offsetd(QDate(2005,6,6), KDateTime::OffsetFromUTC, -5400);  // -01:30
    KDateTime clockd(QDate(2005,6,6), KDateTime::ClockTime);
    KDateTime utcd(QDate(2005,6,6), KDateTime::UTC);

    // To UTC
    utcZone = zoned.toTimeSpec(utc1);
    QVERIFY(utcZone.isUTC());
    QVERIFY(utcZone.isDateOnly());
    QVERIFY(utcZone == KDateTime(QDate(2005,7,1), KDateTime::UTC));

    utcOffset = offsetd.toTimeSpec(utc1);
    QVERIFY(utcOffset.isUTC());
    QVERIFY(utcOffset.isDateOnly());
    QVERIFY(utcOffset == KDateTime(QDate(2005,6,6), KDateTime::UTC));

    utcClock = clockd.toTimeSpec(utc1);
    QVERIFY(utcClock.isUTC());
    QVERIFY(utcClock.isDateOnly());
    QVERIFY(utcClock == KDateTime(QDate(2005,6,6), KDateTime::UTC));

    utcUtc = utcd.toTimeSpec(utc1);
    QVERIFY(utcUtc.isUTC());
    QVERIFY(utcUtc.isDateOnly());
    QVERIFY(utcUtc == KDateTime(QDate(2005,6,6), KDateTime::UTC));

    // To Zone
    zoneZone = zoned.toTimeSpec(zone1);
    QVERIFY(zoneZone.isDateOnly());
    QCOMPARE(zoneZone.timeZone(), cairo);
    QVERIFY(zoneZone == KDateTime(QDate(2005,7,1), cairo));

    zoneOffset = offsetd.toTimeSpec(zone1);
    QVERIFY(zoneOffset.isDateOnly());
    QCOMPARE(zoneOffset.timeZone(), cairo);
    QVERIFY(zoneOffset == KDateTime(QDate(2005,6,6), cairo));

    zoneClock = clockd.toTimeSpec(zone1);
    QVERIFY(zoneClock.isDateOnly());
    QCOMPARE(zoneClock.timeZone(), cairo);
    QVERIFY(zoneClock == KDateTime(QDate(2005,6,6), cairo));

    zoneUtc = utcd.toTimeSpec(zone1);
    QVERIFY(zoneUtc.isDateOnly());
    QCOMPARE(zoneUtc.timeZone(), cairo);
    QVERIFY(zoneUtc == KDateTime(QDate(2005,6,6), cairo));

    // To UTC offset
    offsetZone = zoned.toTimeSpec(offset1);
    QVERIFY(offsetZone.isOffsetFromUTC());
    QVERIFY(offsetZone.isDateOnly());
    QCOMPARE(offsetZone.UTCOffset(), 1200);
    QVERIFY(offsetZone == KDateTime(QDate(2005,7,1), KDateTime::OffsetFromUTC, 1200));

    offsetOffset = offsetd.toTimeSpec(offset1);
    QVERIFY(offsetOffset.isOffsetFromUTC());
    QVERIFY(offsetOffset.isDateOnly());
    QCOMPARE(offsetZone.UTCOffset(), 1200);
    QVERIFY(offsetOffset == KDateTime(QDate(2005,6,6), KDateTime::OffsetFromUTC, 1200));

    offsetClock = clockd.toTimeSpec(offset1);
    QVERIFY(offsetClock.isOffsetFromUTC());
    QVERIFY(offsetClock.isDateOnly());
    QCOMPARE(offsetZone.UTCOffset(), 1200);
    QVERIFY(offsetClock == KDateTime(QDate(2005,6,6), KDateTime::OffsetFromUTC, 1200));

    offsetUtc = utcd.toTimeSpec(offset1);
    QVERIFY(offsetUtc.isOffsetFromUTC());
    QVERIFY(offsetUtc.isDateOnly());
    QCOMPARE(offsetZone.UTCOffset(), 1200);
    QVERIFY(offsetUtc == KDateTime(QDate(2005,6,6), KDateTime::OffsetFromUTC, 1200));

    // To Clock time
    clockZone = zoned.toTimeSpec(clock1);
    QVERIFY(clockZone.isClockTime());
    QVERIFY(clockZone.isDateOnly());
    QVERIFY(clockZone == KDateTime(QDate(2005,7,1), KDateTime::ClockTime));

    clockOffset = offsetd.toTimeSpec(clock1);
    QVERIFY(clockOffset.isClockTime());
    QVERIFY(clockOffset.isDateOnly());
    QVERIFY(clockOffset == KDateTime(QDate(2005,6,6), KDateTime::ClockTime));

    clockClock = clockd.toTimeSpec(clock1);
    QVERIFY(clockClock.isClockTime());
    QVERIFY(clockClock.isDateOnly());
    QVERIFY(clockClock == KDateTime(QDate(2005,6,6), KDateTime::ClockTime));

    clockUtc = utcd.toTimeSpec(clock1);
    QVERIFY(clockUtc.isClockTime());
    QVERIFY(clockUtc.isDateOnly());
    QVERIFY(clockUtc == KDateTime(QDate(2005,6,6), KDateTime::ClockTime));


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
    const KTimezone *london = KSystemTimezones::zone("Europe/London");

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
    QCOMPARE(zone.UTCOffset(), 3600);
    QCOMPARE(zone.toUTC().dateTime(), QDateTime(QDate(2003,6,10), QTime(4,6,7), Qt::UTC));

    // UTC offset
    KDateTime offsetd(QDate(2005,6,6), KDateTime::OffsetFromUTC, -5400);  // -0130
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

    KDateTime offset(QDate(2005,6,6), QTime(1,2,30), KDateTime::OffsetFromUTC, -5400);  // -0130
    offset.setDate(QDate(2004,5,2));
    QCOMPARE(offset.dateTime(), QDateTime(QDate(2004,5,2), QTime(1,2,30), Qt::LocalTime));
    offset.setTime(QTime(12,13,14));
    QCOMPARE(offset.dateTime(), QDateTime(QDate(2004,5,2), QTime(12,13,14), Qt::LocalTime));
    offset.setDateTime(QDateTime(QDate(2003,12,10), QTime(5,6,7)));
    QCOMPARE(offset.dateTime(), QDateTime(QDate(2003,12,10), QTime(5,6,7), Qt::LocalTime));
    QCOMPARE(offset.UTCOffset(), -5400);
    QCOMPARE(offset.toUTC().dateTime(), QDateTime(QDate(2003,12,10), QTime(6,36,7), Qt::UTC));

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
    QCOMPARE(clock.UTCOffset(), 0);
    QCOMPARE(clock.toUTC().dateTime(), QDateTime(QDate(2003,12,10), QTime(13,6,7), Qt::UTC));

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
    QCOMPARE(utc.UTCOffset(), 0);
    QCOMPARE(utc.dateTime(), QDateTime(QDate(2003,12,10), QTime(5,6,7), Qt::UTC));

    // setTimeSpec(TimeSpec)
    QCOMPARE(zone.dateTime(), QDateTime(QDate(2003,6,10), QTime(5,6,7), Qt::LocalTime));
    zone.setTimeSpec(KDateTime::OffsetFromUTC, 7200);
    QVERIFY(zone.isOffsetFromUTC());
    QCOMPARE(zone.UTCOffset(), 7200);
    QCOMPARE(zone.toUTC().dateTime(), QDateTime(QDate(2003,6,10), QTime(3,6,7), Qt::UTC));
    zone.setTimeSpec(KDateTime::LocalZone);
    QVERIFY(zone.isLocalZone());
    QCOMPARE(zone.UTCOffset(), -7*3600);
    QCOMPARE(zone.toUTC().dateTime(), QDateTime(QDate(2003,6,10), QTime(12,6,7), Qt::UTC));
    zone.setTimeSpec(KDateTime::UTC);
    QVERIFY(zone.isUTC());
    QCOMPARE(zone.UTCOffset(), 0);
    QCOMPARE(zone.dateTime(), QDateTime(QDate(2003,6,10), QTime(5,6,7), Qt::UTC));
    zone.setTimeSpec(KDateTime::ClockTime);
    QVERIFY(zone.isClockTime());
    QCOMPARE(zone.UTCOffset(), 0);
    QCOMPARE(zone.toUTC().dateTime(), QDateTime(QDate(2003,6,10), QTime(12,6,7), Qt::UTC));

    // setTimeSpec(KTimezone*)
    zone.setTimeSpec(london);
    QCOMPARE(zone.timeZone(), london);
    QCOMPARE(zone.UTCOffset(), 3600);
    QCOMPARE(zone.toUTC().dateTime(), QDateTime(QDate(2003,6,10), QTime(4,6,7), Qt::UTC));

    // setTimeSpec(KDateTime)
    QCOMPARE(zone.dateTime(), QDateTime(QDate(2003,6,10), QTime(5,6,7), Qt::LocalTime));
    zone.setTimeSpec(offset);
    QVERIFY(zone.isOffsetFromUTC());
    QCOMPARE(zone.toUTC().dateTime(), QDateTime(QDate(2003,6,10), QTime(6,36,7), Qt::UTC));
    QVERIFY(zone.compareTimeSpec(offset));
    zone.setTimeSpec(KDateTime(QDate(2000,1,1), QTime(2,3,4), KDateTime::LocalZone));
    QVERIFY(zone.isLocalZone());
    QCOMPARE(zone.toUTC().dateTime(), QDateTime(QDate(2003,6,10), QTime(12,6,7), Qt::UTC));
    zone.setTimeSpec(utc);
    QVERIFY(zone.isUTC());
    QCOMPARE(zone.dateTime(), QDateTime(QDate(2003,6,10), QTime(5,6,7), Qt::UTC));
    zone.setTimeSpec(clock);
    QVERIFY(zone.isClockTime());
    QCOMPARE(zone.toUTC().dateTime(), QDateTime(QDate(2003,6,10), QTime(12,6,7), Qt::UTC));
    zone.setTimeSpec(KDateTime(QDate(2005,6,1), QTime(3,40,0), london));
    QCOMPARE(zone.timeZone(), london);
    QCOMPARE(zone.toUTC().dateTime(), QDateTime(QDate(2003,6,10), QTime(4,6,7), Qt::UTC));

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
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    const KTimezone *cairo  = KSystemTimezones::zone("Africa/Cairo");

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
    QCOMPARE(KDateTime(QDate(2004,3,1), QTime(0,0,0), cairo).compare(KDateTime(QDate(2004,3,1), cairo)), KDateTime::ContainedBy);
    QCOMPARE(KDateTime(QDate(2004,3,1), QTime(3,45,2), cairo).compare(KDateTime(QDate(2004,3,1), cairo)), KDateTime::ContainedBy);
    QCOMPARE(KDateTime(QDate(2004,3,1), QTime(23,59,59,999), cairo).compare(KDateTime(QDate(2004,3,1), cairo)), KDateTime::ContainedBy);
    QCOMPARE(KDateTime(QDate(2004,3,1), QTime(23,59,59,999), cairo).compare(KDateTime(QDate(2004,3,2), cairo)), KDateTime::Before);
    QCOMPARE(KDateTime(QDate(2004,3,3), QTime(0,0,0), cairo).compare(KDateTime(QDate(2004,3,2), cairo)), KDateTime::After);

    QCOMPARE(KDateTime(QDate(2004,3,2), QTime(9,59,59,999), cairo).compare(KDateTime(QDate(2004,3,2), KDateTime::LocalZone)), KDateTime::Before);
    QCOMPARE(KDateTime(QDate(2004,3,2), QTime(10,0,0), cairo).compare(KDateTime(QDate(2004,3,2), KDateTime::LocalZone)), KDateTime::ContainedBy);
    QCOMPARE(KDateTime(QDate(2004,3,3), QTime(9,59,59,999), cairo).compare(KDateTime(QDate(2004,3,2), KDateTime::LocalZone)), KDateTime::ContainedBy);
    QCOMPARE(KDateTime(QDate(2004,3,3), QTime(10,0,0), cairo).compare(KDateTime(QDate(2004,3,2), KDateTime::LocalZone)), KDateTime::After);

    // Date-only : date/time
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,1), QTime(0,0,0), cairo)), KDateTime::Contains);
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,1), QTime(3,45,2), cairo)), KDateTime::Contains);
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,1), QTime(23,59,59,999), cairo)), KDateTime::Contains);
    QCOMPARE(KDateTime(QDate(2004,3,2), cairo).compare(KDateTime(QDate(2004,3,1), QTime(23,59,59,999), cairo)), KDateTime::After);
    QCOMPARE(KDateTime(QDate(2004,3,2), cairo).compare(KDateTime(QDate(2004,3,3), QTime(0,0,0), cairo)), KDateTime::Before);

    QCOMPARE(KDateTime(QDate(2004,3,2), KDateTime::LocalZone).compare(KDateTime(QDate(2004,3,2), QTime(9,59,59,999), cairo)), KDateTime::After);
    QCOMPARE(KDateTime(QDate(2004,3,2), KDateTime::LocalZone).compare(KDateTime(QDate(2004,3,2), QTime(10,0,0), cairo)), KDateTime::Contains);
    QCOMPARE(KDateTime(QDate(2004,3,2), KDateTime::LocalZone).compare(KDateTime(QDate(2004,3,3), QTime(9,59,59,999), cairo)), KDateTime::Contains);
    QCOMPARE(KDateTime(QDate(2004,3,2), KDateTime::LocalZone).compare(KDateTime(QDate(2004,3,3), QTime(10,0,0), cairo)), KDateTime::Before);

    // Date-only values
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,2), cairo)), KDateTime::Before);
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,2), KDateTime::OffsetFromUTC, 2*3600)), KDateTime::Before);
    QCOMPARE(KDateTime(QDate(2004,3,1), london).compare(KDateTime(QDate(2004,3,2), cairo)), KDateTime::BeforeOverlap);
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,2), KDateTime::OffsetFromUTC, 3*3600)), KDateTime::BeforeOverlap);
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,1), cairo)), KDateTime::Equal);
    QCOMPARE(KDateTime(QDate(2004,3,1), cairo).compare(KDateTime(QDate(2004,3,1), KDateTime::OffsetFromUTC, 2*3600)), KDateTime::Equal);
    QCOMPARE(KDateTime(QDate(2004,3,2), cairo).compare(KDateTime(QDate(2004,3,1), london)), KDateTime::AfterOverlap);
    QCOMPARE(KDateTime(QDate(2004,3,2), KDateTime::OffsetFromUTC, 3*3600).compare(KDateTime(QDate(2004,3,1), cairo)), KDateTime::AfterOverlap);
    QCOMPARE(KDateTime(QDate(2004,3,2), cairo).compare(KDateTime(QDate(2004,3,1), cairo)), KDateTime::After);
    QCOMPARE(KDateTime(QDate(2004,3,2), KDateTime::OffsetFromUTC, 2*3600).compare(KDateTime(QDate(2004,3,1), cairo)), KDateTime::After);
    // Compare days when daylight savings changes occur
    QCOMPARE(KDateTime(QDate(2005,3,27), london).compare(KDateTime(QDate(2005,3,27), KDateTime::OffsetFromUTC, 0)), KDateTime::ContainedBy);
    QCOMPARE(KDateTime(QDate(2005,3,27), KDateTime::OffsetFromUTC, 0).compare(KDateTime(QDate(2005,3,27), london)), KDateTime::Contains);
    QCOMPARE(KDateTime(QDate(2005,10,30), london).compare(KDateTime(QDate(2005,10,30), KDateTime::UTC)), KDateTime::Contains);
    QCOMPARE(KDateTime(QDate(2005,10,30), KDateTime::UTC).compare(KDateTime(QDate(2005,10,30), london)), KDateTime::ContainedBy);

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
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    const KTimezone *losAngeles = KSystemTimezones::zone("America/Los_Angeles");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    // UTC
    KDateTime utc1(QDate(2005,7,6), QTime(3,40,0), KDateTime::UTC);
    KDateTime utc2 = utc1.addSecs(184 * 86400);
    QVERIFY(utc2.isUTC());
    QCOMPARE(utc2.dateTime(), QDateTime(QDate(2006,1,6), QTime(3,40,0), Qt::UTC));
    KDateTime utc3 = utc1.addDays(184);
    QVERIFY(utc3.isUTC());
    QCOMPARE(utc2.dateTime(), utc3.dateTime());
    KDateTime utc4 = utc1.addMonths(6);
    QVERIFY(utc4.isUTC());
    QCOMPARE(utc2.dateTime(), utc4.dateTime());
    KDateTime utc5 = utc1.addYears(4);
    QVERIFY(utc5.isUTC());
    QCOMPARE(utc5.dateTime(), QDateTime(QDate(2009,7,6), QTime(3,40,0), Qt::UTC));
    QCOMPARE(utc1.secsTo(utc2), 184 * 86400);
    QCOMPARE(utc1.secsTo(utc3), 184 * 86400);
    QCOMPARE(utc1.daysTo(utc2), 184);
    QVERIFY(utc1 < utc2);
    QVERIFY(!(utc2 < utc1));
    QVERIFY(utc2 == utc3);

    // UTC offset
    KDateTime offset1(QDate(2005,7,6), QTime(3,40,0), KDateTime::OffsetFromUTC, -5400);  // -0130
    KDateTime offset2 = offset1.addSecs(184 * 86400);
    QVERIFY(offset2.isOffsetFromUTC());
    QCOMPARE(offset2.UTCOffset(), -5400);
    QCOMPARE(offset2.dateTime(), QDateTime(QDate(2006,1,6), QTime(3,40,0), Qt::LocalTime));
    KDateTime offset3 = offset1.addDays(184);
    QVERIFY(offset3.isOffsetFromUTC());
    QCOMPARE(offset3.UTCOffset(), -5400);
    QCOMPARE(offset2.dateTime(), offset3.dateTime());
    KDateTime offset4 = offset1.addMonths(6);
    QVERIFY(offset4.isOffsetFromUTC());
    QCOMPARE(offset4.UTCOffset(), -5400);
    QCOMPARE(offset2.dateTime(), offset4.dateTime());
    KDateTime offset5 = offset1.addYears(4);
    QVERIFY(offset5.isOffsetFromUTC());
    QCOMPARE(offset5.UTCOffset(), -5400);
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

void KDateTimeTest::addSubtractDate()
{
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    const KTimezone *losAngeles = KSystemTimezones::zone("America/Los_Angeles");

    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    // UTC
    KDateTime utc1(QDate(2005,7,6), KDateTime::UTC);
    KDateTime utc2 = utc1.addSecs(184 * 86400 + 100);
    QVERIFY(utc2.isUTC());
    QVERIFY(utc2.isDateOnly());
    QCOMPARE(utc2.dateTime(), QDateTime(QDate(2006,1,6), QTime(0,0,0), Qt::UTC));
    KDateTime utc3 = utc1.addDays(184);
    QVERIFY(utc3.isUTC());
    QVERIFY(utc3.isDateOnly());
    QCOMPARE(utc2.dateTime(), utc3.dateTime());
    KDateTime utc4 = utc1.addMonths(6);
    QVERIFY(utc4.isUTC());
    QVERIFY(utc4.isDateOnly());
    QCOMPARE(utc2.dateTime(), utc4.dateTime());
    KDateTime utc5 = utc1.addYears(4);
    QVERIFY(utc5.isUTC());
    QVERIFY(utc5.isDateOnly());
    QCOMPARE(utc5.dateTime(), QDateTime(QDate(2009,7,6), QTime(0,0,0), Qt::UTC));
    QCOMPARE(utc1.secsTo(utc2), 184 * 86400);
    QCOMPARE(utc1.secsTo(utc3), 184 * 86400);
    QCOMPARE(utc1.daysTo(utc2), 184);
    QVERIFY(utc1 < utc2);
    QVERIFY(!(utc2 < utc1));
    QVERIFY(utc2 == utc3);

    // UTC offset
    KDateTime offset1(QDate(2005,7,6), KDateTime::OffsetFromUTC, -5400);  // -0130
    KDateTime offset2 = offset1.addSecs(184 * 86400);
    QVERIFY(offset2.isDateOnly());
    QVERIFY(offset2.isOffsetFromUTC());
    QCOMPARE(offset2.UTCOffset(), -5400);
    QCOMPARE(offset2.dateTime(), QDateTime(QDate(2006,1,6), QTime(0,0,0), Qt::LocalTime));
    KDateTime offset3 = offset1.addDays(184);
    QVERIFY(offset3.isDateOnly());
    QVERIFY(offset3.isOffsetFromUTC());
    QCOMPARE(offset3.UTCOffset(), -5400);
    QCOMPARE(offset2.dateTime(), offset3.dateTime());
    KDateTime offset4 = offset1.addMonths(6);
    QVERIFY(offset4.isDateOnly());
    QVERIFY(offset4.isOffsetFromUTC());
    QCOMPARE(offset4.UTCOffset(), -5400);
    QCOMPARE(offset2.dateTime(), offset4.dateTime());
    KDateTime offset5 = offset1.addYears(4);
    QVERIFY(offset5.isDateOnly());
    QVERIFY(offset5.isOffsetFromUTC());
    QCOMPARE(offset5.UTCOffset(), -5400);
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

    KDateTime offset1t(QDate(2005,7,6), QTime(3,40,0), KDateTime::OffsetFromUTC, -5400);  // -0130
    QCOMPARE(offset1t.secsTo(offset2), 184 * 86400);

    KDateTime offset2t(QDate(2005,7,6), QTime(0,40,0), KDateTime::OffsetFromUTC, 5400);  // +0130

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
    QVERIFY(utc1t == offset1);
    QVERIFY(offset1 == utc1t);
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
    QVERIFY(utc1t == zone1);
    QVERIFY(zone1 == utc1t);
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
    QVERIFY(utc1 == local2t);
    QVERIFY(local2t == utc1);
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
    QVERIFY(offset1t == zone1);
    QVERIFY(zone1 == offset1t);
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
    QVERIFY(local1t == clock1);
    QVERIFY(local1t == clock1);
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


////////////////////
// String conversion
////////////////////

void KDateTimeTest::strings_iso8601()
{
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
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
    QCOMPARE(dtlocal1.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtlocal1.UTCOffset(), -8*3600);
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
    QCOMPARE(dtzone1.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtzone1.UTCOffset(), 3600);
    QVERIFY(dtzone1 == dtzone);
    dtzone.setDateOnly(true);
    s = dtzone.toString(KDateTime::ISODate);
    QCOMPARE(s, QString("1999-06-11T00:00:00+01:00"));

    KDateTime dtclock(QDate(1999,12,11), QTime(3,45,06), KDateTime::ClockTime);
    s = dtclock.toString(KDateTime::ISODate);
    QCOMPARE(s, QString("1999-12-11T03:45:06"));
    KDateTime dtclock1 = KDateTime::fromString(s, KDateTime::ISODate);
    QCOMPARE(dtclock1.dateTime(), dtclock.dateTime());
    QCOMPARE(dtclock1.timeSpec(), KDateTime::ClockTime);
    QVERIFY(dtclock1 == dtclock);
    dtclock.setDateOnly(true);
    s = dtclock.toString(KDateTime::ISODate);
    QCOMPARE(s, QString("1999-12-11"));
    dtclock1 = KDateTime::fromString(s, KDateTime::ISODate);
    QVERIFY(dtclock1.isDateOnly());
    QCOMPARE(dtclock1.timeSpec(), KDateTime::ClockTime);
    QCOMPARE(dtclock1.date(), QDate(1999,12,11));

    KDateTime dtutc(QDate(1999,12,11), QTime(3,45,00), KDateTime::UTC);
    s = dtutc.toString(KDateTime::ISODate);
    QCOMPARE(s, QString("1999-12-11T03:45:00Z"));
    KDateTime dtutc1 = KDateTime::fromString(s, KDateTime::ISODate);
    QCOMPARE(dtutc1.dateTime(), dtutc.dateTime());
    QCOMPARE(dtutc1.timeSpec(), KDateTime::UTC);
    QVERIFY(dtutc1 == dtutc);
    dtutc.setDateOnly(true);
    s = dtutc.toString(KDateTime::ISODate);
    QCOMPARE(s, QString("1999-12-11T00:00:00Z"));

    // Check basic format strings
    bool negZero = true;
    KDateTime dt = KDateTime::fromString(QString("20000301T1213"), KDateTime::ISODate, &negZero);
    QVERIFY(dt.timeSpec() == KDateTime::ClockTime);
    QVERIFY(!dt.isDateOnly());
    QVERIFY(!negZero);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2000,3,1), QTime(12,13,0), Qt::LocalTime));
    dt = KDateTime::fromString(QString("20000301"), KDateTime::ISODate, &negZero);
    QVERIFY(dt.timeSpec() == KDateTime::ClockTime);
    QVERIFY(dt.isDateOnly());
    QVERIFY(!negZero);
    QCOMPARE(dt.date(), QDate(2000,3,1));
    KDateTime::setFromStringDefault(KDateTime::UTC);
    dt = KDateTime::fromString(QString("20000301T1213"), KDateTime::ISODate);
    QVERIFY(dt.timeSpec() == KDateTime::UTC);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2000,3,1), QTime(12,13,0), Qt::UTC));
    KDateTime::setFromStringDefault(KDateTime::LocalZone);
    dt = KDateTime::fromString(QString("20000301T1213"), KDateTime::ISODate);
    QVERIFY(dt.timeSpec() == KDateTime::LocalZone);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2000,3,1), QTime(12,13,0), Qt::LocalTime));
    KDateTime::setFromStringDefault(london);
    dt = KDateTime::fromString(QString("20000301T1213"), KDateTime::ISODate);
    QVERIFY(dt.timeSpec() == KDateTime::TimeZone);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2000,3,1), QTime(12,13,0), Qt::LocalTime));
    KDateTime::setFromStringDefault(KDateTime::OffsetFromUTC, 5000);  // = +01:23:20
    dt = KDateTime::fromString(QString("20000601T1213"), KDateTime::ISODate);
    QVERIFY(dt.timeSpec() == KDateTime::OffsetFromUTC);
    QCOMPARE(dt.UTCOffset(), 5000);
    QCOMPARE(dt.toUTC().dateTime(), QDateTime(QDate(2000,6,1), QTime(10,49,40), Qt::UTC));
    KDateTime::setFromStringDefault(KDateTime::ClockTime);

    // Check strings containing day-of-the-year
    dt = KDateTime::fromString(QString("1999-060T19:20:21.06-11:20"), KDateTime::ISODate);
    QVERIFY(dt.timeSpec() == KDateTime::OffsetFromUTC);
    QCOMPARE(dt.UTCOffset(), -11*3600 - 20*60);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(1999,3,1), QTime(19,20,21,60), Qt::LocalTime));
    dt = KDateTime::fromString(QString("1999-060T19:20:21,06-11:20"), KDateTime::ISODate);
    QVERIFY(dt.timeSpec() == KDateTime::OffsetFromUTC);
    QCOMPARE(dt.UTCOffset(), -11*3600 - 20*60);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(1999,3,1), QTime(19,20,21,60), Qt::LocalTime));
    dt = KDateTime::fromString(QString("1999060T192021.06-1120"), KDateTime::ISODate);
    QVERIFY(dt.timeSpec() == KDateTime::OffsetFromUTC);
    QCOMPARE(dt.UTCOffset(), -11*3600 - 20*60);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(1999,3,1), QTime(19,20,21,60), Qt::LocalTime));
    dt = KDateTime::fromString(QString("1999-060"), KDateTime::ISODate);
    QVERIFY(dt.timeSpec() == KDateTime::ClockTime);
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
    const KTimezone *london = KSystemTimezones::zone("Europe/London");

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
    QCOMPARE(dtlocal1.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtlocal1.UTCOffset(), -8*3600);
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
    QCOMPARE(dtzone1.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtzone1.UTCOffset(), 3600);
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
    QCOMPARE(dtclock1.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtclock1.UTCOffset(), -8*3600);
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
    QCOMPARE(dtutc1.timeSpec(), KDateTime::UTC);
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
    QVERIFY(dtzname.timeSpec() == KDateTime::UTC);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::UTC));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 GMT"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeSpec() == KDateTime::UTC);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::UTC));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 EDT"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeSpec() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.UTCOffset(), -4*3600);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::LocalTime));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 EST"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeSpec() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.UTCOffset(), -5*3600);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::LocalTime));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 CDT"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeSpec() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.UTCOffset(), -5*3600);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::LocalTime));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 CST"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeSpec() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.UTCOffset(), -6*3600);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::LocalTime));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 MDT"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeSpec() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.UTCOffset(), -6*3600);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::LocalTime));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 MST"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeSpec() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.UTCOffset(), -7*3600);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::LocalTime));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 PDT"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeSpec() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.UTCOffset(), -7*3600);
    QCOMPARE(dtzname.dateTime(), QDateTime(QDate(1999,12,11), QTime(3,45,6), Qt::LocalTime));
    QVERIFY(!negZero);
    dtzname = KDateTime::fromString(QString("11 Dec 1999 03:45:06 PST"), KDateTime::RFCDate, &negZero);
    QVERIFY(dtzname.timeSpec() == KDateTime::OffsetFromUTC);
    QCOMPARE(dtzname.UTCOffset(), -8*3600);
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
    const KTimezone *london = KSystemTimezones::zone("Europe/London");

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
    QCOMPARE(dtlocal1.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtlocal1.UTCOffset(), -8*3600);
    QVERIFY(dtlocal1 == dtlocal);
    QVERIFY(!dtlocal1.isDateOnly());
    QVERIFY(!negZero);
    dtlocal.setDateOnly(true);
    s = dtlocal.toString(KDateTime::QtTextDate);
    QCOMPARE(s, QString("Sat Dec 11 1999 -0800"));
    dtlocal1 = KDateTime::fromString(s, KDateTime::QtTextDate, &negZero);
    QVERIFY(dtlocal1.isDateOnly());
    QCOMPARE(dtlocal1.date(), QDate(1999,12,11));
    QCOMPARE(dtlocal1.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtlocal1.UTCOffset(), -8*3600);

    KDateTime dtzone(QDate(1999,6,11), QTime(3,45,06), london);
    s = dtzone.toString(KDateTime::QtTextDate);
    QCOMPARE(s, QString("Fri Jun 11 03:45:06 1999 +0100"));
    KDateTime dtzone1 = KDateTime::fromString(s, KDateTime::QtTextDate);
    QCOMPARE(dtzone1.dateTime().toUTC(), dtzone.dateTime().toUTC());
    QCOMPARE(dtzone1.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtzone1.UTCOffset(), 3600);
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
    QCOMPARE(dtzone1.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dtzone1.UTCOffset(), 3600);

    KDateTime dtclock(QDate(1999,12,11), QTime(3,45,06), KDateTime::ClockTime);
    s = dtclock.toString(KDateTime::QtTextDate);
    QCOMPARE(s, QString("Sat Dec 11 03:45:06 1999"));
    KDateTime dtclock1 = KDateTime::fromString(s, KDateTime::QtTextDate, &negZero);
    QCOMPARE(dtclock1.dateTime(), dtclock.dateTime());
    QCOMPARE(dtclock1.timeSpec(), KDateTime::ClockTime);
    QVERIFY(dtclock1 == dtclock);
    QVERIFY(!dtclock1.isDateOnly());
    QVERIFY(!negZero);
    dtclock.setDateOnly(true);
    s = dtclock.toString(KDateTime::QtTextDate);
    QCOMPARE(s, QString("Sat Dec 11 1999"));
    dtclock1 = KDateTime::fromString(s, KDateTime::QtTextDate, &negZero);
    QVERIFY(dtclock1.isDateOnly());
    QCOMPARE(dtclock1.date(), QDate(1999,12,11));
    QCOMPARE(dtclock1.timeSpec(), KDateTime::ClockTime);

    KDateTime dtutc(QDate(1999,12,11), QTime(3,45,00), KDateTime::UTC);
    s = dtutc.toString(KDateTime::QtTextDate);
    QCOMPARE(s, QString("Sat Dec 11 03:45:00 1999 +0000"));
    KDateTime dtutc1 = KDateTime::fromString(s, KDateTime::QtTextDate, &negZero);
    QCOMPARE(dtutc1.dateTime(), dtutc.dateTime());
    QCOMPARE(dtutc1.timeSpec(), KDateTime::UTC);
    QVERIFY(dtutc1 == dtutc);
    QVERIFY(!dtutc1.isDateOnly());
    QVERIFY(!negZero);
    dtutc.setDateOnly(true);
    s = dtutc.toString(KDateTime::QtTextDate);
    QCOMPARE(s, QString("Sat Dec 11 1999 +0000"));
    dtutc1 = KDateTime::fromString(s, KDateTime::QtTextDate, &negZero);
    QVERIFY(dtutc1.isDateOnly());
    QCOMPARE(dtutc1.date(), QDate(1999,12,11));
    QCOMPARE(dtutc1.timeSpec(), KDateTime::UTC);

    // Check '-0000'
    KDateTime dtutc2 = KDateTime::fromString(QString("Sat Dec 11 03:45:00 1999 -0000"), KDateTime::QtTextDate, &negZero);
    QVERIFY(dtutc1 == dtutc2);
    QVERIFY(negZero);

    // Check erroneous strings
    dtutc2 = KDateTime::fromString(QString("Sat Dec 11 03:45:00 1999 GMT"), KDateTime::QtTextDate, &negZero);
    QVERIFY(!dtutc2.isValid());
    dtutc2 = KDateTime::fromString(QString("Sun Dec 11 03:45:00 1999 +0000"), KDateTime::QtTextDate);
    QVERIFY(dtutc2.isValid());     // wrong weekday: accepted by Qt!!
    QWARN("Please ignore the QWARN message ('Parameter out of range') produced by the next statement");
    dtutc2 = KDateTime::fromString(QString("Satu, Dec 11 03:45:00 1999 +0000"), KDateTime::QtTextDate);
    QVERIFY(!dtutc2.isValid());     // bad weekday
    dtutc2 = KDateTime::fromString(QString("Sat Dece 11 03:45:00 1999 +0000"), KDateTime::QtTextDate);
    QVERIFY(dtutc2.isValid());     // bad month: accepted by Qt!!

    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}

void KDateTimeTest::strings_format()
{
    KCalendarSystem* calendar = new KCalendarSystemGregorian(KGlobal::locale());
    const KTimezone *london = KSystemTimezones::zone("Europe/London");
    const KTimezone *paris  = KSystemTimezones::zone("Europe/Paris");
    const KTimezone *berlin = KSystemTimezones::zone("Europe/Berlin");
    const KTimezone *cairo  = KSystemTimezones::zone("Africa/Cairo");
    KTimezones zones;
    zones.addConst(london);
    zones.addConst(paris);
    zones.addConst(berlin);
    zones.addConst(cairo);
  
    // Ensure that local time is different from UTC and different from 'london'
    const char *originalZone = ::getenv("TZ");   // save the original local time zone
    ::setenv("TZ", ":America/Los_Angeles", 1);
    ::tzset();

    // toString()
    QString all = QLatin1String("%Y.%y.%m.%:m.%B.%b.%d.%e.%A.%a-%H.%k.%I.%l.%M.%S?%:s?%P.%p.%:u.%z.%Z.%:Z.%:A.%:a.%:B.%:b/%:S.%:z.%%.");
    KDateTime dt(QDate(1999,2,3), QTime(6,5,0), KDateTime::LocalZone);
    QString s = dt.toString(all);
    QCOMPARE(s, QString::fromLatin1("1999.99.02.2.%1.%2.03.3.%3.%4-06.6.06.6.05.00?000?am.AM.-08.-0800.PST.America/Los_Angeles.Wednesday.Wed.February.Feb/.-08:00.%.")
                                   .arg(calendar->monthName(2,1999,false)).arg(calendar->monthName(2,1999,true))
                                   .arg(calendar->weekDayName(3,false)).arg(calendar->weekDayName(3,true)));

    KDateTime dtzone(QDate(1970,4,30), QTime(12,45,16,25), london);
    s = dtzone.toString(all);
    QCOMPARE(s, QString::fromLatin1("1970.70.04.4.%1.%2.30.30.%3.%4-12.12.12.12.45.16?025?pm.PM.+01.+0100.BST.Europe/London.Thursday.Thu.April.Apr/:16.+01:00.%.")
                                   .arg(calendar->monthName(4,2000,false)).arg(calendar->monthName(4,2000,true))
                                   .arg(calendar->weekDayName(4,false)).arg(calendar->weekDayName(4,true)));

    KDateTime dtclock(QDate(2005,9,5), QTime(0,0,06,1), KDateTime::ClockTime);
    s = dtclock.toString(all);
    QCOMPARE(s, QString::fromLatin1("2005.05.09.9.%1.%2.05.5.%3.%4-00.0.12.12.00.06?001?am.AM.....Monday.Mon.September.Sep/:06..%.")
                                   .arg(calendar->monthName(9,2000,false)).arg(calendar->monthName(9,2000,true))
                                   .arg(calendar->weekDayName(1,false)).arg(calendar->weekDayName(1,true)));

    KDateTime dtutc(QDate(2000,12,31), QTime(13,45,16,100), KDateTime::UTC);
    s = dtutc.toString(all);
    QCOMPARE(s, QString::fromLatin1("2000.00.12.12.%1.%2.31.31.%3.%4-13.13.01.1.45.16?100?pm.PM.+00.+0000.UTC.UTC.Sunday.Sun.December.Dec/:16.+00:00.%.")
                                   .arg(calendar->monthName(12,2000,false)).arg(calendar->monthName(12,2000,true))
                                   .arg(calendar->weekDayName(7,false)).arg(calendar->weekDayName(7,true)));

    // fromString() without KTimezones parameter
    dt = KDateTime::fromString(QLatin1String("2005/9/05/20:2,03"), QLatin1String("%Y/%:m/%e/%S:%k,%M"));
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(2,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeSpec(), KDateTime::ClockTime);

    dt = KDateTime::fromString(QString::fromLatin1("%1pm05ab%2t/052/20:2,03+10").arg(calendar->weekDayName(1,false)).arg(calendar->monthName(9,1999,false)),
                               QLatin1String("%a%p%yab%Bt/%e2/%S:%I,%M %z"));
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dt.UTCOffset(), 10*3600);
    dt = KDateTime::fromString(QString::fromLatin1("%1pm05ab%2t/052/20:2,03+10").arg(calendar->weekDayName(1,true)).arg(calendar->monthName(9,1999,true)),
                               QLatin1String("%a%p%yab%Bt/%e2/%S:%I,%M %z"));
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dt.UTCOffset(), 10*3600);
    dt = KDateTime::fromString(QString::fromLatin1("monpm05absEpt/052/20:2,03+10"), QLatin1String("%a%p%yab%Bt/%e2/%S:%I,%M %z"));
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dt.UTCOffset(), 10*3600);
    dt = KDateTime::fromString(QString::fromLatin1("monDAYpm05absEptemBert/052/20:2,03+10"), QLatin1String("%a%p%yab%Bt/%e2/%S:%I,%M %z"));
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dt.UTCOffset(), 10*3600);
    dt = KDateTime::fromString(QString::fromLatin1("monDAYpm05abmzatemer/052/20:2,03+10"), QLatin1String("%a%p%yab%B/%e2/%S:%I,%M %z"));
    QVERIFY(!dt.isValid());    // invalid month name
    dt = KDateTime::fromString(QString::fromLatin1("monDApm05absep/052/20:2,03+10"), QLatin1String("%a%p%yab%B/%e2/%S:%I,%M %z"));
    QVERIFY(!dt.isValid());    // invalid day name
    dt = KDateTime::fromString(QLatin1String("mONdAYPM2005absEpt/052/20:02,03+1000"), QLatin1String("%:A%:p%Yab%Bt/%e2/%S:%I,%M %:u"));
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,3,20), Qt::LocalTime));
    QCOMPARE(dt.UTCOffset(), 10*3600);
    QCOMPARE(dt.timeSpec(), KDateTime::OffsetFromUTC);
    dtclock = KDateTime::fromString(QLatin1String("mONdAYPM2005abSept/052/20:02,03+100"), QLatin1String("%:A%:p%Yab%Bt/%e2/%S:%I,%M %:u"));
    QVERIFY(!dtclock.isValid());    // wrong number of digits in UTC offset
    dtclock = KDateTime::fromString(QLatin1String("mONdAYPM2005abSept/052/20:02,03+1"), QLatin1String("%:A%:p%Yab%Bt/%e2/%S:%I,%M %z"));
    QVERIFY(!dtclock.isValid());    // wrong number of digits in UTC offset
    dtclock = KDateTime::fromString(QLatin1String("mONdAYPM2005absEpt/052/20:13,03+1000"), QLatin1String("%:A%:p%Yab%Bt/%e2/%S:%I,%M %:u"));
    QVERIFY(!dtclock.isValid());    // hours out of range for am/pm
    dtclock = KDateTime::fromString(QLatin1String("mONdAYPM2005absEpt/052/20:00,03+1000"), QLatin1String("%:A%:p%Yab%Bt/%e2/%S:%I,%M %:u"));
    QVERIFY(!dtclock.isValid());    // hours out of range for am/pm

    // fromString() with KTimezones parameter
    dt = KDateTime::fromString(QLatin1String("mon 2005/9/05/20:2,03"), QLatin1String("%:a %Y/%:m/%e/%S:%k,%M"), &zones);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(2,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeSpec(), KDateTime::ClockTime);
    dt = KDateTime::fromString(QLatin1String("tue 2005/9/05/20:2,03"), QLatin1String("%:a %Y/%:m/%e/%S:%k,%M"), &zones);
    QVERIFY(!dt.isValid());    // wrong day-of-week

    dt = KDateTime::fromString(QLatin1String("pm2005absEpt/05monday/20:2,03+03:00"), QLatin1String("%p%Yab%Bt/%e%:A/%S:%I,%M %:z"), &zones);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeSpec(), KDateTime::TimeZone);
    QCOMPARE(dt.UTCOffset(), 3*3600);
    QCOMPARE(dt.timeZone(), cairo);
    dt = KDateTime::fromString(QLatin1String("pm2005absEpt/05sunday/20:2,03+03:00"), QLatin1String("%p%Yab%Bt/%e%A/%S:%I,%M %:z"), &zones);
    QVERIFY(!dt.isValid());    // wrong day-of-week

    dt = KDateTime::fromString(QLatin1String("200509051430:01.3+0100"), QLatin1String("%Y%m%d%H%M%:S%:s%z"), &zones, true);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,30,01,300), Qt::LocalTime));
    QCOMPARE(dt.timeSpec(), KDateTime::TimeZone);
    QCOMPARE(dt.timeZone(), london);
    QCOMPARE(dt.UTCOffset(), 3600);

    dt = KDateTime::fromString(QLatin1String("200509051430:01.3+0500"), QLatin1String("%Y%m%d%H%M%:S%:s%z"), &zones, false);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,30,01,300), Qt::LocalTime));
    QCOMPARE(dt.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dt.UTCOffset(), 5*3600);

    dt = KDateTime::fromString(QLatin1String("200509051430:01.3+0200"), QLatin1String("%Y%m%d%H%M%:S%:s%z"), &zones, true);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,30,01,300), Qt::LocalTime));
    QCOMPARE(dt.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dt.UTCOffset(), 2*3600);
    dt = KDateTime::fromString(QLatin1String("200509051430:01.3+0200"), QLatin1String("%Y%m%d%H%M%:S%:s%z"), &zones, false);
    QVERIFY(!dt.isValid());    // matches paris and berlin

    dt = KDateTime::fromString(QLatin1String("2005September051430 CEST"), QLatin1String("%Y%:B%d%H%M%:S %Z"), &zones, true);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(14,30,0), Qt::LocalTime));
    QCOMPARE(dt.timeSpec(), KDateTime::OffsetFromUTC);
    QCOMPARE(dt.UTCOffset(), 2*3600);
    dt = KDateTime::fromString(QLatin1String("2005September051430 CEST"), QLatin1String("%Y%:B%d%H%M%:S %Z"), &zones, false);
    QVERIFY(!dt.isValid());    // matches paris and berlin

    dt = KDateTime::fromString(QLatin1String("pm05absEptembeRt/   052/   20:12,03+0100"), QLatin1String("%:P%yab%:bt/  %e2/%t%S:%l,%M %z"), &zones);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,5), QTime(12,3,20), Qt::LocalTime));
    QCOMPARE(dt.timeSpec(), KDateTime::TimeZone);
    QCOMPARE(dt.UTCOffset(), 3600);
    QCOMPARE(dt.timeZone(), london);

    dt = KDateTime::fromString(QLatin1String("2005absEpt/042sun/20.0123456:12Am,3Africa/Cairo%"), QLatin1String("%Yab%bt/%e2%a/%S%:s:%l%P,%M %:Z%%"), &zones);
    QCOMPARE(dt.dateTime(), QDateTime(QDate(2005,9,4), QTime(0,3,20,12), Qt::LocalTime));
    QCOMPARE(dt.timeSpec(), KDateTime::TimeZone);
    QCOMPARE(dt.timeZone(), cairo);
    QCOMPARE(dt.UTCOffset(), 3*3600);

    dt = KDateTime::fromString(QLatin1String("110509051430:01.3+0500"), QLatin1String("%Y%m%d%H%M%:S%:s%z"));
    QVERIFY(!dt.isValid());    // too early
    QVERIFY(dt.isTooEarly());
    QVERIFY(!dt.isTooLate());

    dt = KDateTime::fromString(QLatin1String("910509051430:01.3+0500"), QLatin1String("%Y%m%d%H%M%:S%:s%z"));
    QVERIFY(!dt.isValid());    // too early
    QVERIFY(!dt.isTooEarly());
    QVERIFY(dt.isTooLate());


    // Restore the original local time zone
    if (!originalZone)
        ::unsetenv("TZ");
    else
        ::setenv("TZ", originalZone, 1);
    ::tzset();
}
