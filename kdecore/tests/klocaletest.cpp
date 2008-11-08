/* This file is part of the KDE libraries
    Copyright (c) 2005 Thomas Braxton <brax108@cox.net>
    Copyright (c) 2007 David Jarvie <software@astrojar.org.uk>

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

#include "klocaletest.h"
#include "qtest_kde.h"

#include "klocale.h"
#include "kglobal.h"
#include "kcalendarsystem.h"
#include "kdatetime.h"
#include "ksystemtimezone.h"
#include <QtCore/QString>
#include <QtCore/QDate>
#include <QtDBus/QtDBus>

#include "klocaletest.moc"

void
KLocaleTest::initTestCase()
{
    KGlobal::setLocale(new KLocale(QString(), QLatin1String("en_US"), QLatin1String("C")));
	KGlobal::locale()->setThousandsSeparator(QLatin1String(","));
}

void
KLocaleTest::formatNumbers()
{
	KLocale locale(*KGlobal::locale());

	QCOMPARE(locale.formatNumber(70), QString("70.00"));
	QCOMPARE(locale.formatNumber(70, 0), QString("70"));
	QCOMPARE(locale.formatNumber(70.2), QString("70.20"));
	QCOMPARE(locale.formatNumber(70.24), QString("70.24"));
	QCOMPARE(locale.formatNumber(70.245), QString("70.25"));
	QCOMPARE(locale.formatNumber(1234567.89123456789,8),
		QString("1,234,567.89123457"));
}

void
KLocaleTest::formatNumberStrings()
{
	KLocale locale(*KGlobal::locale());

	QCOMPARE(locale.formatNumber("70"), QString("70.00"));
	QCOMPARE(locale.formatNumber("70", true, 2), QString("70.00"));
	QCOMPARE(locale.formatNumber("70", true, 0), QString("70"));
	QCOMPARE(locale.formatNumber("0.2", true, 1), QString("0.2"));
	QCOMPARE(locale.formatNumber("70.9123", true, 0), QString("71"));
	QCOMPARE(locale.formatNumber("-70.2", true, 2), QString("-70.20"));
	QCOMPARE(locale.formatNumber("+70.24", true, 2), QString("70.24"));
	QCOMPARE(locale.formatNumber("70.245", true, 2), QString("70.25"));
	QCOMPARE(locale.formatNumber("99.996", true, 2), QString("100.00"));
	QCOMPARE(locale.formatNumber("12345678901234567.89123456789",false,0),
		QString("12,345,678,901,234,567.89123456789"));
	QCOMPARE(locale.formatNumber("99.9e96", true, 2), QString("99.90e96"));
	QCOMPARE(locale.formatNumber("99e", true, 1), QString("99.0"));
	QCOMPARE(locale.formatNumber("e2", true, 0), QString("0e2"));
	QCOMPARE(locale.formatNumber("1.", true, 1), QString("1.0"));
	QCOMPARE(locale.formatNumber("1.", true, 0), QString("1"));
	QCOMPARE(locale.formatNumber("1.", false, 1), QString("1.")); // no rounding -> 1. unchanged
	QCOMPARE(locale.formatNumber("1.", false, 0), QString("1."));
        QCOMPARE(locale.formatNumber("abcd", true, 2), QString("0.00")); // invalid number
}

void
KLocaleTest::readNumber()
{
	KLocale locale(*KGlobal::locale());
	bool ok=false;

	locale.readNumber("12,1", &ok);
	QVERIFY(!ok);
	QCOMPARE(locale.readNumber("12,100", &ok), 12100.0);
	QVERIFY(ok);
	locale.readNumber("12,100000,00", &ok);
	QVERIFY(!ok);
	locale.readNumber("12,100000000", &ok);
	QVERIFY(!ok);
	locale.readNumber("12,,100,000", &ok);
	QVERIFY(!ok);
	locale.readNumber("12,1000,000", &ok);
	QVERIFY(!ok);
	locale.readNumber("12,0000000,000", &ok);
	QVERIFY(!ok);
	locale.readNumber("12,0000000", &ok);
	QVERIFY(!ok);
	QCOMPARE(locale.readNumber("12,146,131.12", &ok), 12146131.12);
	QVERIFY(ok);
	QCOMPARE(locale.readNumber("1.12345678912", &ok), 1.12345678912);
	QVERIFY(ok);
}

void
KLocaleTest::formatDate()
{
	KLocale locale(*KGlobal::locale());
	QString small("yyyy-MM-dd");
	QString full("dddd dd MMMM yyyy");
	QDate date;

        // Ensure that user configuration isn't messing with us;
        // shouldn't happen though, since qtest_kde.h sets KDEHOME.
        QCOMPARE(locale.dateFormat(), QString("%A %d %B %Y"));

	date.setYMD(2002, 5, 3);
	QCOMPARE(locale.formatDate(date), date.toString(full));
	QCOMPARE(locale.formatDate(date, KLocale::ShortDate), date.toString(small));

	date = QDate::currentDate();
	QCOMPARE(locale.formatDate(date), date.toString(full));
	QCOMPARE(locale.formatDate(date, KLocale::ShortDate), date.toString(small));

	QCOMPARE(locale.formatDate(date, KLocale::FancyLongDate), QString("Today"));
	QCOMPARE(locale.formatDate(date.addDays(-1), KLocale::FancyLongDate), QString("Yesterday"));
	QDate dat = date.addDays(-2);
	QCOMPARE(locale.formatDate(dat, KLocale::FancyLongDate), locale.calendar()->weekDayName(dat));
	dat = date.addDays(-3);
	QCOMPARE(locale.formatDate(dat, KLocale::FancyLongDate), locale.calendar()->weekDayName(dat));
	dat = date.addDays(-4);
	QCOMPARE(locale.formatDate(dat, KLocale::FancyLongDate), locale.calendar()->weekDayName(dat));
	dat = date.addDays(-5);
	QCOMPARE(locale.formatDate(dat, KLocale::FancyLongDate), locale.calendar()->weekDayName(dat));
	dat = date.addDays(-6);
	QCOMPARE(locale.formatDate(dat, KLocale::FancyLongDate), locale.calendar()->weekDayName(dat));
	dat = date.addDays(-7);
	QCOMPARE(locale.formatDate(dat, KLocale::FancyLongDate), dat.toString(full));
	dat = date.addDays(1);
	QCOMPARE(locale.formatDate(dat, KLocale::FancyLongDate), dat.toString(full));
}

void
KLocaleTest::readDate()
{
	KLocale locale(*KGlobal::locale());
	QString small("yyyy-MM-dd");
	QString full("dddd dd MMMM yyyy");
	bool ok=false;
	QDate date;

	date.setYMD( 2002, 5, 3 );
	QCOMPARE(locale.readDate(date.toString(small)), date);
	QCOMPARE(locale.readDate(date.toString(full)), date);
	locale.readDate(date.toString(small), KLocale::NormalFormat, &ok);
	QVERIFY(!ok);
	locale.readDate(date.toString(full), KLocale::ShortFormat, &ok);
	QVERIFY(!ok);

	date = QDate::currentDate();
	QCOMPARE(locale.readDate(date.toString(small)), date);
	QCOMPARE(locale.readDate(date.toString(full)), date);
	locale.readDate(date.toString(small), KLocale::NormalFormat, &ok);
	QVERIFY(!ok);
	locale.readDate(date.toString(full), KLocale::ShortFormat, &ok);
	QVERIFY(!ok);

	locale.readDate("2005-02-31", &ok);
	QVERIFY(!ok);
}

void
KLocaleTest::formatTime()
{
	KLocale locale(*KGlobal::locale());
	QTime time(0,22,33);

	locale.setTimeFormat("%H:%M %p");
	QCOMPARE(locale.formatTime(time, true, false), QString("00:22 am"));
	QCOMPARE(locale.formatTime(time, true, true), QString("00:22"));

	locale.setTimeFormat("%H:%M:%S %p");
	QCOMPARE(locale.formatTime(time, true, false), QString("00:22:33 am"));
	QCOMPARE(locale.formatTime(time, true, true), QString("00:22:33"));

	locale.setTimeFormat("%l : %M : %S %p"); // #164813
	QCOMPARE(locale.formatTime(time, true), QString("12 : 22 : 33 am"));
	QCOMPARE(locale.formatTime(time, false), QString("12 : 22 am"));
}

void
KLocaleTest::formatDateTime()
{
	KLocale locale(*KGlobal::locale());
	QString small("yyyy-MM-dd hh:mm");
	QString smallsecs("yyyy-MM-dd hh:mm:ss");
	QString full("dddd dd MMMM yyyy hh:mm");
	QString fullsecs("dddd dd MMMM yyyy hh:mm:ss");
	QString tfmt(" hh:mm");
	QDateTime qdt;

        // Ensure that user configuration isn't messing with us;
        // shouldn't happen though, since qtest_kde.h sets KDEHOME.
        QCOMPARE(locale.dateFormat(), QString("%A %d %B %Y"));

	qdt = QDateTime(QDate(2002, 5, 3), QTime(10, 20, 30));
	QCOMPARE(locale.formatDateTime(qdt), qdt.toString(small));
	QCOMPARE(locale.formatDateTime(qdt, KLocale::LongDate), qdt.toString(full));

	qdt = QDateTime::currentDateTime();
	QDate today = qdt.date();
	QTime nowt = qdt.time();
	QCOMPARE(locale.formatDateTime(qdt), qdt.toString(small));
	QCOMPARE(locale.formatDateTime(qdt, KLocale::LongDate), qdt.toString(full));

	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), QString("Today") + qdt.time().toString(tfmt));
	qdt = qdt.addSecs(3605);  // more than 1 hour from now
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), qdt.toString(full));
	qdt.setDate(today);
	qdt.setTime(QTime(0,0,0));
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), QString("Today") + qdt.time().toString(tfmt));
	qdt = qdt.addSecs(-1);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), QString("Yesterday") + qdt.time().toString(tfmt));
	qdt = qdt.addDays(2);   // tomorrow
	qdt.setTime(nowt);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), qdt.toString(full));
	qdt = qdt.addDays(-2);   // yesterday
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), QString("Yesterday") + qdt.time().toString(tfmt));
	qdt = qdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), locale.calendar()->weekDayName(qdt.date()) + qdt.time().toString(tfmt));
	qdt = qdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), locale.calendar()->weekDayName(qdt.date()) + qdt.time().toString(tfmt));
	qdt = qdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), locale.calendar()->weekDayName(qdt.date()) + qdt.time().toString(tfmt));
	qdt = qdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), locale.calendar()->weekDayName(qdt.date()) + qdt.time().toString(tfmt));
	qdt = qdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), locale.calendar()->weekDayName(qdt.date()) + qdt.time().toString(tfmt));
	qdt = qdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), qdt.toString(full));

	// The use of KSystemTimeZones requires kded to be running
	if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kded")) {
	    QSKIP( "kded not running", SkipAll );
	}

	small = "%Y-%m-%d %H:%M";
	smallsecs = "%Y-%m-%d %H:%M:%S";
	full = "%A %d %B %Y %H:%M";
	fullsecs = "%A %d %B %Y %H:%M:%S";
	KDateTime kdt;
	const KTimeZone tz = KSystemTimeZones::zone("Pacific/Fiji");
        if (!tz.isValid())
            QSKIP( "Pacific/Fiji timezone not available", SkipAll );
	kdt = KDateTime::currentDateTime(tz);
	today = kdt.date();
	nowt = kdt.time();
	QCOMPARE(locale.formatDateTime(kdt), kdt.toString(small));
//	QCOMPARE(locale.formatDateTime(kdt, KLocale::ShortDate, KLocale::Seconds), kdt.toString(smallsecs));
	QCOMPARE(locale.formatDateTime(kdt, KLocale::LongDate), kdt.toString(full));
//	QCOMPARE(locale.formatDateTime(kdt, KLocale::LongDate, KLocale::Seconds), kdt.toString(fullsecs));

//	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), QString("Today") + kdt.time().toString(tfmt));
	kdt = kdt.addSecs(3605);  // more than 1 hour from now
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), kdt.toString(full));
	kdt.setDate(today);
	kdt.setTime(QTime(0,0,0));
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), QString("Today") + kdt.time().toString(tfmt));
	kdt = kdt.addSecs(-1);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), QString("Yesterday") + kdt.time().toString(tfmt));
	kdt = kdt.addDays(2);   // tomorrow
	kdt.setTime(nowt);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), kdt.toString(full));
	kdt = kdt.addDays(-2);   // yesterday
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), QString("Yesterday") + kdt.time().toString(tfmt));
	kdt = kdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), locale.calendar()->weekDayName(kdt.date()) + kdt.time().toString(tfmt));
	kdt = kdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), locale.calendar()->weekDayName(kdt.date()) + kdt.time().toString(tfmt));
	kdt = kdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), locale.calendar()->weekDayName(kdt.date()) + kdt.time().toString(tfmt));
	kdt = kdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), locale.calendar()->weekDayName(kdt.date()) + kdt.time().toString(tfmt));
	kdt = kdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), locale.calendar()->weekDayName(kdt.date()) + kdt.time().toString(tfmt));
	kdt = kdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), kdt.toString(full));
}

void
KLocaleTest::readTime()
{
	KLocale locale(*KGlobal::locale());
	bool ok = false;

	QCOMPARE(locale.readTime("11:22:33", &ok), QTime(11,22,33));
	QVERIFY(ok);

	QCOMPARE(locale.readTime("11:22", &ok), QTime(11,22,0));
	QVERIFY(ok);

	locale.readTime("11:22:33", KLocale::WithoutSeconds, &ok);
	QVERIFY(!ok);

	QCOMPARE(locale.readTime("11:22", KLocale::WithoutSeconds, &ok), QTime(11, 22, 0));
	QVERIFY(ok);
}

void
KLocaleTest::prettyFormatDuration()
{
	KLocale locale(*KGlobal::locale());

	QCOMPARE(locale.prettyFormatDuration(1000), QString("1 second"));
	QCOMPARE(locale.prettyFormatDuration(5000), QString("5 seconds"));
	QCOMPARE(locale.prettyFormatDuration(60000), QString("1 minute"));
	QCOMPARE(locale.prettyFormatDuration(300000), QString("5 minutes"));
	QCOMPARE(locale.prettyFormatDuration(3600000), QString("1 hour"));
	QCOMPARE(locale.prettyFormatDuration(18000000), QString("5 hours"));
	QCOMPARE(locale.prettyFormatDuration(75000), QString("1 minute and 15 seconds"));
	// Problematic case #1 (there is a reference to this case on klocale.cpp)
	QCOMPARE(locale.prettyFormatDuration(119999), QString("2 minutes"));
	// This case is strictly 2 hours, 15 minutes and 59 seconds. However, since the range is
	// pretty high between hours and seconds, prettyFormatDuration always omits seconds when there
	// are hours in scene.
	QCOMPARE(locale.prettyFormatDuration(8159000), QString("2 hours and 15 minutes"));
	// This case is strictly 1 hour and 10 seconds. For the same reason, prettyFormatDuration
	// detects that 10 seconds is just garbage compared to 1 hour, and omits it on the result.
	QCOMPARE(locale.prettyFormatDuration(3610000), QString("1 hour"));
}

void
KLocaleTest::bug95511()
{
	KLocale locale(*KGlobal::locale());
	bool ok=false;

	locale.setCurrencySymbol("$$");
	QCOMPARE(locale.readMoney("1,234,567,890.12$$", &ok), 1234567890.12);
	QVERIFY(ok);
	QCOMPARE(locale.readMoney("-1,234,567,890.12$$", &ok), -1234567890.12);
	QVERIFY(ok);

	locale.setCurrencySymbol("dollars");
	QCOMPARE(locale.readMoney("12,345,678,901,234,567 dollars", &ok),
		12345678901234567.00);
	QVERIFY(ok);
	QCOMPARE(locale.readMoney("-12,345,678,901.00 dollars", &ok),
		-12345678901.00);
	QVERIFY(ok);
}

void  KLocaleTest::weekDays()
{
    KLocale locale(*KGlobal::locale());

    //Test setting of weekStartDay
    locale.setWeekStartDay(3);
    QCOMPARE(locale.weekStartDay(), 3);
    locale.setWeekStartDay(0);
    QCOMPARE(locale.weekStartDay(), 3);
    locale.setWeekStartDay(8);
    QCOMPARE(locale.weekStartDay(), 3);

    //Test setting of workingWeekStartDay
    locale.setWorkingWeekStartDay(3);
    locale.setWorkingWeekEndDay(6);
    QCOMPARE(locale.workingWeekStartDay(), 3);
    locale.setWorkingWeekStartDay(0);
    QCOMPARE(locale.workingWeekStartDay(), 3);
    locale.setWorkingWeekStartDay(8);
    QCOMPARE(locale.workingWeekStartDay(), 3);

    //Test setting of workingWeekEndDay
    locale.setWorkingWeekStartDay(3);
    locale.setWorkingWeekEndDay(6);
    QCOMPARE(locale.workingWeekEndDay(), 6);
    locale.setWorkingWeekEndDay(0);
    QCOMPARE(locale.workingWeekEndDay(), 6);
    locale.setWorkingWeekEndDay(8);
    QCOMPARE(locale.workingWeekEndDay(), 6);

    //Test setting of weekStartDay
    locale.setWeekDayOfPray(3);
    QCOMPARE(locale.weekDayOfPray(), 3);
    locale.setWeekDayOfPray(-1);
    QCOMPARE(locale.weekDayOfPray(), 3);
    locale.setWeekDayOfPray(8);
    QCOMPARE(locale.weekDayOfPray(), 3);
    locale.setWeekDayOfPray(0);
    QCOMPARE(locale.weekDayOfPray(), 0);
}

QTEST_KDEMAIN_CORE(KLocaleTest)
