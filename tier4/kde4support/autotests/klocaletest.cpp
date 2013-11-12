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
#include <QtTest/QtTest>

#include "klocale.h"
#include "klocale_p.h"
#include "kconfiggroup.h"
#include "kcalendarsystem.h"
#include "kdayperiod_p.h"
#include "kdatetime.h"
#include "ksystemtimezone.h"
#include <QtCore/QString>
#include <QtCore/QDate>
#include <QDBusConnection>
#include <QDBusConnectionInterface>


void
KLocaleTest::initTestCase()
{
    qputenv("LC_ALL", "C");
    QStandardPaths::enableTestMode(true);

    KLocale::global()->setCountry(QLatin1String("C"), 0);
    KLocale::global()->setThousandsSeparator(QLatin1String(","));
}

void KLocaleTest::languages()
{
    QVERIFY(KLocale::global()->installedLanguages().contains("en_US"));
}

void
KLocaleTest::formatNumbers()
{
	KLocale locale(*KLocale::global());

    KLocale::global()->setDecimalPlaces(2);
	QCOMPARE(locale.formatNumber(70), QString("70.00"));
	QCOMPARE(locale.formatNumber(70, 0), QString("70"));
	QCOMPARE(locale.formatNumber(70.2), QString("70.20"));
	QCOMPARE(locale.formatNumber(70.24), QString("70.24"));
	QCOMPARE(locale.formatNumber(70.245), QString("70.25"));
	QCOMPARE(locale.formatNumber(1234567.89123456789,8),
		QString("1,234,567.89123457"));
    QCOMPARE(locale.formatNumber(1234567.89123456789, 0), QString("1,234,568"));

    // Test Grouping
    locale.d->setNumericDigitGrouping(QList<int>());
    QCOMPARE(locale.formatNumber(123456789, 0), QString("123456789"));
    QCOMPARE(locale.formatNumber(123456789.01), QString("123456789.01"));
    locale.d->setNumericDigitGrouping(QList<int>() << 3 << 2);
    QCOMPARE(locale.formatNumber(123456789, 0), QString("12,34,56,789"));
    QCOMPARE(locale.formatNumber(123456789.01), QString("12,34,56,789.01"));
    locale.d->setNumericDigitGrouping(QList<int>() << 3 << -1);
    QCOMPARE(locale.formatNumber(123456789, 0), QString("123456,789"));
    QCOMPARE(locale.formatNumber(123456789.01), QString("123456,789.01"));

    //Test it formats correctly with an empty and space separator.
    locale.d->setNumericDigitGrouping(QList<int>() << 3);
    QCOMPARE(locale.formatNumber(123456789, 0), QString("123,456,789"));
    QCOMPARE(locale.formatNumber(123456789.01), QString("123,456,789.01"));
    locale.setThousandsSeparator(QString());
    QCOMPARE(locale.formatNumber(123456789, 0), QString("123456789"));
    QCOMPARE(locale.formatNumber(123456789.01), QString("123456789.01"));
    locale.setThousandsSeparator(" ");
    QCOMPARE(locale.formatNumber(123456789, 0), QString("123 456 789"));
    QCOMPARE(locale.formatNumber(123456789.01), QString("123 456 789.01"));
}

void
KLocaleTest::formatNumberStrings()
{
	KLocale locale(*KLocale::global());

    KLocale::global()->setDecimalPlaces(2);
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

    // Test Grouping
    locale.d->setNumericDigitGrouping(QList<int>());
    QCOMPARE(locale.formatNumber("123456789", true, 0), QString("123456789"));
    QCOMPARE(locale.formatNumber("123456789.01"),       QString("123456789.01"));
    locale.d->setNumericDigitGrouping(QList<int>() << 3 << 2);
    QCOMPARE(locale.formatNumber("123456789", true, 0), QString("12,34,56,789"));
    QCOMPARE(locale.formatNumber("123456789.01"),       QString("12,34,56,789.01"));
    locale.d->setNumericDigitGrouping(QList<int>() << 3 << -1);
    QCOMPARE(locale.formatNumber("123456789", true, 0), QString("123456,789"));
    QCOMPARE(locale.formatNumber("123456789.01"),       QString("123456,789.01"));

    //Test it formats correctly with an empty and space separator.
    locale.d->setNumericDigitGrouping(QList<int>() << 3);
    QCOMPARE(locale.formatNumber("123456789", true, 0), QString("123,456,789"));
    QCOMPARE(locale.formatNumber("123456789.01"),       QString("123,456,789.01"));
    locale.setThousandsSeparator(QString());
    QCOMPARE(locale.formatNumber("123456789", true, 0), QString("123456789"));
    QCOMPARE(locale.formatNumber("123456789.01"),       QString("123456789.01"));
    locale.setThousandsSeparator(" ");
    QCOMPARE(locale.formatNumber("123456789", true, 0), QString("123 456 789"));
    QCOMPARE(locale.formatNumber("123456789.01"),       QString("123 456 789.01"));
}

void
KLocaleTest::readNumber()
{
	KLocale locale(*KLocale::global());
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

    // Test Grouping
    locale.d->setNumericDigitGrouping(QList<int>());
    QCOMPARE(locale.readNumber(QString("123456789"), &ok), 123456789.0);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString("123456789.01"), &ok), 123456789.01);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString("123,456,789"), &ok), 0.0);
    QVERIFY(!ok);
    QCOMPARE(locale.readNumber(QString("123,456,789.01"), &ok), 0.0);
    QVERIFY(!ok);

    locale.d->setNumericDigitGrouping(QList<int>() << 3 << 2);
    QCOMPARE(locale.readNumber(QString("12,34,56,789"), &ok), 123456789.0);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString("12,34,56,789.01"), &ok), 123456789.01);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString("123,456,789"), &ok), 0.0);
    QVERIFY(!ok);
    QCOMPARE(locale.readNumber(QString("123,456,789.01"), &ok), 0.0);
    QVERIFY(!ok);
    QCOMPARE(locale.readNumber(QString("123456789"), &ok), 123456789.0);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString("123456789.01"), &ok), 123456789.01);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString("123456,789"), &ok), 0.0);
    QVERIFY(!ok);
    QCOMPARE(locale.readNumber(QString("123456,789.01"), &ok), 0.0);
    QVERIFY(!ok);

    //Test it parses correctly with an empty separator.
    locale.d->setNumericDigitGrouping(QList<int>() << 3);
    locale.setThousandsSeparator(QString());
    QCOMPARE(locale.readNumber(QString("123456789"), &ok), 123456789.0);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString("123456789.01"), &ok), 123456789.01);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString("123,456,789"), &ok), 0.0);
    QVERIFY(!ok);
    QCOMPARE(locale.readNumber(QString("123,456,789.01"), &ok), 0.0);
    QVERIFY(!ok);

    //Test it parses correctly with an space separator.
    locale.d->setNumericDigitGrouping(QList<int>() << 3);
    locale.setThousandsSeparator(" ");
    QCOMPARE(locale.readNumber(QString("123 456 789"), &ok), 123456789.0);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString("123 456 789.01"), &ok), 123456789.01);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString("123456789"), &ok), 123456789.0);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString("123456789.01"), &ok), 123456789.01);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString("123456 789"), &ok), 0.0);
    QVERIFY(!ok);
    QCOMPARE(locale.readNumber(QString("123456 789.01"), &ok), 0.0);
    QVERIFY(!ok);
    QCOMPARE(locale.readNumber(QString("123,456,789"), &ok), 0.0);
    QVERIFY(!ok);
    QCOMPARE(locale.readNumber(QString("123,456,789.01"), &ok), 0.0);
    QVERIFY(!ok);

    // Test signs
    locale.setPositiveSign("@");
    locale.setNegativeSign("&");
    locale.setThousandsSeparator(",");
    locale.d->setNumericDigitGrouping(QList<int>() << 3);

    QCOMPARE(locale.readNumber(QString("@123,456,789.12"), &ok), 123456789.12);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString("&123,456,789.12"), &ok), -123456789.12);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString( "123,456,789.12"), &ok), 123456789.12);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString("+123,456,789.12"), &ok), 0.0);
    QVERIFY(!ok);
    QCOMPARE(locale.readNumber(QString("-123,456,789.12"), &ok), 0.0);
    QVERIFY(!ok);
    locale.setNegativeSign(QString());
    QCOMPARE(locale.readNumber(QString( "123,456,789.12"), &ok), -123456789.12);
    QVERIFY(ok);
    QCOMPARE(locale.readNumber(QString("-123,456,789.12"), &ok), 0.0);
    QVERIFY(!ok);
}

void KLocaleTest::formatMoney()
{
    KLocale locale(*KLocale::global());
    locale.setPositiveSign(QString());
    locale.setNegativeSign("-");
    locale.setPositivePrefixCurrencySymbol(true);
    locale.setNegativePrefixCurrencySymbol(true);
    locale.setMonetaryDecimalPlaces(2);
    locale.setMonetaryThousandsSeparator(",");
    locale.setMonetaryDecimalSymbol(".");
    locale.setCurrencySymbol("$");
    locale.setPositiveMonetarySignPosition(KLocale::BeforeQuantityMoney);
    locale.setNegativeMonetarySignPosition(KLocale::BeforeQuantityMoney);
    locale.d->setMonetaryDigitGrouping(QList<int>() << 3);

    // Basic grouping, decimal place and rounding tests
    QCOMPARE(locale.formatMoney(        1), QString(          "$ 1.00"));
    QCOMPARE(locale.formatMoney(       21), QString(         "$ 21.00"));
    QCOMPARE(locale.formatMoney(      321), QString(        "$ 321.00"));
    QCOMPARE(locale.formatMoney(     4321), QString(      "$ 4,321.00"));
    QCOMPARE(locale.formatMoney(    54321), QString(     "$ 54,321.00"));
    QCOMPARE(locale.formatMoney(   654321), QString(    "$ 654,321.00"));
    QCOMPARE(locale.formatMoney(  7654321), QString(  "$ 7,654,321.00"));
    QCOMPARE(locale.formatMoney( 87654321), QString( "$ 87,654,321.00"));
    QCOMPARE(locale.formatMoney(987654321), QString("$ 987,654,321.00"));

    QCOMPARE(locale.formatMoney(        1.1), QString(          "$ 1.10"));
    QCOMPARE(locale.formatMoney(       21.1), QString(         "$ 21.10"));
    QCOMPARE(locale.formatMoney(      321.1), QString(        "$ 321.10"));
    QCOMPARE(locale.formatMoney(     4321.1), QString(      "$ 4,321.10"));
    QCOMPARE(locale.formatMoney(    54321.1), QString(     "$ 54,321.10"));
    QCOMPARE(locale.formatMoney(   654321.1), QString(    "$ 654,321.10"));
    QCOMPARE(locale.formatMoney(  7654321.1), QString(  "$ 7,654,321.10"));
    QCOMPARE(locale.formatMoney( 87654321.1), QString( "$ 87,654,321.10"));
    QCOMPARE(locale.formatMoney(987654321.1), QString("$ 987,654,321.10"));

    QCOMPARE(locale.formatMoney(        1.12), QString(          "$ 1.12"));
    QCOMPARE(locale.formatMoney(       21.12), QString(         "$ 21.12"));
    QCOMPARE(locale.formatMoney(      321.12), QString(        "$ 321.12"));
    QCOMPARE(locale.formatMoney(     4321.12), QString(      "$ 4,321.12"));
    QCOMPARE(locale.formatMoney(    54321.12), QString(     "$ 54,321.12"));
    QCOMPARE(locale.formatMoney(   654321.12), QString(    "$ 654,321.12"));
    QCOMPARE(locale.formatMoney(  7654321.12), QString(  "$ 7,654,321.12"));
    QCOMPARE(locale.formatMoney( 87654321.12), QString( "$ 87,654,321.12"));
    QCOMPARE(locale.formatMoney(987654321.12), QString("$ 987,654,321.12"));

    QCOMPARE(locale.formatMoney(        1.12), QString(          "$ 1.12"));
    QCOMPARE(locale.formatMoney(       21.12), QString(         "$ 21.12"));
    QCOMPARE(locale.formatMoney(      321.12), QString(        "$ 321.12"));
    QCOMPARE(locale.formatMoney(     4321.12), QString(      "$ 4,321.12"));
    QCOMPARE(locale.formatMoney(    54321.12), QString(     "$ 54,321.12"));
    QCOMPARE(locale.formatMoney(   654321.12), QString(    "$ 654,321.12"));
    QCOMPARE(locale.formatMoney(  7654321.12), QString(  "$ 7,654,321.12"));
    QCOMPARE(locale.formatMoney( 87654321.12), QString( "$ 87,654,321.12"));
    QCOMPARE(locale.formatMoney(987654321.12), QString("$ 987,654,321.12"));

    QCOMPARE(locale.formatMoney(        1.123), QString(          "$ 1.12"));
    QCOMPARE(locale.formatMoney(       21.123), QString(         "$ 21.12"));
    QCOMPARE(locale.formatMoney(      321.123), QString(        "$ 321.12"));
    QCOMPARE(locale.formatMoney(     4321.123), QString(      "$ 4,321.12"));
    QCOMPARE(locale.formatMoney(    54321.123), QString(     "$ 54,321.12"));
    QCOMPARE(locale.formatMoney(   654321.123), QString(    "$ 654,321.12"));
    QCOMPARE(locale.formatMoney(  7654321.123), QString(  "$ 7,654,321.12"));
    QCOMPARE(locale.formatMoney( 87654321.123), QString( "$ 87,654,321.12"));
    QCOMPARE(locale.formatMoney(987654321.123), QString("$ 987,654,321.12"));

    QCOMPARE(locale.formatMoney(        1.129), QString(          "$ 1.13"));
    QCOMPARE(locale.formatMoney(       21.129), QString(         "$ 21.13"));
    QCOMPARE(locale.formatMoney(      321.129), QString(        "$ 321.13"));
    QCOMPARE(locale.formatMoney(     4321.129), QString(      "$ 4,321.13"));
    QCOMPARE(locale.formatMoney(    54321.129), QString(     "$ 54,321.13"));
    QCOMPARE(locale.formatMoney(   654321.129), QString(    "$ 654,321.13"));
    QCOMPARE(locale.formatMoney(  7654321.129), QString(  "$ 7,654,321.13"));
    QCOMPARE(locale.formatMoney( 87654321.129), QString( "$ 87,654,321.13"));
    QCOMPARE(locale.formatMoney(987654321.129), QString("$ 987,654,321.13"));

    QCOMPARE(locale.formatMoney(        -1), QString(          "$ -1.00"));
    QCOMPARE(locale.formatMoney(       -21), QString(         "$ -21.00"));
    QCOMPARE(locale.formatMoney(      -321), QString(        "$ -321.00"));
    QCOMPARE(locale.formatMoney(     -4321), QString(      "$ -4,321.00"));
    QCOMPARE(locale.formatMoney(    -54321), QString(     "$ -54,321.00"));
    QCOMPARE(locale.formatMoney(   -654321), QString(    "$ -654,321.00"));
    QCOMPARE(locale.formatMoney(  -7654321), QString(  "$ -7,654,321.00"));
    QCOMPARE(locale.formatMoney( -87654321), QString( "$ -87,654,321.00"));
    QCOMPARE(locale.formatMoney(-987654321), QString("$ -987,654,321.00"));

    QCOMPARE(locale.formatMoney(        -1.1), QString(          "$ -1.10"));
    QCOMPARE(locale.formatMoney(       -21.1), QString(         "$ -21.10"));
    QCOMPARE(locale.formatMoney(      -321.1), QString(        "$ -321.10"));
    QCOMPARE(locale.formatMoney(     -4321.1), QString(      "$ -4,321.10"));
    QCOMPARE(locale.formatMoney(    -54321.1), QString(     "$ -54,321.10"));
    QCOMPARE(locale.formatMoney(   -654321.1), QString(    "$ -654,321.10"));
    QCOMPARE(locale.formatMoney(  -7654321.1), QString(  "$ -7,654,321.10"));
    QCOMPARE(locale.formatMoney( -87654321.1), QString( "$ -87,654,321.10"));
    QCOMPARE(locale.formatMoney(-987654321.1), QString("$ -987,654,321.10"));

    QCOMPARE(locale.formatMoney(        -1.12), QString(          "$ -1.12"));
    QCOMPARE(locale.formatMoney(       -21.12), QString(         "$ -21.12"));
    QCOMPARE(locale.formatMoney(      -321.12), QString(        "$ -321.12"));
    QCOMPARE(locale.formatMoney(     -4321.12), QString(      "$ -4,321.12"));
    QCOMPARE(locale.formatMoney(    -54321.12), QString(     "$ -54,321.12"));
    QCOMPARE(locale.formatMoney(   -654321.12), QString(    "$ -654,321.12"));
    QCOMPARE(locale.formatMoney(  -7654321.12), QString(  "$ -7,654,321.12"));
    QCOMPARE(locale.formatMoney( -87654321.12), QString( "$ -87,654,321.12"));
    QCOMPARE(locale.formatMoney(-987654321.12), QString("$ -987,654,321.12"));

    QCOMPARE(locale.formatMoney(        -1.12), QString(          "$ -1.12"));
    QCOMPARE(locale.formatMoney(       -21.12), QString(         "$ -21.12"));
    QCOMPARE(locale.formatMoney(      -321.12), QString(        "$ -321.12"));
    QCOMPARE(locale.formatMoney(     -4321.12), QString(      "$ -4,321.12"));
    QCOMPARE(locale.formatMoney(    -54321.12), QString(     "$ -54,321.12"));
    QCOMPARE(locale.formatMoney(   -654321.12), QString(    "$ -654,321.12"));
    QCOMPARE(locale.formatMoney(  -7654321.12), QString(  "$ -7,654,321.12"));
    QCOMPARE(locale.formatMoney( -87654321.12), QString( "$ -87,654,321.12"));
    QCOMPARE(locale.formatMoney(-987654321.12), QString("$ -987,654,321.12"));

    QCOMPARE(locale.formatMoney(        -1.123), QString(          "$ -1.12"));
    QCOMPARE(locale.formatMoney(       -21.123), QString(         "$ -21.12"));
    QCOMPARE(locale.formatMoney(      -321.123), QString(        "$ -321.12"));
    QCOMPARE(locale.formatMoney(     -4321.123), QString(      "$ -4,321.12"));
    QCOMPARE(locale.formatMoney(    -54321.123), QString(     "$ -54,321.12"));
    QCOMPARE(locale.formatMoney(   -654321.123), QString(    "$ -654,321.12"));
    QCOMPARE(locale.formatMoney(  -7654321.123), QString(  "$ -7,654,321.12"));
    QCOMPARE(locale.formatMoney( -87654321.123), QString( "$ -87,654,321.12"));
    QCOMPARE(locale.formatMoney(-987654321.123), QString("$ -987,654,321.12"));

    QCOMPARE(locale.formatMoney(        -1.129), QString(          "$ -1.13"));
    QCOMPARE(locale.formatMoney(       -21.129), QString(         "$ -21.13"));
    QCOMPARE(locale.formatMoney(      -321.129), QString(        "$ -321.13"));
    QCOMPARE(locale.formatMoney(     -4321.129), QString(      "$ -4,321.13"));
    QCOMPARE(locale.formatMoney(    -54321.129), QString(     "$ -54,321.13"));
    QCOMPARE(locale.formatMoney(   -654321.129), QString(    "$ -654,321.13"));
    QCOMPARE(locale.formatMoney(  -7654321.129), QString(  "$ -7,654,321.13"));
    QCOMPARE(locale.formatMoney( -87654321.129), QString( "$ -87,654,321.13"));
    QCOMPARE(locale.formatMoney(-987654321.129), QString("$ -987,654,321.13"));

    // Test override defaults
    QCOMPARE(locale.formatMoney(987654321.12, "£",   0), QString(  "£ 987,654,321"));
    QCOMPARE(locale.formatMoney(987654321.12, "USD", 4), QString("USD 987,654,321.1200"));

    // Test Grouping
    locale.d->setMonetaryDigitGrouping(QList<int>());
    QCOMPARE(locale.formatMoney( 987654321.12), QString( "$ 987654321.12"));
    QCOMPARE(locale.formatMoney(-987654321.12), QString("$ -987654321.12"));
    locale.d->setMonetaryDigitGrouping(QList<int>() << 3 << 2);
    QCOMPARE(locale.formatMoney( 987654321.12), QString( "$ 98,76,54,321.12"));
    QCOMPARE(locale.formatMoney(-987654321.12), QString("$ -98,76,54,321.12"));
    locale.d->setMonetaryDigitGrouping(QList<int>() << 3 << -1);
    QCOMPARE(locale.formatMoney( 987654321.12), QString( "$ 987654,321.12"));
    QCOMPARE(locale.formatMoney(-987654321.12), QString("$ -987654,321.12"));

    // Test symbol and sign position options
    locale.setPositiveSign("+");
    locale.setNegativeSign("-");
    locale.d->setMonetaryDigitGrouping(QList<int>() << 3);

    locale.setPositiveMonetarySignPosition(KLocale::ParensAround);
    locale.setNegativeMonetarySignPosition(KLocale::ParensAround);
    locale.setPositivePrefixCurrencySymbol(true);
    locale.setNegativePrefixCurrencySymbol(true);
    QCOMPARE(locale.formatMoney( 987654321.12), QString("$ (987,654,321.12)"));
    QCOMPARE(locale.formatMoney(-987654321.12), QString("$ (987,654,321.12)"));
    locale.setPositivePrefixCurrencySymbol(false);
    locale.setNegativePrefixCurrencySymbol(false);
    QCOMPARE(locale.formatMoney( 987654321.12), QString("(987,654,321.12) $"));
    QCOMPARE(locale.formatMoney(-987654321.12), QString("(987,654,321.12) $"));

    locale.setPositiveMonetarySignPosition(KLocale::BeforeQuantityMoney);
    locale.setNegativeMonetarySignPosition(KLocale::BeforeQuantityMoney);
    locale.setPositivePrefixCurrencySymbol(true);
    locale.setNegativePrefixCurrencySymbol(true);
    QCOMPARE(locale.formatMoney( 987654321.12), QString("$ +987,654,321.12"));
    QCOMPARE(locale.formatMoney(-987654321.12), QString("$ -987,654,321.12"));
    locale.setPositivePrefixCurrencySymbol(false);
    locale.setNegativePrefixCurrencySymbol(false);
    QCOMPARE(locale.formatMoney( 987654321.12), QString("+987,654,321.12 $"));
    QCOMPARE(locale.formatMoney(-987654321.12), QString("-987,654,321.12 $"));

    locale.setPositiveMonetarySignPosition(KLocale::AfterQuantityMoney);
    locale.setNegativeMonetarySignPosition(KLocale::AfterQuantityMoney);
    locale.setPositivePrefixCurrencySymbol(true);
    locale.setNegativePrefixCurrencySymbol(true);
    QCOMPARE(locale.formatMoney( 987654321.12), QString("$ 987,654,321.12+"));
    QCOMPARE(locale.formatMoney(-987654321.12), QString("$ 987,654,321.12-"));
    locale.setPositivePrefixCurrencySymbol(false);
    locale.setNegativePrefixCurrencySymbol(false);
    QCOMPARE(locale.formatMoney( 987654321.12), QString("987,654,321.12+ $"));
    QCOMPARE(locale.formatMoney(-987654321.12), QString("987,654,321.12- $"));

    locale.setPositiveMonetarySignPosition(KLocale::BeforeMoney);
    locale.setNegativeMonetarySignPosition(KLocale::BeforeMoney);
    locale.setPositivePrefixCurrencySymbol(true);
    locale.setNegativePrefixCurrencySymbol(true);
    QCOMPARE(locale.formatMoney( 987654321.12), QString("+$ 987,654,321.12"));
    QCOMPARE(locale.formatMoney(-987654321.12), QString("-$ 987,654,321.12"));
    locale.setPositivePrefixCurrencySymbol(false);
    locale.setNegativePrefixCurrencySymbol(false);
    QCOMPARE(locale.formatMoney( 987654321.12), QString("987,654,321.12 +$"));
    QCOMPARE(locale.formatMoney(-987654321.12), QString("987,654,321.12 -$"));

    locale.setPositiveMonetarySignPosition(KLocale::AfterMoney);
    locale.setNegativeMonetarySignPosition(KLocale::AfterMoney);
    locale.setPositivePrefixCurrencySymbol(true);
    locale.setNegativePrefixCurrencySymbol(true);
    QCOMPARE(locale.formatMoney( 987654321.12), QString("$+ 987,654,321.12"));
    QCOMPARE(locale.formatMoney(-987654321.12), QString("$- 987,654,321.12"));
    locale.setPositivePrefixCurrencySymbol(false);
    locale.setNegativePrefixCurrencySymbol(false);
    QCOMPARE(locale.formatMoney( 987654321.12), QString("987,654,321.12 $+"));
    QCOMPARE(locale.formatMoney(-987654321.12), QString("987,654,321.12 $-"));
}

void KLocaleTest::readMoney()
{
    bool ok = false;
    KLocale locale(*KLocale::global());
    locale.setPositiveSign(QString());
    locale.setNegativeSign("-");
    locale.setPositivePrefixCurrencySymbol(true);
    locale.setNegativePrefixCurrencySymbol(true);
    locale.setMonetaryDecimalPlaces(2);
    locale.setMonetaryThousandsSeparator(",");
    locale.setMonetaryDecimalSymbol(".");
    locale.setCurrencySymbol("$");
    locale.setPositiveMonetarySignPosition(KLocale::BeforeQuantityMoney);
    locale.setNegativeMonetarySignPosition(KLocale::BeforeQuantityMoney);
    locale.d->setMonetaryDigitGrouping(QList<int>() << 3);

    // Basic grouping, decimal place and rounding tests
    QCOMPARE(locale.readMoney(          "$ 1.12", &ok),         1.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(         "$ 21.12", &ok),        21.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(        "$ 321.12", &ok),       321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(      "$ 4,321.12", &ok),      4321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(     "$ 54,321.12", &ok),     54321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(    "$ 654,321.12", &ok),    654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(  "$ 7,654,321.12", &ok),   7654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney( "$ 87,654,321.12", &ok),  87654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("$ 987,654,321.12", &ok), 987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("$ 987654321.12", &ok), 987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("$ 987654,321.12", &ok), 0.0);
    QVERIFY(!ok);

    QCOMPARE(locale.readMoney(          "$ -1.12", &ok),         -1.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(         "$ -21.12", &ok),        -21.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(        "$ -321.12", &ok),       -321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(      "$ -4,321.12", &ok),      -4321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(     "$ -54,321.12", &ok),     -54321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(    "$ -654,321.12", &ok),    -654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(  "$ -7,654,321.12", &ok),   -7654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney( "$ -87,654,321.12", &ok),  -87654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("$ -987,654,321.12", &ok), -987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("$ -987654321.12", &ok), -987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("$ -987654,321.12", &ok), 0.0);
    QVERIFY(!ok);

    // Test incomplete formats
    QCOMPARE(locale.readMoney(          "$ 1", &ok),         1.00);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(         "$ 21", &ok),        21.00);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(        "$ 321", &ok),       321.00);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(      "$ 4,321", &ok),      4321.00);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(     "$ 54,321", &ok),     54321.00);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(    "$ 654,321", &ok),    654321.00);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(  "$ 7,654,321", &ok),   7654321.00);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney( "$ 87,654,321", &ok),  87654321.00);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("$ 987,654,321", &ok), 987654321.00);
    QVERIFY(ok);

    // Test Grouping
    locale.d->setMonetaryDigitGrouping(QList<int>());
    QCOMPARE(locale.readMoney("$ 987654321.12", &ok),  987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("$ -987654321.12", &ok), -987654321.12);
    QVERIFY(ok);
    locale.d->setMonetaryDigitGrouping(QList<int>() << 3 << 2);
    QCOMPARE(locale.readMoney("$ 98,76,54,321.12", &ok),  987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("$ -98,76,54,321.12", &ok), -987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("$ 987654321.12", &ok),  987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("$ -987654321.12", &ok), -987654321.12);
    QVERIFY(ok);
    locale.d->setMonetaryDigitGrouping(QList<int>() << 3 << -1);
    QCOMPARE(locale.readMoney("$ 987654,321.12", &ok),  987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("$ -987654,321.12", &ok), -987654321.12);
    QVERIFY(ok);

    // Test signs
    locale.setPositiveSign("@");
    locale.setNegativeSign("&");
    locale.d->setMonetaryDigitGrouping(QList<int>() << 3);

    QCOMPARE(locale.readMoney(QString("$ @123,456,789.12"),  &ok),  123456789.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(QString( "$ 123,456,789.12@"), &ok),  123456789.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(QString("$ &123,456,789.12"),  &ok), -123456789.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(QString( "$ 123,456,789.12&"), &ok), -123456789.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(QString( "$ 123,456,789.12"),  &ok),  123456789.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(QString("$ +123,456,789.12"),  &ok), 0.0);
    QVERIFY(!ok);
    QCOMPARE(locale.readMoney(QString( "$ 123,456,789.12+"), &ok), 0.0);
    QVERIFY(!ok);
    QCOMPARE(locale.readMoney(QString("$ -123,456,789.12"),  &ok), 0.0);
    QVERIFY(!ok);
    QCOMPARE(locale.readMoney(QString( "$ 123,456,789.12-"), &ok), 0.0);
    QVERIFY(!ok);
    locale.setNegativeSign(QString());
    QCOMPARE(locale.readMoney(QString( "$ 123,456,789.12"),  &ok), -123456789.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney(QString("$ -123,456,789.12"),  &ok), 0.0);
    QVERIFY(!ok);
    QCOMPARE(locale.readMoney(QString( "$ 123,456,789.12-"), &ok), 0.0);
    QVERIFY(!ok);

    // Test symbol and sign position options
    locale.setPositiveSign("+");
    locale.setNegativeSign("-");

    locale.setPositiveMonetarySignPosition(KLocale::ParensAround);
    locale.setNegativeMonetarySignPosition(KLocale::BeforeQuantityMoney);
    locale.setPositivePrefixCurrencySymbol(true);
    QCOMPARE(locale.readMoney("$ (987,654,321.12)", &ok),  987654321.12);
    QVERIFY(ok);
    locale.setPositivePrefixCurrencySymbol(false);
    QCOMPARE(locale.readMoney("(987,654,321.12) $", &ok),  987654321.12);
    QVERIFY(ok);

    locale.setPositiveMonetarySignPosition(KLocale::BeforeQuantityMoney);
    locale.setNegativeMonetarySignPosition(KLocale::ParensAround);
    locale.setNegativePrefixCurrencySymbol(true);
    QCOMPARE(locale.readMoney("$ (987,654,321.12)", &ok), -987654321.12);
    QVERIFY(ok);
    locale.setNegativePrefixCurrencySymbol(false);
    QCOMPARE(locale.readMoney("(987,654,321.12) $", &ok), -987654321.12);
    QVERIFY(ok);

    locale.setPositiveMonetarySignPosition(KLocale::BeforeQuantityMoney);
    locale.setNegativeMonetarySignPosition(KLocale::BeforeQuantityMoney);
    locale.setPositivePrefixCurrencySymbol(true);
    locale.setNegativePrefixCurrencySymbol(true);
    QCOMPARE(locale.readMoney("$ +987,654,321.12", &ok),  987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("$ -987,654,321.12", &ok), -987654321.12);
    QVERIFY(ok);
    locale.setPositivePrefixCurrencySymbol(false);
    locale.setNegativePrefixCurrencySymbol(false);
    QCOMPARE(locale.readMoney("+987,654,321.12 $", &ok),  987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("-987,654,321.12 $", &ok), -987654321.12);
    QVERIFY(ok);

    locale.setPositiveMonetarySignPosition(KLocale::AfterQuantityMoney);
    locale.setNegativeMonetarySignPosition(KLocale::AfterQuantityMoney);
    locale.setPositivePrefixCurrencySymbol(true);
    locale.setNegativePrefixCurrencySymbol(true);
    QCOMPARE(locale.readMoney("$ 987,654,321.12+", &ok),  987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("$ 987,654,321.12-", &ok), -987654321.12);
    QVERIFY(ok);
    locale.setPositivePrefixCurrencySymbol(false);
    locale.setNegativePrefixCurrencySymbol(false);
    QCOMPARE(locale.readMoney("987,654,321.12+ $", &ok),  987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("987,654,321.12- $", &ok), -987654321.12);
    QVERIFY(ok);

    locale.setPositiveMonetarySignPosition(KLocale::BeforeMoney);
    locale.setNegativeMonetarySignPosition(KLocale::BeforeMoney);
    locale.setPositivePrefixCurrencySymbol(true);
    locale.setNegativePrefixCurrencySymbol(true);
    QCOMPARE(locale.readMoney("+$ 987,654,321.12", &ok),  987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("-$ 987,654,321.12", &ok), -987654321.12);
    QVERIFY(ok);
    locale.setPositivePrefixCurrencySymbol(false);
    locale.setNegativePrefixCurrencySymbol(false);
    QCOMPARE(locale.readMoney("987,654,321.12 +$", &ok),  987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("987,654,321.12 -$", &ok), -987654321.12);
    QVERIFY(ok);

    locale.setPositiveMonetarySignPosition(KLocale::AfterMoney);
    locale.setNegativeMonetarySignPosition(KLocale::AfterMoney);
    locale.setPositivePrefixCurrencySymbol(true);
    locale.setNegativePrefixCurrencySymbol(true);
    QCOMPARE(locale.readMoney("$+ 987,654,321.12", &ok),  987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("$- 987,654,321.12", &ok), -987654321.12);
    QVERIFY(ok);
    locale.setPositivePrefixCurrencySymbol(false);
    locale.setNegativePrefixCurrencySymbol(false);
    QCOMPARE(locale.readMoney("987,654,321.12 $+", &ok),  987654321.12);
    QVERIFY(ok);
    QCOMPARE(locale.readMoney("987,654,321.12 $-", &ok), -987654321.12);
    QVERIFY(ok);
}

void
KLocaleTest::formatDate()
{
	KLocale locale(*KLocale::global());
	QString small("yyyy-MM-dd");
	QString full("dddd dd MMMM yyyy");
	QDate date;

        // Ensure that user configuration isn't messing with us;
        // shouldn't happen though, due to QStandardPaths::enableTestMode(true)
        QCOMPARE(locale.dateFormat(), QString("%A %d %B %Y"));

	date.setDate(2002, 5, 3);
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
	KLocale locale(*KLocale::global());
	QString small("yyyy-MM-dd");
	QString full("dddd dd MMMM yyyy");
	bool ok=false;
	QDate date;
	date.setDate( 2002, 5, 3 );
	QCOMPARE(locale.readDate(date.toString(small)), date);
	QCOMPARE(locale.readDate(date.toString(full)), date);
	locale.readDate(date.toString(small), KLocale::NormalFormat, &ok);
	QVERIFY(!ok);
	locale.readDate(date.toString(full), KLocale::ShortFormat, &ok);
	QVERIFY(!ok);

        QString twoDigitYear("dd-MM-yy");
        QCOMPARE(date.toString(twoDigitYear), QString("03-05-02"));
        QCOMPARE(locale.readDate("03-05-02", "%d-%m-%y"), date);

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
    KLocale locale(*KLocale::global());
    QTime time(0,22,33);

    locale.setTimeFormat("%H:%M %p");
    QCOMPARE(locale.formatLocaleTime(time, KLocale::TimeWithoutSeconds), QString("00:22 AM"));
    QCOMPARE(locale.formatLocaleTime(time, KLocale::TimeWithoutSeconds | KLocale::TimeDuration),
             QString("00:22"));

    locale.setTimeFormat("%H:%M:%S %p");
    QCOMPARE(locale.formatLocaleTime(time, KLocale::TimeDefault), QString("00:22:33 AM"));
    QCOMPARE(locale.formatLocaleTime(time, KLocale::TimeDuration), QString("00:22:33"));

    locale.setTimeFormat("%l : %M : %S %p"); // #164813
    QCOMPARE(locale.formatLocaleTime(time, KLocale::TimeDefault), QString("12 : 22 : 33 AM"));
    QCOMPARE(locale.formatLocaleTime(time, KLocale::TimeWithoutSeconds), QString("12 : 22 AM"));
}

void
KLocaleTest::formatDateTime()
{
	KLocale locale(*KLocale::global());
	QString small("yyyy-MM-dd hh:mm");
	QString smallsecs("yyyy-MM-dd hh:mm:ss");
	QString full("dddd dd MMMM yyyy hh:mm");
	QString fullsecs("dddd dd MMMM yyyy hh:mm:ss");
	QString tfmt(" hh:mm");
	QDateTime qdt;

        // Ensure that user configuration isn't messing with us;
        // shouldn't happen though, due to QStandardPaths::enableTestMode(true)
        QCOMPARE(locale.dateFormat(), QString("%A %d %B %Y"));

	qdt = QDateTime(QDate(2002, 5, 3), QTime(10, 20, 30));
	QCOMPARE(locale.formatDateTime(qdt), qdt.toString(small));
	QCOMPARE(locale.formatDateTime(qdt, KLocale::LongDate), qdt.toString(full));

	qdt = QDateTime::currentDateTime();
	QDate today = qdt.date();
	QTime nowt = qdt.time();
	QCOMPARE(locale.formatDateTime(qdt), qdt.toString(small));
	QCOMPARE(locale.formatDateTime(qdt, KLocale::LongDate), qdt.toString(full));

	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), QString("Today" + qdt.time().toString(tfmt)));
	qdt = qdt.addSecs(3605);  // more than 1 hour from now
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), qdt.toString(full));
	qdt.setDate(today);
	qdt.setTime(QTime(0,0,0));
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), QString("Today" + qdt.time().toString(tfmt)));
	qdt = qdt.addSecs(-1);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), QString("Yesterday" + qdt.time().toString(tfmt)));
	qdt = qdt.addDays(2);   // tomorrow
	qdt.setTime(nowt);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), qdt.toString(full));
	qdt = qdt.addDays(-2);   // yesterday
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), QString("Yesterday" + qdt.time().toString(tfmt)));
	qdt = qdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), QString(locale.calendar()->weekDayName(qdt.date()) + qdt.time().toString(tfmt)));
	qdt = qdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), QString(locale.calendar()->weekDayName(qdt.date()) + qdt.time().toString(tfmt)));
	qdt = qdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), QString(locale.calendar()->weekDayName(qdt.date()) + qdt.time().toString(tfmt)));
	qdt = qdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), QString(locale.calendar()->weekDayName(qdt.date()) + qdt.time().toString(tfmt)));
	qdt = qdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), QString(locale.calendar()->weekDayName(qdt.date()) + qdt.time().toString(tfmt)));
	qdt = qdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(qdt, KLocale::FancyLongDate), qdt.toString(full));

	// The use of KSystemTimeZones requires kded to be running
	if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kded5")) {
	    QSKIP( "kded not running" );
	}

	small = "%Y-%m-%d %H:%M";
	smallsecs = "%Y-%m-%d %H:%M:%S";
	full = "%A %d %B %Y %H:%M";
	fullsecs = "%A %d %B %Y %H:%M:%S";
	KDateTime kdt;
	const KTimeZone tz = KSystemTimeZones::zone("Pacific/Fiji");
        if (!tz.isValid())
            QSKIP( "Pacific/Fiji timezone not available" );
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
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), QString("Today" + kdt.time().toString(tfmt)));
	kdt = kdt.addSecs(-1);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), QString("Yesterday" + kdt.time().toString(tfmt)));
	kdt = kdt.addDays(2);   // tomorrow
	kdt.setTime(nowt);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), kdt.toString(full));
	kdt = kdt.addDays(-2);   // yesterday
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), QString("Yesterday" + kdt.time().toString(tfmt)));
	kdt = kdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), QString(locale.calendar()->weekDayName(kdt.date()) + kdt.time().toString(tfmt)));
	kdt = kdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), QString(locale.calendar()->weekDayName(kdt.date()) + kdt.time().toString(tfmt)));
	kdt = kdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), QString(locale.calendar()->weekDayName(kdt.date()) + kdt.time().toString(tfmt)));
	kdt = kdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), QString(locale.calendar()->weekDayName(kdt.date()) + kdt.time().toString(tfmt)));
	kdt = kdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), QString(locale.calendar()->weekDayName(kdt.date()) + kdt.time().toString(tfmt)));
	kdt = kdt.addDays(-1);
	QCOMPARE(locale.formatDateTime(kdt, KLocale::FancyLongDate), kdt.toString(full));
}

void
KLocaleTest::readTime()
{
    KLocale locale(*KLocale::global());
    bool ok = false;

    QCOMPARE(locale.readTime("11:22:33", &ok), QTime(11,22,33));
    QVERIFY(ok);

    QCOMPARE(locale.readTime("11:22", &ok), QTime(11,22,0));
    QVERIFY(ok);

    locale.readLocaleTime("11:22:33", &ok, KLocale::TimeWithoutSeconds);
    QVERIFY(!ok);

    QCOMPARE(locale.readLocaleTime("11:22", &ok, KLocale::TimeWithoutSeconds), QTime(11, 22, 0));
    QVERIFY(ok);

    // strict processing of a time string with a missing space
    locale.setTimeFormat("%I:%M:%S %p");
    QString timeString = QString("11:13:55%1").arg(locale.d->dayPeriodForTime(QTime(11,13,55)).periodName());
    locale.readLocaleTime(timeString, &ok, KLocale::TimeDefault, KLocale::ProcessStrict);
    QVERIFY(!ok);

    // lax processing of the same time
    QCOMPARE(locale.readLocaleTime(timeString, &ok, KLocale::TimeDefault), QTime(11, 13, 55));
    QVERIFY(ok);
}

void KLocaleTest::testDayPeriods()
{
    KLocale locale(*KLocale::global());
    locale.setTimeFormat("%I:%M:%S %p");

    //Test default standard AM/PM Day Periods
    KDayPeriod testPeriod;
    QCOMPARE( testPeriod.isValid(), false );
    testPeriod = locale.d->dayPeriodForTime( QTime( 03, 00, 00 ) );
    QCOMPARE( testPeriod.isValid(), true );
    QCOMPARE( testPeriod.periodName(KLocale::LongName), QString( "Ante Meridiem" ) );
    QCOMPARE( testPeriod.periodName(KLocale::ShortName), QString( "AM" ) );
    QCOMPARE( testPeriod.periodName(KLocale::NarrowName), QString( "A" ) );
    QCOMPARE( testPeriod.periodStart(), QTime( 0, 0, 0 ) );
    QCOMPARE( testPeriod.periodEnd(), QTime( 11, 59, 59, 999 ) );
    QCOMPARE( testPeriod.hourInPeriod( QTime(  0, 0, 0 ) ), 12 );
    QCOMPARE( testPeriod.hourInPeriod( QTime(  1, 0, 0 ) ),  1 );
    QCOMPARE( testPeriod.hourInPeriod( QTime(  6, 0, 0 ) ),  6 );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 11, 59, 59, 999) ), 11 );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 12, 0, 0 ) ), -1 );
    QCOMPARE( testPeriod.time( 12, 0, 0 ), QTime( 0, 0, 0 ) );
    QCOMPARE( testPeriod.time(  1, 0, 0 ), QTime( 1, 0, 0 ) );
    QCOMPARE( testPeriod.time(  6, 0, 0 ), QTime( 6, 0, 0 ) );
    QCOMPARE( testPeriod.time( 11, 59, 59, 999 ), QTime( 11, 59, 59, 999 ) );
    QCOMPARE( testPeriod.time( 13, 0, 0 ), QTime() );

    testPeriod = locale.d->dayPeriodForTime( QTime( 13, 00, 00 ) );
    QCOMPARE( testPeriod.isValid(), true );
    QCOMPARE( testPeriod.periodName(KLocale::LongName), QString( "Post Meridiem" ) );
    QCOMPARE( testPeriod.periodName(KLocale::ShortName), QString( "PM" ) );
    QCOMPARE( testPeriod.periodName(KLocale::NarrowName), QString( "P" ) );
    QCOMPARE( testPeriod.periodStart(), QTime( 12, 0, 0 ) );
    QCOMPARE( testPeriod.periodEnd(), QTime( 23, 59, 59, 999 ) );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 11, 59, 59, 999) ), -1 );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 12, 0, 0 ) ), 12 );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 13, 0, 0 ) ),  1 );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 18, 0, 0 ) ),  6 );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 23, 59, 59, 999) ), 11 );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 24, 0, 0 ) ), -1 );
    QCOMPARE( testPeriod.time( 12, 0, 0 ), QTime( 12, 0, 0 ) );
    QCOMPARE( testPeriod.time(  1, 0, 0 ), QTime( 13, 0, 0 ) );
    QCOMPARE( testPeriod.time(  6, 0, 0 ), QTime( 18, 0, 0 ) );
    QCOMPARE( testPeriod.time( 11, 59, 59, 999 ), QTime( 23, 59, 59, 999 ) );
    QCOMPARE( testPeriod.time( 13, 0, 0 ), QTime() );

    QCOMPARE( locale.dayPeriodText( QTime( 11, 12, 13 ) ), QString( "AM" ) );
    QCOMPARE( locale.dayPeriodText( QTime( 23, 12, 13 ) ), QString( "PM" ) );
    QCOMPARE( locale.formatLocaleTime( QTime( 11, 12, 13 ) ), QString( "11:12:13 AM" ) );
    QCOMPARE( locale.formatLocaleTime( QTime( 23, 12, 13 ) ), QString( "11:12:13 PM" ) );
    QCOMPARE( locale.readLocaleTime( QString( "11:12:13 AM" ) ), QTime( 11, 12, 13 ) );
    QCOMPARE( locale.readLocaleTime( QString( "11:12:13 PM" ) ), QTime( 23, 12, 13 ) );
    QCOMPARE( locale.readLocaleTime( QString( "11:12:13 am" ) ), QTime( 11, 12, 13 ) );
    QCOMPARE( locale.readLocaleTime( QString( "11:12:13 pm" ) ), QTime( 23, 12, 13 ) );

    // Test setting own periods
    // Test alternative periods, e.g. thirds of day with absolute hours in each period numbered 1..8
    QList<KDayPeriod> dayPeriods;
    dayPeriods.append( KDayPeriod( "p1", "Early Day", "Early", "E", QTime( 0, 0, 0 ), QTime( 7, 59, 59, 999 ), 1, 0 ) );
    dayPeriods.append( KDayPeriod( "p2", "Middle Day", "Middle", "M", QTime( 8, 0, 0 ), QTime( 15, 59, 59, 999 ), 1, 0 ) );
    dayPeriods.append( KDayPeriod( "p3", "Late Day", "Late", "L", QTime( 16, 0, 0 ), QTime( 23, 59, 59, 999 ), 1, 0 ) );
    locale.d->setDayPeriods( dayPeriods );
    testPeriod = locale.d->dayPeriodForTime( QTime( 0, 0, 0 ) );
    QCOMPARE( testPeriod.periodName(KLocale::ShortName), QString( "Early" ) );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 0, 0, 0 ) ), 1 );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 4, 0, 0 ) ), 5 );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 7, 0, 0 ) ), 8 );
    QCOMPARE( testPeriod.time( 0, 0, 0 ), QTime() );
    QCOMPARE( testPeriod.time( 1, 0, 0 ), QTime( 0, 0, 0 ) );
    QCOMPARE( testPeriod.time( 5, 0, 0 ), QTime( 4, 0, 0 ) );
    QCOMPARE( testPeriod.time( 8, 0, 0 ), QTime( 7, 0, 0 ) );
    QCOMPARE( testPeriod.time( 9, 0, 0 ), QTime() );
    testPeriod = locale.d->dayPeriodForTime( QTime( 8, 0, 0 ) );
    QCOMPARE( testPeriod.periodName(KLocale::ShortName), QString( "Middle" ) );
    QCOMPARE( testPeriod.hourInPeriod( QTime(  8, 0, 0 ) ), 1 );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 12, 0, 0 ) ), 5 );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 15, 0, 0 ) ), 8 );
    QCOMPARE( testPeriod.time( 0, 0, 0 ), QTime() );
    QCOMPARE( testPeriod.time( 1, 0, 0 ), QTime(  8, 0, 0 ) );
    QCOMPARE( testPeriod.time( 5, 0, 0 ), QTime( 12, 0, 0 ) );
    QCOMPARE( testPeriod.time( 8, 0, 0 ), QTime( 15, 0, 0 ) );
    QCOMPARE( testPeriod.time( 9, 0, 0 ), QTime() );
    testPeriod = locale.d->dayPeriodForTime( QTime( 16, 0, 0 ) );
    QCOMPARE( testPeriod.periodName(KLocale::ShortName), QString( "Late" ) );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 16, 0, 0 ) ), 1 );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 20, 0, 0 ) ), 5 );
    QCOMPARE( testPeriod.hourInPeriod( QTime( 23, 0, 0 ) ), 8 );
    QCOMPARE( testPeriod.time( 0, 0, 0 ), QTime() );
    QCOMPARE( testPeriod.time( 1, 0, 0 ), QTime( 16, 0, 0 ) );
    QCOMPARE( testPeriod.time( 5, 0, 0 ), QTime( 20, 0, 0 ) );
    QCOMPARE( testPeriod.time( 8, 0, 0 ), QTime( 23, 0, 0 ) );
    QCOMPARE( testPeriod.time( 9, 0, 0 ), QTime() );
    QCOMPARE( locale.formatLocaleTime( QTime(  4, 0, 0 ) ), QString( "05:00:00 Early" ) );
    QCOMPARE( locale.formatLocaleTime( QTime( 12, 0, 0 ) ), QString( "05:00:00 Middle" ) );
    QCOMPARE( locale.formatLocaleTime( QTime( 20, 0, 0 ) ), QString( "05:00:00 Late" ) );
    QCOMPARE( locale.readLocaleTime( QString( "05:00:00 Early" ) ), QTime( 4, 0, 0 ) );
    QCOMPARE( locale.readLocaleTime( QString( "05:00:00 Middle" ) ), QTime( 12, 0, 0 ) );
    QCOMPARE( locale.readLocaleTime( QString( "05:00:00 Late" ) ), QTime( 20, 0, 0 ) );

    // Test more conventional alternative periods, e.g. informal periods of day with 12 hour clock
    dayPeriods.clear();
    dayPeriods.append( KDayPeriod( "morning", "Morning", "in the morning", "M", QTime( 6, 0, 0 ), QTime( 11, 59, 59, 999 ), 6, 12 ) );
    dayPeriods.append( KDayPeriod( "noon", "Noon", "noon", "O", QTime( 12, 0, 0 ), QTime( 12, 0, 59, 999 ), 12, 0 ) );
    dayPeriods.append( KDayPeriod( "afternoon", "Afternoon", "in the afternoon", "A", QTime( 12, 1, 0 ), QTime( 17, 59, 59, 999 ), 0, 12 ) );
    dayPeriods.append( KDayPeriod( "evening", "Evening", "in the evening", "E", QTime( 18, 0, 0 ), QTime( 21, 59, 59, 999 ), 6, 12 ) );
    dayPeriods.append( KDayPeriod( "night", "Night", "at night", "N", QTime( 22, 0, 0 ), QTime( 5, 59, 59, 999 ), 10, 12 ) );
    locale.d->setDayPeriods( dayPeriods );
    testPeriod = locale.d->dayPeriodForTime( QTime( 8, 0, 0 ) );
    QCOMPARE( testPeriod.periodName( KLocale::ShortName ), QString( "in the morning" ) );
    QCOMPARE( testPeriod.hourInPeriod( QTime(  8, 0, 0 ) ), 8 );
    QCOMPARE( testPeriod.time( 8, 0, 0 ), QTime(  8, 0, 0 ) );
    testPeriod = locale.d->dayPeriodForTime( QTime( 12, 0, 0 ) );
    QCOMPARE( testPeriod.periodName( KLocale::ShortName ), QString( "noon" ) );
    QCOMPARE( testPeriod.hourInPeriod( QTime(  12, 0, 0 ) ), 12 );
    QCOMPARE( testPeriod.time( 12, 0, 0 ), QTime( 12, 0, 0 ) );
    testPeriod = locale.d->dayPeriodForTime( QTime( 12, 1, 0 ) );
    QCOMPARE( testPeriod.periodName( KLocale::ShortName ), QString( "in the afternoon" ) );
    QCOMPARE( testPeriod.hourInPeriod( QTime(  12, 1, 0 ) ), 12 );
    QCOMPARE( testPeriod.time( 12, 1, 0 ), QTime( 12, 1, 0 ) );
    testPeriod = locale.d->dayPeriodForTime( QTime( 23, 0, 0 ) );
    QCOMPARE( testPeriod.periodName( KLocale::ShortName ), QString( "at night" ) );
    QCOMPARE( testPeriod.hourInPeriod( QTime(  23, 0, 0 ) ), 11 );
    QCOMPARE( testPeriod.hourInPeriod( QTime(  0, 0, 0 ) ), 12 );
    QCOMPARE( testPeriod.hourInPeriod( QTime(  1, 0, 0 ) ), 1 );
    QCOMPARE( testPeriod.time( 11, 0, 0 ), QTime( 23, 0, 0 ) );
    QCOMPARE( testPeriod.time( 12, 0, 0 ), QTime( 0, 0, 0 ) );
    QCOMPARE( testPeriod.time(  1, 0, 0 ), QTime( 1, 0, 0 ) );
    testPeriod = locale.d->dayPeriodForTime( QTime( 0, 0, 0 ) );
    QCOMPARE( testPeriod.periodName( KLocale::ShortName ), QString( "at night" ) );
    QCOMPARE( locale.readLocaleTime( QString( "8:00:00 in the morning" ) ), QTime(  8, 0, 0 ) );
    QCOMPARE( locale.readLocaleTime( QString( "12:00:00 noon" ) ), QTime( 12, 0, 0 ) );
    QCOMPARE( locale.readLocaleTime( QString( "5:00:00 in the afternoon" ) ), QTime( 17, 0, 0 ) );
    QCOMPARE( locale.readLocaleTime( QString( "11:00:00 at night" ) ), QTime( 23, 0, 0 ) );
    QCOMPARE( locale.readLocaleTime( QString( "1:00:00 at night" ) ), QTime(  1, 0, 0 ) );
    locale.setTimeFormat("%l %p");
    QCOMPARE( locale.formatLocaleTime( QTime(  8, 0, 0 ) ), QString( "8 in the morning" ) );
    QCOMPARE( locale.formatLocaleTime( QTime( 12, 0, 0 ) ), QString( "12 noon" ) );
    QCOMPARE( locale.formatLocaleTime( QTime( 17, 0, 0 ) ), QString( "5 in the afternoon" ) );
    QCOMPARE( locale.formatLocaleTime( QTime( 23, 0, 0 ) ), QString( "11 at night" ) );
    QCOMPARE( locale.formatLocaleTime( QTime(  1, 0, 0 ) ), QString( "1 at night" ) );

    // Test load config
    KConfig *testConfig = new KConfig( QString(), KConfig::SimpleConfig );
    KConfigGroup testGroup( testConfig, "Locale" );
    testGroup.writeEntry( "DayPeriod1",
                          QStringList() << "third1" << "First Third" << "T1" << "T" <<
                          QTime( 0, 0, 0 ).toString("HH:mm:ss.zzz") << QTime( 7, 59, 59, 999 ).toString("HH:mm:ss.zzz") <<
                          QString::number(0) << QString::number(12) );
    testGroup.writeEntry( "DayPeriod2",
                          QStringList() << "third2" << "Second Third" << "T2" << "S" <<
                          QTime( 8, 0, 0 ).toString("HH:mm:ss.zzz") << QTime( 15, 59, 59, 999 ).toString("HH:mm:ss.zzz") <<
                          QString::number(8) << QString::number(12) );
    testGroup.writeEntry( "DayPeriod3",
                          QStringList() << "third3" << "First Third" << "T3" << "R" <<
                          QTime( 16, 0, 0 ).toString("HH:mm:ss.zzz") << QTime( 23, 59, 59, 999 ).toString("HH:mm:ss.zzz") <<
                          QString::number(4) << QString::number(12) );
    locale = KLocale("en_us", "us", testConfig);
    QCOMPARE( locale.d->dayPeriodForTime( QTime( 1, 0, 0 ) ).periodName( KLocale::ShortName ), QString( "T1" ) );
    QCOMPARE( locale.d->dayPeriodForTime( QTime( 11, 0, 0 ) ).periodName( KLocale::ShortName ), QString( "T2" ) );
    QCOMPARE( locale.d->dayPeriodForTime( QTime( 21, 0, 0 ) ).periodName( KLocale::ShortName ), QString( "T3" ) );
}

void KLocaleTest::testCalendarSystemType()
{
    KLocale locale(*KLocale::global());

    locale.setCalendar("gregorian");
    QCOMPARE( locale.calendarSystem(), KLocale::GregorianCalendar);
    QCOMPARE( locale.calendarType(),   QString("gregorian") );

    locale.setCalendar("coptic");
    QCOMPARE( locale.calendarSystem(), KLocale::CopticCalendar);
    QCOMPARE( locale.calendarType(),   QString("coptic") );

    locale.setCalendar("ethiopian");
    QCOMPARE( locale.calendarSystem(), KLocale::EthiopianCalendar);
    QCOMPARE( locale.calendarType(),   QString("ethiopian") );

    locale.setCalendar("hebrew");
    QCOMPARE( locale.calendarSystem(), KLocale::HebrewCalendar);
    QCOMPARE( locale.calendarType(),   QString("hebrew") );

    locale.setCalendar("hijri");
    QCOMPARE( locale.calendarSystem(), KLocale::IslamicCivilCalendar);
    QCOMPARE( locale.calendarType(),   QString("hijri") );

    locale.setCalendar("indian-national");
    QCOMPARE( locale.calendarSystem(), KLocale::IndianNationalCalendar);
    QCOMPARE( locale.calendarType(),   QString("indian-national") );

    locale.setCalendar("jalali");
    QCOMPARE( locale.calendarSystem(), KLocale::JalaliCalendar);
    QCOMPARE( locale.calendarType(),   QString("jalali") );

    locale.setCalendar("japanese");
    QCOMPARE( locale.calendarSystem(), KLocale::JapaneseCalendar);
    QCOMPARE( locale.calendarType(),   QString("japanese") );

    locale.setCalendar("julian");
    QCOMPARE( locale.calendarSystem(), KLocale::JulianCalendar);
    QCOMPARE( locale.calendarType(),   QString("julian") );

    locale.setCalendar("minguo");
    QCOMPARE( locale.calendarSystem(), KLocale::MinguoCalendar);
    QCOMPARE( locale.calendarType(),   QString("minguo") );

    locale.setCalendar("thai");
    QCOMPARE( locale.calendarSystem(), KLocale::ThaiCalendar);
    QCOMPARE( locale.calendarType(),   QString("thai") );

    locale.setCalendarSystem(KLocale::GregorianCalendar);
    QCOMPARE( locale.calendarSystem(), KLocale::GregorianCalendar);
    QCOMPARE( locale.calendarType(),   QString("gregorian") );

    locale.setCalendarSystem(KLocale::CopticCalendar);
    QCOMPARE( locale.calendarSystem(), KLocale::CopticCalendar);
    QCOMPARE( locale.calendarType(),   QString("coptic") );

    locale.setCalendarSystem(KLocale::EthiopianCalendar);
    QCOMPARE( locale.calendarSystem(), KLocale::EthiopianCalendar);
    QCOMPARE( locale.calendarType(),   QString("ethiopian") );

    locale.setCalendarSystem(KLocale::HebrewCalendar);
    QCOMPARE( locale.calendarSystem(), KLocale::HebrewCalendar);
    QCOMPARE( locale.calendarType(),   QString("hebrew") );

    locale.setCalendarSystem(KLocale::IslamicCivilCalendar);
    QCOMPARE( locale.calendarSystem(), KLocale::IslamicCivilCalendar);
    QCOMPARE( locale.calendarType(),   QString("hijri") );

    locale.setCalendarSystem(KLocale::IndianNationalCalendar);
    QCOMPARE( locale.calendarSystem(), KLocale::IndianNationalCalendar);
    QCOMPARE( locale.calendarType(),   QString("indian-national") );

    locale.setCalendarSystem(KLocale::JalaliCalendar);
    QCOMPARE( locale.calendarSystem(), KLocale::JalaliCalendar);
    QCOMPARE( locale.calendarType(),   QString("jalali") );

    locale.setCalendarSystem(KLocale::JapaneseCalendar);
    QCOMPARE( locale.calendarSystem(), KLocale::JapaneseCalendar);
    QCOMPARE( locale.calendarType(),   QString("japanese") );

    locale.setCalendarSystem(KLocale::JulianCalendar);
    QCOMPARE( locale.calendarSystem(), KLocale::JulianCalendar);
    QCOMPARE( locale.calendarType(),   QString("julian") );

    locale.setCalendarSystem(KLocale::MinguoCalendar);
    QCOMPARE( locale.calendarSystem(), KLocale::MinguoCalendar);
    QCOMPARE( locale.calendarType(),   QString("minguo") );

    locale.setCalendarSystem(KLocale::ThaiCalendar);
    QCOMPARE( locale.calendarSystem(), KLocale::ThaiCalendar);
    QCOMPARE( locale.calendarType(),   QString("thai") );

    locale.setCalendarSystem(KLocale::QDateCalendar);
    QCOMPARE( locale.calendarSystem(), KLocale::GregorianCalendar);
    QCOMPARE( locale.calendarType(),   QString("gregorian") );
}

void KLocaleTest::testCountryDivision()
{
    KLocale locale(*KLocale::global());
    QCOMPARE( locale.countryDivisionCode(), locale.country().toUpper() );
    QVERIFY( locale.setCountryDivisionCode( "US-NY" ) );
    QCOMPARE( locale.countryDivisionCode(), QString( "US-NY" ) );
}

void
KLocaleTest::prettyFormatDuration()
{
	KLocale locale(*KLocale::global());

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
KLocaleTest::formatByteSize()
{
    KLocale locale(*KLocale::global());

    QCOMPARE(locale.formatByteSize(0), QString("0 B"));
    QCOMPARE(locale.formatByteSize(50), QString("50 B"));
    QCOMPARE(locale.formatByteSize(500), QString("500 B"));
    QCOMPARE(locale.formatByteSize(5000), QString("4.9 KiB"));
    QCOMPARE(locale.formatByteSize(50000), QString("48.8 KiB"));
    QCOMPARE(locale.formatByteSize(500000), QString("488.3 KiB"));
    QCOMPARE(locale.formatByteSize(5000000), QString("4.8 MiB"));
    QCOMPARE(locale.formatByteSize(50000000), QString("47.7 MiB"));
    QCOMPARE(locale.formatByteSize(500000000), QString("476.8 MiB"));
#if (__WORDSIZE == 64) || defined (_LP64) || defined(__LP64__) || defined(__ILP64__)
    QCOMPARE(locale.formatByteSize(5000000000), QString("4.7 GiB"));
    QCOMPARE(locale.formatByteSize(50000000000), QString("46.6 GiB"));
    QCOMPARE(locale.formatByteSize(500000000000), QString("465.7 GiB"));
    QCOMPARE(locale.formatByteSize(5000000000000), QString("4.5 TiB"));
    QCOMPARE(locale.formatByteSize(50000000000000), QString("45.5 TiB"));
    QCOMPARE(locale.formatByteSize(500000000000000), QString("454.7 TiB"));
#endif
}

void
KLocaleTest::bug95511()
{
	KLocale locale(*KLocale::global());
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
    KLocale locale(*KLocale::global());

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

void
KLocaleTest::formatByteSize2()
{
    KLocale locale(*KLocale::global());

    // IEC is also default
    locale.setBinaryUnitDialect(KLocale::IECBinaryDialect);
    QCOMPARE(locale.binaryUnitDialect(), KLocale::IECBinaryDialect);

    QCOMPARE(locale.formatByteSize(1024.0), QString("1.0 KiB"));
    QCOMPARE(locale.formatByteSize(1023.0), QString("1,023 B"));
    QCOMPARE(locale.formatByteSize(1163000.0), QString("1.1 MiB")); // 1.2 metric

    QCOMPARE(locale.formatByteSize(-1024.0), QString("-1.0 KiB"));
    QCOMPARE(locale.formatByteSize(-1023.0), QString("-1,023 B"));
    QCOMPARE(locale.formatByteSize(-1163000.0), QString("-1.1 MiB")); // 1.2 metric

    locale.setBinaryUnitDialect(KLocale::JEDECBinaryDialect);
    QCOMPARE(locale.binaryUnitDialect(), KLocale::JEDECBinaryDialect);

    QCOMPARE(locale.formatByteSize(1024.0), QString("1.0 KB"));
    QCOMPARE(locale.formatByteSize(1023.0), QString("1,023 B"));
    QCOMPARE(locale.formatByteSize(1163000.0), QString("1.1 MB"));

    QCOMPARE(locale.formatByteSize(-1024.0), QString("-1.0 KB"));
    QCOMPARE(locale.formatByteSize(-1023.0), QString("-1,023 B"));
    QCOMPARE(locale.formatByteSize(-1163000.0), QString("-1.1 MB"));

    locale.setBinaryUnitDialect(KLocale::MetricBinaryDialect);
    QCOMPARE(locale.binaryUnitDialect(), KLocale::MetricBinaryDialect);

    QCOMPARE(locale.formatByteSize(1024.0), QString("1.0 kB"));
    QCOMPARE(locale.formatByteSize(1023.0), QString("1.0 kB"));
    QCOMPARE(locale.formatByteSize(1163000.0), QString("1.2 MB"));

    QCOMPARE(locale.formatByteSize(-1024.0), QString("-1.0 kB"));
    QCOMPARE(locale.formatByteSize(-1023.0), QString("-1.0 kB"));
    QCOMPARE(locale.formatByteSize(-1163000.0), QString("-1.2 MB"));

    // Ensure specific dialects work, still on metric by default
    QCOMPARE(locale.formatByteSize(1024.0, 1, KLocale::JEDECBinaryDialect), QString("1.0 KB"));
    QCOMPARE(locale.formatByteSize(2097152.0, 1, KLocale::IECBinaryDialect), QString("2.0 MiB"));

    locale.setBinaryUnitDialect(KLocale::JEDECBinaryDialect);
    QCOMPARE(locale.binaryUnitDialect(), KLocale::JEDECBinaryDialect);
    QCOMPARE(locale.formatByteSize(2097152.0, 1, KLocale::MetricBinaryDialect), QString("2.1 MB"));

    // Ensure all units are represented (still in JEDEC mode)
    QCOMPARE(locale.formatByteSize(2.0e9, 1, KLocale::MetricBinaryDialect), QString("2.0 GB"));
    QCOMPARE(locale.formatByteSize(3.2e12, 1, KLocale::MetricBinaryDialect), QString("3.2 TB"));
    QCOMPARE(locale.formatByteSize(4.1e15, 1, KLocale::MetricBinaryDialect), QString("4.1 PB"));
    QCOMPARE(locale.formatByteSize(6.7e18, 2, KLocale::MetricBinaryDialect), QString("6.70 EB"));
    QCOMPARE(locale.formatByteSize(5.6e20, 2, KLocale::MetricBinaryDialect), QString("560.00 EB"));
    QCOMPARE(locale.formatByteSize(2.3e22, 2, KLocale::MetricBinaryDialect), QString("23.00 ZB"));
    QCOMPARE(locale.formatByteSize(1.0e27, 1, KLocale::MetricBinaryDialect), QString("1,000.0 YB"));

    // Spattering of specific units
    QCOMPARE(locale.formatByteSize(823000, 3, KLocale::IECBinaryDialect, KLocale::UnitMegaByte), QString("0.785 MiB"));
    QCOMPARE(locale.formatByteSize(1234034.0, 4, KLocale::JEDECBinaryDialect, KLocale::UnitByte), QString("1,234,034 B"));
}

QTEST_MAIN(KLocaleTest)
