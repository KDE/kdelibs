/*
    Copyright 2009, 2010 John Layt <john@layt.net>

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

// Derived gregorian kde calendar class

#include "kcalendarsystemgregorian_p.h"
#include "kcalendarsystemgregorianprivate_p.h"
#include "kcalendarera_p.h"

#include "klocale.h"
#include "klocalizedstring.h"
#include "kconfiggroup.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

// Shared d pointer base class definitions

KCalendarSystemGregorianPrivate::KCalendarSystemGregorianPrivate(KCalendarSystemGregorian *q)
                               : KCalendarSystemPrivate(q)
{
}

KCalendarSystemGregorianPrivate::~KCalendarSystemGregorianPrivate()
{
}

void KCalendarSystemGregorianPrivate::loadDefaultEraList()
{
    QString name, shortName, format;

    KConfigGroup lcg(config(), QString::fromLatin1("Locale"));
    KConfigGroup cg = lcg.group(QString::fromLatin1("KCalendarSystem %1").arg(q->calendarType()));
    m_useCommonEra = cg.readEntry("UseCommonEra", false);

    if (m_useCommonEra) {
        name = i18nc("Calendar Era: Gregorian Common Era, years < 0, LongFormat", "Before Common Era");
        shortName = i18nc("Calendar Era: Gregorian Common Era, years < 0, ShortFormat", "BCE");
    } else {
        name = i18nc("Calendar Era: Gregorian Christian Era, years < 0, LongFormat", "Before Christ");
        shortName = i18nc("Calendar Era: Gregorian Christian Era, years < 0, ShortFormat", "BC");
    }
    format = i18nc("(kdedt-format) Gregorian, BC, full era year format used for %EY, e.g. 2000 BC", "%Ey %EC");
    addEra('-', 1, q->epoch().addDays(-1), -1, q->earliestValidDate(), name, shortName, format);

    if (m_useCommonEra) {
        name = i18nc("Calendar Era: Gregorian Common Era, years > 0, LongFormat", "Common Era");
        shortName = i18nc("Calendar Era: Gregorian Common Era, years > 0, ShortFormat", "CE");
    } else {
        name = i18nc("Calendar Era: Gregorian Christian Era, years > 0, LongFormat", "Anno Domini");
        shortName = i18nc("Calendar Era: Gregorian Christian Era, years > 0, ShortFormat", "AD");
    }
    format = i18nc("(kdedt-format) Gregorian, AD, full era year format used for %EY, e.g. 2000 AD", "%Ey %EC");
    addEra('+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format);
}

int KCalendarSystemGregorianPrivate::monthsInYear(int year) const
{
    Q_UNUSED(year)
    return 12;
}

int KCalendarSystemGregorianPrivate::daysInMonth(int year, int month) const
{
    if (hasYearZero() && year <= 0) {
        --year;
    }
    return QDate(year, month, 1).daysInMonth();
}

int KCalendarSystemGregorianPrivate::daysInYear(int year) const
{
    if (isLeapYear(year)) {
        return 366;
    } else {
        return 365;
    }
}

bool KCalendarSystemGregorianPrivate::isLeapYear(int year) const
{
    if (hasYearZero() && year <= 0) {
        --year;
    }
    return QDate::isLeapYear(year);
}

bool KCalendarSystemGregorianPrivate::hasLeapMonths() const
{
    return false;
}

bool KCalendarSystemGregorianPrivate::hasYearZero() const
{
    return false;
}

int KCalendarSystemGregorianPrivate::maxMonthsInYear() const
{
    return 12;
}

int KCalendarSystemGregorianPrivate::earliestValidYear() const
{
    return -9999;
}

int KCalendarSystemGregorianPrivate::latestValidYear() const
{
    return 9999;
}

QString KCalendarSystemGregorianPrivate::monthName(int month, int year, KLocale::DateTimeComponentFormat format, bool possessive) const
{
    Q_UNUSED(year);

    QStringList languages = locale()->languageList();

    if (format == KLocale::NarrowName) {
        switch (month) {
        case 1:
            return ki18nc("Gregorian month 1 - KLocale::NarrowName",  "J").toString(languages);
        case 2:
            return ki18nc("Gregorian month 2 - KLocale::NarrowName",  "F").toString(languages);
        case 3:
            return ki18nc("Gregorian month 3 - KLocale::NarrowName",  "M").toString(languages);
        case 4:
            return ki18nc("Gregorian month 4 - KLocale::NarrowName",  "A").toString(languages);
        case 5:
            return ki18nc("Gregorian month 5 - KLocale::NarrowName",  "M").toString(languages);
        case 6:
            return ki18nc("Gregorian month 6 - KLocale::NarrowName",  "J").toString(languages);
        case 7:
            return ki18nc("Gregorian month 7 - KLocale::NarrowName",  "J").toString(languages);
        case 8:
            return ki18nc("Gregorian month 8 - KLocale::NarrowName",  "A").toString(languages);
        case 9:
            return ki18nc("Gregorian month 9 - KLocale::NarrowName",  "S").toString(languages);
        case 10:
            return ki18nc("Gregorian month 10 - KLocale::NarrowName", "O").toString(languages);
        case 11:
            return ki18nc("Gregorian month 11 - KLocale::NarrowName", "N").toString(languages);
        case 12:
            return ki18nc("Gregorian month 12 - KLocale::NarrowName", "D").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Gregorian month 1 - KLocale::ShortName Possessive",  "of Jan").toString(languages);
        case 2:
            return ki18nc("Gregorian month 2 - KLocale::ShortName Possessive",  "of Feb").toString(languages);
        case 3:
            return ki18nc("Gregorian month 3 - KLocale::ShortName Possessive",  "of Mar").toString(languages);
        case 4:
            return ki18nc("Gregorian month 4 - KLocale::ShortName Possessive",  "of Apr").toString(languages);
        case 5:
            return ki18nc("Gregorian month 5 - KLocale::ShortName Possessive",  "of May").toString(languages);
        case 6:
            return ki18nc("Gregorian month 6 - KLocale::ShortName Possessive",  "of Jun").toString(languages);
        case 7:
            return ki18nc("Gregorian month 7 - KLocale::ShortName Possessive",  "of Jul").toString(languages);
        case 8:
            return ki18nc("Gregorian month 8 - KLocale::ShortName Possessive",  "of Aug").toString(languages);
        case 9:
            return ki18nc("Gregorian month 9 - KLocale::ShortName Possessive",  "of Sep").toString(languages);
        case 10:
            return ki18nc("Gregorian month 10 - KLocale::ShortName Possessive", "of Oct").toString(languages);
        case 11:
            return ki18nc("Gregorian month 11 - KLocale::ShortName Possessive", "of Nov").toString(languages);
        case 12:
            return ki18nc("Gregorian month 12 - KLocale::ShortName Possessive", "of Dec").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && !possessive) {
        switch (month) {
        case 1:
            return ki18nc("Gregorian month 1 - KLocale::ShortName",  "Jan").toString(languages);
        case 2:
            return ki18nc("Gregorian month 2 - KLocale::ShortName",  "Feb").toString(languages);
        case 3:
            return ki18nc("Gregorian month 3 - KLocale::ShortName",  "Mar").toString(languages);
        case 4:
            return ki18nc("Gregorian month 4 - KLocale::ShortName",  "Apr").toString(languages);
        case 5:
            return ki18nc("Gregorian month 5 - KLocale::ShortName",  "May").toString(languages);
        case 6:
            return ki18nc("Gregorian month 6 - KLocale::ShortName",  "Jun").toString(languages);
        case 7:
            return ki18nc("Gregorian month 7 - KLocale::ShortName",  "Jul").toString(languages);
        case 8:
            return ki18nc("Gregorian month 8 - KLocale::ShortName",  "Aug").toString(languages);
        case 9:
            return ki18nc("Gregorian month 9 - KLocale::ShortName",  "Sep").toString(languages);
        case 10:
            return ki18nc("Gregorian month 10 - KLocale::ShortName", "Oct").toString(languages);
        case 11:
            return ki18nc("Gregorian month 11 - KLocale::ShortName", "Nov").toString(languages);
        case 12:
            return ki18nc("Gregorian month 12 - KLocale::ShortName", "Dec").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::LongName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Gregorian month 1 - KLocale::LongName Possessive",  "of January").toString(languages);
        case 2:
            return ki18nc("Gregorian month 2 - KLocale::LongName Possessive",  "of February").toString(languages);
        case 3:
            return ki18nc("Gregorian month 3 - KLocale::LongName Possessive",  "of March").toString(languages);
        case 4:
            return ki18nc("Gregorian month 4 - KLocale::LongName Possessive",  "of April").toString(languages);
        case 5:
            return ki18nc("Gregorian month 5 - KLocale::LongName Possessive",  "of May").toString(languages);
        case 6:
            return ki18nc("Gregorian month 6 - KLocale::LongName Possessive",  "of June").toString(languages);
        case 7:
            return ki18nc("Gregorian month 7 - KLocale::LongName Possessive",  "of July").toString(languages);
        case 8:
            return ki18nc("Gregorian month 8 - KLocale::LongName Possessive",  "of August").toString(languages);
        case 9:
            return ki18nc("Gregorian month 9 - KLocale::LongName Possessive",  "of September").toString(languages);
        case 10:
            return ki18nc("Gregorian month 10 - KLocale::LongName Possessive", "of October").toString(languages);
        case 11:
            return ki18nc("Gregorian month 11 - KLocale::LongName Possessive", "of November").toString(languages);
        case 12:
            return ki18nc("Gregorian month 12 - KLocale::LongName Possessive", "of December").toString(languages);
        default:
            return QString();
        }
    }

    // Default to LongName
    switch (month) {
    case 1:
        return ki18nc("Gregorian month 1 - KLocale::LongName",  "January").toString(languages);
    case 2:
        return ki18nc("Gregorian month 2 - KLocale::LongName",  "February").toString(languages);
    case 3:
        return ki18nc("Gregorian month 3 - KLocale::LongName",  "March").toString(languages);
    case 4:
        return ki18nc("Gregorian month 4 - KLocale::LongName",  "April").toString(languages);
    case 5:
        return ki18nc("Gregorian month 5 - KLocale::LongName",  "May").toString(languages);
    case 6:
        return ki18nc("Gregorian month 6 - KLocale::LongName",  "June").toString(languages);
    case 7:
        return ki18nc("Gregorian month 7 - KLocale::LongName",  "July").toString(languages);
    case 8:
        return ki18nc("Gregorian month 8 - KLocale::LongName",  "August").toString(languages);
    case 9:
        return ki18nc("Gregorian month 9 - KLocale::LongName",  "September").toString(languages);
    case 10:
        return ki18nc("Gregorian month 10 - KLocale::LongName", "October").toString(languages);
    case 11:
        return ki18nc("Gregorian month 11 - KLocale::LongName", "November").toString(languages);
    case 12:
        return ki18nc("Gregorian month 12 - KLocale::LongName", "December").toString(languages);
    default:
        return QString();
    }
}

QString KCalendarSystemGregorianPrivate::weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const
{
    QStringList languages = locale()->languageList();

    if (format == KLocale::NarrowName) {
        switch (weekDay) {
        case 1:
            return ki18nc("Gregorian weekday 1 - KLocale::NarrowName ", "M").toString(languages);
        case 2:
            return ki18nc("Gregorian weekday 2 - KLocale::NarrowName ", "T").toString(languages);
        case 3:
            return ki18nc("Gregorian weekday 3 - KLocale::NarrowName ", "W").toString(languages);
        case 4:
            return ki18nc("Gregorian weekday 4 - KLocale::NarrowName ", "T").toString(languages);
        case 5:
            return ki18nc("Gregorian weekday 5 - KLocale::NarrowName ", "F").toString(languages);
        case 6:
            return ki18nc("Gregorian weekday 6 - KLocale::NarrowName ", "S").toString(languages);
        case 7:
            return ki18nc("Gregorian weekday 7 - KLocale::NarrowName ", "S").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName  || format == KLocale:: ShortNumber) {
        switch (weekDay) {
        case 1:
            return ki18nc("Gregorian weekday 1 - KLocale::ShortName", "Mon").toString(languages);
        case 2:
            return ki18nc("Gregorian weekday 2 - KLocale::ShortName", "Tue").toString(languages);
        case 3:
            return ki18nc("Gregorian weekday 3 - KLocale::ShortName", "Wed").toString(languages);
        case 4:
            return ki18nc("Gregorian weekday 4 - KLocale::ShortName", "Thu").toString(languages);
        case 5:
            return ki18nc("Gregorian weekday 5 - KLocale::ShortName", "Fri").toString(languages);
        case 6:
            return ki18nc("Gregorian weekday 6 - KLocale::ShortName", "Sat").toString(languages);
        case 7:
            return ki18nc("Gregorian weekday 7 - KLocale::ShortName", "Sun").toString(languages);
        default: return QString();
        }
    }

    switch (weekDay) {
    case 1:
        return ki18nc("Gregorian weekday 1 - KLocale::LongName", "Monday").toString(languages);
    case 2:
        return ki18nc("Gregorian weekday 2 - KLocale::LongName", "Tuesday").toString(languages);
    case 3:
        return ki18nc("Gregorian weekday 3 - KLocale::LongName", "Wednesday").toString(languages);
    case 4:
        return ki18nc("Gregorian weekday 4 - KLocale::LongName", "Thursday").toString(languages);
    case 5:
        return ki18nc("Gregorian weekday 5 - KLocale::LongName", "Friday").toString(languages);
    case 6:
        return ki18nc("Gregorian weekday 6 - KLocale::LongName", "Saturday").toString(languages);
    case 7:
        return ki18nc("Gregorian weekday 7 - KLocale::LongName", "Sunday").toString(languages);
    default:
        return QString();
    }
}


KCalendarSystemGregorian::KCalendarSystemGregorian(const KSharedConfig::Ptr config,
                                                                     const KLocale *locale)
                        : KCalendarSystem(*new KCalendarSystemGregorianPrivate(this), config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemGregorian::KCalendarSystemGregorian(KCalendarSystemGregorianPrivate &dd,
                                                                     const KSharedConfig::Ptr config,
                                                                     const KLocale *locale)
                        : KCalendarSystem(dd, config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemGregorian::~KCalendarSystemGregorian()
{
}

QString KCalendarSystemGregorian::calendarType() const
{
    return QLatin1String("gregorian");
}

KLocale::CalendarSystem KCalendarSystemGregorian::calendarSystem() const
{
    return KLocale::GregorianCalendar;
}

QDate KCalendarSystemGregorian::epoch() const
{
    return QDate::fromJulianDay(1721426);
}

QDate KCalendarSystemGregorian::earliestValidDate() const
{
    Q_D(const KCalendarSystemGregorian);

    // Set to first day of year 9999 until confirm date formats & widgets support > 9999
    return QDate(d->hasYearZero() ? -10000 : -9999, 1, 1);
    // return QDate(std::num_limits<int>::min(), 1, 1);
}

QDate KCalendarSystemGregorian::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    // In Gregorian this is 9999-12-31, which is  is jd 5373484
    return QDate(9999, 12, 31);
    // return QDate(std::num_limits<int>::max(), 12, 31);
}

QString KCalendarSystemGregorian::monthName(int month, int year, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(month, year, format);
}

QString KCalendarSystemGregorian::monthName(const QDate &date, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(date, format);
}

QString KCalendarSystemGregorian::weekDayName(int weekDay, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(weekDay, format);
}

QString KCalendarSystemGregorian::weekDayName(const QDate &date, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(date, format);
}

int KCalendarSystemGregorian::yearStringToInteger(const QString &sNum, int &iLength) const
{
    return KCalendarSystem::yearStringToInteger(sNum, iLength);
}

bool KCalendarSystemGregorian::isLunar() const
{
    return false;
}

bool KCalendarSystemGregorian::isLunisolar() const
{
    return false;
}

bool KCalendarSystemGregorian::isSolar() const
{
    return true;
}

bool KCalendarSystemGregorian::isProleptic() const
{
    return true;
}

bool KCalendarSystemGregorian::julianDayToDate(qint64 jd, int &year, int &month, int &day) const
{
    Q_D(const KCalendarSystemGregorian);

    QDate date = QDate::fromJulianDay(jd);
    date.getDate(&year, &month, &day);

    if (d->hasYearZero() && year < 0) {
        ++year;
    }

    return date.isValid();
}

bool KCalendarSystemGregorian::dateToJulianDay(int year, int month, int day, qint64 &jd) const
{
    Q_D(const KCalendarSystemGregorian);

    if (d->hasYearZero() && year <= 0) {
        --year;
    }

    QDate date;
    date.setDate(year, month, day);

    jd = date.toJulianDay();
    return date.isValid();
}
