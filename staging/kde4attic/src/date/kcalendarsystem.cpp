/*
    Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
    Copyright (c) 2002 Hans Petter Bieker <bieker@kde.org>
    Copyright 2007, 2008, 2009, 2010 John Layt <john@layt.net>

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

#include "kcalendarsystem.h"
#include "kcalendarsystemprivate_p.h"

#include "kconfiggroup.h"

#include <QtCore/QDateTime>

#include "klocalizedstring.h"
#include "kdatetime.h"
#include "kdatetimeformatter_p.h"
#include "kdatetimeparser_p.h"
#include "kcalendarera_p.h"
#include "kcalendarsystemcoptic_p.h"
#include "kcalendarsystemethiopian_p.h"
#include "kcalendarsystemgregorian_p.h"
#include "kcalendarsystemhebrew_p.h"
#include "kcalendarsystemindiannational_p.h"
#include "kcalendarsystemislamiccivil_p.h"
#include "kcalendarsystemjalali_p.h"
#include "kcalendarsystemjapanese_p.h"
#include "kcalendarsystemjulian_p.h"
#include "kcalendarsystemminguo_p.h"
#include "kcalendarsystemthai_p.h"

KCalendarSystem *KCalendarSystem::create(KLocale::CalendarSystem calendarSystem, const KLocale *locale)
{
    return create(calendarSystem, KSharedConfig::Ptr(), locale);
}

KCalendarSystem *KCalendarSystem::create(KLocale::CalendarSystem calendarSystem,
                                         KSharedConfig::Ptr config,
                                         const KLocale *locale)
{
    switch (calendarSystem) {
    case KLocale::GregorianCalendar:
        return new KCalendarSystemGregorian(config, locale);
    case KLocale::CopticCalendar:
        return new KCalendarSystemCoptic(config, locale);
    case KLocale::EthiopianCalendar:
        return new KCalendarSystemEthiopian(config, locale);
    case KLocale::HebrewCalendar:
        return new KCalendarSystemHebrew(config, locale);
    case KLocale::IndianNationalCalendar:
        return new KCalendarSystemIndianNational(config, locale);
    case KLocale::IslamicCivilCalendar:
        return new KCalendarSystemIslamicCivil(config, locale);
    case KLocale::JalaliCalendar:
        return new KCalendarSystemJalali(config, locale);
    case KLocale::JapaneseCalendar:
        return new KCalendarSystemJapanese(config, locale);
    case KLocale::JulianCalendar:
        return new KCalendarSystemJulian(config, locale);
    case KLocale::MinguoCalendar:
        return new KCalendarSystemMinguo(config, locale);
    case KLocale::ThaiCalendar:
        return new KCalendarSystemThai(config, locale);
    default:
        return new KCalendarSystemGregorian(config, locale);
    }
}

QList<KLocale::CalendarSystem> KCalendarSystem::calendarSystemsList()
{
    QList<KLocale::CalendarSystem> list;

    list.append(KLocale::GregorianCalendar);
    list.append(KLocale::CopticCalendar);
    list.append(KLocale::EthiopianCalendar);
    list.append(KLocale::HebrewCalendar);
    list.append(KLocale::IslamicCivilCalendar);
    list.append(KLocale::IndianNationalCalendar);
    list.append(KLocale::JalaliCalendar);
    list.append(KLocale::JapaneseCalendar);
    list.append(KLocale::JulianCalendar);
    list.append(KLocale::MinguoCalendar);
    list.append(KLocale::ThaiCalendar);

    return list;
}

QString KCalendarSystem::calendarLabel(KLocale::CalendarSystem calendarSystem, const KLocale *locale)
{
    QStringList languages = locale->languageList();

    switch (calendarSystem) {
    case KLocale::GregorianCalendar:
        return ki18nc("@item Calendar system", "Gregorian").toString(languages);
    case KLocale::CopticCalendar:
        return ki18nc("@item Calendar system", "Coptic").toString(languages);
    case KLocale::EthiopianCalendar:
        return ki18nc("@item Calendar system", "Ethiopian").toString(languages);
    case KLocale::HebrewCalendar:
        return ki18nc("@item Calendar system", "Hebrew").toString(languages);
    case KLocale::IslamicCivilCalendar:
        return ki18nc("@item Calendar system", "Islamic / Hijri (Civil)").toString(languages);
    case KLocale::IndianNationalCalendar:
        return ki18nc("@item Calendar system", "Indian National").toString(languages);
    case KLocale::JalaliCalendar:
        return ki18nc("@item Calendar system", "Jalali").toString(languages);
    case KLocale::JapaneseCalendar:
        return ki18nc("@item Calendar system", "Japanese").toString(languages);
    case KLocale::JulianCalendar:
        return ki18nc("@item Calendar system", "Julian").toString(languages);
    case KLocale::MinguoCalendar:
        return ki18nc("@item Calendar system", "Taiwanese").toString(languages);
    case KLocale::ThaiCalendar:
        return ki18nc("@item Calendar system", "Thai").toString(languages);
    }

    return ki18nc("@item Calendar system", "Invalid Calendar Type").toString(languages);
}

// Shared d pointer base class definitions

KCalendarSystemPrivate::KCalendarSystemPrivate(KCalendarSystem *q_ptr)
                      : q(q_ptr),
                        m_eraList(0),
                        m_shortYearWindowStartYear(2000)
{
}

KCalendarSystemPrivate::~KCalendarSystemPrivate()
{
    delete m_eraList;
}

int KCalendarSystemPrivate::isoWeekNumber(const QDate &date, int *yearNum) const
{
    int y, m, d;
    q->julianDayToDate(date.toJulianDay(), y, m, d);

    QDate firstDayWeek1, lastDay;
    int week;
    int weekDay1, dayOfWeek1InYear;

    // let's guess 1st day of 1st week
    firstDayWeek1 = firstDayOfYear(y);
    weekDay1 = dayOfWeek(firstDayWeek1);

    // iso 8601: week 1  is the first containing thursday and week starts on monday
    if (weekDay1 > 4 /*Thursday*/) {
        firstDayWeek1 = q->addDays(firstDayWeek1 , daysInWeek() - weekDay1 + 1);   // next monday
    }

    dayOfWeek1InYear = dayOfYear(firstDayWeek1);

    // our date in prev year's week
    if (dayOfYear(date) < dayOfWeek1InYear) {
        if (yearNum) {
            *yearNum = addYears(y, - 1);
        }
        return isoWeeksInYear(addYears(y, - 1));
    }

    // let's check if its last week belongs to next year
    lastDay = lastDayOfYear(y);

    // if our date is in last week && 1st week in next year has thursday
    if ((dayOfYear(date) >= daysInYear(y) - dayOfWeek(lastDay) + 1)
            && dayOfWeek(lastDay) < 4) {
        if (yearNum) {
            * yearNum = addYears(y, 1);
        }
        week = 1;
    } else {
        // To calculate properly the number of weeks from day a to x let's make a day 1 of week
        if (weekDay1 < 5) {
            firstDayWeek1 = q->addDays(firstDayWeek1, -(weekDay1 - 1));
        }

        if (yearNum) {
            * yearNum = y;
        }

        week = firstDayWeek1.daysTo(date) / daysInWeek() + 1;
    }

    return week;
}

int KCalendarSystemPrivate::regularWeekNumber(const QDate &date, int weekStartDay, int firstWeekNumber, int *weekYear) const
{
    int y, m, d;
    q->julianDayToDate(date.toJulianDay(), y, m, d);

    int firstWeekDayOffset = (dayOfWeek(date) - weekStartDay + daysInWeek()) % daysInWeek();
    int dayInYear = date.toJulianDay() - firstDayOfYear(y).toJulianDay();   // 0 indexed
    int week = ((dayInYear - firstWeekDayOffset + daysInWeek()) / daysInWeek());

    if (dayOfWeek(firstDayOfYear(y)) != weekStartDay) {
        week = week + firstWeekNumber;
    }

    if (week < 1) {
        y = y - 1;
        week = regularWeeksInYear(y, weekStartDay, firstWeekNumber);
    }

    if (weekYear) {
        *weekYear = y;
    }

    return week;
}

int KCalendarSystemPrivate::simpleWeekNumber(const QDate &date, int *yearNum) const
{
    int y, m, d;
    q->julianDayToDate(date.toJulianDay(), y, m, d);
    if (yearNum) {
        *yearNum = y;
    }
    return ((date.toJulianDay() - firstDayOfYear(y).toJulianDay()) / daysInWeek()) + 1;
}

int KCalendarSystemPrivate::isoWeeksInYear(int year) const
{
    QDate lastDayOfThisYear = lastDayOfYear(year);

    int weekYear = year;
    int lastWeekInThisYear = isoWeekNumber(lastDayOfThisYear, &weekYear);

    // If error, or the last day of the year is in the first week of next year use the week before
    if (lastWeekInThisYear < 1 || weekYear != year) {
        lastWeekInThisYear = isoWeekNumber(q->addDays(lastDayOfThisYear, -7), &weekYear);
    }

    return lastWeekInThisYear;
}

int KCalendarSystemPrivate::regularWeeksInYear(int year, int weekStartDay, int firstWeekNumber) const
{
    return regularWeekNumber(lastDayOfYear(year), weekStartDay, firstWeekNumber, 0);
}

int KCalendarSystemPrivate::simpleWeeksInYear(int year) const
{
    return simpleWeekNumber(lastDayOfYear(year), 0);
}

// Reimplement if special maths handling required, e.g. Hebrew.
// Works for calendars with constant number of months, or where leap month is last month of year
// Will not work for Hebrew or others where leap month is inserted in middle of year
void KCalendarSystemPrivate::dateDifference(const QDate &fromDate, const QDate &toDate,
                                            int *yearsDiff, int *monthsDiff, int *daysDiff, int *direction) const
{
    // This could be optimised a little but is left in full as it's easier to understand
    int dy = 0;
    int dm = 0;
    int dd = 0;
    int dir = 1;

    if (toDate < fromDate) {
        dateDifference(toDate, fromDate, &dy, &dm, &dd, 0);
        dir = -1;
    } else if (toDate > fromDate) {

        int fromYear = q->year(fromDate);
        int toYear = q->year(toDate);
        int fromMonth = q->month(fromDate);
        int toMonth = q->month(toDate);
        int fromDay = q->day(fromDate);
        int toDay = q->day(toDate);

        int monthsInPrevYear = monthsInYear(addYears(toYear, -1));
        int daysInPrevMonth = q->daysInMonth(q->addMonths(toDate, -1));
        int daysInFromMonth = daysInMonth(fromYear, fromMonth);
        int daysInToMonth = daysInMonth(toYear, toMonth);

        // Calculate years difference
        if (toYear == fromYear) {
            dy = 0;
        } else if (toMonth > fromMonth) {
            dy = differenceYearNumbers(fromYear, toYear);
        } else if (toMonth < fromMonth) {
            dy = differenceYearNumbers(fromYear, toYear) - 1;
        } else { // toMonth == fromMonth
            // Allow for last day of month to last day of month and leap days
            // e.g. 2000-02-29 to 2001-02-28 is 1 year not 0 years
            if ((toDay >= fromDay) || (fromDay == daysInFromMonth && toDay == daysInToMonth)) {
                dy = differenceYearNumbers(fromYear, toYear);
            } else {
                dy = differenceYearNumbers(fromYear, toYear) - 1;
            }
        }

        // Calculate months and days difference
        if (toDay >= fromDay) {
            dm = (monthsInPrevYear + toMonth - fromMonth) % monthsInPrevYear;
            dd = toDay - fromDay;
        } else { // toDay < fromDay
            // Allow for last day of month to last day of month and leap days
            // e.g. 2010-03-31 to 2010-04-30 is 1 month
            //      2000-02-29 to 2001-02-28 is 1 year
            //      2000-02-29 to 2001-03-01 is 1 year 1 day
            int prevMonth = q->month(q->addMonths(toDate, -1));
            if (fromDay == daysInFromMonth && toDay == daysInToMonth) {
                dm = (monthsInPrevYear + toMonth - fromMonth) % monthsInPrevYear;
                dd = 0;
            } else if (prevMonth == fromMonth && daysInPrevMonth < daysInFromMonth) {
                // Special case where fromDate = leap day and toDate in month following but non-leap year
                // e.g. 2000-02-29 to 2001-03-01 needs to use 29 to calculate day number not 28
                dm = (monthsInPrevYear + toMonth - fromMonth - 1) % monthsInPrevYear;
                dd = (daysInFromMonth + toDay - fromDay) % daysInFromMonth;
            } else {
                dm = (monthsInPrevYear + toMonth - fromMonth - 1) % monthsInPrevYear;
                dd = (daysInPrevMonth + toDay - fromDay) % daysInPrevMonth;
            }
        }

    }

    // Only return values if we have a valid pointer
    if (yearsDiff) {
        *yearsDiff = dy;
    }
    if (monthsDiff) {
        *monthsDiff = dm;
    }
    if (daysDiff) {
        *daysDiff = dd;
    }
    if (direction) {
        *direction = dir;
    }
}

// Reimplement if special maths handling required, e.g. Hebrew
// Allows for calendars with leap months at end of year but not during year
int KCalendarSystemPrivate::yearsDifference(const QDate &fromDate, const QDate &toDate) const
{
    // This could be optimised a little but is left in full as it's easier to understand
    // Alternatively could just call dateDifference(), but this is slightly more efficient

    if (toDate < fromDate) {
        return 0 - yearsDifference(toDate, fromDate);
    }

    if (toDate == fromDate) {
        return 0;
    }

    int fromYear = q->year(fromDate);
    int toYear = q->year(toDate);

    if (toYear == fromYear) {
        return 0;
    }

    int fromMonth = q->month(fromDate);
    int toMonth = q->month(toDate);

    if (toMonth > fromMonth) {
        return differenceYearNumbers(fromYear, toYear);
    }

    if (toMonth < fromMonth) {
        return differenceYearNumbers(fromYear, toYear) - 1;
    }

    // toMonth == fromMonth
    int fromDay = q->day(fromDate);
    int toDay = q->day(toDate);

    // Adjust for month numbers in from and to year
    // Allow for last day of month to last day of month and leap days
    // e.g. 2000-02-29 to 2001-02-28 is 1 year not 0 years
    if ((toDay >= fromDay) ||
            (fromDay == daysInMonth(fromYear, fromMonth) &&
             toDay == daysInMonth(toYear, toMonth))) {
        return differenceYearNumbers(fromYear, toYear);
    } else {
        return differenceYearNumbers(fromYear, toYear) - 1;
    }

}

// Reimplement if special maths handling required, e.g. maybe Hebrew?
// Allows for calendars with leap months
int KCalendarSystemPrivate::monthsDifference(const QDate &fromDate, const QDate &toDate) const
{
    if (toDate < fromDate) {
        return 0 - monthsDifference(toDate, fromDate);
    }

    if (toDate == fromDate) {
        return 0;
    }

    int fromYear = q->year(fromDate);
    int toYear = q->year(toDate);
    int fromMonth = q->month(fromDate);
    int toMonth = q->month(toDate);
    int fromDay = q->day(fromDate);
    int toDay = q->day(toDate);

    int monthsInPreceedingYears;

    // Calculate number of months in full years preceding toYear
    if (toYear == fromYear) {
        monthsInPreceedingYears = 0;
    } else if (hasLeapMonths()) {
        monthsInPreceedingYears = 0;
        for (int y = fromYear; y < toYear; y = addYears(y, 1)) {
            monthsInPreceedingYears = monthsInPreceedingYears + monthsInYear(y);
        }
    } else {
        monthsInPreceedingYears = differenceYearNumbers(fromYear, toYear) * monthsInYear(toYear);
    }

    // Adjust for months in from and to year
    // Allow for last day of month to last day of month and leap days
    // e.g. 2010-03-31 to 2010-04-30 is 1 month not 0 months
    // also 2000-02-29 to 2001-02-28 is 12 months not 11 months
    if ((toDay >= fromDay) ||
            (fromDay == daysInMonth(fromYear, fromMonth) &&
             toDay == daysInMonth(toYear, toMonth))) {
        return monthsInPreceedingYears + toMonth - fromMonth;
    } else {
        return monthsInPreceedingYears + toMonth - fromMonth - 1;
    }
}

// Reimplement if special string to integer handling required, e.g. Hebrew.
// Peel a number off the front of a string which may have other trailing chars after the number
// Stop either at either maxLength, eos, or first non-digit char
int KCalendarSystemPrivate::integerFromString(const QString &string, int maxLength, int &readLength) const
{
    int value = -1;
    int position = 0;
    readLength = 0;
    bool ok = false;

    if (maxLength < 0) {
        maxLength = string.length();
    }

    while (position < string.length() &&
            position < maxLength &&
            string.at(position).isDigit()) {
        position++;
    }

    if (position > 0) {
        value = string.left(position).toInt(&ok);
        if (ok) {
            readLength = position;
        } else {
            value = -1;
        }
    }

    return value;
}

// Reimplement if special integer to string handling required, e.g. Hebrew.
// Utility to convert an integer into the correct display string form
QString KCalendarSystemPrivate::stringFromInteger(int number, int padWidth, QChar padChar) const
{
    return stringFromInteger(number, padWidth, padChar, q->locale()->dateTimeDigitSet());
}

// Reimplement if special integer to string handling required, e.g. Hebrew.
// Utility to convert an integer into the correct display string form
QString KCalendarSystemPrivate::stringFromInteger(int number, int padWidth, QChar padChar, KLocale::DigitSet digitSet) const
{
    if (padChar == QLatin1Char('\0') || padWidth == 0) {
        return q->locale()->convertDigits(QString::number(number), digitSet);
    } else {
        return q->locale()->convertDigits(QString::number(number).rightJustified(padWidth, padChar), digitSet);
    }
}

// Utility to correctly add years to a year number because some systems such as
// Julian and Gregorian calendars don't have a year 0
int KCalendarSystemPrivate::addYears(int originalYear, int addYears) const
{
    int newYear = originalYear + addYears;

    if (!hasYearZero()) {
        if (originalYear > 0 && newYear <= 0) {
            newYear = newYear - 1;
        } else if (originalYear < 0 && newYear >= 0) {
            newYear = newYear + 1;
        }
    }

    return newYear;
}

// Utility to correctly return number of years between two year numbers because some systems such as
// Julian and Gregorian calendars don't have a year 0
int KCalendarSystemPrivate::differenceYearNumbers(int fromYear, int toYear) const
{
    int dy = toYear - fromYear;

    if (!hasYearZero()) {
        if (toYear > 0 && fromYear < 0) {
            dy = dy - 1;
        } else if (toYear < 0 && fromYear > 0) {
            dy = dy + 1;
        }
    }

    return dy;
}

QString KCalendarSystemPrivate::simpleDateString(const QString &str) const
{
    QString newStr;
    for (int i = 0; i < str.length(); i++) {
        if (str.at(i).isLetterOrNumber()) {
            newStr.append(str.at(i));
        } else {
            newStr.append(QLatin1Char(' '));
        }
    }
    newStr.simplified();
    return newStr;
}

// If we ever want to support a calendar system with another week length,this
// method can be changed to virtual. Doing so will, however, likely break some
// code, so don't do that without testing and fixing at least all of KDE SC.
int KCalendarSystemPrivate::daysInWeek() const
{
    return 7;
}

int KCalendarSystemPrivate::dayOfYear(const QDate &date) const
{
    qint64 jdFirstDayOfYear;
    int y, m, d;
    q->julianDayToDate(date.toJulianDay(), y, m, d);
    q->dateToJulianDay(y, 1, 1, jdFirstDayOfYear);
    //Take the jd of the given date, and subtract the jd of the first day of that year
    return (date.toJulianDay() - jdFirstDayOfYear + 1);
}

// This assumes all calendar systems agree on the day of week for a given
// julian day number. That is true of all current calendar systems. If we
// ever want to support one for which it isn't this implementation can be
// changed. Doing so will, however, likely break some code, so don't do that
// without testing and fixing at least all of KDE SC.
int KCalendarSystemPrivate::dayOfWeek(const QDate &date) const
{
    return date.dayOfWeek();
}

QDate KCalendarSystemPrivate::firstDayOfYear(int year) const
{
    qint64 jd;
    q->dateToJulianDay(year, 1, 1, jd);
    return QDate::fromJulianDay(jd);
}

QDate KCalendarSystemPrivate::lastDayOfYear(int year) const
{
    qint64 jd;
    q->dateToJulianDay(year, 1, 1, jd);
    jd = jd + daysInYear(year) - 1;
    return QDate::fromJulianDay(jd);
}

QDate KCalendarSystemPrivate::firstDayOfMonth(int year, int month) const
{
    qint64 jd;
    q->dateToJulianDay(year, month, 1, jd);
    return QDate::fromJulianDay(jd);
}

QDate KCalendarSystemPrivate::lastDayOfMonth(int year, int month) const
{
    qint64 jd;
    q->dateToJulianDay(year, month, 1, jd);
    jd = jd + daysInMonth(year, month) - 1;
    return QDate::fromJulianDay(jd);
}

const KLocale * KCalendarSystemPrivate::locale() const
{
    if (m_locale) {
        return m_locale;
    } else {
        return KLocale::global();
    }
}

QList<KCalendarEra> *KCalendarSystemPrivate::eraList() const
{
    return m_eraList;
}

KCalendarEra KCalendarSystemPrivate::era(const QDate &eraDate) const
{
    for (int i = m_eraList->count() - 1; i >= 0; --i) {
        if (m_eraList->at(i).isInEra(eraDate)) {
            return m_eraList->at(i);
        }
    }
    return KCalendarEra();
}

KCalendarEra KCalendarSystemPrivate::era(const QString &eraName, int yearInEra) const
{
    Q_UNUSED(yearInEra)

    for (int i = m_eraList->count() - 1; i >= 0; --i) {
        KCalendarEra era = m_eraList->at(i);
        if (era.name(KLocale::LongName).toLower() == eraName.toLower() ||
                era.name(KLocale::ShortName).toLower() == eraName.toLower()) {
            return era;
        }
    }
    return KCalendarEra();
}

void KCalendarSystemPrivate::loadEraList(const KConfigGroup & cg)
{
    delete m_eraList;
    m_eraList = new QList<KCalendarEra>;
    QString eraKey = QString::fromLatin1("Era1");
    int i = 1;
    while (cg.hasKey(eraKey)) {
        QString eraEntry = cg.readEntry(eraKey, QString());
        if (!eraEntry.isEmpty()) {
            // Based on LC_TIME, but different!
            // Includes long and short names, uses ISO fomat dates
            // e.g. +:1:0001-01-01:9999-12-31:Anno Domini:AD:%EC %Ey
            QChar direction = eraEntry.section(QLatin1Char(':'), 0, 0).at(0);
            QDate startDate, endDate;
            int startYear;
            QString buffer = eraEntry.section(QLatin1Char(':'), 2, 2);
            if (buffer.isEmpty()) {
                if (direction == QLatin1Char('-')) {
                    startDate = q->latestValidDate();
                } else {
                    startDate = q->earliestValidDate();
                }
            } else {
                startDate = q->readDate(buffer, KLocale::IsoFormat);
            }
            if (q->isValid(startDate)) {
                startYear = q->year(startDate);
            } else {
                startYear = eraEntry.section(QLatin1Char(':'), 1, 1).toInt();   //Use offset
            }

            buffer = eraEntry.section(QLatin1Char(':'), 3, 3);
            if (buffer.isEmpty()) {
                if (direction == QLatin1Char('-')) {
                    endDate = q->earliestValidDate();
                } else {
                    endDate = q->latestValidDate();
                }
            } else {
                endDate = q->readDate(buffer, KLocale::IsoFormat);
            }
            addEra(direction.toLatin1(), eraEntry.section(QLatin1Char(':'), 1, 1).toInt(),
                   startDate, startYear, endDate, eraEntry.section(QLatin1Char(':'), 4, 4),
                   eraEntry.section(QLatin1Char(':'), 5, 5), eraEntry.section(QLatin1Char(':'), 6));
        }
        ++i;
        eraKey = QString::fromLatin1("Era%1").arg(i);
    }

    if (m_eraList->isEmpty()) {
        loadDefaultEraList();
    }
}

void KCalendarSystemPrivate::addEra(char direction, int offset,
                                    const QDate &startDate, int startYear, const QDate &endDate,
                                    const QString &name, const QString &shortName,
                                    const QString &format)
{
    KCalendarEra newEra;

    newEra.m_sequence = m_eraList->count() + 1;
    if (direction == '-') {
        newEra.m_direction = -1;
    } else {
        newEra.m_direction = 1;
    }
    newEra.m_offset = offset;
    newEra.m_startDate = startDate;
    newEra.m_startYear = startYear;
    newEra.m_endDate = endDate;
    newEra.m_longName = name;
    newEra.m_shortName = shortName;
    newEra.m_format = format;

    m_eraList->append(newEra);
}

int KCalendarSystemPrivate::shortYearWindowStartYear() const
{
    return m_shortYearWindowStartYear;
}

int KCalendarSystemPrivate::applyShortYearWindow(int inputYear) const
{
    if (inputYear >= 0 && inputYear <= 99) {
        int shortStartYear = m_shortYearWindowStartYear % 100;
        int yearOffset = m_shortYearWindowStartYear - shortStartYear;
        if (inputYear >= shortStartYear) {
            return inputYear + yearOffset;
        } else {
            return inputYear + yearOffset + 100;
        }
    } else {
        return inputYear;
    }
}

void KCalendarSystemPrivate::loadShortYearWindowStartYear(const KConfigGroup & cg)
{
    // Default to 2000 for backwards compatibility
    // as that's the old readDate() default value
    int startYear = 2000;
    if (cg.exists()) {
        startYear = cg.readEntry("ShortYearWindowStartYear", 2000);
    }
    m_shortYearWindowStartYear = startYear;
}

KSharedConfig::Ptr KCalendarSystemPrivate::config()
{
    if (m_config == KSharedConfig::Ptr()) {
        return KSharedConfig::openConfig();
    } else {
        return m_config;
    }
}

void KCalendarSystemPrivate::loadConfig(const QString & calendarType)
{
    KConfigGroup localeGroup(config(), QString::fromLatin1("Locale"));
    KConfigGroup calendarGroup = localeGroup.group(QString::fromLatin1("KCalendarSystem %1").arg(calendarType));
    loadEraList(calendarGroup);
    loadShortYearWindowStartYear(calendarGroup);
}

KCalendarSystem::KCalendarSystem(KCalendarSystemPrivate &dd, const KSharedConfig::Ptr config, const KLocale *locale)
               : d_ptr(&dd)
{
    d_ptr->m_config = config;
    d_ptr->m_locale = locale;
}

KCalendarSystem::~KCalendarSystem()
{
    delete d_ptr;
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::calendarLabel() const
{
    return KCalendarSystem::calendarLabel(calendarSystem());
}

bool KCalendarSystem::isValid(int year, int month, int day) const
{
    Q_D(const KCalendarSystem);

    return year >= d->earliestValidYear() && year <= d->latestValidYear() && (d->hasYearZero() || year != 0) &&
           month >= 1 && month <= d->monthsInYear(year) &&
           day >= 1 && day <= d->daysInMonth(year, month);
}

bool KCalendarSystem::isValid(int year, int dayOfYear) const
{
    Q_D(const KCalendarSystem);

    return (isValid(year, 1, 1) && dayOfYear > 0 && dayOfYear <= d->daysInYear(year));
}

bool KCalendarSystem::isValid(const QString &eraName, int yearInEra, int month, int day) const
{
    Q_D(const KCalendarSystem);

    KCalendarEra era = d->era(eraName, yearInEra);
    return (era.isValid() && isValid(era.year(yearInEra), month, day));
}

// NOT VIRTUAL - If override needed use shared-d
bool KCalendarSystem::isValidIsoWeekDate(int year, int isoWeekNumber, int dayOfIsoWeek) const
{
    Q_D(const KCalendarSystem);

    //Tests Year value in standard YMD isValid()
    if (!isValid(year, 1, 1)) {
        return false;
    }

    //Test Week Number falls in valid range for this year
    int weeksInThisYear = weeksInYear(year);
    if (isoWeekNumber < 1 || isoWeekNumber  > weeksInThisYear) {
        return false;
    }

    //Test Day of Week Number falls in valid range
    if (dayOfIsoWeek < 1 || dayOfIsoWeek > d->daysInWeek()) {
        return false;
    }

    //If not in earliest or latest years then all OK
    //Otherwise need to check don't fall into previous or next year that would be invalid
    if (year == d->earliestValidYear() && isoWeekNumber == 1) {
        //If firstDayOfYear falls on or before Thursday then firstDayOfYear falls in week 1 this
        //year and if wanted dayOfIsoWeek falls before firstDayOfYear then falls in previous year
        //and so in invalid year
        int dowFirstDay = dayOfWeek(d->firstDayOfYear(year));
        if (dowFirstDay <= 4 && dayOfIsoWeek < dowFirstDay) {
            return false;
        }
    } else if (year == d->latestValidYear() && isoWeekNumber == weeksInThisYear) {
        //If lastDayOfYear falls on or after Thursday then lastDayOfYear falls in last week this
        //year and if wanted dayOfIsoWeek falls after lastDayOfYear then falls in next year
        //and so in invalid year
        int dowLastDay = dayOfWeek(d->lastDayOfYear(year));
        if (dowLastDay >= 4 && dayOfIsoWeek > dowLastDay) {
            return false;
        }
    }

    return true;
}

bool KCalendarSystem::setDate(QDate &date, int year, int month, int day) const
{
    date = QDate();

    if (isValid(year, month, day)) {
        qint64 jd;
        dateToJulianDay(year, month, day, jd);
        QDate calcDate = QDate::fromJulianDay(jd);

        if (isValid(calcDate)) {
            date = calcDate;
            return true;
        }
    }

    return false;
}

// NOT VIRTUAL - If override needed use shared-d
bool KCalendarSystem::setDate(QDate &date, int year, int dayOfYear) const
{
    date = QDate();

    if (isValid(year, dayOfYear)) {
        qint64 jd;
        dateToJulianDay(year, 1, 1, jd);
        QDate calcDate = QDate::fromJulianDay(jd + dayOfYear - 1);
        if (isValid(calcDate)) {
            date = calcDate;
            return true;
        }
    }

    return false;
}

// NOT VIRTUAL - If override needed use shared-d
bool KCalendarSystem::setDate(QDate &date, QString eraName, int yearInEra, int month, int day) const
{
    Q_D(const KCalendarSystem);

    KCalendarEra era = d->era(eraName, yearInEra);
    return (era.isValid() && setDate(date, era.year(yearInEra), month, day));
}

// NOT VIRTUAL - If override needed use shared-d
bool KCalendarSystem::setDateIsoWeek(QDate &date, int year, int isoWeekNumber, int dayOfIsoWeek) const
{
    Q_D(const KCalendarSystem);

    date = QDate();

    if (isValidIsoWeekDate(year, isoWeekNumber, dayOfIsoWeek)) {

        QDate calcDate = d->firstDayOfYear(year);
        int dowFirstDayOfYear = dayOfWeek(calcDate);

        int daysToAdd = (d->daysInWeek() * (isoWeekNumber - 1)) + dayOfIsoWeek;

        if (dowFirstDayOfYear <= 4) {
            calcDate = calcDate.addDays(daysToAdd - dowFirstDayOfYear);
        } else {
            calcDate = calcDate.addDays(daysInWeek(calcDate) + daysToAdd - dowFirstDayOfYear);
        }

        if (isValid(calcDate)) {
            date = calcDate;
            return true;
        }
    }

    return false;
}

// Deprecated
bool KCalendarSystem::setYMD(QDate &date, int year, int month, int day) const
{
    return setDate(date, year, month, day);
}

// NOT VIRTUAL - If override needed use shared-d
void KCalendarSystem::getDate(const QDate date, int *year, int *month, int *day) const
{
    int y, m, d;

    if (isValid(date)) {
        julianDayToDate(date.toJulianDay(), y, m, d);
    } else {
        y = 0;  // How do you denote invalid year when we support -ve years?
        m = 0;
        d = 0;
    }

    if (year) {
        *year = y;
    }
    if (month) {
        *month = m;
    }
    if (day) {
        *day = d;
    }

}

int KCalendarSystem::year(const QDate &date) const
{
    if (isValid(date)) {
        int year, month, day;

        julianDayToDate(date.toJulianDay(), year, month, day);

        return year;
    }

    return 0;  // How do you denote invalid year when we support -ve years?
}

int KCalendarSystem::month(const QDate &date) const
{
    if (isValid(date)) {
        int year, month, day;

        julianDayToDate(date.toJulianDay(), year, month, day);

        return month;
    }

    return 0;
}

int KCalendarSystem::day(const QDate &date) const
{
    if (isValid(date)) {
        int year, month, day;

        julianDayToDate(date.toJulianDay(), year, month, day);

        return day;
    }

    return 0;
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::eraName(const QDate &date, StringFormat format) const
{
    Q_D(const KCalendarSystem);

    if (isValid(date)) {
        if (format == LongFormat) {
            return d->era(date).name(KLocale::LongName);
        } else {
            return d->era(date).name(KLocale::ShortName);
        }
    }

    return QString();
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::eraYear(const QDate &date, StringFormat format) const
{
    Q_UNUSED(format)
    Q_D(const KCalendarSystem);

    if (isValid(date)) {
        return formatDate(date, d->era(date).format());
    }

    return QString();
}

// NOT VIRTUAL - If override needed use shared-d
int KCalendarSystem::yearInEra(const QDate &date) const
{
    Q_D(const KCalendarSystem);

    if (isValid(date)) {
        return d->era(date).yearInEra(year(date));
    }

    return -1;
}

// NOT VIRTUAL - If override needed use shared-d
QList<KCalendarEra> *KCalendarSystem::eraList() const
{
    Q_D(const KCalendarSystem);

    return d->eraList();
}

// NOT VIRTUAL - If override needed use shared-d
KCalendarEra KCalendarSystem::era(const QDate &eraDate) const
{
    Q_D(const KCalendarSystem);

    return d->era(eraDate);
}

// NOT VIRTUAL - If override needed use shared-d
KCalendarEra KCalendarSystem::era(const QString &eraName, int yearInEra) const
{
    Q_D(const KCalendarSystem);

    return d->era(eraName, yearInEra);
}

QDate KCalendarSystem::addYears(const QDate &date, int numYears) const
{
    Q_D(const KCalendarSystem);

    if (isValid(date)) {

        int originalYear, originalMonth, originalDay;
        julianDayToDate(date.toJulianDay(), originalYear, originalMonth, originalDay);

        int newYear = d->addYears(originalYear, numYears);
        int newMonth = originalMonth;
        int newDay = originalDay;

        //Adjust day number if new month has fewer days than old month
        int daysInNewMonth = d->daysInMonth(newYear, newMonth);
        if (daysInNewMonth < originalDay) {
            newDay = daysInNewMonth;
        }

        QDate newDate;
        setDate(newDate, newYear, newMonth, newDay);
        return newDate;

    }

    return QDate();
}

QDate KCalendarSystem::addMonths(const QDate &date, int numMonths) const
{
    Q_D(const KCalendarSystem);

    if (isValid(date)) {

        int originalYear, originalMonth, originalDay;
        julianDayToDate(date.toJulianDay(), originalYear, originalMonth, originalDay);

        int monthsInOriginalYear = d->monthsInYear(originalYear);

        int newYear = d->addYears(originalYear, (originalMonth + numMonths) / monthsInOriginalYear);
        int newMonth = (originalMonth + numMonths) % monthsInOriginalYear;
        int newDay = originalDay;

        if (newMonth == 0) {
            newYear = d->addYears(newYear, - 1);
            newMonth = monthsInOriginalYear;
        }
        if (newMonth < 0) {
            newYear = d->addYears(newYear, - 1);
            newMonth = newMonth + monthsInOriginalYear;
        }

        //Adjust day number if new month has fewer days than old month
        int daysInNewMonth = d->daysInMonth(newYear, newMonth);
        if (daysInNewMonth < originalDay) {
            newDay = daysInNewMonth;
        }

        QDate newDate;
        setDate(newDate, newYear, newMonth, newDay);
        return newDate;

    }

    return QDate();
}

// NOT VIRTUAL - Uses shared-d instead
void KCalendarSystem::dateDifference(const QDate &fromDate, const QDate &toDate,
                                     int *yearsDiff, int *monthsDiff, int *daysDiff, int *direction) const
{
    Q_D(const KCalendarSystem);

    if (isValid(fromDate) && isValid(toDate)) {
        d->dateDifference(fromDate, toDate, yearsDiff, monthsDiff, daysDiff, direction);
    }
}

// NOT VIRTUAL - Uses shared-d instead
int KCalendarSystem::yearsDifference(const QDate &fromDate, const QDate &toDate) const
{
    Q_D(const KCalendarSystem);

    if (isValid(fromDate) && isValid(toDate)) {
        return d->yearsDifference(fromDate, toDate);
    }

    return 0;
}

// NOT VIRTUAL - Uses shared-d instead
int KCalendarSystem::monthsDifference(const QDate &fromDate, const QDate &toDate) const
{
    Q_D(const KCalendarSystem);

    if (isValid(fromDate) && isValid(toDate)) {
        return d->monthsDifference(fromDate, toDate);
    }

    return 0;
}

int KCalendarSystem::monthsInYear(const QDate &date) const
{
    Q_D(const KCalendarSystem);

    if (isValid(date)) {
        return d->monthsInYear(year(date));
    }

    return -1;
}

int KCalendarSystem::monthsInYear(int year) const
{
    Q_D(const KCalendarSystem);

    if (isValid(year, 1, 1)) {
        return d->monthsInYear(year);
    }

    return -1;
}

int KCalendarSystem::weeksInYear(int year, KLocale::WeekNumberSystem weekNumberSystem) const
{
    Q_D(const KCalendarSystem);

    if (!isValid(year, 1, 1))
        return -1;

    switch (weekNumberSystem) {
    case KLocale::IsoWeekNumber:
        return d->isoWeeksInYear(year);
    case KLocale::FirstFullWeek:
        return d->regularWeeksInYear(year, locale()->weekStartDay(), 0);
    case KLocale::FirstPartialWeek:
        return d->regularWeeksInYear(year, locale()->weekStartDay(), 1);
    case KLocale::SimpleWeek:
        return d->simpleWeeksInYear(year);
    case KLocale::DefaultWeekNumber:
    default:
        return weeksInYear(year, locale()->weekNumberSystem());
    }
}

int KCalendarSystem::daysInYear(const QDate &date) const
{
    Q_D(const KCalendarSystem);

    if (isValid(date)) {
        return d->daysInYear(year(date));
    }

    return -1;
}

int KCalendarSystem::daysInYear(int year) const
{
    Q_D(const KCalendarSystem);

    if (isValid(year, 1, 1)) {
        return d->daysInYear(year);
    }

    return -1;
}

int KCalendarSystem::daysInMonth(const QDate &date) const
{
    Q_D(const KCalendarSystem);

    if (isValid(date)) {
        int year, month, day;
        julianDayToDate(date.toJulianDay(), year, month, day);
        return d->daysInMonth(year, month);
    }

    return -1;
}

int KCalendarSystem::daysInMonth(int year, int month) const
{
    Q_D(const KCalendarSystem);

    if (isValid(year, month, 1)) {
        return d->daysInMonth(year, month);
    }

    return -1;
}

// If ever changed to depend on the date, a lot of code will break.
// Don't do that without testing and fixing at least all of KDE SC.
int KCalendarSystem::daysInWeek(const QDate &date) const
{
    Q_UNUSED(date)
    Q_D(const KCalendarSystem);
    return d->daysInWeek();
}

int KCalendarSystem::dayOfYear(const QDate &date) const
{
    Q_D(const KCalendarSystem);

    if (isValid(date)) {
        return d->dayOfYear(date);
    }

    return -1;
}

int KCalendarSystem::dayOfWeek(const QDate &date) const
{
    Q_D(const KCalendarSystem);

    if (isValid(date)) {
        return d->dayOfWeek(date);
    }

    return -1;
}

int KCalendarSystem::week(const QDate &date, KLocale::WeekNumberSystem weekNumberSystem, int *yearNum) const
{
    Q_D(const KCalendarSystem);

    if (!isValid(date))
        return -1;

    switch (weekNumberSystem) {
    case KLocale::IsoWeekNumber:
        return d->isoWeekNumber(date, yearNum);
    case KLocale::FirstFullWeek:
        return d->regularWeekNumber(date, locale()->weekStartDay(), 0, yearNum);
    case KLocale::FirstPartialWeek:
        return d->regularWeekNumber(date, locale()->weekStartDay(), 1, yearNum);
    case KLocale::SimpleWeek:
        return d->simpleWeekNumber(date, yearNum);
    case KLocale::DefaultWeekNumber:
    default:
        return week(date, locale()->weekNumberSystem(), yearNum);
    }
}

bool KCalendarSystem::isLeapYear(int year) const
{
    Q_D(const KCalendarSystem);

    if (isValid(year, 1, 1)) {
        return d->isLeapYear(year);
    }

    return false;
}

bool KCalendarSystem::isLeapYear(const QDate &date) const
{
    Q_D(const KCalendarSystem);

    if (isValid(date)) {
        return d->isLeapYear(year(date));
    }

    return false;
}

// NOT VIRTUAL - If override needed use shared-d
QDate KCalendarSystem::firstDayOfYear(int year) const
{
    Q_D(const KCalendarSystem);

    if (isValid(year, 1, 1)) {
        return d->firstDayOfYear(year);
    }

    return QDate();
}

// NOT VIRTUAL - If override needed use shared-d
QDate KCalendarSystem::lastDayOfYear(int year) const
{
    Q_D(const KCalendarSystem);

    if (isValid(year, 1, 1)) {
        return d->lastDayOfYear(year);
    }

    return QDate();
}

// NOT VIRTUAL - If override needed use shared-d
QDate KCalendarSystem::firstDayOfYear(const QDate &date) const
{
    Q_D(const KCalendarSystem);

    if (isValid(date)) {
        return d->firstDayOfYear(year(date));
    }

    return QDate();
}

// NOT VIRTUAL - If override needed use shared-d
QDate KCalendarSystem::lastDayOfYear(const QDate &date) const
{
    Q_D(const KCalendarSystem);

    if (isValid(date)) {
        return d->lastDayOfYear(year(date));
    }

    return QDate();
}

// NOT VIRTUAL - If override needed use shared-d
QDate KCalendarSystem::firstDayOfMonth(int year, int month) const
{
    Q_D(const KCalendarSystem);

    if (isValid(year, month, 1)) {
        return d->firstDayOfMonth(year, month);
    }

    return QDate();
}

// NOT VIRTUAL - If override needed use shared-d
QDate KCalendarSystem::lastDayOfMonth(int year, int month) const
{
    Q_D(const KCalendarSystem);

    if (isValid(year, month, 1)) {
        return d->lastDayOfMonth(year, month);
    }

    return QDate();
}

// NOT VIRTUAL - If override needed use shared-d
QDate KCalendarSystem::firstDayOfMonth(const QDate &date) const
{
    Q_D(const KCalendarSystem);

    if (isValid(date)) {
        int year, month;
        getDate(date, &year, &month, 0);
        return d->firstDayOfMonth(year, month);
    }

    return QDate();
}

// NOT VIRTUAL - If override needed use shared-d
QDate KCalendarSystem::lastDayOfMonth(const QDate &date) const
{
    Q_D(const KCalendarSystem);

    if (isValid(date)) {
        int year, month;
        getDate(date, &year, &month, 0);
        return d->lastDayOfMonth(year, month);
    }

    return QDate();
}

QString KCalendarSystem::monthName(int month, int year, KCalendarSystem::MonthNameFormat format) const
{
    Q_D(const KCalendarSystem);

    if (!isValid(year, month, 1)) {
        return QString();
    }

    if (format == KCalendarSystem::NarrowName) {
        return d->monthName(month, year, KLocale::NarrowName, false);
    }

    if (format == KCalendarSystem::ShortNamePossessive) {
        return d->monthName(month, year, KLocale::ShortName, true);
    }

    if (format == KCalendarSystem::ShortName) {
        return d->monthName(month, year, KLocale::ShortName, false);
    }

    if (format == KCalendarSystem::LongNamePossessive) {
        return d->monthName(month, year, KLocale::LongName, true);
    }

    // KCalendarSystem::LongName or any other
    return d->monthName(month, year, KLocale::LongName, false);
}

QString KCalendarSystem::monthName(const QDate &date, MonthNameFormat format) const
{
    if (isValid(date)) {
        int year, month;
        getDate(date, &year, &month, 0);
        return monthName(month, year, format);
    }

    return QString();
}

QString KCalendarSystem::weekDayName(int weekDay, KCalendarSystem::WeekDayNameFormat format) const
{
    Q_D(const KCalendarSystem);

    if (weekDay < 1 || weekDay > d->daysInWeek()) {
        return QString();
    }

    if (format == KCalendarSystem::NarrowDayName) {
        return d->weekDayName(weekDay, KLocale::NarrowName);
    }

    if (format == KCalendarSystem::ShortDayName) {
        return d->weekDayName(weekDay, KLocale::ShortName);
    }

    return d->weekDayName(weekDay, KLocale::LongName);
}

QString KCalendarSystem::weekDayName(const QDate &date, WeekDayNameFormat format) const
{
    if (isValid(date)) {
        return weekDayName(dayOfWeek(date), format);
    }

    return QString();
}

QString KCalendarSystem::yearString(const QDate &date, StringFormat format) const
{
    if (format == ShortFormat) {
        return formatDate(date, KLocale::Year, KLocale::ShortNumber);
    } else {
        return formatDate(date, KLocale::Year, KLocale::LongNumber);
    }
}

QString KCalendarSystem::monthString(const QDate &date, StringFormat format) const
{
    if (format == ShortFormat) {
        return formatDate(date, KLocale::Month, KLocale::ShortNumber);
    } else {
        return formatDate(date, KLocale::Month, KLocale::LongNumber);
    }
}

QString KCalendarSystem::dayString(const QDate &date, StringFormat format) const
{
    if (format == ShortFormat) {
        return formatDate(date, KLocale::Day, KLocale::ShortNumber);
    } else {
        return formatDate(date, KLocale::Day, KLocale::LongNumber);
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::yearInEraString(const QDate &date, StringFormat format) const
{
    if (format == ShortFormat) {
        return formatDate(date, KLocale::YearInEra, KLocale::ShortNumber);
    } else {
        return formatDate(date, KLocale::YearInEra, KLocale::LongNumber);
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::dayOfYearString(const QDate &date, StringFormat format) const
{
    if (format == ShortFormat) {
        return formatDate(date, KLocale::DayOfYear, KLocale::ShortNumber);
    } else {
        return formatDate(date, KLocale::DayOfYear, KLocale::LongNumber);
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::dayOfWeekString(const QDate &date) const
{
    return formatDate(date, KLocale::DayOfWeek, KLocale::ShortNumber);
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::weekNumberString(const QDate &date, StringFormat format) const
{
    if (format == ShortFormat) {
        return formatDate(date, KLocale::Week, KLocale::ShortNumber);
    } else {
        return formatDate(date, KLocale::Week, KLocale::LongNumber);
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::monthsInYearString(const QDate &date, StringFormat format) const
{
    if (format == ShortFormat) {
        return formatDate(date, KLocale::MonthsInYear, KLocale::ShortNumber);
    } else {
        return formatDate(date, KLocale::MonthsInYear, KLocale::LongNumber);
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::weeksInYearString(const QDate &date, StringFormat format) const
{
    if (format == ShortFormat) {
        return formatDate(date, KLocale::WeeksInYear, KLocale::ShortNumber);
    } else {
        return formatDate(date, KLocale::WeeksInYear, KLocale::LongNumber);
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::daysInYearString(const QDate &date, StringFormat format) const
{
    if (format == ShortFormat) {
        return formatDate(date, KLocale::DaysInYear, KLocale::ShortNumber);
    } else {
        return formatDate(date, KLocale::DaysInYear, KLocale::LongNumber);
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::daysInMonthString(const QDate &date, StringFormat format) const
{
    if (format == ShortFormat) {
        return formatDate(date, KLocale::DaysInMonth, KLocale::ShortNumber);
    } else {
        return formatDate(date, KLocale::DaysInMonth, KLocale::LongNumber);
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::daysInWeekString(const QDate &date) const
{
    return formatDate(date, KLocale::DaysInWeek, KLocale::ShortNumber);
}

int KCalendarSystem::yearStringToInteger(const QString &yearString, int &readLength) const
{
    Q_D(const KCalendarSystem);

    QString minus = i18nc("Negative symbol as used for year numbers, e.g. -5 = 5 BC", "-");
    if (yearString.startsWith(minus)) {
        int value = d->integerFromString(yearString.mid(minus.length()), 4, readLength);
        if (readLength > 0 && value >= 0) {
            readLength = readLength + minus.length();
            return value * -1;
        } else {
            return value;
        }
    }

    return d->integerFromString(yearString, 4, readLength);
}

int KCalendarSystem::monthStringToInteger(const QString &monthString, int &readLength) const
{
    Q_D(const KCalendarSystem);
    return d->integerFromString(monthString, 2, readLength);
}

int KCalendarSystem::dayStringToInteger(const QString &dayString, int &readLength) const
{
    Q_D(const KCalendarSystem);
    return d->integerFromString(dayString, 2, readLength);
}

QString KCalendarSystem::formatDate(const QDate &fromDate, KLocale::DateFormat toFormat) const
{
    if (!fromDate.isValid()) {
        return QString();
    }

    if (toFormat == KLocale::FancyShortDate || toFormat == KLocale::FancyLongDate) {
        QDate now = KDateTime::currentLocalDate();
        int daysToNow = fromDate.daysTo(now);
        switch (daysToNow) {
        case 0:
            return i18n("Today");
        case 1:
            return i18n("Yesterday");
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            return weekDayName(fromDate);
        default:
            break;
        }
    }

    switch (toFormat) {
    case KLocale::LongDate:
    case KLocale::FancyLongDate:
        return formatDate(fromDate, locale()->dateFormat());
    case KLocale::IsoDate:
        return formatDate(fromDate, QLatin1String("%Y-%m-%d"));
    case KLocale::IsoWeekDate:
        return formatDate(fromDate, QLatin1String("%Y-W%V-%u"));
    case KLocale::IsoOrdinalDate:
        return formatDate(fromDate, QLatin1String("%Y-%j"));
    case KLocale::ShortDate:
    case KLocale::FancyShortDate:
    default:
        return formatDate(fromDate, locale()->dateFormatShort());
    }

}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::formatDate(const QDate &fromDate, const QString &toFormat,
                                    KLocale::DateTimeFormatStandard standard) const
{
    return formatDate(fromDate, toFormat, locale()->dateTimeDigitSet(), standard);
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::formatDate(const QDate &fromDate, const QString &toFormat, KLocale::DigitSet digitSet,
                                    KLocale::DateTimeFormatStandard formatStandard) const
{
    if (!isValid(fromDate) || toFormat.isEmpty()) {
        return QString();
    }

    KDateTimeFormatter formatter;
    return formatter.formatDate(fromDate, toFormat, this, locale(), digitSet, formatStandard);
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::formatDate(const QDate &date, KLocale::DateTimeComponent component,
                                    KLocale::DateTimeComponentFormat format,
                                    KLocale::WeekNumberSystem weekNumberSystem) const
{
    Q_D(const KCalendarSystem);

    switch (component) {
    case KLocale::Year:
    case KLocale::YearName:
        switch (format) {
        case KLocale::ShortName:
        case KLocale::NarrowName:
        case KLocale::ShortNumber:
            return formatDate(date, QLatin1String("%y"));
        case KLocale::LongNumber:
        case KLocale::LongName:
        case KLocale::DefaultComponentFormat:
        default:
            return formatDate(date, QLatin1String("%Y"));
        }
    case KLocale::Month:
        switch (format) {
        case KLocale::LongName:
            return monthName(date, KCalendarSystem::LongName);
        case KLocale::ShortName:
            return monthName(date, KCalendarSystem::ShortName);
        case KLocale::NarrowName:
            return monthName(date, KCalendarSystem::NarrowName);
        case KLocale::LongNumber:
            return formatDate(date, QLatin1String("%m"));
        case KLocale::ShortNumber:
        case KLocale::DefaultComponentFormat:
        default:
            return formatDate(date, QLatin1String("%n"));
        }
    case KLocale::MonthName:
        switch (format) {
        case KLocale::NarrowName:
            return monthName(date, KCalendarSystem::NarrowName);
        case KLocale::ShortName:
        case KLocale::ShortNumber:
            return monthName(date, KCalendarSystem::ShortName);
        case KLocale::LongName:
        case KLocale::LongNumber:
        case KLocale::DefaultComponentFormat:
        default:
            return monthName(date, KCalendarSystem::LongName);
        }
    case KLocale::Day:
    case KLocale::DayName:
        switch (format) {
        case KLocale::LongNumber:
        case KLocale::LongName:
            return formatDate(date, QLatin1String("%d"));
        case KLocale::ShortName:
        case KLocale::NarrowName:
        case KLocale::ShortNumber:
        case KLocale::DefaultComponentFormat:
        default:
            return formatDate(date, QLatin1String("%e"));
        }
    case KLocale::JulianDay:
        return d->stringFromInteger(date.toJulianDay(), 0);
    case KLocale::EraName:
        switch (format) {
        case KLocale::LongNumber:
        case KLocale::LongName:
            return eraName(date, KCalendarSystem::LongFormat);
        case KLocale::ShortName:
        case KLocale::NarrowName:
        case KLocale::ShortNumber:
        case KLocale::DefaultComponentFormat:
        default:
            return eraName(date, KCalendarSystem::ShortFormat);
        }
    case KLocale::EraYear:
        switch (format) {
        case KLocale::LongNumber:
        case KLocale::LongName:
            return eraYear(date, KCalendarSystem::LongFormat);
        case KLocale::ShortName:
        case KLocale::NarrowName:
        case KLocale::ShortNumber:
        case KLocale::DefaultComponentFormat:
        default:
            return eraYear(date, KCalendarSystem::ShortFormat);
        }
    case KLocale::YearInEra:
        switch (format) {
        case KLocale::LongNumber:
        case KLocale::LongName:
            return formatDate(date, QLatin1String("%4Ey"));
        case KLocale::ShortName:
        case KLocale::NarrowName:
        case KLocale::ShortNumber:
        case KLocale::DefaultComponentFormat:
        default:
            return formatDate(date, QLatin1String("%Ey"));
        }
    case KLocale::DayOfYear:
    case KLocale::DayOfYearName:
        switch (format) {
        case KLocale::LongNumber:
        case KLocale::LongName:
            return formatDate(date, QLatin1String("%j"));
        case KLocale::ShortName:
        case KLocale::NarrowName:
        case KLocale::ShortNumber:
        case KLocale::DefaultComponentFormat:
        default:
            return formatDate(date, QLatin1String("%-j"));
        }
    case KLocale::DayOfWeek:
        switch (format) {
        case KLocale::LongName:
            return weekDayName(date, KCalendarSystem::LongDayName);
        case KLocale::ShortName:
            return weekDayName(date, KCalendarSystem::ShortDayName);
        case KLocale::NarrowName:
            return weekDayName(date, KCalendarSystem::NarrowDayName);
        case KLocale::LongNumber:
        case KLocale::ShortNumber:
        case KLocale::DefaultComponentFormat:
        default:
            return formatDate(date, QLatin1String("%-u"));
        }
    case KLocale::DayOfWeekName:
        switch (format) {
        case KLocale::NarrowName:
            return weekDayName(date, KCalendarSystem::NarrowDayName);
        case KLocale::ShortName:
        case KLocale::ShortNumber:
            return weekDayName(date, KCalendarSystem::ShortDayName);
        case KLocale::LongName:
        case KLocale::LongNumber:
        case KLocale::DefaultComponentFormat:
        default:
            return weekDayName(date, KCalendarSystem::LongDayName);
        }
    case KLocale::Week:
        switch (format) {
        case KLocale::LongNumber:
        case KLocale::LongName:
            return d->stringFromInteger(week(date, weekNumberSystem, 0), 2, QLatin1Char('0'));
        case KLocale::ShortName:
        case KLocale::NarrowName:
        case KLocale::ShortNumber:
        case KLocale::DefaultComponentFormat:
        default:
            return d->stringFromInteger(week(date, weekNumberSystem, 0), 0, QLatin1Char('0'));
        }
    case KLocale::WeekYear: {
        int weekYear;
        QDate yearDate;
        week(date, weekNumberSystem, &weekYear);
        setDate(yearDate, weekYear, 1, 1);
        return formatDate(yearDate, KLocale::Year, format);
    }
    case KLocale::MonthsInYear:
        switch (format) {
        case KLocale::LongNumber:
        case KLocale::LongName:
            return d->stringFromInteger(monthsInYear(date), 2, QLatin1Char('0'));
        case KLocale::ShortName:
        case KLocale::NarrowName:
        case KLocale::ShortNumber:
        case KLocale::DefaultComponentFormat:
        default:
            return d->stringFromInteger(monthsInYear(date), 0, QLatin1Char('0'));
        }
    case KLocale::WeeksInYear:
        switch (format) {
        case KLocale::LongNumber:
        case KLocale::LongName:
            return d->stringFromInteger(weeksInYear(date), 2, QLatin1Char('0'));
        case KLocale::ShortName:
        case KLocale::NarrowName:
        case KLocale::ShortNumber:
        case KLocale::DefaultComponentFormat:
        default:
            return d->stringFromInteger(weeksInYear(date), 0, QLatin1Char('0'));
        }
    case KLocale::DaysInYear:
        switch (format) {
        case KLocale::LongNumber:
        case KLocale::LongName:
            return d->stringFromInteger(daysInYear(date), 3, QLatin1Char('0'));
        case KLocale::ShortName:
        case KLocale::NarrowName:
        case KLocale::ShortNumber:
        case KLocale::DefaultComponentFormat:
        default:
            return d->stringFromInteger(daysInYear(date), 0, QLatin1Char('0'));
        }
    case KLocale::DaysInMonth:
        switch (format) {
        case KLocale::LongNumber:
        case KLocale::LongName:
            return d->stringFromInteger(daysInMonth(date), 2, QLatin1Char('0'));
        case KLocale::ShortName:
        case KLocale::NarrowName:
        case KLocale::ShortNumber:
        case KLocale::DefaultComponentFormat:
        default:
            return d->stringFromInteger(daysInMonth(date), 0, QLatin1Char('0'));
        }
    case KLocale::DaysInWeek:
        switch (format) {
        case KLocale::LongNumber:
        case KLocale::LongName:
        case KLocale::ShortName:
        case KLocale::NarrowName:
        case KLocale::ShortNumber:
        case KLocale::DefaultComponentFormat:
        default:
            return d->stringFromInteger(d->daysInWeek(), 0);
        }
    default:
        return QString();
    }
}

QDate KCalendarSystem::readDate(const QString &str, bool *ok) const
{
    //Try each standard format in turn, start with the locale ones,
    //then the well defined standards
    QDate date = readDate(str, KLocale::ShortFormat, ok);
    if (!isValid(date)) {
        date = readDate(str, KLocale::NormalFormat, ok);
        if (!isValid(date)) {
            date = readDate(str, KLocale::IsoFormat, ok);
            if (!isValid(date)) {
                date = readDate(str, KLocale::IsoWeekFormat, ok);
                if (!isValid(date)) {
                    date = readDate(str, KLocale::IsoOrdinalFormat, ok);
                }
            }
        }
    }

    return date;
}

QDate KCalendarSystem::readDate(const QString &str, KLocale::ReadDateFlags flags, bool *ok) const
{
    if (flags & KLocale::ShortFormat) {
        return readDate(str, locale()->dateFormatShort(), ok);
    } else if (flags & KLocale::NormalFormat) {
        return readDate(str, locale()->dateFormat(), ok);
    } else if (flags & KLocale::IsoFormat) {
        return readDate(str, QLatin1String("%Y-%m-%d"), ok);
    } else if (flags & KLocale::IsoWeekFormat) {
        return readDate(str, QLatin1String("%Y-W%V-%u"), ok);
    } else if (flags & KLocale::IsoOrdinalFormat) {
        return readDate(str, QLatin1String("%Y-%j"), ok);
    }
    return QDate();
}

QDate KCalendarSystem::readDate(const QString &inputString, const QString &formatString, bool *ok) const
{
    return readDate(inputString, formatString, ok, KLocale::KdeFormat);
}

// NOT VIRTUAL - If override needed use shared-d
QDate KCalendarSystem::readDate(const QString &inputString, const QString &formatString, bool *ok,
                                KLocale::DateTimeFormatStandard formatStandard) const
{
    KDateTimeParser parser;
    QDate resultDate = parser.parseDate(inputString, formatString, this, locale(), locale()->dateTimeDigitSet(), formatStandard);
    if (ok) {
        *ok = resultDate.isValid();
    }
    return resultDate;
}

// NOT VIRTUAL - If override needed use shared-d
int KCalendarSystem::shortYearWindowStartYear() const
{
    Q_D(const KCalendarSystem);

    return d->shortYearWindowStartYear();
}

// NOT VIRTUAL - If override needed use shared-d
int KCalendarSystem::applyShortYearWindow(int inputYear) const
{
    Q_D(const KCalendarSystem);

    return d->applyShortYearWindow(inputYear);
}

const KLocale * KCalendarSystem::locale() const
{
    Q_D(const KCalendarSystem);

    return d->locale();
}
