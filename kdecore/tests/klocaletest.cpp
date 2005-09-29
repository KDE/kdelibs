/* This file is part of the KDE libraries
    Copyright (c) 2005 Thomas Braxton <brax108@cox.net>

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
#include "QtTest/qttest_kde.h"

#include "klocale.h"
#include "kglobal.h"
#include <qstring.h>
#include <qdatetime.h>

#include "klocaletest.moc"

void
KLocaleTest::initTestCase()
{
	KGlobal::locale()->setLanguage(QLatin1String("en_US"));
	KGlobal::locale()->setCountry(QLatin1String("C"));
	KGlobal::locale()->setThousandsSeparator(QLatin1String(","));
}

void
KLocaleTest::formatNumbers()
{
	KLocale locale(*KGlobal::locale());

	COMPARE(locale.formatNumber(70), QString("70.00"));
	COMPARE(locale.formatNumber(70, 0), QString("70"));
	COMPARE(locale.formatNumber(70.2), QString("70.20"));
	COMPARE(locale.formatNumber(70.24), QString("70.24"));
	COMPARE(locale.formatNumber(70.245), QString("70.25"));
	COMPARE(locale.formatNumber(1234567.89123456789,8),
		QString("1,234,567.89123457"));
}

void
KLocaleTest::formatNumberStrings()
{
	KLocale locale(*KGlobal::locale());

	COMPARE(locale.formatNumber("70"), QString("70.00"));
	COMPARE(locale.formatNumber("70", true, 2), QString("70.00"));
	COMPARE(locale.formatNumber("70", true, 0), QString("70"));
	COMPARE(locale.formatNumber("70.9123", true, 0), QString("71"));
	COMPARE(locale.formatNumber("-70.2", true, 2), QString("-70.20"));
	COMPARE(locale.formatNumber("+70.24", true, 2), QString("70.24"));
	COMPARE(locale.formatNumber("70.245", true, 2), QString("70.25"));
	COMPARE(locale.formatNumber("99.996", true, 2), QString("100.00"));
	COMPARE(locale.formatNumber("12345678901234567.89123456789",false,0),
		QString("12,345,678,901,234,567.89123456789"));
}

void
KLocaleTest::readNumber()
{
	KLocale locale(*KGlobal::locale());
	bool ok=false;

	locale.readNumber("12,1", &ok);
	VERIFY(!ok);
	COMPARE(locale.readNumber("12,100", &ok), 12100.0);
	VERIFY(ok);
	locale.readNumber("12,100000,00", &ok);
	VERIFY(!ok);
	locale.readNumber("12,100000000", &ok);
	VERIFY(!ok);
	locale.readNumber("12,,100,000", &ok);
	VERIFY(!ok);
	locale.readNumber("12,1000,000", &ok);
	VERIFY(!ok);
	locale.readNumber("12,0000000,000", &ok);
	VERIFY(!ok);
	locale.readNumber("12,0000000", &ok);
	VERIFY(!ok);
	COMPARE(locale.readNumber("12,146,131.12", &ok), 12146131.12);
	VERIFY(ok);
	COMPARE(locale.readNumber("1.12345678912", &ok), 1.12345678912);
	VERIFY(ok);
}

void
KLocaleTest::formatDate()
{
	KLocale locale(*KGlobal::locale());
	QString small("yyyy-MM-dd");
	QString full("dddd dd MMMM yyyy");
	QDate date;

	date.setYMD(2002, 5, 3);
	COMPARE(locale.formatDate(date), date.toString(full));
	COMPARE(locale.formatDate(date, true), date.toString(small));

	date = QDate::currentDate();
	COMPARE(locale.formatDate(date), date.toString(full));
	COMPARE(locale.formatDate(date, true), date.toString(small));
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
	COMPARE(locale.readDate(date.toString(small)), date);
	COMPARE(locale.readDate(date.toString(full)), date);
	locale.readDate(date.toString(small), KLocale::NormalFormat, &ok);
	VERIFY(!ok);
	locale.readDate(date.toString(full), KLocale::ShortFormat, &ok);
	VERIFY(!ok);

	date = QDate::currentDate();
	COMPARE(locale.readDate(date.toString(small)), date);
	COMPARE(locale.readDate(date.toString(full)), date);
	locale.readDate(date.toString(small), KLocale::NormalFormat, &ok);
	VERIFY(!ok);
	locale.readDate(date.toString(full), KLocale::ShortFormat, &ok);
	VERIFY(!ok);

	locale.readDate("2005-02-31", &ok);
	VERIFY(!ok);
}

void
KLocaleTest::formatTime()
{
	KLocale locale(*KGlobal::locale());
	QTime time(0,22,33);

	locale.setTimeFormat("%H:%M %p");
	COMPARE(locale.formatTime(time, true, false), QString("00:22 am"));
	COMPARE(locale.formatTime(time, true, true), QString("00:22"));

	locale.setTimeFormat("%H:%M:%S %p");
	COMPARE(locale.formatTime(time, true, false), QString("00:22:33 am"));
	COMPARE(locale.formatTime(time, true, true), QString("00:22:33"));
}

void
KLocaleTest::readTime()
{
	KLocale locale(*KGlobal::locale());
	bool ok = false;

	COMPARE(locale.readTime("11:22:33", &ok), QTime(11,22,33));
	VERIFY(ok);

	COMPARE(locale.readTime("11:22", &ok), QTime(11,22,0));
	VERIFY(ok);

	locale.readTime("11:22:33", KLocale::WithoutSeconds, &ok);
	VERIFY(!ok);

	locale.readTime("11:22", KLocale::WithoutSeconds, &ok);
	VERIFY(ok);
}

void
KLocaleTest::bug95511()
{
	KLocale locale(*KGlobal::locale());
	bool ok=false;

	locale.setCurrencySymbol("$$");
	COMPARE(locale.readMoney("1,234,567,890.12$$", &ok), 1234567890.12);
	VERIFY(ok);
	COMPARE(locale.readMoney("-1,234,567,890.12$$", &ok), -1234567890.12);
	VERIFY(ok);

	locale.setCurrencySymbol("dollars");
	COMPARE(locale.readMoney("12,345,678,901,234,567 dollars", &ok),
		12345678901234567.00);
	VERIFY(ok);
	COMPARE(locale.readMoney("-12,345,678,901.00 dollars", &ok),
		-12345678901.00);
	VERIFY(ok);
}

QTTEST_KDEMAIN(KLocaleTest, NoGUI)
