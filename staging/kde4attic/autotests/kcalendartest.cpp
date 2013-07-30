/*
    Copyright 2005 Brad Hards <bradh@kde.org>
    Copyright 2007, 2009, 2010, 2011 John Layt <john@layt.net>

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

#include "kcalendartest.h"

#include <locale.h>

#include <QtCore/QString>

#include <QtTest/QtTest>
#include "kcalendarsystem.h"
#include "klocalizeddate.h"
#include "klocalizedstring.h"
#include "kconfiggroup.h"

QTEST_MAIN(KCalendarTest)

void KCalendarTest::testTypes()
{
    const KCalendarSystem *calendar;

    calendar = KCalendarSystem::create(KLocale::GregorianCalendar);
    QCOMPARE(calendar->calendarType(), QString("gregorian"));
    QCOMPARE(calendar->calendarSystem(), KLocale::GregorianCalendar);
    delete calendar;
    calendar = KCalendarSystem::create(KLocale::CopticCalendar);
    QCOMPARE(calendar->calendarType(), QString("coptic"));
    QCOMPARE(calendar->calendarSystem(), KLocale::CopticCalendar);
    delete calendar;
    calendar = KCalendarSystem::create(KLocale::EthiopianCalendar);
    QCOMPARE(calendar->calendarType(), QString("ethiopian"));
    QCOMPARE(calendar->calendarSystem(), KLocale::EthiopianCalendar);
    delete calendar;
    calendar = KCalendarSystem::create(KLocale::HebrewCalendar);
    QCOMPARE(calendar->calendarType(), QString("hebrew"));
    QCOMPARE(calendar->calendarSystem(), KLocale::HebrewCalendar);
    delete calendar;
    calendar = KCalendarSystem::create(KLocale::IslamicCivilCalendar);
    QCOMPARE(calendar->calendarType(), QString("hijri"));
    QCOMPARE(calendar->calendarSystem(), KLocale::IslamicCivilCalendar);
    delete calendar;
    calendar = KCalendarSystem::create(KLocale::IndianNationalCalendar);
    QCOMPARE(calendar->calendarType(), QString("indian-national"));
    QCOMPARE(calendar->calendarSystem(), KLocale::IndianNationalCalendar);
    delete calendar;
    calendar = KCalendarSystem::create(KLocale::JalaliCalendar);
    QCOMPARE(calendar->calendarType(), QString("jalali"));
    QCOMPARE(calendar->calendarSystem(), KLocale::JalaliCalendar);
    delete calendar;
    calendar = KCalendarSystem::create(KLocale::JapaneseCalendar);
    QCOMPARE(calendar->calendarType(), QString("japanese"));
    QCOMPARE(calendar->calendarSystem(), KLocale::JapaneseCalendar);
    delete calendar;
    calendar = KCalendarSystem::create(KLocale::JulianCalendar);
    QCOMPARE(calendar->calendarType(), QString("julian"));
    QCOMPARE(calendar->calendarSystem(), KLocale::JulianCalendar);
    delete calendar;
    calendar = KCalendarSystem::create(KLocale::MinguoCalendar);
    QCOMPARE(calendar->calendarType(), QString("minguo"));
    QCOMPARE(calendar->calendarSystem(), KLocale::MinguoCalendar);
    delete calendar;
    calendar = KCalendarSystem::create(KLocale::ThaiCalendar);
    QCOMPARE(calendar->calendarType(), QString("thai"));
    QCOMPARE(calendar->calendarSystem(), KLocale::ThaiCalendar);
    delete calendar;
    calendar = KCalendarSystem::create(KLocale::QDateCalendar);
    QCOMPARE(calendar->calendarType(), QString("gregorian"));
    QCOMPARE(calendar->calendarSystem(), KLocale::GregorianCalendar);
    delete calendar;

    QList<KLocale::CalendarSystem> list = KCalendarSystem::calendarSystemsList();
    QCOMPARE(list.count(), 11);
    QVERIFY(list.contains(KLocale::GregorianCalendar));
    QVERIFY(list.contains(KLocale::CopticCalendar));
    QVERIFY(list.contains(KLocale::EthiopianCalendar));
    QVERIFY(list.contains(KLocale::HebrewCalendar));
    QVERIFY(list.contains(KLocale::IslamicCivilCalendar));
    QVERIFY(list.contains(KLocale::IndianNationalCalendar));
    QVERIFY(list.contains(KLocale::JalaliCalendar));
    QVERIFY(list.contains(KLocale::JapaneseCalendar));
    QVERIFY(list.contains(KLocale::JulianCalendar));
    QVERIFY(list.contains(KLocale::MinguoCalendar));
    QVERIFY(list.contains(KLocale::ThaiCalendar));
    QVERIFY(list.contains(KLocale::QDateCalendar));

    QCOMPARE(KCalendarSystem::calendarLabel(KLocale::GregorianCalendar),      QString("Gregorian"));
    QCOMPARE(KCalendarSystem::calendarLabel(KLocale::CopticCalendar),         QString("Coptic"));
    QCOMPARE(KCalendarSystem::calendarLabel(KLocale::EthiopianCalendar),      QString("Ethiopian"));
    QCOMPARE(KCalendarSystem::calendarLabel(KLocale::HebrewCalendar),         QString("Hebrew"));
    QCOMPARE(KCalendarSystem::calendarLabel(KLocale::IslamicCivilCalendar),   QString("Islamic / Hijri (Civil)"));
    QCOMPARE(KCalendarSystem::calendarLabel(KLocale::IndianNationalCalendar), QString("Indian National"));
    QCOMPARE(KCalendarSystem::calendarLabel(KLocale::JalaliCalendar),         QString("Jalali"));
    QCOMPARE(KCalendarSystem::calendarLabel(KLocale::JapaneseCalendar),       QString("Japanese"));
    QCOMPARE(KCalendarSystem::calendarLabel(KLocale::JulianCalendar),         QString("Julian"));
    QCOMPARE(KCalendarSystem::calendarLabel(KLocale::MinguoCalendar),         QString("Taiwanese"));
    QCOMPARE(KCalendarSystem::calendarLabel(KLocale::ThaiCalendar),           QString("Thai"));
    QCOMPARE(KCalendarSystem::calendarLabel(KLocale::QDateCalendar),          QString("Gregorian"));

}

void KCalendarTest::testFormatDate()
{
    KConfigGroup lcg(KSharedConfig::openConfig(), QString("Locale"));
    KConfigGroup cg = lcg.group(QString("KCalendarSystem %1").arg("gregorian"));
    cg.deleteGroup();

    KLocale *locale = new KLocale(*KLocale::global());
    locale->setDateFormatShort("%y-%m-%d");
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar, locale);

    //Test default settings
    QDate testDate(2005, 10, 20);
    QCOMPARE(calendar->formatDate(testDate, "%Y"), QString("2005"));
    QCOMPARE(calendar->formatDate(testDate, "%C"), QString("20"));
    QCOMPARE(calendar->formatDate(testDate, "%y"), QString("05"));
    QCOMPARE(calendar->formatDate(testDate, "%m"), QString("10"));
    QCOMPARE(calendar->formatDate(testDate, "%n"), QString("10"));
    QCOMPARE(calendar->formatDate(testDate, "%d"), QString("20"));
    QCOMPARE(calendar->formatDate(testDate, "%e"), QString("20"));
    QCOMPARE(calendar->formatDate(testDate, "%B"), QString("October"));
    QCOMPARE(calendar->formatDate(testDate, "%b"), QString("Oct"));
    QCOMPARE(calendar->formatDate(testDate, "%h"), QString("Oct"));
    QCOMPARE(calendar->formatDate(testDate, "%A"), QString("Thursday"));
    QCOMPARE(calendar->formatDate(testDate, "%a"), QString("Thu"));
    QCOMPARE(calendar->formatDate(testDate, "%j"), QString("293"));
    QCOMPARE(calendar->formatDate(testDate, "%V"), QString("42"));
    QCOMPARE(calendar->formatDate(testDate, "%G"), QString("2005"));
    QCOMPARE(calendar->formatDate(testDate, "%g"), QString("05"));
    QCOMPARE(calendar->formatDate(testDate, "%u"), QString("4"));
    QCOMPARE(calendar->formatDate(testDate, "%D"), QString("10/20/05"));
    QCOMPARE(calendar->formatDate(testDate, "%F"), QString("2005-10-20"));
    QCOMPARE(calendar->formatDate(testDate, "%x"), QString("05-10-20"));
    QCOMPARE(calendar->formatDate(testDate, "%%"), QString("%"));
    QCOMPARE(calendar->formatDate(testDate, "%t"), QString("\t"));

    QCOMPARE(calendar->formatDate(testDate, "-%Y-%C-%y-%m-%n-%d-%e-%B-%b-%h-%A-%a-%j-%V-%G-%g-%u:%D:%F:%x:%%-%t-"),
             QString("-2005-20-05-10-10-20-20-October-Oct-Oct-Thursday-Thu-293-42-2005-05-4:10/20/05:2005-10-20:05-10-20:%-\t-"));

    //Test the minimum widths are respected
    QCOMPARE(calendar->formatDate(testDate, "%1Y"),  QString("2005"));
    QCOMPARE(calendar->formatDate(testDate, "%1C"),  QString("20"));
    QCOMPARE(calendar->formatDate(testDate, "%1y"),  QString("05"));
    QCOMPARE(calendar->formatDate(testDate, "%1m"),  QString("10"));
    QCOMPARE(calendar->formatDate(testDate, "%1n"),  QString("10"));
    QCOMPARE(calendar->formatDate(testDate, "%1d"),  QString("20"));
    QCOMPARE(calendar->formatDate(testDate, "%1e"),  QString("20"));
    QCOMPARE(calendar->formatDate(testDate, "%01B"), QString("October"));
    QCOMPARE(calendar->formatDate(testDate, "%01b"), QString("Oct"));
    QCOMPARE(calendar->formatDate(testDate, "%01h"), QString("Oct"));
    QCOMPARE(calendar->formatDate(testDate, "%01A"), QString("Thursday"));
    QCOMPARE(calendar->formatDate(testDate, "%01a"), QString("Thu"));
    QCOMPARE(calendar->formatDate(testDate, "%1j"),  QString("293"));
    QCOMPARE(calendar->formatDate(testDate, "%1V"),  QString("42"));
    QCOMPARE(calendar->formatDate(testDate, "%1G"),  QString("2005"));
    QCOMPARE(calendar->formatDate(testDate, "%1g"),  QString("05"));
    QCOMPARE(calendar->formatDate(testDate, "%1u"),  QString("4"));
    QCOMPARE(calendar->formatDate(testDate, "%1D"),  QString("10/20/05"));
    QCOMPARE(calendar->formatDate(testDate, "%1F"),  QString("2005-10-20"));
    QCOMPARE(calendar->formatDate(testDate, "%1x"),  QString("05-10-20"));
    QCOMPARE(calendar->formatDate(testDate, "%1%"),  QString("%"));
    QCOMPARE(calendar->formatDate(testDate, "%1t"),  QString("\t"));

    testDate.setDate(2005, 1, 2);

    //Test various padding options with minimum width
    QCOMPARE(calendar->formatDate(testDate, "%"),     QString(""));
    QCOMPARE(calendar->formatDate(testDate, "%m"),    QString("01"));
    QCOMPARE(calendar->formatDate(testDate, "%n"),    QString("1"));
    QCOMPARE(calendar->formatDate(testDate, "%d"),    QString("02"));
    QCOMPARE(calendar->formatDate(testDate, "%e"),    QString("2"));
    QCOMPARE(calendar->formatDate(testDate, "%j"),    QString("002"));
    QCOMPARE(calendar->formatDate(testDate, "%_m"),   QString(" 1"));
    QCOMPARE(calendar->formatDate(testDate, "%_n"),   QString("1"));
    QCOMPARE(calendar->formatDate(testDate, "%_d"),   QString(" 2"));
    QCOMPARE(calendar->formatDate(testDate, "%_e"),   QString("2"));
    QCOMPARE(calendar->formatDate(testDate, "%_j"),   QString("  2"));
    QCOMPARE(calendar->formatDate(testDate, "%-m"),   QString("1"));
    QCOMPARE(calendar->formatDate(testDate, "%-n"),   QString("1"));
    QCOMPARE(calendar->formatDate(testDate, "%-d"),   QString("2"));
    QCOMPARE(calendar->formatDate(testDate, "%-e"),   QString("2"));
    QCOMPARE(calendar->formatDate(testDate, "%-j"),   QString("2"));
    QCOMPARE(calendar->formatDate(testDate, "%0m"),   QString("01"));
    QCOMPARE(calendar->formatDate(testDate, "%0n"),   QString("1"));
    QCOMPARE(calendar->formatDate(testDate, "%0d"),   QString("02"));
    QCOMPARE(calendar->formatDate(testDate, "%0e"),   QString("2"));
    QCOMPARE(calendar->formatDate(testDate, "%0j"),   QString("002"));
    QCOMPARE(calendar->formatDate(testDate, "%-_j"),  QString("  2"));
    QCOMPARE(calendar->formatDate(testDate, "%_-j"),  QString("2"));
    QCOMPARE(calendar->formatDate(testDate, "%-_0j"), QString("002"));

    //Test various padding options with width override
    QCOMPARE(calendar->formatDate(testDate, "%1m"),   QString("01"));
    QCOMPARE(calendar->formatDate(testDate, "%-05m"), QString("00001"));
    QCOMPARE(calendar->formatDate(testDate, "%10m"),  QString("0000000001"));
    QCOMPARE(calendar->formatDate(testDate, "%15m"),  QString("000000000000001"));
    QCOMPARE(calendar->formatDate(testDate, "%-5m"),  QString("1"));
    QCOMPARE(calendar->formatDate(testDate, "%_5m"),  QString("    1"));
    QCOMPARE(calendar->formatDate(testDate, "%_15m"), QString("              1"));
    QCOMPARE(calendar->formatDate(testDate, "%1B"),   QString("January"));
    QCOMPARE(calendar->formatDate(testDate, "%15B"),  QString("        January"));
    QCOMPARE(calendar->formatDate(testDate, "%-15B"), QString("January"));
    QCOMPARE(calendar->formatDate(testDate, "%_15B"), QString("        January"));
    QCOMPARE(calendar->formatDate(testDate, "%015B"), QString("00000000January"));

    //Test week number in previous year
    QCOMPARE(calendar->formatDate(testDate, "%V"), QString("53"));
    QCOMPARE(calendar->formatDate(testDate, "%G"), QString("2004"));
    QCOMPARE(calendar->formatDate(testDate, "%g"), QString("04"));

    //test case overrides
    QCOMPARE(calendar->formatDate(testDate, "%B"),  QString("January"));
    QCOMPARE(calendar->formatDate(testDate, "%^B"), QString("JANUARY"));
    QCOMPARE(calendar->formatDate(testDate, "%#B"), QString("JANUARY"));
    QCOMPARE(calendar->formatDate(testDate, "%m"),  QString("01"));
    QCOMPARE(calendar->formatDate(testDate, "%^m"), QString("01"));
    QCOMPARE(calendar->formatDate(testDate, "%#m"), QString("01"));

    //Test various year options

    testDate.setDate(789, 1, 1);
    QCOMPARE(calendar->formatDate(testDate, "%Y"),  QString("0789"));
    QCOMPARE(calendar->formatDate(testDate, "%C"),  QString("07"));
    QCOMPARE(calendar->formatDate(testDate, "%y"),  QString("89"));
    QCOMPARE(calendar->formatDate(testDate, "%-Y"), QString("789"));
    QCOMPARE(calendar->formatDate(testDate, "%-C"), QString("7"));
    QCOMPARE(calendar->formatDate(testDate, "%-y"), QString("89"));
    QCOMPARE(calendar->formatDate(testDate, "%_Y"), QString(" 789"));
    QCOMPARE(calendar->formatDate(testDate, "%_C"), QString(" 7"));
    QCOMPARE(calendar->formatDate(testDate, "%_y"), QString("89"));
    QCOMPARE(calendar->formatDate(testDate, "%0Y"), QString("0789"));
    QCOMPARE(calendar->formatDate(testDate, "%0C"), QString("07"));
    QCOMPARE(calendar->formatDate(testDate, "%0y"), QString("89"));
    QCOMPARE(calendar->formatDate(testDate, "%EC"), QString("AD"));
    QCOMPARE(calendar->formatDate(testDate, "%Ey"), QString("789"));
    QCOMPARE(calendar->formatDate(testDate, "%EY"), QString("789 AD"));

    testDate.setDate(709, 1, 1);
    QCOMPARE(calendar->formatDate(testDate, "%Y"),  QString("0709"));
    QCOMPARE(calendar->formatDate(testDate, "%C"),  QString("07"));
    QCOMPARE(calendar->formatDate(testDate, "%y"),  QString("09"));
    QCOMPARE(calendar->formatDate(testDate, "%-Y"), QString("709"));
    QCOMPARE(calendar->formatDate(testDate, "%-C"), QString("7"));
    QCOMPARE(calendar->formatDate(testDate, "%-y"), QString("9"));
    QCOMPARE(calendar->formatDate(testDate, "%_Y"), QString(" 709"));
    QCOMPARE(calendar->formatDate(testDate, "%_C"), QString(" 7"));
    QCOMPARE(calendar->formatDate(testDate, "%_y"), QString(" 9"));
    QCOMPARE(calendar->formatDate(testDate, "%0Y"), QString("0709"));
    QCOMPARE(calendar->formatDate(testDate, "%0C"), QString("07"));
    QCOMPARE(calendar->formatDate(testDate, "%0y"), QString("09"));
    QCOMPARE(calendar->formatDate(testDate, "%EC"), QString("AD"));
    QCOMPARE(calendar->formatDate(testDate, "%Ey"), QString("709"));
    QCOMPARE(calendar->formatDate(testDate, "%EY"), QString("709 AD"));

    testDate.setDate(89, 1, 1);
    QCOMPARE(calendar->formatDate(testDate, "%Y"),  QString("0089"));
    QCOMPARE(calendar->formatDate(testDate, "%C"),  QString("00"));
    QCOMPARE(calendar->formatDate(testDate, "%y"),  QString("89"));
    QCOMPARE(calendar->formatDate(testDate, "%-Y"), QString("89"));
    QCOMPARE(calendar->formatDate(testDate, "%-C"), QString("0"));
    QCOMPARE(calendar->formatDate(testDate, "%-y"), QString("89"));
    QCOMPARE(calendar->formatDate(testDate, "%_Y"), QString("  89"));
    QCOMPARE(calendar->formatDate(testDate, "%_C"), QString(" 0"));
    QCOMPARE(calendar->formatDate(testDate, "%_y"), QString("89"));
    QCOMPARE(calendar->formatDate(testDate, "%0Y"), QString("0089"));
    QCOMPARE(calendar->formatDate(testDate, "%0C"), QString("00"));
    QCOMPARE(calendar->formatDate(testDate, "%0y"), QString("89"));
    QCOMPARE(calendar->formatDate(testDate, "%EC"), QString("AD"));
    QCOMPARE(calendar->formatDate(testDate, "%Ey"), QString("89"));
    QCOMPARE(calendar->formatDate(testDate, "%EY"), QString("89 AD"));

    testDate.setDate(9, 1, 1);
    QCOMPARE(calendar->formatDate(testDate, "%Y"),  QString("0009"));
    QCOMPARE(calendar->formatDate(testDate, "%C"),  QString("00"));
    QCOMPARE(calendar->formatDate(testDate, "%y"),  QString("09"));
    QCOMPARE(calendar->formatDate(testDate, "%-Y"), QString("9"));
    QCOMPARE(calendar->formatDate(testDate, "%-C"), QString("0"));
    QCOMPARE(calendar->formatDate(testDate, "%-y"), QString("9"));
    QCOMPARE(calendar->formatDate(testDate, "%_Y"), QString("   9"));
    QCOMPARE(calendar->formatDate(testDate, "%_C"), QString(" 0"));
    QCOMPARE(calendar->formatDate(testDate, "%_y"), QString(" 9"));
    QCOMPARE(calendar->formatDate(testDate, "%0Y"), QString("0009"));
    QCOMPARE(calendar->formatDate(testDate, "%0C"), QString("00"));
    QCOMPARE(calendar->formatDate(testDate, "%0y"), QString("09"));
    QCOMPARE(calendar->formatDate(testDate, "%EC"), QString("AD"));
    QCOMPARE(calendar->formatDate(testDate, "%Ey"), QString("9"));
    QCOMPARE(calendar->formatDate(testDate, "%EY"), QString("9 AD"));

    testDate.setDate(-9, 1, 1);
    QCOMPARE(calendar->formatDate(testDate, "%Y"),  QString("-0009"));
    QCOMPARE(calendar->formatDate(testDate, "%C"),  QString("-00"));
    QCOMPARE(calendar->formatDate(testDate, "%y"),  QString("-09"));
    QCOMPARE(calendar->formatDate(testDate, "%-Y"), QString("-9"));
    QCOMPARE(calendar->formatDate(testDate, "%-C"), QString("-0"));
    QCOMPARE(calendar->formatDate(testDate, "%-y"), QString("-9"));
    QCOMPARE(calendar->formatDate(testDate, "%_Y"), QString("  -9"));
    QCOMPARE(calendar->formatDate(testDate, "%_C"), QString("-0"));
    QCOMPARE(calendar->formatDate(testDate, "%_y"), QString("-9"));
    QCOMPARE(calendar->formatDate(testDate, "%0Y"), QString("-0009"));
    QCOMPARE(calendar->formatDate(testDate, "%0C"), QString("-00"));
    QCOMPARE(calendar->formatDate(testDate, "%0y"), QString("-09"));
    QCOMPARE(calendar->formatDate(testDate, "%EC"), QString("BC"));
    QCOMPARE(calendar->formatDate(testDate, "%Ey"), QString("9"));
    QCOMPARE(calendar->formatDate(testDate, "%EY"), QString("9 BC"));

    testDate.setDate(-89, 1, 1);
    QCOMPARE(calendar->formatDate(testDate, "%Y"),  QString("-0089"));
    QCOMPARE(calendar->formatDate(testDate, "%C"),  QString("-00"));
    QCOMPARE(calendar->formatDate(testDate, "%y"),  QString("-89"));
    QCOMPARE(calendar->formatDate(testDate, "%-Y"), QString("-89"));
    QCOMPARE(calendar->formatDate(testDate, "%-C"), QString("-0"));
    QCOMPARE(calendar->formatDate(testDate, "%-y"), QString("-89"));
    QCOMPARE(calendar->formatDate(testDate, "%_Y"), QString(" -89"));
    QCOMPARE(calendar->formatDate(testDate, "%_C"), QString("-0"));
    QCOMPARE(calendar->formatDate(testDate, "%_y"), QString("-89"));
    QCOMPARE(calendar->formatDate(testDate, "%0Y"), QString("-0089"));
    QCOMPARE(calendar->formatDate(testDate, "%0C"), QString("-00"));
    QCOMPARE(calendar->formatDate(testDate, "%0y"), QString("-89"));
    QCOMPARE(calendar->formatDate(testDate, "%EC"), QString("BC"));
    QCOMPARE(calendar->formatDate(testDate, "%Ey"), QString("89"));
    QCOMPARE(calendar->formatDate(testDate, "%EY"), QString("89 BC"));

    testDate.setDate(-789, 1, 1);
    QCOMPARE(calendar->formatDate(testDate, "%Y"),  QString("-0789"));
    QCOMPARE(calendar->formatDate(testDate, "%C"),  QString("-07"));
    QCOMPARE(calendar->formatDate(testDate, "%y"),  QString("-89"));
    QCOMPARE(calendar->formatDate(testDate, "%-Y"), QString("-789"));
    QCOMPARE(calendar->formatDate(testDate, "%-C"), QString("-7"));
    QCOMPARE(calendar->formatDate(testDate, "%-y"), QString("-89"));
    QCOMPARE(calendar->formatDate(testDate, "%_Y"), QString("-789"));
    QCOMPARE(calendar->formatDate(testDate, "%_C"), QString("-7"));
    QCOMPARE(calendar->formatDate(testDate, "%_y"), QString("-89"));
    QCOMPARE(calendar->formatDate(testDate, "%0Y"), QString("-0789"));
    QCOMPARE(calendar->formatDate(testDate, "%0C"), QString("-07"));
    QCOMPARE(calendar->formatDate(testDate, "%0y"), QString("-89"));
    QCOMPARE(calendar->formatDate(testDate, "%EC"), QString("BC"));
    QCOMPARE(calendar->formatDate(testDate, "%Ey"), QString("789"));
    QCOMPARE(calendar->formatDate(testDate, "%EY"), QString("789 BC"));

    testDate.setDate(-709, 1, 1);
    QCOMPARE(calendar->formatDate(testDate, "%Y"),  QString("-0709"));
    QCOMPARE(calendar->formatDate(testDate, "%C"),  QString("-07"));
    QCOMPARE(calendar->formatDate(testDate, "%y"),  QString("-09"));
    QCOMPARE(calendar->formatDate(testDate, "%-Y"), QString("-709"));
    QCOMPARE(calendar->formatDate(testDate, "%-C"), QString("-7"));
    QCOMPARE(calendar->formatDate(testDate, "%-y"), QString("-9"));
    QCOMPARE(calendar->formatDate(testDate, "%_Y"), QString("-709"));
    QCOMPARE(calendar->formatDate(testDate, "%_C"), QString("-7"));
    QCOMPARE(calendar->formatDate(testDate, "%_y"), QString("-9"));
    QCOMPARE(calendar->formatDate(testDate, "%0Y"), QString("-0709"));
    QCOMPARE(calendar->formatDate(testDate, "%0C"), QString("-07"));
    QCOMPARE(calendar->formatDate(testDate, "%0y"), QString("-09"));
    QCOMPARE(calendar->formatDate(testDate, "%EC"), QString("BC"));
    QCOMPARE(calendar->formatDate(testDate, "%Ey"), QString("709"));
    QCOMPARE(calendar->formatDate(testDate, "%EY"), QString("709 BC"));

    testDate.setDate(-1789, 1, 1);
    QCOMPARE(calendar->formatDate(testDate, "%Y"),  QString("-1789"));
    QCOMPARE(calendar->formatDate(testDate, "%C"),  QString("-17"));
    QCOMPARE(calendar->formatDate(testDate, "%y"),  QString("-89"));
    QCOMPARE(calendar->formatDate(testDate, "%-Y"), QString("-1789"));
    QCOMPARE(calendar->formatDate(testDate, "%-C"), QString("-17"));
    QCOMPARE(calendar->formatDate(testDate, "%-y"), QString("-89"));
    QCOMPARE(calendar->formatDate(testDate, "%_Y"), QString("-1789"));
    QCOMPARE(calendar->formatDate(testDate, "%_C"), QString("-17"));
    QCOMPARE(calendar->formatDate(testDate, "%_y"), QString("-89"));
    QCOMPARE(calendar->formatDate(testDate, "%0Y"), QString("-1789"));
    QCOMPARE(calendar->formatDate(testDate, "%0C"), QString("-17"));
    QCOMPARE(calendar->formatDate(testDate, "%0y"), QString("-89"));
    QCOMPARE(calendar->formatDate(testDate, "%EC"), QString("BC"));
    QCOMPARE(calendar->formatDate(testDate, "%Ey"), QString("1789"));
    QCOMPARE(calendar->formatDate(testDate, "%EY"), QString("1789 BC"));

    testDate.setDate(-1709, 1, 1);
    QCOMPARE(calendar->formatDate(testDate, "%Y"),  QString("-1709"));
    QCOMPARE(calendar->formatDate(testDate, "%C"),  QString("-17"));
    QCOMPARE(calendar->formatDate(testDate, "%y"),  QString("-09"));
    QCOMPARE(calendar->formatDate(testDate, "%-Y"), QString("-1709"));
    QCOMPARE(calendar->formatDate(testDate, "%-C"), QString("-17"));
    QCOMPARE(calendar->formatDate(testDate, "%-y"), QString("-9"));
    QCOMPARE(calendar->formatDate(testDate, "%_Y"), QString("-1709"));
    QCOMPARE(calendar->formatDate(testDate, "%_C"), QString("-17"));
    QCOMPARE(calendar->formatDate(testDate, "%_y"), QString("-9"));
    QCOMPARE(calendar->formatDate(testDate, "%0Y"), QString("-1709"));
    QCOMPARE(calendar->formatDate(testDate, "%0C"), QString("-17"));
    QCOMPARE(calendar->formatDate(testDate, "%0y"), QString("-09"));
    QCOMPARE(calendar->formatDate(testDate, "%EC"), QString("BC"));
    QCOMPARE(calendar->formatDate(testDate, "%Ey"), QString("1709"));
    QCOMPARE(calendar->formatDate(testDate, "%EY"), QString("1709 BC"));

    // Test POSIX format
    testDate.setDate(2010, 1, 2);
    QCOMPARE(calendar->formatDate(testDate, "%Y-%n-%d", KLocale::KdeFormat),   QString("2010-1-02"));
    QCOMPARE(calendar->formatDate(testDate, "%Y-%n-%d", KLocale::PosixFormat), QString("2010-\n-02"));
    QCOMPARE(calendar->formatDate(testDate, "%e", KLocale::KdeFormat),   QString("2"));
    QCOMPARE(calendar->formatDate(testDate, "%e", KLocale::PosixFormat), QString(" 2"));

    delete calendar;
    delete locale;
}

void KCalendarTest::testFormatUnicode()
{
    KLocale *locale = new KLocale(*KLocale::global());
    locale->setDateFormatShort("%y-%m-%d");
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar, locale);

    QDate testDate(2005, 10, 20);
    compareFormatUnicode(calendar, testDate, "yyyy");
    compareFormatUnicode(calendar, testDate, "yy");
    compareFormatUnicode(calendar, testDate, "MMMM");
    compareFormatUnicode(calendar, testDate, "MMM");
    compareFormatUnicode(calendar, testDate, "MM");
    compareFormatUnicode(calendar, testDate, "M");
    compareFormatUnicode(calendar, testDate, "dddd");
    compareFormatUnicode(calendar, testDate, "ddd");
    compareFormatUnicode(calendar, testDate, "dd");
    compareFormatUnicode(calendar, testDate, "d");
    compareFormatUnicode(calendar, testDate, "yyyy-MM-dd");

    testDate.setDate(-2005, 10, 20);
    compareFormatUnicode(calendar, testDate, "yyyy");
    compareFormatUnicode(calendar, testDate, "yy");
    compareFormatUnicode(calendar, testDate, "MMMM");
    compareFormatUnicode(calendar, testDate, "MMM");
    compareFormatUnicode(calendar, testDate, "MM");
    compareFormatUnicode(calendar, testDate, "M");
    compareFormatUnicode(calendar, testDate, "dddd");
    compareFormatUnicode(calendar, testDate, "ddd");
    compareFormatUnicode(calendar, testDate, "dd");
    compareFormatUnicode(calendar, testDate, "d");
    compareFormatUnicode(calendar, testDate, "yyyy-MM-dd");
}

void KCalendarTest::compareFormatUnicode(const KCalendarSystem *calendar, const QDate &testDate, const QString &testFormat)
{
    QCOMPARE(calendar->formatDate(testDate, testFormat, KLocale::UnicodeFormat), testDate.toString(testFormat));
}

void KCalendarTest::testReadDate()
{
    KLocale *locale = new KLocale(*KLocale::global());
    locale->setDateFormatShort("%y-%m-%d");
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar, locale);
    int currentYear = QDate::currentDate().year();

    //Test default settings
    QDate testDate(2005, 10, 20);

    //Test Month and Day
    QCOMPARE(calendar->readDate("2005-01-01",  "%Y-%m-%d"), QDate(2005, 1, 1));
    QCOMPARE(calendar->readDate("-2005-01-01", "%Y-%m-%d"), QDate(-2005, 1, 1));
    QCOMPARE(calendar->readDate("05-01-01",    "%y-%m-%d"), QDate(2005, 1, 1));
    QCOMPARE(calendar->readDate("01-01",       "%m-%d"),    QDate(currentYear, 1, 1));

    //Test Era and Year In Era
    QCOMPARE(calendar->readDate("1 1 2005 AD", "%d %m %Ey %EC"), QDate(2005, 1, 1));
    QCOMPARE(calendar->readDate("1 1 2005 Anno Domini", "%d %m %Ey %EC"), QDate(2005, 1, 1));
    QCOMPARE(calendar->readDate("1 1 2005 BC", "%d %m %Ey %EC"), QDate(-2005, 1, 1));
    QCOMPARE(calendar->readDate("1 1 2005 Before Christ", "%d %m %Ey %EC"), QDate(-2005, 1, 1));
    QCOMPARE(calendar->readDate("1 1 2005 AD", "%d %m %EY"), QDate(2005, 1, 1));
    QCOMPARE(calendar->readDate("1 1 2005 Anno Domini", "%d %m %EY"), QDate(2005, 1, 1));
    QCOMPARE(calendar->readDate("1 1 2005 BC", "%d %m %EY"), QDate(-2005, 1, 1));
    QCOMPARE(calendar->readDate("1 1 2005 Before Christ", "%d %m %EY"), QDate(-2005, 1, 1));
    QCOMPARE(calendar->readDate("1 2005 AD January", "%d %Ey %EC %B"), QDate(2005, 1, 1));
    QCOMPARE(calendar->readDate("1 2005 AD January", "%d %EY %B"), QDate(2005, 1, 1));
    QCOMPARE(calendar->readDate("2005 AD January 1", "%Ey %EC %B %d"), QDate(2005, 1, 1));
    QCOMPARE(calendar->readDate("2005 AD January 1", "%EY %B %d"), QDate(2005, 1, 1));

    //Test Day-of-Year
    QCOMPARE(calendar->readDate("2005-001", "%Y-%j"), QDate(2005, 1, 1));
    QCOMPARE(calendar->readDate("2005-1",   "%Y-%j"), QDate(2005, 1, 1));
    QCOMPARE(calendar->readDate("001",      "%j"),    QDate(currentYear, 1, 1));
    QCOMPARE(calendar->readDate("2005-365", "%Y-%j"), QDate(2005, 12, 31));
    QCOMPARE(calendar->readDate("2000-366", "%Y-%j"), QDate(2000, 12, 31));

    QCOMPARE(calendar->readDate("2005-000", "%Y-%j"), QDate());
    QCOMPARE(calendar->readDate("2005-366", "%Y-%j"), QDate());
    QCOMPARE(calendar->readDate("2000-367", "%Y-%j"), QDate());

    //Test Weekday and Month names
    QCOMPARE(calendar->readDate("Monday 2004-01-05", "%A %Y-%m-%d"), QDate(2004,  1,  5));
    QCOMPARE(calendar->readDate("Mon 2004-01-05",    "%A %Y-%m-%d"), QDate(2004,  1,  5));
    QCOMPARE(calendar->readDate("Monday 2004-01-05", "%a %Y-%m-%d"), QDate(2004,  1,  5));
    QCOMPARE(calendar->readDate("Mon 2004-01-05",    "%a %Y-%m-%d"), QDate(2004,  1,  5));

    QCOMPARE(calendar->readDate("Sunday 2004-01-11", "%A %Y-%m-%d"), QDate(2004,  1, 11));
    QCOMPARE(calendar->readDate("Sun 2004-01-11",    "%A %Y-%m-%d"), QDate(2004,  1, 11));
    QCOMPARE(calendar->readDate("Sunday 2004-01-11", "%a %Y-%m-%d"), QDate(2004,  1, 11));
    QCOMPARE(calendar->readDate("Sun 2004-01-11",    "%a %Y-%m-%d"), QDate(2004,  1, 11));

    QCOMPARE(calendar->readDate("2004 January 01",  "%Y %B %d"), QDate(2004,  1,  1));
    QCOMPARE(calendar->readDate("2004 Jan 01",      "%Y %B %d"), QDate(2004,  1,  1));
    QCOMPARE(calendar->readDate("2004 January 01",  "%Y %b %d"), QDate(2004,  1,  1));
    QCOMPARE(calendar->readDate("2004 Jan 01",      "%Y %b %d"), QDate(2004,  1,  1));

    QCOMPARE(calendar->readDate("2004 December 01", "%Y %B %d"), QDate(2004, 12,  1));
    QCOMPARE(calendar->readDate("2004 Dec 01",      "%Y %B %d"), QDate(2004, 12,  1));
    QCOMPARE(calendar->readDate("2004 December 01", "%Y %b %d"), QDate(2004, 12,  1));
    QCOMPARE(calendar->readDate("2004 Dec 01",      "%Y %b %d"), QDate(2004, 12,  1));

    QCOMPARE(calendar->readDate("Wednesday 2004 December 01", "%A %Y %B %d"), QDate(2004, 12,  1));
    QCOMPARE(calendar->readDate("Wed 2004 Dec 01",            "%A %Y %B %d"), QDate(2004, 12,  1));
    QCOMPARE(calendar->readDate("Wednesday 2004 December 01", "%a %Y %b %d"), QDate(2004, 12,  1));
    QCOMPARE(calendar->readDate("Wed 2004 Dec 01",            "%a %Y %b %d"), QDate(2004, 12,  1));

    //Test ISO Week and Day-of-Week
    QCOMPARE(calendar->readDate("2004-W01-1", "%Y-W%V-%u"), QDate(2003, 12, 29));
    QCOMPARE(calendar->readDate("2004-W01-2", "%Y-W%V-%u"), QDate(2003, 12, 30));
    QCOMPARE(calendar->readDate("2004-W01-3", "%Y-W%V-%u"), QDate(2003, 12, 31));
    QCOMPARE(calendar->readDate("2004-W01-4", "%Y-W%V-%u"), QDate(2004, 1, 1));
    QCOMPARE(calendar->readDate("2004-W01-5", "%Y-W%V-%u"), QDate(2004, 1, 2));
    QCOMPARE(calendar->readDate("2004-W01-6", "%Y-W%V-%u"), QDate(2004, 1, 3));
    QCOMPARE(calendar->readDate("2004-W01-7", "%Y-W%V-%u"), QDate(2004, 1, 4));

    QCOMPARE(calendar->readDate("2004-W24-1", "%Y-W%V-%u"), QDate(2004, 6, 7));
    QCOMPARE(calendar->readDate("2004-W24-2", "%Y-W%V-%u"), QDate(2004, 6, 8));
    QCOMPARE(calendar->readDate("2004-W24-3", "%Y-W%V-%u"), QDate(2004, 6, 9));
    QCOMPARE(calendar->readDate("2004-W24-4", "%Y-W%V-%u"), QDate(2004, 6, 10));
    QCOMPARE(calendar->readDate("2004-W24-5", "%Y-W%V-%u"), QDate(2004, 6, 11));
    QCOMPARE(calendar->readDate("2004-W24-6", "%Y-W%V-%u"), QDate(2004, 6, 12));
    QCOMPARE(calendar->readDate("2004-W24-7", "%Y-W%V-%u"), QDate(2004, 6, 13));

    QCOMPARE(calendar->readDate("2004-W53-1", "%Y-W%V-%u"), QDate(2004, 12, 27));
    QCOMPARE(calendar->readDate("2004-W53-2", "%Y-W%V-%u"), QDate(2004, 12, 28));
    QCOMPARE(calendar->readDate("2004-W53-3", "%Y-W%V-%u"), QDate(2004, 12, 29));
    QCOMPARE(calendar->readDate("2004-W53-4", "%Y-W%V-%u"), QDate(2004, 12, 30));
    QCOMPARE(calendar->readDate("2004-W53-5", "%Y-W%V-%u"), QDate(2004, 12, 31));
    QCOMPARE(calendar->readDate("2004-W53-6", "%Y-W%V-%u"), QDate(2005, 1, 1));
    QCOMPARE(calendar->readDate("2004-W53-7", "%Y-W%V-%u"), QDate(2005, 1, 2));

    calendar->setDateIsoWeek(testDate, currentYear, 46, 5);
    QCOMPARE(calendar->readDate("W46-5", "W%V-%u"), testDate);

    QCOMPARE(calendar->readDate("2004-W00-1", "%Y-W%V-%u"),   QDate());
    QCOMPARE(calendar->readDate("2004-W01-0", "%Y-W%V-%u"),   QDate());
    QCOMPARE(calendar->readDate("2004-W01-8", "%Y-W%V-%u"),   QDate());
    QCOMPARE(calendar->readDate("2004-W54-1", "%Y-W%V-%u"),   QDate());
    QCOMPARE(calendar->readDate("2004-W53-01", "%Y-W%V-%u"),  QDate());
    QCOMPARE(calendar->readDate("2004-W053-01", "%Y-W%V-%u"), QDate());

    //Test all 3 options, should go with most accurate
    QCOMPARE(calendar->readDate("2004-01-01-365-W52-1", "%Y-%m-%d-%j-W%V-%u"), QDate(2004, 1, 1));
    QCOMPARE(calendar->readDate("2004-001-W52-1",       "%Y-%j-W%V-%u"),       QDate(2004, 1, 1));

    //Test spaces and literals
    QCOMPARE(calendar->readDate("2004- 1-1",    "%Y-%m-%d"), QDate(2004, 1, 1));
    QCOMPARE(calendar->readDate("2004-1-1",     "%Y-%m-%e"), QDate(2004, 1, 1));
    QCOMPARE(calendar->readDate("2004 01 01",   "%Y %m %d"), QDate(2004, 1, 1));
    QCOMPARE(calendar->readDate("2004  01  01", "%Y %m %d"), QDate(2004, 1, 1));
    QCOMPARE(calendar->readDate("2004W01-1",   "%YW%m-%d"),  QDate(2004, 1, 1));
    QCOMPARE(calendar->readDate("2004w01-1",   "%YW%m-%d"),  QDate(2004, 1, 1));
    QCOMPARE(calendar->readDate("2004W01-1",   "%Yw%m-%d"),  QDate(2004, 1, 1));

    //Test enums and non-strict version of readDate()
    locale->setDateFormat("%d/%m/%Y");
    locale->setDateFormatShort("%Y/%m/%d");
    QCOMPARE(calendar->readDate("01/02/2004"),  QDate(2004, 2, 1));        //KLocale::NormalFormat
    QCOMPARE(calendar->readDate("01/02/-2004"), QDate(-2004, 2, 1));       //KLocale::NormalFormat
    QCOMPARE(calendar->readDate("2004/02/01"),  QDate(2004, 2, 1));        //KLocale::ShortFormat
    QCOMPARE(calendar->readDate("-2004/02/01"), QDate(-2004, 2, 1));       //KLocale::ShortFormat
    QCOMPARE(calendar->readDate("2004-02-01"),  QDate(2004, 2, 1));        //KLocale::IsoFormat
    QCOMPARE(calendar->readDate("-2004-02-01"), QDate(-2004, 2, 1));       //KLocale::IsoFormat
    QCOMPARE(calendar->readDate("2004-W02-1"),  QDate(2004, 1, 5));        //KLocale::IsoWeekFormat
    QCOMPARE(calendar->readDate("2004-001"),    QDate(2004, 1, 1));        //KLocale::IsoOrdinalFormat
    QCOMPARE(calendar->readDate("-2004-001"),   QDate(-2004, 1, 1));       //KLocale::IsoOrdinalFormat

    //Test enums and strict version of readDate()
    QCOMPARE(calendar->readDate("01/02/2004",   KLocale::NormalFormat),     QDate(2004, 2, 1));
    QCOMPARE(calendar->readDate("01/02/-2004",  KLocale::NormalFormat),     QDate(-2004, 2, 1));
    QCOMPARE(calendar->readDate("2004/02/01",   KLocale::ShortFormat),      QDate(2004, 2, 1));
    QCOMPARE(calendar->readDate("-2004/02/01",  KLocale::ShortFormat),      QDate(-2004, 2, 1));
    QCOMPARE(calendar->readDate("2004-02-01",   KLocale::IsoFormat),        QDate(2004, 2, 1));
    QCOMPARE(calendar->readDate("-2004-02-01",  KLocale::IsoFormat),        QDate(-2004, 2, 1));
    QCOMPARE(calendar->readDate("2004-W02-1",   KLocale::IsoWeekFormat),    QDate(2004, 1, 5));
    QCOMPARE(calendar->readDate("2004-001",     KLocale::IsoOrdinalFormat), QDate(2004, 1, 1));
    QCOMPARE(calendar->readDate("-2004-001",    KLocale::IsoOrdinalFormat), QDate(-2004, 1, 1));

    //Test ISO standard date form with no spaces
    locale->setDateFormatShort("%Y%m%d");
    QCOMPARE(calendar->readDate("20040201"),  QDate(2004, 2, 1));
    QCOMPARE(calendar->readDate("-20040201"), QDate(-2004, 2, 1));

    // Test POSIX format
    testDate.setDate(2010, 1, 2);
    QCOMPARE(calendar->readDate("2010-1-2", "%Y-%n-%d", 0, KLocale::KdeFormat),       QDate(2010, 1, 2));
    QCOMPARE(calendar->readDate("2010-\n-2",  "%Y-%n-%d", 0, KLocale::PosixFormat),   QDate());
    QCOMPARE(calendar->readDate("2010-1\n-2", "%Y-%n\n-%d", 0, KLocale::KdeFormat),   QDate(2010, 1, 2));
    QCOMPARE(calendar->readDate("2010-1\n-2", "%Y-%m%n-%d", 0, KLocale::PosixFormat), QDate(2010, 1, 2));
    QCOMPARE(calendar->readDate("2010-1-2", "%Y-%m-%e", 0, KLocale::KdeFormat),       QDate(2010, 1, 2));
    QCOMPARE(calendar->readDate("2010-1-2", "%Y-%m-%e", 0, KLocale::PosixFormat),     QDate(2010, 1, 2));

    delete calendar;
    delete locale;
}

void KCalendarTest::testStringForms()
{
    testStrings(KLocale::ArabicDigits);
    testStrings(KLocale::ArabicIndicDigits);
}

void KCalendarTest::testStrings(KLocale::DigitSet testDigitSet)
{
    KLocale *locale = new KLocale(*KLocale::global());
    locale->setDigitSet(testDigitSet);
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar, locale);

    //Test default settings
    QDate testDate(2005, 1, 3);

    //Test Year String
    QCOMPARE(calendar->yearString(testDate), locale->convertDigits("2005", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->yearString(testDate, KCalendarSystem::LongFormat), locale->convertDigits("2005", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->yearString(testDate, KCalendarSystem::ShortFormat), locale->convertDigits("05", locale->dateTimeDigitSet()));

    //Test Month String
    QCOMPARE(calendar->monthString(testDate), locale->convertDigits("01", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->monthString(testDate, KCalendarSystem::LongFormat), locale->convertDigits("01", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->monthString(testDate, KCalendarSystem::ShortFormat), locale->convertDigits("1", locale->dateTimeDigitSet()));

    //Test Day String
    QCOMPARE(calendar->dayString(testDate), locale->convertDigits("03", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->dayString(testDate, KCalendarSystem::LongFormat), locale->convertDigits("03", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->dayString(testDate, KCalendarSystem::ShortFormat), locale->convertDigits("3", locale->dateTimeDigitSet()));

    //Test Day of Year String
    QCOMPARE(calendar->dayOfYearString(testDate), locale->convertDigits("003", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->dayOfYearString(testDate, KCalendarSystem::LongFormat), locale->convertDigits("003", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->dayOfYearString(testDate, KCalendarSystem::ShortFormat), locale->convertDigits("3", locale->dateTimeDigitSet()));

    //Test Day of Week String
    QCOMPARE(calendar->dayOfWeekString(testDate), locale->convertDigits("1", locale->dateTimeDigitSet()));

    //Test Week Number String
    QCOMPARE(calendar->weekNumberString(testDate), locale->convertDigits("01", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->weekNumberString(testDate, KCalendarSystem::LongFormat), locale->convertDigits("01", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->weekNumberString(testDate, KCalendarSystem::ShortFormat), locale->convertDigits("1", locale->dateTimeDigitSet()));

    //Test Months In Year String
    QCOMPARE(calendar->monthsInYearString(testDate), locale->convertDigits("12", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->monthsInYearString(testDate, KCalendarSystem::LongFormat), locale->convertDigits("12", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->monthsInYearString(testDate, KCalendarSystem::ShortFormat), locale->convertDigits("12", locale->dateTimeDigitSet()));

    //Test Weeks In Year String
    QCOMPARE(calendar->weeksInYearString(testDate), locale->convertDigits("52", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->weeksInYearString(testDate, KCalendarSystem::LongFormat), locale->convertDigits("52", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->weeksInYearString(testDate, KCalendarSystem::ShortFormat), locale->convertDigits("52", locale->dateTimeDigitSet()));

    //Test Days In Year String
    QCOMPARE(calendar->daysInYearString(testDate), locale->convertDigits("365", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->daysInYearString(testDate, KCalendarSystem::LongFormat), locale->convertDigits("365", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->daysInYearString(testDate, KCalendarSystem::ShortFormat), locale->convertDigits("365", locale->dateTimeDigitSet()));

    //Test Days In Month String
    QCOMPARE(calendar->daysInMonthString(testDate), locale->convertDigits("31", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->daysInMonthString(testDate, KCalendarSystem::LongFormat), locale->convertDigits("31", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->daysInMonthString(testDate, KCalendarSystem::ShortFormat), locale->convertDigits("31", locale->dateTimeDigitSet()));

    //Test Days In Week String
    QCOMPARE(calendar->daysInWeekString(testDate), locale->convertDigits("7", locale->dateTimeDigitSet()));

    //Test Year In Era String
    testDate.setDate(5, 1, 1);
    QCOMPARE(calendar->yearInEraString(testDate), locale->convertDigits("5", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->yearInEraString(testDate, KCalendarSystem::LongFormat), locale->convertDigits("0005", locale->dateTimeDigitSet()));
    QCOMPARE(calendar->yearInEraString(testDate, KCalendarSystem::ShortFormat), locale->convertDigits("5", locale->dateTimeDigitSet()));

    delete calendar;
    delete locale;
}

void KCalendarTest::testIsoWeekDate()
{
    KLocale *locale = new KLocale(*KLocale::global());
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar, locale);

    int earliestValidYear = calendar->year(calendar->earliestValidDate());
    int latestValidYear = calendar->year(calendar->latestValidDate());

    QCOMPARE(calendar->isValidIsoWeekDate(earliestValidYear - 1, 1, 1), false);
    QCOMPARE(calendar->isValidIsoWeekDate(earliestValidYear,     1, 1), false);     //In prev year
    QCOMPARE(calendar->isValidIsoWeekDate(earliestValidYear,     1, 2), true);
    QCOMPARE(calendar->isValidIsoWeekDate(-1,                    1, 1), true);
    QCOMPARE(calendar->isValidIsoWeekDate(0,                     1, 1), false);
    QCOMPARE(calendar->isValidIsoWeekDate(1,                     1, 1), true);
    QCOMPARE(calendar->isValidIsoWeekDate(latestValidYear,      52, 5), true);
    QCOMPARE(calendar->isValidIsoWeekDate(latestValidYear,      52, 6), false);     //In following year
    QCOMPARE(calendar->isValidIsoWeekDate(latestValidYear,      52, 7), false);     //In following year
    QCOMPARE(calendar->isValidIsoWeekDate(latestValidYear + 1,   1, 1), false);

    QCOMPARE(calendar->isValidIsoWeekDate(2004,  0, 1), false);
    QCOMPARE(calendar->isValidIsoWeekDate(2004,  1, 1), true);
    QCOMPARE(calendar->isValidIsoWeekDate(2004, 53, 1), true);
    QCOMPARE(calendar->isValidIsoWeekDate(2000, 54, 1), false);

    QCOMPARE(calendar->isValidIsoWeekDate(2004, 1, 0), false);
    QCOMPARE(calendar->isValidIsoWeekDate(2004, 1, 1), true);
    QCOMPARE(calendar->isValidIsoWeekDate(2004, 1, 7), true);
    QCOMPARE(calendar->isValidIsoWeekDate(2004, 1, 8), false);

    QCOMPARE(setIsoWeekDate(calendar, earliestValidYear - 1, 1, 1), QDate());
    QCOMPARE(setIsoWeekDate(calendar, earliestValidYear,     1, 1), QDate());     //In prev year
    QCOMPARE(setIsoWeekDate(calendar, earliestValidYear,     1, 2), QDate(-9999,  1,  1));
    QCOMPARE(setIsoWeekDate(calendar, -1,                    1, 1), QDate(-1, 1, 3));
    QCOMPARE(setIsoWeekDate(calendar, 0,                     1, 1), QDate());
    QCOMPARE(setIsoWeekDate(calendar, 1,                     1, 1), QDate(1,  1,  1));
    QCOMPARE(setIsoWeekDate(calendar, latestValidYear,      52, 5), QDate(9999, 12, 31));
    QCOMPARE(setIsoWeekDate(calendar, latestValidYear,      52, 6), QDate());     //In next year
    QCOMPARE(setIsoWeekDate(calendar, latestValidYear,      52, 7), QDate());     //In next year
    QCOMPARE(setIsoWeekDate(calendar, latestValidYear + 1,   1, 1), QDate());

    QCOMPARE(setIsoWeekDate(calendar, 2004,  0, 1), QDate());
    QCOMPARE(setIsoWeekDate(calendar, 2004,  1, 1), QDate(2003, 12, 29));
    QCOMPARE(setIsoWeekDate(calendar, 2004, 53, 1), QDate(2004, 12, 27));
    QCOMPARE(setIsoWeekDate(calendar, 2000, 54, 1), QDate());

    QCOMPARE(setIsoWeekDate(calendar, 2004,  1, 0), QDate());
    QCOMPARE(setIsoWeekDate(calendar, 2004,  1, 1), QDate(2003, 12, 29));
    QCOMPARE(setIsoWeekDate(calendar, 2004,  1, 7), QDate(2004,  1,  4));
    QCOMPARE(setIsoWeekDate(calendar, 2004,  1, 8), QDate());

    QCOMPARE(setIsoWeekDate(calendar, 2004,  1, 1), QDate(2003, 12, 29));
    QCOMPARE(setIsoWeekDate(calendar, 2004,  1, 2), QDate(2003, 12, 30));
    QCOMPARE(setIsoWeekDate(calendar, 2004,  1, 3), QDate(2003, 12, 31));
    QCOMPARE(setIsoWeekDate(calendar, 2004,  1, 4), QDate(2004,  1,  1));
    QCOMPARE(setIsoWeekDate(calendar, 2004,  1, 5), QDate(2004,  1,  2));
    QCOMPARE(setIsoWeekDate(calendar, 2004,  1, 6), QDate(2004,  1,  3));
    QCOMPARE(setIsoWeekDate(calendar, 2004,  1, 7), QDate(2004,  1,  4));

    QCOMPARE(setIsoWeekDate(calendar, 2004, 24, 1), QDate(2004, 6,  7));
    QCOMPARE(setIsoWeekDate(calendar, 2004, 24, 2), QDate(2004, 6,  8));
    QCOMPARE(setIsoWeekDate(calendar, 2004, 24, 3), QDate(2004, 6,  9));
    QCOMPARE(setIsoWeekDate(calendar, 2004, 24, 4), QDate(2004, 6, 10));
    QCOMPARE(setIsoWeekDate(calendar, 2004, 24, 5), QDate(2004, 6, 11));
    QCOMPARE(setIsoWeekDate(calendar, 2004, 24, 6), QDate(2004, 6, 12));
    QCOMPARE(setIsoWeekDate(calendar, 2004, 24, 7), QDate(2004, 6, 13));

    QCOMPARE(setIsoWeekDate(calendar, 2004, 53, 1), QDate(2004, 12, 27));
    QCOMPARE(setIsoWeekDate(calendar, 2004, 53, 2), QDate(2004, 12, 28));
    QCOMPARE(setIsoWeekDate(calendar, 2004, 53, 3), QDate(2004, 12, 29));
    QCOMPARE(setIsoWeekDate(calendar, 2004, 53, 4), QDate(2004, 12, 30));
    QCOMPARE(setIsoWeekDate(calendar, 2004, 53, 5), QDate(2004, 12, 31));
    QCOMPARE(setIsoWeekDate(calendar, 2004, 53, 6), QDate(2005,  1,  1));
    QCOMPARE(setIsoWeekDate(calendar, 2004, 53, 7), QDate(2005,  1,  2));

    delete calendar;
    delete locale;
}

void KCalendarTest::testHebrewStrings()
{
    /*
        Ref table for numbers to Hebrew chars

        Value     1       2       3        4        5       6         7        8      9

        x 1    Alef א  Bet  ב  Gimel ג  Dalet ד  He   ה  Vav  ו    Zayen ז  Het  ח  Tet  ט
               0x05D0  0x05D1  0x05D2   0x05D3   0x05D4  0x05D5    0x05D6   0x05D7  0x05D8

        x 10   Yod  י  Kaf  כ  Lamed ל  Mem  מ   Nun  נ  Samekh ס  Ayin ע   Pe   פ  Tzadi צ
               0x05D9  0x05DB  0x05DC   0x05DE   0x05E0  0x05E1    0x05E2   0x05E4  0x05E6

        x 100  Qof  ק  Resh ר  Shin ש   Tav  ת
               0x05E7  0x05E8  0x05E9   0x05EA

        Note special cases 15 = 9 + 6 טו and 16 = 9 + 7 טז
    */

    QString oldLocale = setlocale(LC_ALL, "he.utf8");
    KLocale *locale = new KLocale(*KLocale::global());
    locale->setLanguage(QStringList() << "he");
    locale->setDateFormat("%d %B %Y");
    KLocalizedString::insertCatalog("kdecalendarsystems");
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::HebrewCalendar, locale);
    if (locale->language() == "he") {
        QDate testDate;
        QString yearString, monthString, dayString, testString, resultString;
        calendar->setDate(testDate, 5765, 1, 1);
        // Have to use unicode values, for some reason directly using the chars doesn't work?
        yearString.append(QChar(0x05EA)).append(QChar(0x05E9)).append(QChar(0x05E1)).append(QChar('\"')).append(QChar(0x05D4));
        monthString = QString::fromUtf8("תשרי");
        dayString.append(QChar(0x05D0)).append(QChar('\''));
        testString = yearString + ' ' + monthString + ' ' + dayString;
        QCOMPARE(calendar->readDate(calendar->formatDate(testDate, KLocale::LongDate), KLocale::NormalFormat), testDate);
        QCOMPARE(calendar->formatDate(testDate, "%Y"), yearString);
        // Skip month name for now as won't translate for some reason, confirmed works visually in GUI
        //QCOMPARE( calendar->formatDate( testDate, "%B" ), monthString );
        QCOMPARE(calendar->formatDate(testDate, "%d"), dayString);
        //QCOMPARE( calendar->formatDate( testDate,  locale->dateFormat() ), testString );
        //QCOMPARE( calendar->readDate( testString ), testDate );
        for (int yy = 5400; yy <= 6400; ++yy) {
            calendar->setDate(testDate, yy, 1, 1);
            QCOMPARE(calendar->readDate(calendar->formatDate(testDate, KLocale::LongDate), KLocale::NormalFormat), testDate);
        }
    } else {
        qDebug() << "Cannot set Hebrew language, please install and re-test!";
    }
    setlocale(LC_ALL, oldLocale.toLatin1());
}

QDate KCalendarTest::setIsoWeekDate(const KCalendarSystem * calendar, int year, int isoWeek, int dayOfWeek)
{
    QDate resultDate;
    calendar->setDateIsoWeek(resultDate, year, isoWeek, dayOfWeek);
    return resultDate;
}

void KCalendarTest::testDayOfYearDate()
{
    KLocale *locale = new KLocale(*KLocale::global());
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar, locale);

    int earliestValidYear = calendar->year(calendar->earliestValidDate());
    int latestValidYear = calendar->year(calendar->latestValidDate());

    QCOMPARE(calendar->isValid(earliestValidYear - 1, 1), false);
    QCOMPARE(calendar->isValid(earliestValidYear,     1), true);
    QCOMPARE(calendar->isValid(-1,                    1), true);
    QCOMPARE(calendar->isValid(0,                     1), false);
    QCOMPARE(calendar->isValid(1,                     1), true);
    QCOMPARE(calendar->isValid(latestValidYear,       1), true);
    QCOMPARE(calendar->isValid(latestValidYear + 1,   1), false);

    QCOMPARE(calendar->isValid(2005,   0), false);
    QCOMPARE(calendar->isValid(2005,   1), true);
    QCOMPARE(calendar->isValid(2005, 365), true);
    QCOMPARE(calendar->isValid(2005, 366), false);
    QCOMPARE(calendar->isValid(2000, 366), true);
    QCOMPARE(calendar->isValid(2000, 367), false);

    QCOMPARE(setDayOfYearDate(calendar, earliestValidYear - 1, 1), QDate());
    QCOMPARE(setDayOfYearDate(calendar, earliestValidYear,     1), QDate(earliestValidYear, 1, 1));
    QCOMPARE(setDayOfYearDate(calendar, -1,                    1), QDate(-1,                1, 1));
    QCOMPARE(setDayOfYearDate(calendar, 0,                     1), QDate());
    QCOMPARE(setDayOfYearDate(calendar, 1,                     1), QDate(1,                 1, 1));
    QCOMPARE(setDayOfYearDate(calendar, latestValidYear,       1), QDate(latestValidYear,   1, 1));
    QCOMPARE(setDayOfYearDate(calendar, latestValidYear + 1,   1), QDate());

    QCOMPARE(setDayOfYearDate(calendar, 2005,   0), QDate());
    QCOMPARE(setDayOfYearDate(calendar, 2005,   1), QDate(2005,  1,  1));
    QCOMPARE(setDayOfYearDate(calendar, 2005, 365), QDate(2005, 12, 31));
    QCOMPARE(setDayOfYearDate(calendar, 2005, 366), QDate());
    QCOMPARE(setDayOfYearDate(calendar, 2000, 366), QDate(2000, 12, 31));
    QCOMPARE(setDayOfYearDate(calendar, 2000, 367), QDate());

    delete calendar;
    delete locale;
}

QDate KCalendarTest::setDayOfYearDate(const KCalendarSystem * calendar, int year, int dayOfYear)
{
    QDate resultDate;
    calendar->setDate(resultDate, year, dayOfYear);
    return resultDate;
}

void KCalendarTest::testDateDifference()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar);
    QDate testDate1, testDate2;

    // Date Difference

    compareDateDifference(calendar, QDate(2010, 6,  1), QDate(2010, 6,  1),  0,  0,  0);
    compareDateDifference(calendar, QDate(2010, 6, 10), QDate(2010, 6, 20),  0,  0, 10);
    compareDateDifference(calendar, QDate(2010, 6,  1), QDate(2010, 7,  1),  0,  1,  0);
    compareDateDifference(calendar, QDate(2010, 6, 10), QDate(2010, 7, 20),  0,  1, 10);
    compareDateDifference(calendar, QDate(2010, 6, 20), QDate(2010, 7, 10),  0,  0, 20);
    compareDateDifference(calendar, QDate(2010, 6, 30), QDate(2010, 7, 31),  0,  1,  1);
    compareDateDifference(calendar, QDate(2010, 8, 31), QDate(2010, 9, 30),  0,  1,  0);
    compareDateDifference(calendar, QDate(2010, 7,  1), QDate(2011, 6,  1),  0, 11,  0);
    compareDateDifference(calendar, QDate(2010, 7, 10), QDate(2011, 6, 20),  0, 11, 10);
    compareDateDifference(calendar, QDate(2010, 7, 20), QDate(2011, 6, 10),  0, 10, 21);
    compareDateDifference(calendar, QDate(2010, 7, 31), QDate(2011, 6, 30),  0, 11,  0);
    compareDateDifference(calendar, QDate(2010, 8, 30), QDate(2011, 7, 31),  0, 11,  1);
    compareDateDifference(calendar, QDate(2010, 6, 10), QDate(2011, 6, 10),  1,  0,  0);
    compareDateDifference(calendar, QDate(2010, 6, 10), QDate(2011, 6, 20),  1,  0, 10);
    compareDateDifference(calendar, QDate(2010, 6, 10), QDate(2011, 7, 10),  1,  1,  0);
    compareDateDifference(calendar, QDate(2010, 6, 10), QDate(2011, 7, 20),  1,  1, 10);
    compareDateDifference(calendar, QDate(2010, 6, 20), QDate(2011, 8, 10),  1,  1, 21);
    compareDateDifference(calendar, QDate(2010, 6, 30), QDate(2011, 7, 31),  1,  1,  1);
    compareDateDifference(calendar, QDate(2010, 8, 31), QDate(2011, 9, 30),  1,  1,  0);
    compareDateDifference(calendar, QDate(2010, 7, 31), QDate(2012, 6, 30),  1, 11,  0);
    compareDateDifference(calendar, QDate(2010, 8, 30), QDate(2012, 7, 31),  1, 11,  1);
    compareDateDifference(calendar, QDate(2000, 2, 29), QDate(2001, 2, 28),  1,  0,  0);
    compareDateDifference(calendar, QDate(2000, 2, 29), QDate(2001, 3,  1),  1,  0,  1);
    compareDateDifference(calendar, QDate(2000, 2, 29), QDate(2001, 4,  1),  1,  1,  3);
    calendar->setDate(testDate1, -1, 1, 1);
    calendar->setDate(testDate2,  1, 1, 1);
    compareDateDifference(calendar, testDate1, testDate2,  1,  0,  0);

    // Years Difference

    compareYearDifference(calendar, QDate(2010, 1,  1), QDate(2010,  1,  1), 0);
    compareYearDifference(calendar, QDate(2010, 1,  1), QDate(2010, 12, 31), 0);
    compareYearDifference(calendar, QDate(2010, 6,  1), QDate(2010,  7,  1), 0);
    compareYearDifference(calendar, QDate(2010, 6,  1), QDate(2011,  5,  1), 0);
    compareYearDifference(calendar, QDate(2010, 6,  1), QDate(2011,  7,  1), 1);
    compareYearDifference(calendar, QDate(2010, 6, 20), QDate(2012,  6, 20), 2);
    compareYearDifference(calendar, QDate(2010, 6, 20), QDate(2012,  6, 30), 2);
    compareYearDifference(calendar, QDate(2010, 6, 20), QDate(2012,  6, 10), 1);
    compareYearDifference(calendar, QDate(2000, 2, 29), QDate(2001,  2, 28), 1);
    calendar->setDate(testDate1, -1, 12, 31);
    calendar->setDate(testDate2,  1,  1,  1);
    compareYearDifference(calendar, testDate1, testDate2, 0);
    calendar->setDate(testDate1, -1,  1,  1);
    compareYearDifference(calendar, testDate1, testDate2, 1);

    // Months Difference

    compareMonthDifference(calendar, QDate(2010, 1,  1), QDate(2010,  1,  1), 0);
    compareMonthDifference(calendar, QDate(2010, 1,  1), QDate(2010,  2,  1), 1);
    compareMonthDifference(calendar, QDate(2010, 1, 10), QDate(2010,  2,  1), 0);
    compareMonthDifference(calendar, QDate(2010, 1, 10), QDate(2010,  2, 20), 1);
    compareMonthDifference(calendar, QDate(2010, 1,  1), QDate(2011,  1,  1), 12);
    compareMonthDifference(calendar, QDate(2010, 1,  1), QDate(2011,  2,  1), 13);
    compareMonthDifference(calendar, QDate(2010, 1, 10), QDate(2011,  2,  1), 12);
    compareMonthDifference(calendar, QDate(2010, 1, 10), QDate(2011,  2, 20), 13);
    compareMonthDifference(calendar, QDate(2000, 2, 29), QDate(2001,  2, 27), 11);
    compareMonthDifference(calendar, QDate(2000, 2, 29), QDate(2001,  2, 28), 12);
    compareMonthDifference(calendar, QDate(2000, 2, 27), QDate(2001,  2, 28), 12);
    compareMonthDifference(calendar, QDate(2000, 2, 29), QDate(2001,  3,  1), 12);
    calendar->setDate(testDate1, -1, 12, 31);
    calendar->setDate(testDate2,  1,  1,  1);
    compareMonthDifference(calendar, testDate1, testDate2, 0);
    calendar->setDate(testDate1, -1, 12,  1);
    compareMonthDifference(calendar, testDate1, testDate2, 1);

    // Days Difference

    // Directly uses QDate::toJulianDay() so only basic testing needed
    testDate1.setDate(2010, 1, 1);
    testDate2.setDate(2010, 1, 2);
    QCOMPARE(calendar->daysDifference(testDate1, testDate2), 1);
    QCOMPARE(calendar->daysDifference(testDate2, testDate1), -1);

    // Test for crossing Julian/Gregorian switch!!!
}

void KCalendarTest::compareDateDifference(const KCalendarSystem *calendar,
        const QDate &lowDate, const QDate &highDate,
        int yearsDiff, int monthsDiff, int daysDiff)
{
    int y, m, d, s;
    if (highDate >= lowDate) {
        calendar->dateDifference(lowDate, highDate, &y, &m, &d, &s);
        QCOMPARE(y, yearsDiff);
        QCOMPARE(m, monthsDiff);
        QCOMPARE(d, daysDiff);
        QCOMPARE(s, 1);
    }
    if (highDate > lowDate) {
        calendar->dateDifference(highDate, lowDate, &y, &m, &d, &s);
        QCOMPARE(y, yearsDiff);
        QCOMPARE(m, monthsDiff);
        QCOMPARE(d, daysDiff);
        QCOMPARE(s, -1);
    }
}

void KCalendarTest::compareYearDifference(const KCalendarSystem *calendar,
        const QDate &lowDate, const QDate &highDate,
        int yearsDiff)
{
    if (highDate >= lowDate) {
        QCOMPARE(calendar->yearsDifference(lowDate, highDate), yearsDiff);
    }
    if (highDate > lowDate) {
        QCOMPARE(calendar->yearsDifference(highDate, lowDate), -yearsDiff);
    }
}

void KCalendarTest::compareMonthDifference(const KCalendarSystem *calendar,
        const QDate &lowDate, const QDate &highDate,
        int monthsDiff)
{
    if (highDate >= lowDate) {
        QCOMPARE(calendar->monthsDifference(lowDate, highDate), monthsDiff);
    }
    if (highDate > lowDate) {
        QCOMPARE(calendar->monthsDifference(highDate, lowDate), -monthsDiff);
    }
}

void KCalendarTest::testFirstLast()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar);
    QDate testDate = QDate(2010, 6, 15);

    QCOMPARE(calendar->firstDayOfYear(testDate),  QDate(2010,  1, 1));
    QCOMPARE(calendar->lastDayOfYear(testDate),   QDate(2010, 12, 31));
    QCOMPARE(calendar->firstDayOfYear(2010),      QDate(2010,  1,  1));
    QCOMPARE(calendar->lastDayOfYear(2010),       QDate(2010, 12, 31));
    QCOMPARE(calendar->firstDayOfMonth(testDate), QDate(2010,  6,  1));
    QCOMPARE(calendar->lastDayOfMonth(testDate),  QDate(2010,  6, 30));
    QCOMPARE(calendar->firstDayOfMonth(2010, 6),  QDate(2010,  6,  1));
    QCOMPARE(calendar->lastDayOfMonth(2010, 6),   QDate(2010,  6, 30));
}

void KCalendarTest::testEra()
{
    KConfigGroup lcg(KSharedConfig::openConfig(), QString("Locale"));
    KConfigGroup cg = lcg.group(QString("KCalendarSystem %1").arg("gregorian"));
    cg.deleteGroup();

    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar);

    int earliestValidYear = calendar->year(calendar->earliestValidDate());
    int latestValidYear = calendar->year(calendar->latestValidDate());

    QDate testDate;
    calendar->setDate(testDate, 2005, 1, 1);
    QCOMPARE(calendar->yearInEra(testDate), 2005);
    QCOMPARE(calendar->eraName(testDate), QString("AD"));
    QCOMPARE(calendar->eraName(testDate, KCalendarSystem::ShortFormat), QString("AD"));
    QCOMPARE(calendar->eraName(testDate, KCalendarSystem::LongFormat), QString("Anno Domini"));
    calendar->setDate(testDate, -5, 1, 1);
    QCOMPARE(calendar->yearInEra(testDate), 5);
    QCOMPARE(calendar->eraName(testDate), QString("BC"));
    QCOMPARE(calendar->eraName(testDate, KCalendarSystem::ShortFormat), QString("BC"));
    QCOMPARE(calendar->eraName(testDate, KCalendarSystem::LongFormat), QString("Before Christ"));

    QCOMPARE(calendar->isValid("BC",            qAbs(earliestValidYear - 1), 1, 1), false);
    QCOMPARE(calendar->isValid("BC",            qAbs(earliestValidYear),     1, 1), true);
    QCOMPARE(calendar->isValid("Before Christ", qAbs(earliestValidYear),     1, 1), true);
    QCOMPARE(calendar->isValid("BC",            -1,                            1, 1), true);
    QCOMPARE(calendar->isValid("Before Christ", -1,                            1, 1), true);
    QCOMPARE(calendar->isValid("BC", 0,                                        1, 1), false);
    QCOMPARE(calendar->isValid("AD", 1,                                        1, 1), true);
    QCOMPARE(calendar->isValid("Anno Domini",   1,                             1, 1), true);
    QCOMPARE(calendar->isValid("AD",            latestValidYear,               1, 1), true);
    QCOMPARE(calendar->isValid("Anno Domini",   latestValidYear,               1, 1), true);
    QCOMPARE(calendar->isValid("AD",            latestValidYear + 1,           1, 1), false);

    QCOMPARE(calendar->isValid("AD", 2005,  0,  1), false);
    QCOMPARE(calendar->isValid("AD", 2005, 13,  1), false);
    QCOMPARE(calendar->isValid("AD", 2005,  1,  0), false);
    QCOMPARE(calendar->isValid("AD", 2005,  1, 32), false);
    QCOMPARE(calendar->isValid("AD", 2005,  1,  1), true);
    QCOMPARE(calendar->isValid("AD", 2005,  1, 31), true);
    QCOMPARE(calendar->isValid("AD", 2005, 12,  1), true);
    QCOMPARE(calendar->isValid("AD", 2005, 12, 31), true);

    QCOMPARE(setEraDate(calendar, "BC",            qAbs(earliestValidYear - 1), 1, 1), QDate());
    QCOMPARE(setEraDate(calendar, "BC",            qAbs(earliestValidYear),     1, 1), QDate(earliestValidYear, 1, 1));
    QCOMPARE(setEraDate(calendar, "Before Christ", qAbs(earliestValidYear),     1, 1), QDate(earliestValidYear, 1, 1));
    QCOMPARE(setEraDate(calendar, "BC",            1,                             1, 1), QDate(-1,                1, 1));
    QCOMPARE(setEraDate(calendar, "Before Christ", 1,                             1, 1), QDate(-1,                1, 1));
    QCOMPARE(setEraDate(calendar, "BC",            0,                             1, 1), QDate());
    QCOMPARE(setEraDate(calendar, "AD",            1,                             1, 1), QDate(1,                 1, 1));
    QCOMPARE(setEraDate(calendar, "Anno Domini",   1,                             1, 1), QDate(1,                 1, 1));
    QCOMPARE(setEraDate(calendar, "AD",            latestValidYear,               1, 1), QDate(latestValidYear,   1, 1));
    QCOMPARE(setEraDate(calendar, "Anno Domini",   latestValidYear,               1, 1), QDate(latestValidYear,   1, 1));
    QCOMPARE(setEraDate(calendar, "AD",            latestValidYear + 1,           1, 1), QDate());

    QCOMPARE(setEraDate(calendar, "AD", 2005,  0,  1), QDate());
    QCOMPARE(setEraDate(calendar, "AD", 2005, 13,  1), QDate());
    QCOMPARE(setEraDate(calendar, "AD", 2005,  1,  0), QDate());
    QCOMPARE(setEraDate(calendar, "AD", 2005,  1, 32), QDate());
    QCOMPARE(setEraDate(calendar, "AD", 2005,  1,  1), QDate(2005,  1,  1));
    QCOMPARE(setEraDate(calendar, "AD", 2005,  1, 31), QDate(2005,  1, 31));
    QCOMPARE(setEraDate(calendar, "AD", 2005, 12,  1), QDate(2005, 12,  1));
    QCOMPARE(setEraDate(calendar, "AD", 2005, 12, 31), QDate(2005, 12, 31));

    delete calendar;
    cg.writeEntry("Era1", QString::fromUtf8("-:1:-0001-01-01::Test Era 1:TE1:£%Ey£%EC£"));
    cg.writeEntry("Era2", QString::fromUtf8("+:1:0001-01-01::Test Era 2:TE2:^%Ey^%EC^"));
    calendar = KCalendarSystem::create(KLocale::GregorianCalendar);
    testEraDate(calendar, QDate(2010,  1,  1), 2010, "2010", "2010", "TE2", "Test Era 2");
    testEraDate(calendar, QDate(-5,  1,  1),    5, "5",    "0005", "TE1", "Test Era 1");
    QCOMPARE(calendar->formatDate(QDate(2010, 1, 1), "%EY"), QString::fromUtf8("^2010^TE2^"));
    QCOMPARE(calendar->formatDate(QDate(-5, 1, 1), "%EY"), QString::fromUtf8("£5£TE1£"));

    cg.deleteGroup();
    cg.markAsClean();
    lcg.markAsClean();
    delete calendar;
}

QDate KCalendarTest::setEraDate(const KCalendarSystem * calendar, const QString &era, int yearInEra, int month, int day)
{
    QDate resultDate;
    calendar->setDate(resultDate, era, yearInEra, month, day);
    return resultDate;
}

void KCalendarTest::testGregorianCalendar()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar);
    QDate testDate(2005, 9, 10);

    QCOMPARE(calendar->dayOfYear(testDate), 253);

    QVERIFY(calendar->setDate(testDate, 2000, 3, 1));
    QCOMPARE(calendar->year(testDate), 2000);
    QCOMPARE(calendar->month(testDate), 3);
    QCOMPARE(calendar->day(testDate), 1);
    QCOMPARE(calendar->daysInYear(testDate), 366);

    QDate newDate = calendar->addYears(testDate, 4);
    QCOMPARE(newDate.year(), 2004);
    QCOMPARE(calendar->daysInYear(newDate), 366);

    newDate = calendar->addMonths(testDate, -4);
    QCOMPARE(newDate.year(), 1999);
    QCOMPARE(newDate.month(), 11);
    QCOMPARE(newDate.day(), 1);

    newDate = calendar->addDays(newDate, 20);
    QCOMPARE(newDate.year(), 1999);
    QCOMPARE(newDate.month(), 11);
    QCOMPARE(newDate.day(), 21);

    delete calendar;
}

void KCalendarTest::testHebrew()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::HebrewCalendar);
    QDate testDate(2005, 9, 10);   // 5756-13-06
    QCOMPARE(calendar->dayOfYear(testDate), 360);

    QVERIFY(calendar->setDate(testDate, 5760, 12, 24));
    QCOMPARE(calendar->year(testDate), 5760);
    QCOMPARE(calendar->month(testDate), 12);
    QCOMPARE(calendar->day(testDate), 24);
    QCOMPARE(calendar->daysInYear(testDate), 385);

    testDate = calendar->addYears(testDate, 4);
    QCOMPARE(calendar->year(testDate), 5764);
    QCOMPARE(calendar->month(testDate), 12);
    QCOMPARE(calendar->day(testDate), 24);
    QCOMPARE(calendar->daysInYear(testDate), 355);

    testDate = calendar->addMonths(testDate, -4);
    QCOMPARE(calendar->year(testDate), 5764);
    QCOMPARE(calendar->month(testDate), 8);
    QCOMPARE(calendar->day(testDate), 24);
    QCOMPARE(calendar->daysInYear(testDate), 355);

    testDate = calendar->addDays(testDate, 20);
    QCOMPARE(calendar->year(testDate), 5764);
    QCOMPARE(calendar->month(testDate), 9);
    QCOMPARE(calendar->day(testDate), 15);
    QCOMPARE(calendar->daysInYear(testDate), 355);

    delete calendar;
}

void KCalendarTest::testIslamicCivil()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::IslamicCivilCalendar);

    QDate testDate(2008, 12, 29);   //1430-01-01, jd 2454830, not leap
    QCOMPARE(calendar->year(testDate), 1430);
    QCOMPARE(calendar->month(testDate), 1);
    QCOMPARE(calendar->day(testDate), 1);
    QCOMPARE(calendar->isLeapYear(testDate), false);
    QCOMPARE(calendar->daysInYear(testDate), 354);
    QCOMPARE(calendar->daysInMonth(testDate), 30);
    QCOMPARE(calendar->dayOfYear(testDate), 1);

    testDate.setDate(2009, 12, 17);   //1430-12-29, jd 2455183, not leap
    QCOMPARE(calendar->year(testDate), 1430);
    QCOMPARE(calendar->month(testDate), 12);
    QCOMPARE(calendar->day(testDate), 29);
    QCOMPARE(calendar->isLeapYear(testDate), false);
    QCOMPARE(calendar->daysInYear(testDate), 354);
    QCOMPARE(calendar->daysInMonth(testDate), 29);
    QCOMPARE(calendar->dayOfYear(testDate), 354);

    testDate.setDate(2009, 12, 18);   //1431-01-01, jd 2455184, leap
    QCOMPARE(calendar->year(testDate), 1431);
    QCOMPARE(calendar->month(testDate), 1);
    QCOMPARE(calendar->day(testDate), 1);
    QCOMPARE(calendar->isLeapYear(testDate), true);
    QCOMPARE(calendar->daysInYear(testDate), 355);
    QCOMPARE(calendar->daysInMonth(testDate), 30);
    QCOMPARE(calendar->dayOfYear(testDate), 1);

    testDate.setDate(2010, 12, 7);   //1431-12-30, jd 2455538, leap
    QCOMPARE(calendar->year(testDate), 1431);
    QCOMPARE(calendar->month(testDate), 12);
    QCOMPARE(calendar->day(testDate), 30);
    QCOMPARE(calendar->isLeapYear(testDate), true);
    QCOMPARE(calendar->daysInYear(testDate), 355);
    QCOMPARE(calendar->daysInMonth(testDate), 30);
    QCOMPARE(calendar->dayOfYear(testDate), 355);

    testDate.setDate(2005, 9, 10);   //1426-08-06
    QCOMPARE(calendar->year(testDate), 1426);
    QCOMPARE(calendar->month(testDate), 8);
    QCOMPARE(calendar->day(testDate), 6);
    QCOMPARE(calendar->daysInYear(testDate), 355);
    QCOMPARE(calendar->daysInMonth(testDate), 29);
    QCOMPARE(calendar->weeksInYear(testDate.year()), 50);
    QCOMPARE(calendar->weekNumber(testDate), 31);
    QCOMPARE(calendar->dayOfYear(testDate), 213);

    QVERIFY(calendar->setDate(testDate, 2000, 3, 1));
    QCOMPARE(calendar->year(testDate), 2000);
    QCOMPARE(calendar->month(testDate), 3);
    QCOMPARE(calendar->day(testDate), 1);

    QDate newDate = calendar->addYears(testDate, 4);
    QCOMPARE(newDate.year(), 2566);
    QCOMPARE(calendar->daysInYear(newDate), 355);

    newDate = calendar->addMonths(testDate, -4);
    QCOMPARE(newDate.year(), 2561);
    QCOMPARE(newDate.month(), 11);
    QCOMPARE(newDate.day(), 10);

    newDate = calendar->addDays(newDate, 20);
    QCOMPARE(newDate.year(), 2561);
    QCOMPARE(newDate.month(), 11);
    QCOMPARE(newDate.day(), 30);

    delete calendar;
}


void KCalendarTest::testIndianNational()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::IndianNationalCalendar);
    QDate testDate(2005, 9, 10);

    calendar->setDate(testDate, 2000, 1, 1);
    calendar->setDate(testDate, 2000, 1, 10);
    calendar->setDate(testDate, 2000, 2, 10);
    calendar->setDate(testDate, 2000, 5, 20);
    calendar->setDate(testDate, 2000, 7, 1);
    calendar->setDate(testDate, 2000, 7, 10);
    calendar->setDate(testDate, 2000, 9, 10);
    calendar->setDate(testDate, 2000, 12, 20);

    QCOMPARE(calendar->isLeapYear(2009), false);
    QCOMPARE(calendar->isLeapYear(testDate), false);

    QVERIFY(calendar->setDate(testDate, 2000, 3, 1));
    QCOMPARE(calendar->year(testDate), 2000);
    QCOMPARE(calendar->month(testDate), 3);
    QCOMPARE(calendar->day(testDate), 1);

    // Test some key dates
    testYmd(calendar,    0,  1,  1, 1749630);
    testYmd(calendar,    0, 12, 30, 1749994);
    testYmd(calendar, 1922,  1,  1, QDate(2000,  3, 21).toJulianDay());
    testYmd(calendar, 1922, 12, 30, QDate(2001,  3, 21).toJulianDay());
    testYmd(calendar, 1932,  1,  1, QDate(2010,  3, 22).toJulianDay());
    testYmd(calendar, 1932, 12, 30, QDate(2011,  3, 21).toJulianDay());

    testRoundTrip(calendar);

    delete calendar;
}

void KCalendarTest::testGregorianCalendarBasic()
{
    KConfigGroup lcg(KSharedConfig::openConfig(), QString("Locale"));
    KConfigGroup cg = lcg.group(QString("KCalendarSystem %1").arg("gregorian"));
    cg.deleteGroup();

    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar);

    testCalendarSystemType(calendar, KLocale::GregorianCalendar, "gregorian", "Gregorian");

    QCOMPARE(calendar->epoch(), QDate::fromJulianDay(1721426));
    QCOMPARE(calendar->epoch(), QDate(1, 1, 1));
    QCOMPARE(calendar->earliestValidDate(), QDate(-9999, 1, 1));
    QCOMPARE(calendar->latestValidDate(), QDate(9999, 12, 31));

    testValid(calendar, 0, 10000, 13, 32, QDate(-10000, 1, 1));

    QCOMPARE(calendar->isLeapYear(2007), false);
    QCOMPARE(calendar->isLeapYear(2008), true);
    QCOMPARE(calendar->isLeapYear(1900), false);
    QCOMPARE(calendar->isLeapYear(2000), true);
    QCOMPARE(calendar->isLeapYear(QDate(2007, 1, 1)), false);
    QCOMPARE(calendar->isLeapYear(QDate(2008, 1, 1)), true);

    QCOMPARE(calendar->daysInWeek(QDate(2007, 1, 1)), 7);

    QCOMPARE(calendar->monthsInYear(2007),                12);
    QCOMPARE(calendar->monthsInYear(QDate(2007, 1, 1)), 12);

    testYear(calendar, QDate(2007, 7, 9), 2007, QString("07"), QString("2007"));
    testMonth(calendar, QDate(2007, 7, 9),    7, QString("7"),  QString("07"));
    testDay(calendar, QDate(2007, 7, 9),    9, QString("9"),  QString("09"));

    testEraDate(calendar, QDate(2005, 1, 1), 2005, "2005", "2005", "AD", "Anno Domini");
    testEraDate(calendar, QDate(-5, 1, 1),    5, "5",    "0005", "BC", "Before Christ");

    testWeekDayName(calendar, 6, QDate(2007, 7, 28), "S", "Sat", "Saturday");
    testMonthName(calendar, 12, 2007, QDate(2007, 12, 20), "D", "Dec", "December", "of Dec", "of December");

    QCOMPARE(calendar->monthsInYear(QDate(2007, 1, 1)), 12);

    QCOMPARE(calendar->isProleptic(), true);
    QCOMPARE(calendar->isLunar(), false);
    QCOMPARE(calendar->isLunisolar(), false);
    QCOMPARE(calendar->isSolar(), true);

    testRoundTrip(calendar);

    delete calendar;
}

void KCalendarTest::testGregorianCalendarYmd()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar);
    testYmd(calendar, 2007, 1, 1, QDate(2007, 1, 1).toJulianDay());
    delete calendar;
}

void KCalendarTest::testGregorianCalendarSpecialCases()
{
    KConfigGroup lcg(KSharedConfig::openConfig(), QString("Locale"));
    KConfigGroup cg = lcg.group(QString("KCalendarSystem %1").arg("gregorian"));
    cg.deleteGroup();

    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar);

    testEraDate(calendar, QDate(2010,  1,  1), 2010, "2010", "2010", "AD", "Anno Domini");
    testEraDate(calendar, QDate(-5,  1,  1),    5, "5",    "0005", "BC", "Before Christ");

    delete calendar;
    cg.writeEntry("UseCommonEra", false, KConfigGroup::Global);
    calendar = KCalendarSystem::create(KLocale::GregorianCalendar);
    testEraDate(calendar, QDate(2010,  1,  1), 2010, "2010", "2010", "AD", "Anno Domini");
    testEraDate(calendar, QDate(-5,  1,  1),    5, "5",    "0005", "BC", "Before Christ");

    delete calendar;
    cg.writeEntry("UseCommonEra", true, KConfigGroup::Global);
    calendar = KCalendarSystem::create(KLocale::GregorianCalendar);
    testEraDate(calendar, QDate(2010,  1,  1), 2010, "2010", "2010", "CE", "Common Era");
    testEraDate(calendar, QDate(-5,  1,  1),    5, "5",    "0005", "BCE", "Before Common Era");

    cg.deleteGroup();
    cg.markAsClean();
    delete calendar;
}

void KCalendarTest::testHebrewBasic()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::HebrewCalendar);
    QDate testDate;

    testCalendarSystemType(calendar, KLocale::HebrewCalendar, "hebrew", "Hebrew");

    testEpoch(calendar,                1,  1,  1,  347998);
    testEarliestValidDate(calendar, 5344,  1,  1, 2299498);
    testLatestValidDate(calendar, 8119, 13, 29, 3313431);

    testDate = QDate::fromJulianDay(2450340);   //5756-12-29 Not Leap
    QCOMPARE(calendar->isLeapYear(5756), false);
    QCOMPARE(calendar->isLeapYear(testDate), false);
    QCOMPARE(calendar->monthsInYear(testDate), 12);
    QCOMPARE(calendar->month(testDate), 12);
    QCOMPARE(calendar->monthName(testDate), QString("Elul"));

    testDate = QDate::fromJulianDay(2450723);   //5757-13-29 Leap
    QCOMPARE(calendar->isLeapYear(5757), true);
    QCOMPARE(calendar->isLeapYear(testDate), true);
    QCOMPARE(calendar->monthsInYear(testDate), 13);
    QCOMPARE(calendar->month(testDate), 13);
    QCOMPARE(calendar->monthName(testDate), QString("Elul"));

    // 5707-01-01 Thursday
    testDate = QDate::fromJulianDay(2432090);

    QCOMPARE(calendar->daysInWeek(testDate), 7);

    testYear(calendar, testDate, 5707, "07", "5707");
    testMonth(calendar, testDate,    1,  "1",   "01");
    testDay(calendar, testDate,    1,  "1",   "01");

    testEraDate(calendar, testDate, 5707, "5707", "5707", "AM", "Anno Mundi");

    testWeekDayName(calendar, 4, testDate, "T", "Thu", "Thursday");
    testMonthName(calendar, 1, 5707, testDate, "T", "Tis", "Tishrey", "of Tis", "of Tishrey");

    QCOMPARE(calendar->isProleptic(), false);
    QCOMPARE(calendar->isLunar(), false);
    QCOMPARE(calendar->isLunisolar(), true);
    QCOMPARE(calendar->isSolar(), false);

    testRoundTrip(calendar);

    delete calendar;
}

void KCalendarTest::testHebrewYmd()
{
//    const KCalendarSystem *calendar = KCalendarSystem::create( KLocale::HebrewCalendar );
//    testYmd( calendar, 2007, 1, 1, QDate( 2007, 1, 1 ).toJulianDay() );
//    delete calendar;
}

void KCalendarTest::testHebrewCompare()
{
    /*
        const KCalendarSystemHebrew *calendar = new KCalendarSystemHebrew();
        QDate setOldDate, setNewDate;

        for ( QDate testDate( 1900, 1, 1 ); testDate.year() == 2100; testDate.addDays(1) ) {
            QCOMPARE( calendar->year(  testDate ), calendar->oldyear(  testDate ) );
            QCOMPARE( calendar->month( testDate ), calendar->oldmonth( testDate ) );
            QCOMPARE( calendar->day(   testDate ), calendar->oldday(   testDate ) );

            calendar->setDate(setOldDate, calendar->oldyear( testDate ), calendar->oldmonth( testDate ), calendar->oldday( testDate ) );
            calendar->setDate(setNewDate, calendar->oldyear( testDate ), calendar->oldmonth( testDate ), calendar->oldday( testDate ) );
            QCOMPARE( setNewDate.toJulianDay(), setOldDate.toJulianDay() );
        }

        delete calendar;
    */
}

// Test Islamic Civil Calendar System

void KCalendarTest::testIslamicCivilBasic()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::IslamicCivilCalendar);
    QDate testDate;

    testCalendarSystemType(calendar, KLocale::IslamicCivilCalendar, "hijri", "Islamic / Hijri (Civil)");

    QCOMPARE(calendar->epoch(), QDate::fromJulianDay(1948440));
    QCOMPARE(calendar->earliestValidDate(), QDate::fromJulianDay(1948440));
    QCOMPARE(calendar->latestValidDate(), QDate(10323, 10, 21));

    testValid(calendar, 0, 10000, 13, 31, QDate(1, 1, 1));

    QCOMPARE(calendar->isLeapYear(1427), false);
    QCOMPARE(calendar->isLeapYear(1428), true);
    QCOMPARE(calendar->isLeapYear(QDate(2007, 1, 1)), false);
    QCOMPARE(calendar->isLeapYear(QDate(2008, 1, 1)), true);

    // 1426-08-06 Saturday
    testDate = QDate(2005, 9, 10);

    QCOMPARE(calendar->daysInWeek(testDate), 7);

    QCOMPARE(calendar->monthsInYear(1426),     12);
    QCOMPARE(calendar->monthsInYear(testDate), 12);

    testYear(calendar, testDate, 1426, "26", "1426");
    testMonth(calendar, testDate,    8, "8",  "08");
    testDay(calendar, testDate,    6, "6",  "06");

    testEraDate(calendar, testDate, 1426, "1426", "1426", "AH", "Anno Hegirae");

    testWeekDayName(calendar, 6, testDate, "S", "Sab", "Yaum al-Sabt");
    testMonthName(calendar, 12, 1428, QDate(2007, 12, 20), "H", "Hij", "Thu al-Hijjah", "of Hij", "of Thu al-Hijjah");

    QCOMPARE(calendar->monthsInYear(testDate), 12);

    QCOMPARE(calendar->isProleptic(), false);
    QCOMPARE(calendar->isLunar(), true);
    QCOMPARE(calendar->isLunisolar(), false);
    QCOMPARE(calendar->isSolar(), false);

    testRoundTrip(calendar);

    delete calendar;
}

void KCalendarTest::testIslamicCivilYmd()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::IslamicCivilCalendar);
    testYmd(calendar, 1426, 8, 6, QDate(2005, 9, 10).toJulianDay());
    delete calendar;
}


// Jalali Calendar System

void KCalendarTest::testJalaliBasic()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::JalaliCalendar);
    QDate testDate;

    testCalendarSystemType(calendar, KLocale::JalaliCalendar, "jalali", "Jalali");

    //Birashk argorithm only good between AP 1244-01-01 to 1530-12-29 (AD 1865 to 2152)
    QCOMPARE(calendar->epoch(), QDate::fromJulianDay(1948321));
    QCOMPARE(calendar->earliestValidDate(), QDate(1865, 03, 21));
    QCOMPARE(calendar->latestValidDate(), QDate(2152, 03, 19));
    QCOMPARE(calendar->isValid(1243, 12, 29), false);
    QCOMPARE(calendar->isValid(1531, 1, 1), false);

    QCOMPARE(calendar->isLeapYear(1386), false);
    QCOMPARE(calendar->isLeapYear(1387), true);
    QCOMPARE(calendar->isLeapYear(QDate(2008, 1, 1)), false);
    QCOMPARE(calendar->isLeapYear(QDate(2009, 1, 1)), true);

    testDate = QDate(2005, 8, 31);   // 1384-06-09 Wednesday

    QCOMPARE(calendar->daysInWeek(testDate),    7);
    QCOMPARE(calendar->monthsInYear(1384),     12);
    QCOMPARE(calendar->monthsInYear(testDate), 12);

    testYear(calendar, testDate, 1384, "84", "1384");
    testMonth(calendar, testDate,    6, "6",  "06");
    testDay(calendar, testDate,    9, "9",  "09");

    testEraDate(calendar, testDate, 1384, "1384", "1384", "AP", "Anno Persico");

    testWeekDayName(calendar, 3, testDate, "4", "4sh", "Chahar shanbe");
    testMonthName(calendar, 6, 1384, testDate, "S", "Sha", "Shahrivar", "of Sha", "of Shahrivar");

    QCOMPARE(calendar->monthsInYear(testDate), 12);

    QCOMPARE(calendar->isProleptic(), false);
    QCOMPARE(calendar->isLunar(), false);
    QCOMPARE(calendar->isLunisolar(), false);
    QCOMPARE(calendar->isSolar(), true);

    testRoundTrip(calendar);

    //Special cases using Birashk argorithm
    QCOMPARE(calendar->isLeapYear(1403), true);
    QCOMPARE(calendar->isLeapYear(1404), false);
    QCOMPARE(calendar->isLeapYear(1436), true);
    QCOMPARE(calendar->isLeapYear(1437), false);

    delete calendar;
}

void KCalendarTest::testJalaliYmd()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::JalaliCalendar);
    testYmd(calendar, 1384, 6, 9, QDate(2005, 8, 31).toJulianDay());
    delete calendar;
}

void KCalendarTest::testJapanese()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::JapaneseCalendar);
    QDate testDate(2005, 9, 10);

    QCOMPARE(calendar->dayOfYear(testDate), 253);

    QVERIFY(calendar->setDate(testDate, 2000, 3, 1));
    QCOMPARE(calendar->year(testDate), 2000);
    QCOMPARE(calendar->month(testDate), 3);
    QCOMPARE(calendar->day(testDate), 1);
    QCOMPARE(calendar->daysInYear(testDate), 366);

    QDate newDate = calendar->addYears(testDate, 4);
    QCOMPARE(newDate.year(), 2004);
    QCOMPARE(calendar->daysInYear(newDate), 366);

    newDate = calendar->addMonths(testDate, -4);
    QCOMPARE(newDate.year(), 1999);
    QCOMPARE(newDate.month(), 11);
    QCOMPARE(newDate.day(), 1);

    newDate = calendar->addDays(newDate, 20);
    QCOMPARE(newDate.year(), 1999);
    QCOMPARE(newDate.month(), 11);
    QCOMPARE(newDate.day(), 21);

    delete calendar;
}

void KCalendarTest::testJapaneseBasic()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::JapaneseCalendar);
    QDate testDate;

    testCalendarSystemType(calendar, KLocale::JapaneseCalendar, "japanese", "Japanese");

    QCOMPARE(calendar->epoch(), QDate::fromJulianDay(1721426));
    QCOMPARE(calendar->earliestValidDate(), QDate::fromJulianDay(1721426));
    QCOMPARE(calendar->latestValidDate(), QDate(9999, 12, 31));

    testValid(calendar, 0, 10000, 13, 32, QDate(-5000, 1, 1));

    QCOMPARE(calendar->isLeapYear(2007), false);
    QCOMPARE(calendar->isLeapYear(2008), true);
    QCOMPARE(calendar->isLeapYear(1900), false);
    QCOMPARE(calendar->isLeapYear(2000), true);
    QCOMPARE(calendar->isLeapYear(QDate(2007, 1, 1)), false);
    QCOMPARE(calendar->isLeapYear(QDate(2008, 1, 1)), true);

    QCOMPARE(calendar->daysInWeek(QDate(2007, 1, 1)), 7);
    QCOMPARE(calendar->monthsInYear(QDate(2007, 1, 1)), 12);

    testDate = QDate(2007, 7, 9);   // 2007-07-09
    testYear(calendar, testDate, 2007, "07", "2007");
    testMonth(calendar, testDate,    7, "7",  "07");
    testDay(calendar, testDate,    9, "9",  "09");

    testEraDate(calendar, testDate,   19, "19", "0019", "Heisei", "Heisei");

    testWeekDayName(calendar, 6, QDate(2007, 7, 28), "S", "Sat", "Saturday");
    testMonthName(calendar, 12, 2007, QDate(2007, 12, 20), "D", "Dec", "December", "of Dec", "of December");

    QCOMPARE(calendar->monthsInYear(QDate(2007, 1, 1)), 12);

    QCOMPARE(calendar->isProleptic(), false);
    QCOMPARE(calendar->isLunar(), false);
    QCOMPARE(calendar->isLunisolar(), false);
    QCOMPARE(calendar->isSolar(), true);

    testRoundTrip(calendar);

    delete calendar;
}

void KCalendarTest::testJapaneseYmd()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::JapaneseCalendar);
    testYmd(calendar, 2007, 1, 1, QDate(2007, 1, 1).toJulianDay());
    delete calendar;
}

void KCalendarTest::testJapaneseSpecialCases()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::JapaneseCalendar);

    // Test Japanese Eras
    // Heisei > 1        1990-01-01 onwards
    // Heisei 1 (Gannen) 1989-01-08 to 1989-12-31
    // Shōwa > 1         1927-01-01 to 1989-01-07
    // Shōwa 1 (Gannen)  1926-12-25 to 1926-12-31
    // Taishō > 1        1913-01-01 to 1926-12-24
    // Taishō 1 (Gannen) 1912-07-30 to 1912-12-31
    // Meiji > 1         1869-01-01 to 1912-07-29
    // Meiji 1 (Gannen)  1868-09-08 to 1868-12-31
    // Gregorian         0001-01-01 to 1868-09-07

    testEraDate(calendar, QDate(1990,  1,  1),    2,    "2", "0002", "Heisei", "Heisei");
    testEraDate(calendar, QDate(1989,  1,  8),    1,    "1", "0001", "Heisei", "Heisei");
    //testEraDate( calendar, QDate( 1989,  1,  7 ),   64,   "64", "0064", "Shōwa",  "Shōwa" );
    //testEraDate( calendar, QDate( 1927,  1,  1 ),    2,    "2", "0002", "Shōwa",  "Shōwa" );
    //testEraDate( calendar, QDate( 1926, 12, 25 ),    1,    "1", "0001", "Shōwa",  "Shōwa" );
    //testEraDate( calendar, QDate( 1926, 12, 24 ),   15,   "15", "0015", "Taishō", "Taishō" );
    //testEraDate( calendar, QDate( 1913,  1,  1 ),    2,    "2", "0002", "Taishō", "Taishō" );
    //testEraDate( calendar, QDate( 1912,  7, 30 ),    1,    "1", "0001", "Taishō", "Taishō" );
    testEraDate(calendar, QDate(1912,  7, 29),   45,   "45", "0045", "Meiji",  "Meiji");
    testEraDate(calendar, QDate(1869,  1,  1),    2,    "2", "0002", "Meiji",  "Meiji");
    testEraDate(calendar, QDate(1868,  9,  8),    1,    "1", "0001", "Meiji",  "Meiji");
    testEraDate(calendar, QDate(1868,  9,  7), 1868, "1868", "1868", "AD",     "Anno Domini");

    // Should always be able to read Gannen as year 1
    QCOMPARE(calendar->readDate("1 2 Heisei 1", "%e %n %EC %Ey"), QDate(1989, 2, 1));
    QCOMPARE(calendar->readDate("1 2 Heisei 1", "%e %n %EY"), QDate(1989, 2, 1));
    QCOMPARE(calendar->readDate("1 2 Heisei Gannen", "%e %n %EC %Ey"), QDate(1989, 2, 1));
    QCOMPARE(calendar->readDate("1 2 Heisei Gannen", "%e %n %EY"), QDate(1989, 2, 1));

    // Should only format Gannen if defined in %EY format, never for %Ey
    QCOMPARE(calendar->formatDate(QDate(1989, 2, 1), "%e %n %EC %Ey"), QString("1 2 Heisei 1"));
    QCOMPARE(calendar->formatDate(QDate(1989, 2, 1), "%e %n %EY"),     QString("1 2 Heisei Gannen"));
    QCOMPARE(calendar->formatDate(QDate(1990, 2, 1), "%e %n %EC %Ey"), QString("1 2 Heisei 2"));
    QCOMPARE(calendar->formatDate(QDate(1990, 2, 1), "%e %n %EY"),     QString("1 2 Heisei 2"));

    delete calendar;
}

void KCalendarTest::testMinguoBasic()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::MinguoCalendar);

    testCalendarSystemType(calendar, KLocale::MinguoCalendar, "minguo", "Taiwanese");

    QCOMPARE(calendar->epoch(), QDate(1912, 1, 1));
    QCOMPARE(calendar->earliestValidDate(), QDate(1912, 1, 1));
    QCOMPARE(calendar->latestValidDate(), QDate(11910, 12, 31));

    testValid(calendar, -1, 10000, 13, 32, QDate());

    QCOMPARE(calendar->isLeapYear(2007 - 1911), false);
    QCOMPARE(calendar->isLeapYear(2008 - 1911), true);
    QCOMPARE(calendar->isLeapYear(1900 - 1911), false);
    QCOMPARE(calendar->isLeapYear(2000 - 1911), true);
    QCOMPARE(calendar->isLeapYear(QDate(2007, 1, 1)), false);
    QCOMPARE(calendar->isLeapYear(QDate(2008, 1, 1)), true);

    QCOMPARE(calendar->daysInWeek(QDate(2007, 1, 1)), 7);

    QCOMPARE(calendar->monthsInYear(2007 - 1911),         12);
    QCOMPARE(calendar->monthsInYear(QDate(2007, 1, 1)), 12);

    testYear(calendar, QDate(2007, 7, 9), 2007 - 1911, QString("96"), QString("0096"));
    testMonth(calendar, QDate(2007, 7, 9),    7, QString("7"),  QString("07"));
    testDay(calendar, QDate(2007, 7, 9),    9, QString("9"),  QString("09"));

    testEraDate(calendar, QDate(2007, 1, 1), 2007 - 1911, "96", "0096", "ROC", "Republic of China Era");

    testWeekDayName(calendar, 6, QDate(2007, 7, 28), "S", "Sat", "Saturday");
    testMonthName(calendar, 12, 2007 - 1911, QDate(2007, 12, 20), "D", "Dec", "December", "of Dec", "of December");

    QCOMPARE(calendar->monthsInYear(QDate(2007, 1, 1)), 12);

    QCOMPARE(calendar->isProleptic(), false);
    QCOMPARE(calendar->isLunar(), false);
    QCOMPARE(calendar->isLunisolar(), false);
    QCOMPARE(calendar->isSolar(), true);

    testRoundTrip(calendar);

    delete calendar;
}


void KCalendarTest::testThaiBasic()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::ThaiCalendar);

    testCalendarSystemType(calendar, KLocale::ThaiCalendar, "thai", "Thai");

    QCOMPARE(calendar->epoch(), QDate::fromJulianDay(1522734));
    QCOMPARE(calendar->earliestValidDate(), QDate::fromJulianDay(1522734));
    QCOMPARE(calendar->latestValidDate(), QDate(9456, 12, 31));

    testValid(calendar, -1, 10000, 13, 32, QDate());

    QCOMPARE(calendar->isLeapYear(2007 + 543), false);
    QCOMPARE(calendar->isLeapYear(2008 + 543), true);
    QCOMPARE(calendar->isLeapYear(1900 + 543), false);
    QCOMPARE(calendar->isLeapYear(2000 + 543), true);
    QCOMPARE(calendar->isLeapYear(QDate(2007, 1, 1)), false);
    QCOMPARE(calendar->isLeapYear(QDate(2008, 1, 1)), true);

    QCOMPARE(calendar->daysInWeek(QDate(2007, 1, 1)), 7);

    QCOMPARE(calendar->monthsInYear(2007 + 543),          12);
    QCOMPARE(calendar->monthsInYear(QDate(2007, 1, 1)), 12);

    testYear(calendar, QDate(2007, 7, 9), 2007 + 543, QString("50"), QString("2550"));
    testMonth(calendar, QDate(2007, 7, 9),    7, QString("7"),  QString("07"));
    testDay(calendar, QDate(2007, 7, 9),    9, QString("9"),  QString("09"));

    testEraDate(calendar, QDate(2007, 1, 1), 2007 + 543, "2550", "2550", "BE", "Buddhist Era");

    testWeekDayName(calendar, 6, QDate(2007, 7, 28), "S", "Sat", "Saturday");
    testMonthName(calendar, 12, 2007 + 543, QDate(2007, 12, 20), "D", "Dec", "December", "of Dec", "of December");

    QCOMPARE(calendar->monthsInYear(QDate(2007, 1, 1)), 12);

    QCOMPARE(calendar->isProleptic(), false);
    QCOMPARE(calendar->isLunar(), false);
    QCOMPARE(calendar->isLunisolar(), false);
    QCOMPARE(calendar->isSolar(), true);

    testRoundTrip(calendar);

    delete calendar;
}


// generic test functions, call from calendar system specific ones
void KCalendarTest::testCalendarSystemType(const KCalendarSystem *calendar,
                                           KLocale::CalendarSystem system,
                                           const QString &type, const QString &label)
{
    QCOMPARE(calendar->calendarType(),               type);
    QCOMPARE(calendar->calendarSystem(),             system);
    QCOMPARE(calendar->calendarLabel(),              label);
    QCOMPARE(KCalendarSystem::calendarLabel(system), label);
}

// Simply tests valid ranges of ymd values, testYmd covers all other dates
void KCalendarTest::testValid(const KCalendarSystem *calendar, int lowInvalidYear, int highInvalidYear,
                              int highInvalidMonth, int highInvalidDay, const QDate &invalidDate)
{
    // min/max year
    QCOMPARE(calendar->isValid(lowInvalidYear, 1, 1), false);
    QCOMPARE(calendar->isValid(highInvalidYear, 1, 1), false);

    // min/max month
    QCOMPARE(calendar->isValid(1, 0, 1), false);
    QCOMPARE(calendar->isValid(1, highInvalidMonth, 1), false);

    // min/max day
    QCOMPARE(calendar->isValid(1, 1, 0), false);
    QCOMPARE(calendar->isValid(1, 1, highInvalidDay), false);

    QCOMPARE(calendar->isValid(1, 1,  1), true);

    QCOMPARE(calendar->isValid(invalidDate), false);
    QCOMPARE(calendar->isValid(QDate(2000, 1, 1)), true);
}

void KCalendarTest::testEpoch(const KCalendarSystem *calendar, int y, int m, int d, int jd)
{
    QCOMPARE(calendar->epoch(), QDate::fromJulianDay(jd));
    if (calendar->epoch() >= calendar->earliestValidDate()) {
        testYmd(calendar, y, m, d, jd);
    }
}

void KCalendarTest::testEarliestValidDate(const KCalendarSystem *calendar, int y, int m, int d, int jd)
{
    QCOMPARE(calendar->earliestValidDate(), QDate::fromJulianDay(jd));
    testYmd(calendar, y, m, d, jd);
}

void KCalendarTest::testLatestValidDate(const KCalendarSystem *calendar, int y, int m, int d, int jd)
{
    QCOMPARE(calendar->latestValidDate(), QDate::fromJulianDay(jd));
    testYmd(calendar, y, m, d, jd);
}

void KCalendarTest::testYmd(const KCalendarSystem *calendar, int y, int m, int d, int jd)
{
    QDate testDate1 = QDate::fromJulianDay(jd);
    QCOMPARE(calendar->year(testDate1), y);
    QCOMPARE(calendar->month(testDate1), m);
    QCOMPARE(calendar->day(testDate1), d);

    QDate testDate2;
    QVERIFY(calendar->setDate(testDate2, y, m, d));
    QCOMPARE(testDate1, testDate2);
}

void KCalendarTest::testYear(const KCalendarSystem *calendar, const QDate &date,
                             int year, const QString &shortString, const QString &longString)
{
    QCOMPARE(calendar->year(date), year);
    QCOMPARE(calendar->yearString(date, KCalendarSystem::ShortFormat), shortString);
    QCOMPARE(calendar->yearString(date, KCalendarSystem::LongFormat), longString);
    int i;
    QCOMPARE(calendar->yearStringToInteger(longString, i), year);
    QCOMPARE(i, longString.length());
}

void KCalendarTest::testMonth(const KCalendarSystem *calendar, const QDate &date,
                              int month, const QString &shortString, const QString &longString)
{
    QCOMPARE(calendar->month(date), month);
    QCOMPARE(calendar->monthString(date, KCalendarSystem::ShortFormat), shortString);
    QCOMPARE(calendar->monthString(date, KCalendarSystem::LongFormat), longString);
    int i;
    QCOMPARE(calendar->monthStringToInteger(longString, i), month);
    QCOMPARE(i, longString.length());
}

void KCalendarTest::testDay(const KCalendarSystem *calendar, const QDate &date,
                            int day, const QString &shortString, const QString &longString)
{
    QCOMPARE(calendar->day(date), day);
    QCOMPARE(calendar->dayString(date, KCalendarSystem::ShortFormat), shortString);
    QCOMPARE(calendar->dayString(date, KCalendarSystem::LongFormat), longString);
    int i;
    QCOMPARE(calendar->dayStringToInteger(longString, i), day);
    QCOMPARE(i, longString.length());
}

void KCalendarTest::testEraDate(const KCalendarSystem *calendar, const QDate &date,
                                int yearInEra, const QString &shortYearInEraString, const QString &longYearInEraString,
                                const QString &shortEraName, const QString &longEraName)
{
    QCOMPARE(calendar->yearInEra(date),                                     yearInEra);
    QCOMPARE(calendar->yearInEraString(date),                               shortYearInEraString);
    QCOMPARE(calendar->yearInEraString(date, KCalendarSystem::ShortFormat), shortYearInEraString);
    QCOMPARE(calendar->yearInEraString(date, KCalendarSystem::LongFormat),  longYearInEraString);
    QCOMPARE(calendar->eraName(date),                                       shortEraName);
    QCOMPARE(calendar->eraName(date, KCalendarSystem::ShortFormat),         shortEraName);
    QCOMPARE(calendar->eraName(date, KCalendarSystem::LongFormat),          longEraName);
}

// Pass in the week day number, the equivalent QDate, and the short and long day names
//  e.g. testWeekDayName( cal, 6, QDate(y,m,d), QString("Monday"), QString("Mon") )

void KCalendarTest::testWeekDayName(const KCalendarSystem *calendar, int weekDay, const QDate &date,
                                    const QString &narrowName, const QString & shortName, const QString & longName)
{
    QCOMPARE(calendar->weekDayName(weekDay, KCalendarSystem::ShortDayName), shortName);
    QCOMPARE(calendar->weekDayName(weekDay, KCalendarSystem::LongDayName), longName);
    QCOMPARE(calendar->weekDayName(weekDay), longName);

    // Test week day < min
    QCOMPARE(calendar->weekDayName(0, KCalendarSystem::ShortDayName), QString());
    QCOMPARE(calendar->weekDayName(0, KCalendarSystem::LongDayName), QString());

    // Test week day > max
    QCOMPARE(calendar->weekDayName(calendar->daysInWeek(date) + 1, KCalendarSystem::ShortDayName), QString());
    QCOMPARE(calendar->weekDayName(calendar->daysInWeek(date) + 1, KCalendarSystem::LongDayName), QString());

    // QDate parameter tests, so always in valid range
    QCOMPARE(calendar->weekDayName(date, KCalendarSystem::ShortDayName), shortName);
    QCOMPARE(calendar->weekDayName(date, KCalendarSystem::LongDayName), longName);
    QCOMPARE(calendar->weekDayName(date), longName);

    // Test format date componant
    QCOMPARE(calendar->formatDate(date, KLocale::DayOfWeekName, KLocale::NarrowName), narrowName);
    QCOMPARE(calendar->formatDate(date, KLocale::DayOfWeekName, KLocale::ShortName), shortName);
    QCOMPARE(calendar->formatDate(date, KLocale::DayOfWeekName, KLocale::LongName), longName);
}

void KCalendarTest::testMonthName(const KCalendarSystem *calendar, int month, int year, const QDate &date,
                                  const QString &narrowName, const QString &shortName, const QString &longName,
                                  const QString &shortNamePossessive, const QString &longNamePossessive)
{
    QCOMPARE(calendar->monthName(month, year, KCalendarSystem::ShortName), shortName);
    QCOMPARE(calendar->monthName(month, year, KCalendarSystem::LongName), longName);
    QCOMPARE(calendar->monthName(month, year, KCalendarSystem::ShortNamePossessive),
             shortNamePossessive);
    QCOMPARE(calendar->monthName(month, year, KCalendarSystem::LongNamePossessive),
             longNamePossessive);
    QCOMPARE(calendar->monthName(month, year), longName);

    // Test month < min
    QCOMPARE(calendar->monthName(0, year, KCalendarSystem::ShortName), QString());
    QCOMPARE(calendar->monthName(0, year, KCalendarSystem::LongName),  QString());
    QCOMPARE(calendar->monthName(0, year, KCalendarSystem::ShortName), QString());
    QCOMPARE(calendar->monthName(0, year, KCalendarSystem::LongName),  QString());

    // Test month > max
    QCOMPARE(calendar->monthName(calendar->monthsInYear(date) + 1, year,
                                 KCalendarSystem::ShortName), QString());
    QCOMPARE(calendar->monthName(calendar->monthsInYear(date) + 1, year,
                                 KCalendarSystem::LongName), QString());
    QCOMPARE(calendar->monthName(calendar->monthsInYear(date) + 1, year,
                                 KCalendarSystem::ShortName), QString());
    QCOMPARE(calendar->monthName(calendar->monthsInYear(date) + 1, year,
                                 KCalendarSystem::LongName), QString());

    // QDate parameter tests, so always in valid range
    QCOMPARE(calendar->monthName(date, KCalendarSystem::ShortName), shortName);
    QCOMPARE(calendar->monthName(date, KCalendarSystem::LongName), longName);
    QCOMPARE(calendar->monthName(date, KCalendarSystem::ShortNamePossessive),
             shortNamePossessive);
    QCOMPARE(calendar->monthName(date, KCalendarSystem::LongNamePossessive),
             longNamePossessive);
    QCOMPARE(calendar->monthName(date), longName);

    // Test format date componant
    QCOMPARE(calendar->formatDate(date, KLocale::MonthName, KLocale::NarrowName), narrowName);
    QCOMPARE(calendar->formatDate(date, KLocale::MonthName, KLocale::ShortName), shortName);
    QCOMPARE(calendar->formatDate(date, KLocale::MonthName, KLocale::LongName), longName);
}

void KCalendarTest::testRoundTrip(const KCalendarSystem *calendar, bool testFullRange)
{
    qDebug() << "Testing round trip of dates for Calendar System " << calendar->calendarLabel();
    qDebug() << "This may take some time, or you may have created an infinite loop.";
    qDebug() << "Uncomment the loop debug message to see each date comparison.";

    int testYear, testMonth, testDay;
    QDate testDate;
    // Limit to a short interesting date range for regular testing
    // Later add an Env var to choose full test whenever the formulas are changed.
    QDate loopDate, terminateDate;
    if (testFullRange) {
        loopDate = calendar->earliestValidDate();
        terminateDate = calendar->latestValidDate();
    } else {
        loopDate = qMax(QDate(1990, 1, 1), calendar->earliestValidDate());
        terminateDate = qMin(QDate(2020, 1, 1), calendar->latestValidDate());
    }
    QByteArray msg;
    while (loopDate <= terminateDate) {
        testYear = calendar->year(loopDate);
        testMonth = calendar->month(loopDate);
        testDay = calendar->day(loopDate);
        calendar->setDate(testDate, testYear, testMonth, testDay);
        // Uncomment this to see each comparison printed
        // or
        // Uncomment the QEXPECT_FAIL statements to check all dates regardless of occasional failures
        QByteArray msg = QByteArray::number(loopDate.toJulianDay()) + " = " +
                         QByteArray::number(testYear) + '-' +
                         QByteArray::number(testMonth) + '-' +
                         QByteArray::number(testDay) + " = " +
                         QByteArray::number(testDate.toJulianDay());
        /*
        // qDebug() << msg;
        */
        if (testMonth <= 0 || testDay <= 0) {   // year <= 0 is OK
            msg.prepend("Round Trip : JD to Date failed : ");
            QEXPECT_FAIL("", msg.data(), Continue);
        } else if (testDate.toJulianDay() == 0) {
            msg.prepend("Round Trip : Date to JD failed : ");
            QEXPECT_FAIL("", msg.data(), Continue);
        } else if (loopDate != testDate) {
            msg.prepend("Round Trip : JD's differ       : ");
            QEXPECT_FAIL("", msg.data(), Continue);
        }
        QCOMPARE(loopDate.toJulianDay(), testDate.toJulianDay());
        loopDate = loopDate.addDays(1);
    }
}

void KCalendarTest::testGregorianAddYears()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar);

    QDate testDate;

    calendar->setDate(testDate, 2000,  1,  1);
    QCOMPARE(calendar->addYears(testDate, -1),  testDate.addYears(-1));
    QCOMPARE(calendar->addYears(testDate,  1),  testDate.addYears(1));

    calendar->setDate(testDate, 2000,  2, 29);
    QCOMPARE(calendar->addYears(testDate,  1),  testDate.addYears(1));

    calendar->setDate(testDate, -2000,  1,  1);
    QCOMPARE(calendar->addYears(testDate, -1),  testDate.addYears(-1));
    QCOMPARE(calendar->addYears(testDate,  1),  testDate.addYears(1));

    delete calendar;
}

void KCalendarTest::testGregorianAddMonths()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar);

    QDate testDate;

    calendar->setDate(testDate, 2000,  1,  1);
    QCOMPARE(calendar->addMonths(testDate, -1),  testDate.addMonths(-1));
    QCOMPARE(calendar->addMonths(testDate,  1),  testDate.addMonths(1));

    calendar->setDate(testDate, 2000,  3,  1);
    QCOMPARE(calendar->addMonths(testDate, -1),  testDate.addMonths(-1));
    QCOMPARE(calendar->addMonths(testDate,  1),  testDate.addMonths(1));

    calendar->setDate(testDate, 2000, 12,  1);
    QCOMPARE(calendar->addMonths(testDate, -1),  testDate.addMonths(-1));
    QCOMPARE(calendar->addMonths(testDate,  1),  testDate.addMonths(1));

    calendar->setDate(testDate, 2000,  1,  1);
    QCOMPARE(calendar->addMonths(testDate, -30),  testDate.addMonths(-30));
    QCOMPARE(calendar->addMonths(testDate,  30),  testDate.addMonths(30));

    calendar->setDate(testDate, -2000,  1,  1);
    QCOMPARE(calendar->addMonths(testDate, -1),  testDate.addMonths(-1));
    QCOMPARE(calendar->addMonths(testDate,  1),  testDate.addMonths(1));

    calendar->setDate(testDate, -2000,  3,  1);
    QCOMPARE(calendar->addMonths(testDate, -1),  testDate.addMonths(-1));
    QCOMPARE(calendar->addMonths(testDate,  1),  testDate.addMonths(1));

    calendar->setDate(testDate, -2000, 12,  1);
    QCOMPARE(calendar->addMonths(testDate, -1),  testDate.addMonths(-1));
    QCOMPARE(calendar->addMonths(testDate,  1),  testDate.addMonths(1));

    calendar->setDate(testDate, -2000,  1,  1);
    QCOMPARE(calendar->addMonths(testDate, -30),  testDate.addMonths(-30));
    QCOMPARE(calendar->addMonths(testDate,  30),  testDate.addMonths(30));

    delete calendar;
}

void KCalendarTest::testGregorianAddDays()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar);

    QDate testDate;

    calendar->setDate(testDate, 2000,  1,  1);
    QCOMPARE(calendar->addDays(testDate, -1),  testDate.addDays(-1));
    QCOMPARE(calendar->addDays(testDate,  1),  testDate.addDays(1));

    calendar->setDate(testDate, -2000,  1,  1);
    QCOMPARE(calendar->addDays(testDate, -1),  testDate.addDays(-1));
    QCOMPARE(calendar->addDays(testDate,  1),  testDate.addDays(1));

    delete calendar;
}

void KCalendarTest::testGregorianDaysInYear()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar);

    QDate testDate;

    calendar->setDate(testDate, 1900, 1, 1);
    QCOMPARE(calendar->daysInYear(testDate),  testDate.daysInYear());
    calendar->setDate(testDate, 1999, 1, 1);
    QCOMPARE(calendar->daysInYear(testDate),  testDate.daysInYear());
    calendar->setDate(testDate, 2000, 1, 1);
    QCOMPARE(calendar->daysInYear(testDate),  testDate.daysInYear());
    calendar->setDate(testDate, 2001, 1, 1);
    QCOMPARE(calendar->daysInYear(testDate),  testDate.daysInYear());
    calendar->setDate(testDate, 2002, 1, 1);
    QCOMPARE(calendar->daysInYear(testDate),  testDate.daysInYear());
    calendar->setDate(testDate, 2003, 1, 1);
    QCOMPARE(calendar->daysInYear(testDate),  testDate.daysInYear());
    calendar->setDate(testDate, 2004, 1, 1);
    QCOMPARE(calendar->daysInYear(testDate),  testDate.daysInYear());
    calendar->setDate(testDate, 2005, 1, 1);
    QCOMPARE(calendar->daysInYear(testDate),  testDate.daysInYear());

    calendar->setDate(testDate, -4700, 1, 1);
#if QT_VERSION < 0x040601
    QEXPECT_FAIL("", "Returns 365 instead of 366", Continue); // Fix submitted to Qt
#endif
    QCOMPARE(calendar->daysInYear(testDate),  testDate.daysInYear());
    calendar->setDate(testDate, -4000, 1, 1);
#if QT_VERSION < 0x040601
    QEXPECT_FAIL("", "Returns 365 instead of 366", Continue); // Fix submitted to Qt
#endif
    QCOMPARE(calendar->daysInYear(testDate),  testDate.daysInYear());

    calendar->setDate(testDate, 1, 1, 1);
    QCOMPARE(calendar->daysInYear(testDate),  testDate.daysInYear());

    calendar->setDate(testDate, 9996, 1, 1);
    QCOMPARE(calendar->daysInYear(testDate),  testDate.daysInYear());
    calendar->setDate(testDate, 9999, 1, 1);
    QCOMPARE(calendar->daysInYear(testDate),  testDate.daysInYear());

    delete calendar;
}


void KCalendarTest::testGregorianDaysInMonth()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar);

    QDate testDate;

    // Test all months
    calendar->setDate(testDate, 2000,  1, 1);
    QCOMPARE(calendar->daysInMonth(testDate), testDate.daysInMonth());
    calendar->setDate(testDate, 2000,  2, 1);
    QCOMPARE(calendar->daysInMonth(testDate), testDate.daysInMonth());
    calendar->setDate(testDate, 2000,  3, 1);
    QCOMPARE(calendar->daysInMonth(testDate), testDate.daysInMonth());
    calendar->setDate(testDate, 2000,  4, 1);
    QCOMPARE(calendar->daysInMonth(testDate), testDate.daysInMonth());
    calendar->setDate(testDate, 2000,  5, 1);
    QCOMPARE(calendar->daysInMonth(testDate), testDate.daysInMonth());
    calendar->setDate(testDate, 2000,  6, 1);
    QCOMPARE(calendar->daysInMonth(testDate), testDate.daysInMonth());
    calendar->setDate(testDate, 2000,  7, 1);
    QCOMPARE(calendar->daysInMonth(testDate), testDate.daysInMonth());
    calendar->setDate(testDate, 2000,  8, 1);
    QCOMPARE(calendar->daysInMonth(testDate), testDate.daysInMonth());
    calendar->setDate(testDate, 2000,  9, 1);
    QCOMPARE(calendar->daysInMonth(testDate), testDate.daysInMonth());
    calendar->setDate(testDate, 2000, 10, 1);
    QCOMPARE(calendar->daysInMonth(testDate), testDate.daysInMonth());
    calendar->setDate(testDate, 2000, 11, 1);
    QCOMPARE(calendar->daysInMonth(testDate), testDate.daysInMonth());
    calendar->setDate(testDate, 2000, 12, 1);
    QCOMPARE(calendar->daysInMonth(testDate), testDate.daysInMonth());

    // Test Feb in leap and normal years
    calendar->setDate(testDate, 2000, 2, 1);
    QCOMPARE(calendar->daysInMonth(testDate), testDate.daysInMonth());
    calendar->setDate(testDate, 2001, 2, 1);
    QCOMPARE(calendar->daysInMonth(testDate), testDate.daysInMonth());

    // Test max date
    calendar->setDate(testDate, 9999, 12, 1);
    QCOMPARE(calendar->daysInMonth(testDate), testDate.daysInMonth());

    delete calendar;
}

void KCalendarTest::testGregorianDayOfYear()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar);

    QDate testDate;

    calendar->setDate(testDate, 2000,  1,  1);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());
    calendar->setDate(testDate, 2000,  2, 29);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());
    calendar->setDate(testDate, 2000,  6,  1);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());
    calendar->setDate(testDate, 2000, 12,  1);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());
    calendar->setDate(testDate, 2000, 12, 31);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());

    calendar->setDate(testDate, -2000,  1,  1);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());
    calendar->setDate(testDate, -2000,  2, 28);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());
    calendar->setDate(testDate, -2000,  6,  1);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());
    calendar->setDate(testDate, -2000, 12,  1);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());
    calendar->setDate(testDate, -2000, 12, 31);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());

    calendar->setDate(testDate, -4712,  1,  2);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());
    calendar->setDate(testDate, -4712,  2, 28);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());
    calendar->setDate(testDate, -4712,  6,  1);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());
    calendar->setDate(testDate, -4712, 12,  1);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());
    calendar->setDate(testDate, -4712, 12, 31);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());

    calendar->setDate(testDate, 9999,  1,  1);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());
    //QDate::dayOfYear doesn't check if invalid!!!
    //calendar->setDate( testDate, 9999,  2, 29 );
    //QCOMPARE( calendar->dayOfYear( testDate ), testDate.dayOfYear() );
    calendar->setDate(testDate, 9999,  6,  1);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());
    calendar->setDate(testDate, 9999, 12,  1);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());
    calendar->setDate(testDate, 9999, 12, 31);
    QCOMPARE(calendar->dayOfYear(testDate), testDate.dayOfYear());

    delete calendar;
}

void KCalendarTest::testGregorianDayOfWeek()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar);

    QDate testDate;

    calendar->setDate(testDate, 2000,  1,  1);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, 2000,  1,  2);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, 2000,  1,  3);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, 2000,  1,  4);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, 2000,  1,  5);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, 2000,  1,  6);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, 2000,  1,  7);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());

    calendar->setDate(testDate, -4712,  1,  2);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, -4712,  1,  3);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, -4712,  1,  4);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, -4712,  1,  5);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, -4712,  1,  6);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, -4712,  1,  7);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, -4712,  1,  8);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());

    calendar->setDate(testDate, 9999,  1,  1);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, 9999,  1,  2);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, 9999,  1,  3);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, 9999,  1,  4);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, 9999,  1,  5);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, 9999,  1,  6);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());
    calendar->setDate(testDate, 9999,  1,  7);
    QCOMPARE(calendar->dayOfWeek(testDate), testDate.dayOfWeek());

    delete calendar;
}

// Don't really need this as Gregorian currently uses QDate directly
void KCalendarTest::testGregorianIsLeapYear()
{
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar);

    QDate testDate;

    calendar->setDate(testDate, 2000,  1,  1);
    QCOMPARE(calendar->isLeapYear(2000),     testDate.isLeapYear(2000));
    QCOMPARE(calendar->isLeapYear(testDate), testDate.isLeapYear(2000));
    calendar->setDate(testDate, 2001,  1,  1);
    QCOMPARE(calendar->isLeapYear(2001),     testDate.isLeapYear(2001));
    QCOMPARE(calendar->isLeapYear(testDate), testDate.isLeapYear(2001));
    calendar->setDate(testDate, 2004,  1,  1);
    QCOMPARE(calendar->isLeapYear(2004),     testDate.isLeapYear(2004));
    QCOMPARE(calendar->isLeapYear(testDate), testDate.isLeapYear(2004));

    calendar->setDate(testDate, 1900,  1,  1);
    QCOMPARE(calendar->isLeapYear(1900),     testDate.isLeapYear(1900));
    QCOMPARE(calendar->isLeapYear(testDate), testDate.isLeapYear(1900));
    calendar->setDate(testDate, 1901,  1,  1);
    QCOMPARE(calendar->isLeapYear(1901),     testDate.isLeapYear(1901));
    QCOMPARE(calendar->isLeapYear(testDate), testDate.isLeapYear(1901));
    calendar->setDate(testDate, 1904,  1,  1);
    QCOMPARE(calendar->isLeapYear(1904),     testDate.isLeapYear(1904));
    QCOMPARE(calendar->isLeapYear(testDate), testDate.isLeapYear(1904));

    calendar->setDate(testDate, -2000,  1,  1);
    QCOMPARE(calendar->isLeapYear(-2000),    testDate.isLeapYear(-2000));
    QCOMPARE(calendar->isLeapYear(testDate), testDate.isLeapYear(-2000));
    calendar->setDate(testDate, -2001,  1,  1);
    QCOMPARE(calendar->isLeapYear(-2001),    testDate.isLeapYear(-2001));
    QCOMPARE(calendar->isLeapYear(testDate), testDate.isLeapYear(-2001));
    calendar->setDate(testDate, -2004,  1,  1);
    QCOMPARE(calendar->isLeapYear(-2004),    testDate.isLeapYear(-2004));
    QCOMPARE(calendar->isLeapYear(testDate), testDate.isLeapYear(-2004));

    delete calendar;
}

void KCalendarTest::testKLocalizedDate()
{
    KLocale::global()->setDateFormatShort("%y-%m-%d");
    const KCalendarSystem *calendar = KLocale::global()->calendar();

    // Set up a bunch of KLocalizedDates and QDates that should be equal
    QDate testQDate(2010, 1, 1);
    KLocalizedDate testKLocalizedDate(testQDate);

    // Test KLocalizedDate output calls not in KCalendarSystem
    QCOMPARE(KLocalizedDate().isNull(),        true);
    QCOMPARE(KLocalizedDate().isValid(),       false);
    QCOMPARE(testKLocalizedDate.isNull(),      testQDate.isNull());
    QCOMPARE(testKLocalizedDate.date(),        testQDate);
    QCOMPARE((qint64)testKLocalizedDate.toJulianDay(), (qint64)testQDate.toJulianDay());
    testKLocalizedDate.setDate(QDate(2010, 6, 1));
    testQDate.setDate(2010, 6, 1);
    QCOMPARE(testKLocalizedDate.date(),        testQDate);

    // Test static KLocalizedDate methods
    QCOMPARE(KLocalizedDate::currentDate().date(),           QDate::currentDate());
    QCOMPARE(KLocalizedDate::fromDate(testQDate).date(),   testQDate);
    QCOMPARE(KLocalizedDate::fromJulianDay(100000).date(), QDate::fromJulianDay(100000));

    // Compare KLocalizedDate output to KCalendarSystem output calls
    QCOMPARE(testKLocalizedDate.isValid(),                     calendar->isValid(testQDate));
    QCOMPARE(testKLocalizedDate.year(),                        calendar->year(testQDate));
    QCOMPARE(testKLocalizedDate.month(),                       calendar->month(testQDate));
    QCOMPARE(testKLocalizedDate.day(),                         calendar->day(testQDate));
    QCOMPARE(testKLocalizedDate.eraName(),                     calendar->eraName(testQDate));
    QCOMPARE(testKLocalizedDate.eraYear(),                     calendar->eraYear(testQDate));
    QCOMPARE(testKLocalizedDate.yearInEra(),                   calendar->yearInEra(testQDate));
    QCOMPARE(testKLocalizedDate.addYears(5).date(),          calendar->addYears(testQDate, 5));
    QCOMPARE(testKLocalizedDate.addMonths(5).date(),         calendar->addMonths(testQDate, 5));
    QCOMPARE(testKLocalizedDate.addDays(5).date(),           calendar->addDays(testQDate, 5));
    QCOMPARE(testKLocalizedDate.monthsInYear(),                calendar->monthsInYear(testQDate));
    QCOMPARE(testKLocalizedDate.weeksInYear(),                 calendar->weeksInYear(testQDate));
    QCOMPARE(testKLocalizedDate.daysInYear(),                  calendar->daysInYear(testQDate));
    QCOMPARE(testKLocalizedDate.daysInMonth(),                 calendar->daysInMonth(testQDate));
    QCOMPARE(testKLocalizedDate.daysInWeek(),                  calendar->daysInWeek(testQDate));
    QCOMPARE(testKLocalizedDate.dayOfYear(),                   calendar->dayOfYear(testQDate));
    QCOMPARE(testKLocalizedDate.dayOfWeek(),                   calendar->dayOfWeek(testQDate));
    QCOMPARE(testKLocalizedDate.week(),                        calendar->week(testQDate));
    QCOMPARE(testKLocalizedDate.isLeapYear(),                  calendar->isLeapYear(testQDate));
    QCOMPARE(testKLocalizedDate.firstDayOfYear().date(),       calendar->firstDayOfYear(testQDate));
    QCOMPARE(testKLocalizedDate.lastDayOfYear().date(),        calendar->lastDayOfYear(testQDate));
    QCOMPARE(testKLocalizedDate.firstDayOfMonth().date(),      calendar->firstDayOfMonth(testQDate));
    QCOMPARE(testKLocalizedDate.lastDayOfMonth().date(),       calendar->lastDayOfMonth(testQDate));

    QCOMPARE(testKLocalizedDate.week(KLocale::IsoWeekNumber),        calendar->week(testQDate, KLocale::IsoWeekNumber));
    QCOMPARE(testKLocalizedDate.weeksInYear(KLocale::IsoWeekNumber), calendar->weeksInYear(testQDate, KLocale::IsoWeekNumber));

    // Compare KLocalizedDate setDate to KCalendarSystem setDate
    testKLocalizedDate.setDate(2010, 3, 1);
    calendar->setDate(testQDate, 2010, 3, 1);
    QCOMPARE(testKLocalizedDate.date(), testQDate);
    testKLocalizedDate.setDate(2010, 100);
    calendar->setDate(testQDate, 2010, 100);
    QCOMPARE(testKLocalizedDate.date(), testQDate);
    testKLocalizedDate.setDate("AD", 2010, 9, 1);
    calendar->setDate(testQDate, "AD", 2010, 9, 1);
    QCOMPARE(testKLocalizedDate.date(), testQDate);
    testKLocalizedDate.setDate(KLocale::IsoWeekNumber, 2010, 15, 1);
    calendar->setDateIsoWeek(testQDate, 2010, 15, 1);
    QCOMPARE(testKLocalizedDate.date(), testQDate);
    testKLocalizedDate.setCurrentDate();
    testQDate = QDate::currentDate();
    QCOMPARE(testKLocalizedDate.date(), testQDate);

    // Compare KLocalizedDate getDate to KCalendarSystem getDate
    int ky, km, kd, qy, qm, qd;
    testKLocalizedDate.getDate(&ky, &km, &kd);
    calendar->getDate(testQDate, &qy, &qm, &qd);
    QCOMPARE(ky, qy);
    QCOMPARE(km, qm);
    QCOMPARE(kd, qd);

    // Compare KLocalizedDate format/read to KCalendarSystem format
    QCOMPARE(testKLocalizedDate.formatDate(),           calendar->formatDate(testQDate));
    QCOMPARE(testKLocalizedDate.formatDate("%d-%m-%y"), calendar->formatDate(testQDate, "%d-%m-%y"));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::Year),              calendar->yearString(testQDate));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::YearName),          calendar->yearString(testQDate));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::Month),             calendar->monthString(testQDate, KCalendarSystem::ShortFormat));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::MonthName),         calendar->monthName(testQDate));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::Day),               calendar->dayString(testQDate, KCalendarSystem::ShortFormat));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::DayName),           calendar->dayString(testQDate, KCalendarSystem::ShortFormat));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::JulianDay),         QString::number(testQDate.toJulianDay()));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::EraName),           calendar->eraName(testQDate));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::EraYear),           calendar->eraYear(testQDate));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::YearInEra),         calendar->yearInEraString(testQDate));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::DayOfYear),         calendar->dayOfYearString(testQDate, KCalendarSystem::ShortFormat));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::DayOfYearName),     calendar->dayOfYearString(testQDate, KCalendarSystem::ShortFormat));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::DayOfWeek),         calendar->dayOfWeekString(testQDate));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::DayOfWeekName),     calendar->weekDayName(testQDate));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::Week),              calendar->weekNumberString(testQDate, KCalendarSystem::ShortFormat));
    int weekYear;
    QDate weekDate;
    calendar->weekNumber(testQDate, &weekYear);
    calendar->setDate(weekDate, weekYear, 1, 1);
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::WeekYear),          calendar->yearString(weekDate));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::MonthsInYear),      calendar->monthsInYearString(testQDate));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::WeeksInYear),       calendar->weeksInYearString(testQDate));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::DaysInYear),        calendar->daysInYearString(testQDate));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::DaysInMonth),       calendar->daysInMonthString(testQDate));
    QCOMPARE(testKLocalizedDate.formatDate(KLocale::DaysInWeek),        calendar->daysInWeekString(testQDate));

    // Compare KLocalizedDate format/read to KCalendarSystem read
    QCOMPARE(testKLocalizedDate.readDate("2010-01-01").date(),                        calendar->readDate("2010-01-01"));
    QCOMPARE(testKLocalizedDate.readDate("2010-01-01", KLocale::NormalFormat).date(), calendar->readDate("2010-01-01", KLocale::NormalFormat));
    QCOMPARE(testKLocalizedDate.readDate("2010-01-01", "%y-%m-%d").date(),            calendar->readDate("2010-01-01", "%y-%m-%d"));

    // Test new addOn methods
    testKLocalizedDate.addYearsTo(5);
    testQDate = calendar->addYears(testQDate, 5);
    QCOMPARE(testKLocalizedDate.date(), testQDate);
    testKLocalizedDate.addMonthsTo(5);
    testQDate = calendar->addMonths(testQDate, 5);
    QCOMPARE(testKLocalizedDate.date(), testQDate);
    testKLocalizedDate.addDaysTo(5);
    testQDate = calendar->addDays(testQDate, 5);
    QCOMPARE(testKLocalizedDate.date(), testQDate);

    // Set up some dates for comparisons
    QDate lowQDate(2000, 1, 1);
    KLocalizedDate lowKLocalizedDate(lowQDate);
    QDate highQDate(2010, 1, 1);
    KLocalizedDate highKLocalizedDate(highQDate);

    // Compare the difference methods
    int kdy, kdm, kdd, kdir, qdy, qdm, qdd, qdir;
    lowKLocalizedDate.dateDifference(highKLocalizedDate, &kdy, &kdm, &kdd, &kdir);
    calendar->dateDifference(lowKLocalizedDate.date(), highKLocalizedDate.date(), &qdy, &qdm, &qdd, &qdir);
    QCOMPARE(kdy, qdy);
    QCOMPARE(kdm, qdm);
    QCOMPARE(kdd, qdd);
    QCOMPARE(kdir, qdir);
    lowKLocalizedDate.dateDifference(highQDate, &kdy, &kdm, &kdd, &kdir);
    calendar->dateDifference(lowKLocalizedDate.date(), highQDate, &qdy, &qdm, &qdd, &qdir);
    QCOMPARE(kdy, qdy);
    QCOMPARE(kdm, qdm);
    QCOMPARE(kdd, qdd);
    QCOMPARE(kdir, qdir);
    QCOMPARE(lowKLocalizedDate.yearsDifference(highKLocalizedDate),  calendar->yearsDifference(lowQDate, highQDate));
    QCOMPARE(lowKLocalizedDate.yearsDifference(highQDate),  calendar->yearsDifference(lowQDate, highQDate));
    QCOMPARE(lowKLocalizedDate.monthsDifference(highKLocalizedDate), calendar->monthsDifference(lowQDate, highQDate));
    QCOMPARE(lowKLocalizedDate.monthsDifference(highQDate), calendar->monthsDifference(lowQDate, highQDate));
    QCOMPARE(lowKLocalizedDate.daysDifference(highKLocalizedDate),   calendar->daysDifference(lowQDate, highQDate));
    QCOMPARE(lowKLocalizedDate.daysDifference(highQDate),   calendar->daysDifference(lowQDate, highQDate));

    // Test new operators
    QCOMPARE(lowKLocalizedDate == lowKLocalizedDate,   true);
    QCOMPARE(lowKLocalizedDate == highKLocalizedDate,  false);
    QCOMPARE(lowKLocalizedDate == lowQDate,   true);
    QCOMPARE(lowKLocalizedDate == highQDate,  false);
    QCOMPARE(lowKLocalizedDate != lowKLocalizedDate,   false);
    QCOMPARE(lowKLocalizedDate != highKLocalizedDate,  true);
    QCOMPARE(lowKLocalizedDate != lowQDate,   false);
    QCOMPARE(lowKLocalizedDate != highQDate,  true);
    QCOMPARE(highKLocalizedDate < lowKLocalizedDate,   false);
    QCOMPARE(lowKLocalizedDate  < highKLocalizedDate,  true);
    QCOMPARE(highKLocalizedDate < lowQDate,   false);
    QCOMPARE(lowKLocalizedDate  < highQDate,  true);
    QCOMPARE(highKLocalizedDate <= lowKLocalizedDate,  false);
    QCOMPARE(lowKLocalizedDate  <= lowKLocalizedDate,  true);
    QCOMPARE(lowKLocalizedDate  <= highKLocalizedDate, true);
    QCOMPARE(highKLocalizedDate <= lowQDate,  false);
    QCOMPARE(lowKLocalizedDate  <= lowQDate,  true);
    QCOMPARE(lowKLocalizedDate  <= highQDate, true);
    QCOMPARE(highKLocalizedDate > lowKLocalizedDate,   true);
    QCOMPARE(lowKLocalizedDate  > highKLocalizedDate,  false);
    QCOMPARE(highKLocalizedDate > lowQDate,   true);
    QCOMPARE(lowKLocalizedDate  > highQDate,  false);
    QCOMPARE(highKLocalizedDate >= lowKLocalizedDate,  true);
    QCOMPARE(lowKLocalizedDate  >= lowKLocalizedDate,  true);
    QCOMPARE(lowKLocalizedDate  >= highKLocalizedDate, false);
    QCOMPARE(highKLocalizedDate >= lowQDate,  true);
    QCOMPARE(lowKLocalizedDate  >= lowQDate,  true);
    QCOMPARE(lowKLocalizedDate  >= highQDate, false);

    // Test changing Calendar System
    KCalendarSystem *hijri = KCalendarSystem::create(KLocale::IslamicCivilCalendar);
    testKLocalizedDate.setCalendarSystem(KLocale::IslamicCivilCalendar);
    QCOMPARE(testKLocalizedDate.year(), hijri->year(testQDate));

    // Test copy of calendar object
    if (true) {
        KLocalizedDate test2 = testKLocalizedDate;
        test2.day();
    }
    testKLocalizedDate.day();

    KLocalizedDate newOne = testKLocalizedDate.currentDate();
    qDebug() << newOne;
    newOne = KLocalizedDate::currentDate();
    qDebug() << newOne;

    KLocalizedDate calendarDate(testQDate, calendar);
    QCOMPARE(calendarDate.date(), testQDate);
    QCOMPARE(calendarDate.day(), calendar->day(testQDate));

    KCalendarSystem *hijriCalendar =  KCalendarSystem::create(KLocale::IslamicCivilCalendar);
    KLocalizedDate hijriDate(testQDate, hijriCalendar);
    QCOMPARE(hijriDate.date(), testQDate);
    QCOMPARE(hijriDate.day(), hijriCalendar->day(testQDate));
    delete hijriCalendar;
}

void KCalendarTest::testWeekNumberSystem()
{
    KLocale *locale = new KLocale(*KLocale::global());
    const KCalendarSystem *calendar = KCalendarSystem::create(KLocale::GregorianCalendar, locale);
    int weekYear = 0;

    // Test the 7 days the year can start on:
    // Monday    == 2018
    // Tuesday   == 2013
    // Wednesday == 2014
    // Thursday  == 2015
    // Friday    == 2016
    // Saturday  == 2011
    // Sunday    == 2012

    // ISO Week
    // Monday    == 2018 == Week 1 2018
    // Tuesday   == 2013 == Week 1 2013
    // Wednesday == 2014 == Week 1 2014
    // Thursday  == 2015 == Week 1 2015
    // Friday    == 2016 == Week 53 2015
    // Saturday  == 2011 == Week 52 2010
    // Sunday    == 2012 == Week 52 2011

    locale->setWeekStartDay(1);
    locale->setWeekNumberSystem(KLocale::IsoWeekNumber);

    QCOMPARE(calendar->week(QDate(2018, 1, 1), &weekYear), 1);
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2013, 1, 1), &weekYear), 1);
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2014, 1, 1), &weekYear), 1);
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2015, 1, 1), &weekYear), 1);
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2016, 1, 1), &weekYear), 53);
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2011, 1, 1), &weekYear), 52);
    QCOMPARE(weekYear, 2010);
    QCOMPARE(calendar->week(QDate(2012, 1, 1), &weekYear), 52);
    QCOMPARE(weekYear, 2011);

    QCOMPARE(calendar->week(QDate(2014, 12, 31), &weekYear), 1);
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 12, 31), &weekYear), 53);
    QCOMPARE(weekYear, 2015);

    QCOMPARE(calendar->weeksInYear(2010), 52);
    QCOMPARE(calendar->weeksInYear(2015), 53);

    // First Full Week starting Sunday, aka US Week
    locale->setWeekStartDay(7);
    locale->setWeekNumberSystem(KLocale::FirstFullWeek);

    // Mon = Jan 1 2018 = Week 52 2017, Sun = Jan 7 = Week 1
    QCOMPARE(calendar->week(QDate(2018, 1, 1), &weekYear), 53);        //First Day
    QCOMPARE(weekYear, 2017);
    QCOMPARE(calendar->week(QDate(2018, 1, 6), &weekYear), 53);        //First Saturday
    QCOMPARE(weekYear, 2017);
    QCOMPARE(calendar->week(QDate(2018, 1, 7), &weekYear), 1);         //First Sunday
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 1, 13), &weekYear), 1);        //Second Saturday
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 1, 14), &weekYear), 2);        //Second Sunday
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 12, 31), &weekYear), 52);       //Last Day
    QCOMPARE(weekYear, 2018);

    // Tue = Jan 1 2013 = Week 52 2012, Sun = Jan 6 = Week 1
    QCOMPARE(calendar->week(QDate(2013, 1, 1), &weekYear), 53);        //First Day
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2013, 1, 5), &weekYear), 53);        //First Saturday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2013, 1, 6), &weekYear), 1);         //First Sunday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 12), &weekYear), 1);        //Second Saturday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 13), &weekYear), 2);        //Second Sunday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 12, 31), &weekYear), 52);       //Last Day
    QCOMPARE(weekYear, 2013);

    // Wed = Jan 1 2014 = Week 52 2013, Sun = Jan 5 = Week 1
    QCOMPARE(calendar->week(QDate(2014, 1, 1), &weekYear), 52);        //First Day
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2014, 1, 4), &weekYear), 52);        //First Saturday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2014, 1, 5), &weekYear), 1);         //First Sunday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 11), &weekYear), 1);        //Second Saturday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 12), &weekYear), 2);        //Second Sunday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 12, 31), &weekYear), 52);       //Last Day
    QCOMPARE(weekYear, 2014);

    // Thu = Jan 1 2015 = Week 52 2014, Sun = Jan 4 = Week 1
    QCOMPARE(calendar->week(QDate(2015, 1, 1), &weekYear), 52);        //First Day
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2015, 1, 3), &weekYear), 52);        //First Saturday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2015, 1, 4), &weekYear), 1);         //First Sunday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 10), &weekYear), 1);        //Second Saturday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 11), &weekYear), 2);        //Second Sunday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 12, 31), &weekYear), 52);       //Last Day
    QCOMPARE(weekYear, 2015);

    // Fri = Jan 1 2016 = Week 52 2015, Sun = Jan 3 = Week 1
    QCOMPARE(calendar->week(QDate(2016, 1, 1), &weekYear), 52);        //First Day
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2016, 1, 2), &weekYear), 52);        //First Saturday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2016, 1, 3), &weekYear), 1);         //First Sunday
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 9), &weekYear), 1);         //Second Saturday
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 10), &weekYear), 2);        //Second Sunday
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 12, 31), &weekYear), 52);       //Last Day
    QCOMPARE(weekYear, 2016);

    // Sat = Jan 1 2011 = Week 52 2010, Sun = Jan 2 = Week 1
    QCOMPARE(calendar->week(QDate(2011, 1, 1), &weekYear), 52);        //First Day, First Saturday
    QCOMPARE(weekYear, 2010);
    QCOMPARE(calendar->week(QDate(2011, 1, 2), &weekYear), 1);         //First Sunday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 1, 8), &weekYear), 1);         //Second Saturday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 1, 9), &weekYear), 2);         //Second Sunday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 12, 31), &weekYear), 52);       //Last Day
    QCOMPARE(weekYear, 2011);

    // Sun = Jan 1 2012 = Week 1 2012, Sun = Jan 1 = Week 1
    QCOMPARE(calendar->week(QDate(2012, 1, 1), &weekYear), 1);         //First Day, First Sunday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 1, 7), &weekYear), 1);         //First Saturday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 1, 8), &weekYear), 2);         //Second Sunday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2012);

    // First Full Week starting Monday
    locale->setWeekStartDay(1);
    locale->setWeekNumberSystem(KLocale::FirstFullWeek);

    // Mon = Jan 1 2018 = Week 52 2017, Mon = Jan 1 = Week 1
    QCOMPARE(calendar->week(QDate(2018, 1, 1), &weekYear), 1);          //First Day, First Monday
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 1, 7), &weekYear), 1);          //First Sunday
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 1, 8), &weekYear), 2);          //Second Monday
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2018);

    // Tue = Jan 1 2013 = Week 52 2012, Mon = Jan 7 = Week 1
    QCOMPARE(calendar->week(QDate(2013, 1, 1), &weekYear), 53);         //First Day
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2013, 1, 6), &weekYear), 53);         //First Sunday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2013, 1, 7), &weekYear), 1);          //First Monday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 13), &weekYear), 1);         //Second Saturday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 14), &weekYear), 2);         //Second Monday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 12, 31), &weekYear), 52);       //Last Day
    QCOMPARE(weekYear, 2013);

    // Wed = Jan 1 2014 = Week 52 2013, Mon = Jan 6 = Week 1
    QCOMPARE(calendar->week(QDate(2014, 1, 1), &weekYear), 52);         //First Day
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2014, 1, 5), &weekYear), 52);         //First Sunday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2014, 1, 6), &weekYear), 1);          //First Monday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 12), &weekYear), 1);         //Second Saturday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 13), &weekYear), 2);         //Second Monday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 12, 31), &weekYear), 52);       //Last Day
    QCOMPARE(weekYear, 2014);

    // Thu = Jan 1 2015 = Week 52 2014, Mon = Jan 5 = Week 1
    QCOMPARE(calendar->week(QDate(2015, 1, 1), &weekYear), 52);         //First Day
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2015, 1, 4), &weekYear), 52);         //First Sunday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2015, 1, 5), &weekYear), 1);          //First Monday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 11), &weekYear), 1);         //Second Saturday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 12), &weekYear), 2);         //Second Monday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 12, 31), &weekYear), 52);       //Last Day
    QCOMPARE(weekYear, 2015);

    // Fri = Jan 1 2016 = Week 52 2015, Mon = Jan 4 = Week 1
    QCOMPARE(calendar->week(QDate(2016, 1, 1), &weekYear), 52);         //First Day
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2016, 1, 3), &weekYear), 52);         //First Sunday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2016, 1, 4), &weekYear), 1);          //First Monday
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 10), &weekYear), 1);         //Second Saturday
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 11), &weekYear), 2);         //Second Monday
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 12, 31), &weekYear), 52);       //Last Day
    QCOMPARE(weekYear, 2016);

    // Sat = Jan 1 2011 = Week 52 2010, Mon = Jan 3 = Week 1
    QCOMPARE(calendar->week(QDate(2011, 1, 1), &weekYear), 52);        //First Day
    QCOMPARE(weekYear, 2010);
    QCOMPARE(calendar->week(QDate(2011, 1, 2), &weekYear), 52);        //First Sunday
    QCOMPARE(weekYear, 2010);
    QCOMPARE(calendar->week(QDate(2011, 1, 3), &weekYear), 1);         //First Monday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 1, 9), &weekYear), 1);         //Second Sunday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 1, 10), &weekYear), 2);        //Second Monday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 12, 31), &weekYear), 52);       //Last Day
    QCOMPARE(weekYear, 2011);

    // Sun = Jan 1 2012 = Week 52 2011, Mon = Jan 2 = Week 1
    QCOMPARE(calendar->week(QDate(2012, 1, 1), &weekYear), 52);         //First Day, First Sunday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2012, 1, 2), &weekYear), 1);          //First Monday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 1, 8), &weekYear), 1);          //Second Sunday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 1, 9), &weekYear), 2);          //Second Monday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2012);


    // First Partial Week, second week starting Sunday
    locale->setWeekStartDay(7);
    locale->setWeekNumberSystem(KLocale::FirstPartialWeek);

    // Mon = Jan 1 2018 = Week 1, Sun = Jan 7 = Week 2
    QCOMPARE(calendar->week(QDate(2018, 1, 1), &weekYear), 1);         //First Day
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 1, 6), &weekYear), 1);         //First Saturday
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 1, 7), &weekYear), 2);         //First Sunday
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 1, 13), &weekYear), 2);        //Second Saturday
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 1, 14), &weekYear), 3);        //Second Sunday
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2018);

    // Tue = Jan 1 2013 = Week 1, Sun = Jan 6 = Week 2
    QCOMPARE(calendar->week(QDate(2013, 1, 1), &weekYear), 1);        //First Day
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 5), &weekYear), 1);        //First Saturday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 6), &weekYear), 2);         //First Sunday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 12), &weekYear), 2);        //Second Saturday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 13), &weekYear), 3);        //Second Sunday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2013);

    // Wed = Jan 1 2014 = Week 1, Sun = Jan 5 = Week 2
    QCOMPARE(calendar->week(QDate(2014, 1, 1), &weekYear), 1);        //First Day
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 4), &weekYear), 1);        //First Saturday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 5), &weekYear), 2);         //First Sunday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 11), &weekYear), 2);        //Second Saturday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 12), &weekYear), 3);        //Second Sunday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2014);

    // Thu = Jan 1 2015 = Week 1, Sun = Jan 4 = Week 2
    QCOMPARE(calendar->week(QDate(2015, 1, 1), &weekYear), 1);        //First Day
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 3), &weekYear), 1);        //First Saturday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 4), &weekYear), 2);         //First Sunday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 10), &weekYear), 2);        //Second Saturday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 11), &weekYear), 3);        //Second Sunday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2015);

    // Fri = Jan 1 2016 = Week 1, Sun = Jan 3 = Week 2
    QCOMPARE(calendar->week(QDate(2016, 1, 1), &weekYear), 1);        //First Day
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 2), &weekYear), 1);        //First Saturday
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 3), &weekYear), 2);         //First Sunday
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 9), &weekYear), 2);         //Second Saturday
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 10), &weekYear), 3);        //Second Sunday
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2016);

    // Sat = Jan 1 2011 = Week 1, Sun = Jan 2 = Week 2
    QCOMPARE(calendar->week(QDate(2011, 1, 1), &weekYear), 1);        //First Day, First Saturday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 1, 2), &weekYear), 2);         //First Sunday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 1, 8), &weekYear), 2);         //Second Saturday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 1, 9), &weekYear), 3);         //Second Sunday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2011);

    // Sun = Jan 1 2012 = Week 1, Sun = Jan 1 = Week 1
    QCOMPARE(calendar->week(QDate(2012, 1, 1), &weekYear), 1);         //First Day, First Sunday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 1, 7), &weekYear), 1);         //First Saturday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 1, 8), &weekYear), 2);         //Second Sunday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2012);

    // First Partial Week starting Monday
    locale->setWeekStartDay(1);
    locale->setWeekNumberSystem(KLocale::FirstPartialWeek);

    // Mon = Jan 1 2018 = Week 1, Mon = Jan 1 = Week 2
    QCOMPARE(calendar->week(QDate(2018, 1, 1), &weekYear), 1);          //First Day, First Monday
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 1, 7), &weekYear), 1);          //First Sunday
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 1, 8), &weekYear), 2);          //Second Monday
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2018);

    // Tue = Jan 1 2013 = Week 1, Mon = Jan 7 = Week 2
    QCOMPARE(calendar->week(QDate(2013, 1, 1), &weekYear), 1);          //First Day
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 6), &weekYear), 1);          //First Sunday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 7), &weekYear), 2);          //First Monday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 13), &weekYear), 2);         //Second Saturday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 14), &weekYear), 3);         //Second Monday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2013);

    // Wed = Jan 1 2014 = Week 1, Mon = Jan 6 = Week 2
    QCOMPARE(calendar->week(QDate(2014, 1, 1), &weekYear), 1);          //First Day
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 5), &weekYear), 1);          //First Sunday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 6), &weekYear), 2);          //First Monday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 12), &weekYear), 2);         //Second Saturday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 13), &weekYear), 3);         //Second Monday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2014);

    // Thu = Jan 1 2015 = Week 1, Mon = Jan 5 = Week 2
    QCOMPARE(calendar->week(QDate(2015, 1, 1), &weekYear), 1);          //First Day
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 4), &weekYear), 1);          //First Sunday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 5), &weekYear), 2);          //First Monday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 11), &weekYear), 2);         //Second Saturday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 12), &weekYear), 3);         //Second Monday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2015);

    // Fri = Jan 1 2016 = Week 1, Mon = Jan 4 = Week 2
    QCOMPARE(calendar->week(QDate(2016, 1, 1), &weekYear), 1);          //First Day
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 3), &weekYear), 1);          //First Sunday
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 4), &weekYear), 2);          //First Monday
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 10), &weekYear), 2);         //Second Saturday
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 11), &weekYear), 3);         //Second Monday
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2016);

    // Sat = Jan 1 2011 = Week 1, Mon = Jan 3 = Week 2
    QCOMPARE(calendar->week(QDate(2011, 1, 1), &weekYear), 1);         //First Day
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 1, 2), &weekYear), 1);         //First Sunday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 1, 3), &weekYear), 2);         //First Monday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 1, 9), &weekYear), 2);         //Second Sunday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 1, 10), &weekYear), 3);        //Second Monday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 12, 31), &weekYear), 53);       //Last Day
    QCOMPARE(weekYear, 2011);

    // Sun = Jan 1 2012 = Week 1, Mon = Jan 2 = Week 2
    QCOMPARE(calendar->week(QDate(2012, 1, 1), &weekYear), 1);         //First Day, First Sunday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 1, 2), &weekYear), 2);          //First Monday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 1, 8), &weekYear), 2);          //Second Sunday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 1, 9), &weekYear), 3);          //Second Monday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 12, 31), &weekYear), 54);       //Last Day
    QCOMPARE(weekYear, 2012);

    // Simple weeks starting Jan 1
    locale->setWeekStartDay(1);
    locale->setWeekNumberSystem(KLocale::SimpleWeek);

    // Mon = Jan 1 2018
    QCOMPARE(calendar->week(QDate(2018, 1, 1), &weekYear), 1);         //First Day, Monday
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 1, 7), &weekYear), 1);
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 1, 8), &weekYear), 2);         //Second Week
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 1, 14), &weekYear), 2);
    QCOMPARE(weekYear, 2018);
    QCOMPARE(calendar->week(QDate(2018, 1, 15), &weekYear), 3);        //Third Week
    QCOMPARE(weekYear, 2018);

    // Tue = Jan 1 2013
    QCOMPARE(calendar->week(QDate(2013, 1, 1), &weekYear), 1);         //First Day, Tuesday
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 7), &weekYear), 1);
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 8), &weekYear), 2);         //Second Week
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 14), &weekYear), 2);
    QCOMPARE(weekYear, 2013);
    QCOMPARE(calendar->week(QDate(2013, 1, 15), &weekYear), 3);        //Third Week
    QCOMPARE(weekYear, 2013);

    // Wed = Jan 1 2014
    QCOMPARE(calendar->week(QDate(2014, 1, 1), &weekYear), 1);         //First Day, Wednesday
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 7), &weekYear), 1);
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 8), &weekYear), 2);         //Second Week
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 14), &weekYear), 2);
    QCOMPARE(weekYear, 2014);
    QCOMPARE(calendar->week(QDate(2014, 1, 15), &weekYear), 3);        //Third Week
    QCOMPARE(weekYear, 2014);

    // Thu = Jan 1 2015
    QCOMPARE(calendar->week(QDate(2015, 1, 1), &weekYear), 1);         //First Day, Thursday
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 7), &weekYear), 1);
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 8), &weekYear), 2);         //Second Week
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 14), &weekYear), 2);
    QCOMPARE(weekYear, 2015);
    QCOMPARE(calendar->week(QDate(2015, 1, 15), &weekYear), 3);        //Third Week
    QCOMPARE(weekYear, 2015);

    // Fri = Jan 1 2016
    QCOMPARE(calendar->week(QDate(2016, 1, 1), &weekYear), 1);         //First Day, Monday
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 7), &weekYear), 1);
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 8), &weekYear), 2);         //Second Week
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 14), &weekYear), 2);
    QCOMPARE(weekYear, 2016);
    QCOMPARE(calendar->week(QDate(2016, 1, 15), &weekYear), 3);        //Third Week
    QCOMPARE(weekYear, 2016);

    // Sat = Jan 1 2011
    QCOMPARE(calendar->week(QDate(2011, 1, 1), &weekYear), 1);         //First Day, Monday
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 1, 7), &weekYear), 1);
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 1, 8), &weekYear), 2);         //Second Week
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 1, 14), &weekYear), 2);
    QCOMPARE(weekYear, 2011);
    QCOMPARE(calendar->week(QDate(2011, 1, 15), &weekYear), 3);        //Third Week
    QCOMPARE(weekYear, 2011);

    // Sun = Jan 1 2012
    QCOMPARE(calendar->week(QDate(2012, 1, 1), &weekYear), 1);         //First Day, Monday
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 1, 7), &weekYear), 1);
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 1, 8), &weekYear), 2);         //Second Week
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 1, 14), &weekYear), 2);
    QCOMPARE(weekYear, 2012);
    QCOMPARE(calendar->week(QDate(2012, 1, 15), &weekYear), 3);        //Third Week
    QCOMPARE(weekYear, 2012);

    delete calendar;
    delete locale;
}
