/*
    Copyright 2010 John Layt <john@layt.net>

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

#include "kdatetimeformattertest.h"

#include <locale.h>

#include <QtCore/QString>

#include <QtTest/QtTest>
#include "kdatetime.h"
#include "kcalendarsystem.h"
#include "kdatetimeformatter_p.h"
#include "klocale.h"
#include "kconfiggroup.h"


QTEST_MAIN(KDateTimeFormatterTest)

void KDateTimeFormatterTest::testFormatDateTimePosix()
{
    KLocale::global()->setLanguage(QStringList("en_US"));
    KLocale::global()->setCalendarSystem(KLocale::QDateCalendar);
    KLocale::global()->setDateFormatShort("%y-%m-%d");

    KDateTimeFormatter formatter;

    //Test default settings
    KDateTime testDate(QDate(2005, 10, 20));
    QCOMPARE(formatter.formatDateTime(testDate, "%Y"), QString("2005"));
    QCOMPARE(formatter.formatDateTime(testDate, "%C"), QString("20"));
    QCOMPARE(formatter.formatDateTime(testDate, "%y"), QString("05"));
    QCOMPARE(formatter.formatDateTime(testDate, "%m"), QString("10"));
    QCOMPARE(formatter.formatDateTime(testDate, "%n"), QString("10"));
    QCOMPARE(formatter.formatDateTime(testDate, "%d"), QString("20"));
    QCOMPARE(formatter.formatDateTime(testDate, "%e"), QString("20"));
    QCOMPARE(formatter.formatDateTime(testDate, "%B"), QString("October"));
    QCOMPARE(formatter.formatDateTime(testDate, "%b"), QString("Oct"));
    QCOMPARE(formatter.formatDateTime(testDate, "%h"), QString("Oct"));
    QCOMPARE(formatter.formatDateTime(testDate, "%A"), QString("Thursday"));
    QCOMPARE(formatter.formatDateTime(testDate, "%a"), QString("Thu"));
    QCOMPARE(formatter.formatDateTime(testDate, "%j"), QString("293"));
    QCOMPARE(formatter.formatDateTime(testDate, "%V"), QString("42"));
    QCOMPARE(formatter.formatDateTime(testDate, "%G"), QString("2005"));
    QCOMPARE(formatter.formatDateTime(testDate, "%g"), QString("05"));
    QCOMPARE(formatter.formatDateTime(testDate, "%u"), QString("4"));
    QCOMPARE(formatter.formatDateTime(testDate, "%D"), QString("10/20/05"));
    QCOMPARE(formatter.formatDateTime(testDate, "%F"), QString("2005-10-20"));
    QCOMPARE(formatter.formatDateTime(testDate, "%x"), QString("05-10-20"));
    QCOMPARE(formatter.formatDateTime(testDate, "%%"), QString("%"));
    QCOMPARE(formatter.formatDateTime(testDate, "%t"), QString("\t"));

    QCOMPARE(formatter.formatDateTime(testDate, "-%Y-%C-%y-%m-%n-%d-%e-%B-%b-%h-%A-%a-%j-%V-%G-%g-%u:%D:%F:%x:%%-%t-"),
             QString("-2005-20-05-10-10-20-20-October-Oct-Oct-Thursday-Thu-293-42-2005-05-4:10/20/05:2005-10-20:05-10-20:%-\t-"));

    //Test the minimum widths are respected
    QCOMPARE(formatter.formatDateTime(testDate, "%1Y"),  QString("2005"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1C"),  QString("20"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1y"),  QString("05"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1m"),  QString("10"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1n"),  QString("10"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1d"),  QString("20"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1e"),  QString("20"));
    QCOMPARE(formatter.formatDateTime(testDate, "%01B"), QString("October"));
    QCOMPARE(formatter.formatDateTime(testDate, "%01b"), QString("Oct"));
    QCOMPARE(formatter.formatDateTime(testDate, "%01h"), QString("Oct"));
    QCOMPARE(formatter.formatDateTime(testDate, "%01A"), QString("Thursday"));
    QCOMPARE(formatter.formatDateTime(testDate, "%01a"), QString("Thu"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1j"),  QString("293"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1V"),  QString("42"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1G"),  QString("2005"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1g"),  QString("05"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1u"),  QString("4"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1D"),  QString("10/20/05"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1F"),  QString("2005-10-20"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1x"),  QString("05-10-20"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1%"),  QString("%"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1t"),  QString("\t"));

    testDate.setDate(QDate(2005, 1, 2));

    //Test various padding options with minimum width
    QCOMPARE(formatter.formatDateTime(testDate, "%"),     QString(""));
    QCOMPARE(formatter.formatDateTime(testDate, "%m"),    QString("01"));
    QCOMPARE(formatter.formatDateTime(testDate, "%n"),    QString("1"));
    QCOMPARE(formatter.formatDateTime(testDate, "%d"),    QString("02"));
    QCOMPARE(formatter.formatDateTime(testDate, "%e"),    QString("2"));
    QCOMPARE(formatter.formatDateTime(testDate, "%j"),    QString("002"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_m"),   QString(" 1"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_n"),   QString("1"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_d"),   QString(" 2"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_e"),   QString("2"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_j"),   QString("  2"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-m"),   QString("1"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-n"),   QString("1"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-d"),   QString("2"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-e"),   QString("2"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-j"),   QString("2"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0m"),   QString("01"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0n"),   QString("1"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0d"),   QString("02"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0e"),   QString("2"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0j"),   QString("002"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-_j"),  QString("  2"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_-j"),  QString("2"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-_0j"), QString("002"));

    //Test various padding options with width override
    QCOMPARE(formatter.formatDateTime(testDate, "%1m"),   QString("01"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-05m"), QString("00001"));
    QCOMPARE(formatter.formatDateTime(testDate, "%10m"),  QString("0000000001"));
    QCOMPARE(formatter.formatDateTime(testDate, "%15m"),  QString("000000000000001"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-5m"),  QString("1"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_5m"),  QString("    1"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_15m"), QString("              1"));
    QCOMPARE(formatter.formatDateTime(testDate, "%1B"),   QString("January"));
    QCOMPARE(formatter.formatDateTime(testDate, "%15B"),  QString("        January"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-15B"), QString("January"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_15B"), QString("        January"));
    QCOMPARE(formatter.formatDateTime(testDate, "%015B"), QString("00000000January"));

    //Test week number in previous year
    QCOMPARE(formatter.formatDateTime(testDate, "%V"), QString("53"));
    QCOMPARE(formatter.formatDateTime(testDate, "%G"), QString("2004"));
    QCOMPARE(formatter.formatDateTime(testDate, "%g"), QString("04"));

    //test case overrides
    QCOMPARE(formatter.formatDateTime(testDate, "%B"),  QString("January"));
    QCOMPARE(formatter.formatDateTime(testDate, "%^B"), QString("JANUARY"));
    QCOMPARE(formatter.formatDateTime(testDate, "%#B"), QString("JANUARY"));
    QCOMPARE(formatter.formatDateTime(testDate, "%m"),  QString("01"));
    QCOMPARE(formatter.formatDateTime(testDate, "%^m"), QString("01"));
    QCOMPARE(formatter.formatDateTime(testDate, "%#m"), QString("01"));

    //Test various year options

    testDate.setDate(QDate(789, 1, 1));
    QCOMPARE(formatter.formatDateTime(testDate, "%Y"),  QString("0789"));
    QCOMPARE(formatter.formatDateTime(testDate, "%C"),  QString("07"));
    QCOMPARE(formatter.formatDateTime(testDate, "%y"),  QString("89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-Y"), QString("789"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-C"), QString("7"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-y"), QString("89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_Y"), QString(" 789"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_C"), QString(" 7"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_y"), QString("89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0Y"), QString("0789"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0C"), QString("07"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0y"), QString("89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EC"), QString("AD"));
    QCOMPARE(formatter.formatDateTime(testDate, "%Ey"), QString("789"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EY"), QString("789 AD"));

    testDate.setDate(QDate(709, 1, 1));
    QCOMPARE(formatter.formatDateTime(testDate, "%Y"),  QString("0709"));
    QCOMPARE(formatter.formatDateTime(testDate, "%C"),  QString("07"));
    QCOMPARE(formatter.formatDateTime(testDate, "%y"),  QString("09"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-Y"), QString("709"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-C"), QString("7"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-y"), QString("9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_Y"), QString(" 709"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_C"), QString(" 7"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_y"), QString(" 9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0Y"), QString("0709"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0C"), QString("07"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0y"), QString("09"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EC"), QString("AD"));
    QCOMPARE(formatter.formatDateTime(testDate, "%Ey"), QString("709"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EY"), QString("709 AD"));

    testDate.setDate(QDate(89, 1, 1));
    QCOMPARE(formatter.formatDateTime(testDate, "%Y"),  QString("0089"));
    QCOMPARE(formatter.formatDateTime(testDate, "%C"),  QString("00"));
    QCOMPARE(formatter.formatDateTime(testDate, "%y"),  QString("89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-Y"), QString("89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-C"), QString("0"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-y"), QString("89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_Y"), QString("  89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_C"), QString(" 0"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_y"), QString("89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0Y"), QString("0089"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0C"), QString("00"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0y"), QString("89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EC"), QString("AD"));
    QCOMPARE(formatter.formatDateTime(testDate, "%Ey"), QString("89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EY"), QString("89 AD"));

    testDate.setDate(QDate(9, 1, 1));
    QCOMPARE(formatter.formatDateTime(testDate, "%Y"),  QString("0009"));
    QCOMPARE(formatter.formatDateTime(testDate, "%C"),  QString("00"));
    QCOMPARE(formatter.formatDateTime(testDate, "%y"),  QString("09"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-Y"), QString("9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-C"), QString("0"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-y"), QString("9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_Y"), QString("   9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_C"), QString(" 0"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_y"), QString(" 9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0Y"), QString("0009"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0C"), QString("00"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0y"), QString("09"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EC"), QString("AD"));
    QCOMPARE(formatter.formatDateTime(testDate, "%Ey"), QString("9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EY"), QString("9 AD"));

    testDate.setDate(QDate(-9, 1, 1));
    QCOMPARE(formatter.formatDateTime(testDate, "%Y"),  QString("-0009"));
    QCOMPARE(formatter.formatDateTime(testDate, "%C"),  QString("-00"));
    QCOMPARE(formatter.formatDateTime(testDate, "%y"),  QString("-09"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-Y"), QString("-9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-C"), QString("-0"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-y"), QString("-9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_Y"), QString("  -9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_C"), QString("-0"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_y"), QString("-9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0Y"), QString("-0009"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0C"), QString("-00"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0y"), QString("-09"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EC"), QString("BC"));
    QCOMPARE(formatter.formatDateTime(testDate, "%Ey"), QString("9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EY"), QString("9 BC"));

    testDate.setDate(QDate(-89, 1, 1));
    QCOMPARE(formatter.formatDateTime(testDate, "%Y"),  QString("-0089"));
    QCOMPARE(formatter.formatDateTime(testDate, "%C"),  QString("-00"));
    QCOMPARE(formatter.formatDateTime(testDate, "%y"),  QString("-89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-Y"), QString("-89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-C"), QString("-0"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-y"), QString("-89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_Y"), QString(" -89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_C"), QString("-0"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_y"), QString("-89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0Y"), QString("-0089"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0C"), QString("-00"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0y"), QString("-89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EC"), QString("BC"));
    QCOMPARE(formatter.formatDateTime(testDate, "%Ey"), QString("89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EY"), QString("89 BC"));

    testDate.setDate(QDate(-789, 1, 1));
    QCOMPARE(formatter.formatDateTime(testDate, "%Y"),  QString("-0789"));
    QCOMPARE(formatter.formatDateTime(testDate, "%C"),  QString("-07"));
    QCOMPARE(formatter.formatDateTime(testDate, "%y"),  QString("-89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-Y"), QString("-789"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-C"), QString("-7"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-y"), QString("-89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_Y"), QString("-789"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_C"), QString("-7"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_y"), QString("-89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0Y"), QString("-0789"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0C"), QString("-07"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0y"), QString("-89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EC"), QString("BC"));
    QCOMPARE(formatter.formatDateTime(testDate, "%Ey"), QString("789"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EY"), QString("789 BC"));

    testDate.setDate(QDate(-709, 1, 1));
    QCOMPARE(formatter.formatDateTime(testDate, "%Y"),  QString("-0709"));
    QCOMPARE(formatter.formatDateTime(testDate, "%C"),  QString("-07"));
    QCOMPARE(formatter.formatDateTime(testDate, "%y"),  QString("-09"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-Y"), QString("-709"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-C"), QString("-7"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-y"), QString("-9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_Y"), QString("-709"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_C"), QString("-7"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_y"), QString("-9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0Y"), QString("-0709"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0C"), QString("-07"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0y"), QString("-09"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EC"), QString("BC"));
    QCOMPARE(formatter.formatDateTime(testDate, "%Ey"), QString("709"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EY"), QString("709 BC"));

    testDate.setDate(QDate(-1789, 1, 1));
    QCOMPARE(formatter.formatDateTime(testDate, "%Y"),  QString("-1789"));
    QCOMPARE(formatter.formatDateTime(testDate, "%C"),  QString("-17"));
    QCOMPARE(formatter.formatDateTime(testDate, "%y"),  QString("-89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-Y"), QString("-1789"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-C"), QString("-17"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-y"), QString("-89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_Y"), QString("-1789"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_C"), QString("-17"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_y"), QString("-89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0Y"), QString("-1789"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0C"), QString("-17"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0y"), QString("-89"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EC"), QString("BC"));
    QCOMPARE(formatter.formatDateTime(testDate, "%Ey"), QString("1789"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EY"), QString("1789 BC"));

    testDate.setDate(QDate(-1709, 1, 1));
    QCOMPARE(formatter.formatDateTime(testDate, "%Y"),  QString("-1709"));
    QCOMPARE(formatter.formatDateTime(testDate, "%C"),  QString("-17"));
    QCOMPARE(formatter.formatDateTime(testDate, "%y"),  QString("-09"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-Y"), QString("-1709"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-C"), QString("-17"));
    QCOMPARE(formatter.formatDateTime(testDate, "%-y"), QString("-9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_Y"), QString("-1709"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_C"), QString("-17"));
    QCOMPARE(formatter.formatDateTime(testDate, "%_y"), QString("-9"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0Y"), QString("-1709"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0C"), QString("-17"));
    QCOMPARE(formatter.formatDateTime(testDate, "%0y"), QString("-09"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EC"), QString("BC"));
    QCOMPARE(formatter.formatDateTime(testDate, "%Ey"), QString("1709"));
    QCOMPARE(formatter.formatDateTime(testDate, "%EY"), QString("1709 BC"));
    /*
        // Test POSIX format
        testDate.setDate( QDate( 2010, 1, 2 ) );
        QCOMPARE( formatter.formatDateTime( testDate, "%Y-%n-%d", KLocale::KdeFormat ),   QString( "2010-1-02" ) );
        QCOMPARE( formatter.formatDateTime( testDate, "%Y-%n-%d", KLocale::PosixFormat ), QString( "2010-\n-02" ) );
        QCOMPARE( formatter.formatDateTime( testDate, "%e", KLocale::KdeFormat ),   QString( "2" ) );
        QCOMPARE( formatter.formatDateTime( testDate, "%e", KLocale::PosixFormat ), QString( " 2" ) );
    */
}

void KDateTimeFormatterTest::testFormatUnicode()
{
    KLocale::global()->setLanguage(QStringList("en_US"));
    KLocale::global()->setCalendarSystem(KLocale::QDateCalendar);
    KLocale::global()->setDateFormatShort("%y-%m-%d");

    KDateTimeFormatter formatter;

    KDateTime testDateTime(QDate(2005, 10, 20));
    compareFormatUnicode(formatter, testDateTime, "yyyy");
    compareFormatUnicode(formatter, testDateTime, "yy");
    compareFormatUnicode(formatter, testDateTime, "MMMM");
    compareFormatUnicode(formatter, testDateTime, "MMM");
    compareFormatUnicode(formatter, testDateTime, "MM");
    compareFormatUnicode(formatter, testDateTime, "M");
    compareFormatUnicode(formatter, testDateTime, "dddd");
    compareFormatUnicode(formatter, testDateTime, "ddd");
    compareFormatUnicode(formatter, testDateTime, "dd");
    compareFormatUnicode(formatter, testDateTime, "d");
    compareFormatUnicode(formatter, testDateTime, "yyyy-MM-dd");

    testDateTime.setDate(QDate(-2005, 10, 20));
    compareFormatUnicode(formatter, testDateTime, "yyyy");
    compareFormatUnicode(formatter, testDateTime, "yy");
    compareFormatUnicode(formatter, testDateTime, "MMMM");
    compareFormatUnicode(formatter, testDateTime, "MMM");
    compareFormatUnicode(formatter, testDateTime, "MM");
    compareFormatUnicode(formatter, testDateTime, "M");
    compareFormatUnicode(formatter, testDateTime, "dddd");
    compareFormatUnicode(formatter, testDateTime, "ddd");
    compareFormatUnicode(formatter, testDateTime, "dd");
    compareFormatUnicode(formatter, testDateTime, "d");
    compareFormatUnicode(formatter, testDateTime, "yyyy-MM-dd");
}

void KDateTimeFormatterTest::compareFormatUnicode(KDateTimeFormatter formatter, const KDateTime &testDateTime, const QString &testFormat)
{
    QCOMPARE(formatter.formatDateTime(testDateTime, testFormat, 0, KLocale::global()->calendar(), KLocale::global(), KLocale::global()->dateTimeDigitSet(), KLocale::UnicodeFormat), testDateTime.dateTime().toString(testFormat));
}
