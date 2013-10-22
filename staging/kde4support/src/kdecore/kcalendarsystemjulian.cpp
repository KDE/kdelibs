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

#include "kcalendarsystemjulian_p.h"
#include "kcalendarsystemprivate_p.h"

#include "klocale.h"
#include "klocalizedstring.h"
#include "kconfiggroup.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

class KCalendarSystemJulianPrivate : public KCalendarSystemPrivate
{
public:
    explicit KCalendarSystemJulianPrivate(KCalendarSystemJulian *q);

    virtual ~KCalendarSystemJulianPrivate();

    // Virtual methods each calendar system must re-implement
    virtual void loadDefaultEraList();
    virtual int monthsInYear(int year) const;
    virtual int daysInMonth(int year, int month) const;
    virtual int daysInYear(int year) const;
    virtual bool isLeapYear(int year) const;
    virtual bool hasLeapMonths() const;
    virtual bool hasYearZero() const;
    virtual int maxMonthsInYear() const;
    virtual int earliestValidYear() const;
    virtual int latestValidYear() const;
    virtual QString monthName(int month, int year, KLocale::DateTimeComponentFormat format, bool possessive) const;
    virtual QString weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const;

    bool m_useCommonEra;
};

// Shared d pointer base class definitions

KCalendarSystemJulianPrivate::KCalendarSystemJulianPrivate(KCalendarSystemJulian *q)
                            : KCalendarSystemPrivate(q)
{
}

KCalendarSystemJulianPrivate::~KCalendarSystemJulianPrivate()
{
}

void KCalendarSystemJulianPrivate::loadDefaultEraList()
{
    QString name, shortName, format;

    KConfigGroup cg(config(), QString::fromLatin1("KCalendarSystem %1").arg(q->calendarType()));
    m_useCommonEra = cg.readEntry("UseCommonEra", false);

    if (m_useCommonEra) {
        name = i18nc("Calendar Era: Julian Common Era, years < 0, LongFormat", "Before Common Era");
        shortName = i18nc("Calendar Era: Julian Common Era, years < 0, ShortFormat", "BCE");
    } else {
        name = i18nc("Calendar Era: Julian Christian Era, years < 0, LongFormat", "Before Christ");
        shortName = i18nc("Calendar Era: Julian Christian Era, years < 0, ShortFormat", "BC");
    }
    format = i18nc("(kdedt-format) Julian, BC, full era year format used for %EY, e.g. 2000 BC", "%Ey %EC");
    addEra('-', 1, q->epoch().addDays(-1), -1, q->earliestValidDate(), name, shortName, format);

    if (m_useCommonEra) {
        name = i18nc("Calendar Era: Julian Common Era, years > 0, LongFormat", "Common Era");
        shortName = i18nc("Calendar Era: Julian Common Era, years > 0, ShortFormat", "CE");
    } else {
        name = i18nc("Calendar Era: Julian Christian Era, years > 0, LongFormat", "Anno Domini");
        shortName = i18nc("Calendar Era: Julian Christian Era, years > 0, ShortFormat", "AD");
    }
    format = i18nc("(kdedt-format) Julian, AD, full era year format used for %EY, e.g. 2000 AD", "%Ey %EC");
    addEra('+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format);
}

int KCalendarSystemJulianPrivate::monthsInYear(int year) const
{
    Q_UNUSED(year)
    return 12;
}

int KCalendarSystemJulianPrivate::daysInMonth(int year, int month) const
{
    if (month == 2) {
        if (isLeapYear(year)) {
            return 29;
        } else {
            return 28;
        }
    }

    if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    }

    return 31;
}

int KCalendarSystemJulianPrivate::daysInYear(int year) const
{
    if (isLeapYear(year)) {
        return 366;
    } else {
        return 365;
    }
}

bool KCalendarSystemJulianPrivate::isLeapYear(int year) const
{
    if (year < 1) {
        year = year + 1;
    }

    if (year % 4 == 0) {
        return true;
    }

    return false;
}

bool KCalendarSystemJulianPrivate::hasLeapMonths() const
{
    return false;
}

bool KCalendarSystemJulianPrivate::hasYearZero() const
{
    return false;
}

int KCalendarSystemJulianPrivate::maxMonthsInYear() const
{
    return 12;
}

int KCalendarSystemJulianPrivate::earliestValidYear() const
{
    return -4712;
}

int KCalendarSystemJulianPrivate::latestValidYear() const
{
    return 9999;
}

QString KCalendarSystemJulianPrivate::monthName(int month, int year, KLocale::DateTimeComponentFormat format, bool possessive) const
{
    Q_UNUSED(year);

    QStringList languages = locale()->languageList();

    if (format == KLocale::NarrowName) {
        switch (month) {
        case 1:
            return ki18nc("Julian month 1 - KLocale::NarrowName",  "J").toString(languages);
        case 2:
            return ki18nc("Julian month 2 - KLocale::NarrowName",  "F").toString(languages);
        case 3:
            return ki18nc("Julian month 3 - KLocale::NarrowName",  "M").toString(languages);
        case 4:
            return ki18nc("Julian month 4 - KLocale::NarrowName",  "A").toString(languages);
        case 5:
            return ki18nc("Julian month 5 - KLocale::NarrowName",  "M").toString(languages);
        case 6:
            return ki18nc("Julian month 6 - KLocale::NarrowName",  "J").toString(languages);
        case 7:
            return ki18nc("Julian month 7 - KLocale::NarrowName",  "J").toString(languages);
        case 8:
            return ki18nc("Julian month 8 - KLocale::NarrowName",  "A").toString(languages);
        case 9:
            return ki18nc("Julian month 9 - KLocale::NarrowName",  "S").toString(languages);
        case 10:
            return ki18nc("Julian month 10 - KLocale::NarrowName", "O").toString(languages);
        case 11:
            return ki18nc("Julian month 11 - KLocale::NarrowName", "N").toString(languages);
        case 12:
            return ki18nc("Julian month 12 - KLocale::NarrowName", "D").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Julian month 1 - KLocale::ShortName Possessive",  "of Jan").toString(languages);
        case 2:
            return ki18nc("Julian month 2 - KLocale::ShortName Possessive",  "of Feb").toString(languages);
        case 3:
            return ki18nc("Julian month 3 - KLocale::ShortName Possessive",  "of Mar").toString(languages);
        case 4:
            return ki18nc("Julian month 4 - KLocale::ShortName Possessive",  "of Apr").toString(languages);
        case 5:
            return ki18nc("Julian month 5 - KLocale::ShortName Possessive",  "of May").toString(languages);
        case 6:
            return ki18nc("Julian month 6 - KLocale::ShortName Possessive",  "of Jun").toString(languages);
        case 7:
            return ki18nc("Julian month 7 - KLocale::ShortName Possessive",  "of Jul").toString(languages);
        case 8:
            return ki18nc("Julian month 8 - KLocale::ShortName Possessive",  "of Aug").toString(languages);
        case 9:
            return ki18nc("Julian month 9 - KLocale::ShortName Possessive",  "of Sep").toString(languages);
        case 10:
            return ki18nc("Julian month 10 - KLocale::ShortName Possessive", "of Oct").toString(languages);
        case 11:
            return ki18nc("Julian month 11 - KLocale::ShortName Possessive", "of Nov").toString(languages);
        case 12:
            return ki18nc("Julian month 12 - KLocale::ShortName Possessive", "of Dec").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && !possessive) {
        switch (month) {
        case 1:
            return ki18nc("Julian month 1 - KLocale::ShortName",  "Jan").toString(languages);
        case 2:
            return ki18nc("Julian month 2 - KLocale::ShortName",  "Feb").toString(languages);
        case 3:
            return ki18nc("Julian month 3 - KLocale::ShortName",  "Mar").toString(languages);
        case 4:
            return ki18nc("Julian month 4 - KLocale::ShortName",  "Apr").toString(languages);
        case 5:
            return ki18nc("Julian month 5 - KLocale::ShortName",  "May").toString(languages);
        case 6:
            return ki18nc("Julian month 6 - KLocale::ShortName",  "Jun").toString(languages);
        case 7:
            return ki18nc("Julian month 7 - KLocale::ShortName",  "Jul").toString(languages);
        case 8:
            return ki18nc("Julian month 8 - KLocale::ShortName",  "Aug").toString(languages);
        case 9:
            return ki18nc("Julian month 9 - KLocale::ShortName",  "Sep").toString(languages);
        case 10:
            return ki18nc("Julian month 10 - KLocale::ShortName", "Oct").toString(languages);
        case 11:
            return ki18nc("Julian month 11 - KLocale::ShortName", "Nov").toString(languages);
        case 12:
            return ki18nc("Julian month 12 - KLocale::ShortName", "Dec").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::LongName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Julian month 1 - KLocale::LongName Possessive",  "of January").toString(languages);
        case 2:
            return ki18nc("Julian month 2 - KLocale::LongName Possessive",  "of February").toString(languages);
        case 3:
            return ki18nc("Julian month 3 - KLocale::LongName Possessive",  "of March").toString(languages);
        case 4:
            return ki18nc("Julian month 4 - KLocale::LongName Possessive",  "of April").toString(languages);
        case 5:
            return ki18nc("Julian month 5 - KLocale::LongName Possessive",  "of May").toString(languages);
        case 6:
            return ki18nc("Julian month 6 - KLocale::LongName Possessive",  "of June").toString(languages);
        case 7:
            return ki18nc("Julian month 7 - KLocale::LongName Possessive",  "of July").toString(languages);
        case 8:
            return ki18nc("Julian month 8 - KLocale::LongName Possessive",  "of August").toString(languages);
        case 9:
            return ki18nc("Julian month 9 - KLocale::LongName Possessive",  "of September").toString(languages);
        case 10:
            return ki18nc("Julian month 10 - KLocale::LongName Possessive", "of October").toString(languages);
        case 11:
            return ki18nc("Julian month 11 - KLocale::LongName Possessive", "of November").toString(languages);
        case 12:
            return ki18nc("Julian month 12 - KLocale::LongName Possessive", "of December").toString(languages);
        default:
            return QString();
        }
    }

    // Default to LongName
    switch (month) {
    case 1:
        return ki18nc("Julian month 1 - KLocale::LongName",  "January").toString(languages);
    case 2:
        return ki18nc("Julian month 2 - KLocale::LongName",  "February").toString(languages);
    case 3:
        return ki18nc("Julian month 3 - KLocale::LongName",  "March").toString(languages);
    case 4:
        return ki18nc("Julian month 4 - KLocale::LongName",  "April").toString(languages);
    case 5:
        return ki18nc("Julian month 5 - KLocale::LongName",  "May").toString(languages);
    case 6:
        return ki18nc("Julian month 6 - KLocale::LongName",  "June").toString(languages);
    case 7:
        return ki18nc("Julian month 7 - KLocale::LongName",  "July").toString(languages);
    case 8:
        return ki18nc("Julian month 8 - KLocale::LongName",  "August").toString(languages);
    case 9:
        return ki18nc("Julian month 9 - KLocale::LongName",  "September").toString(languages);
    case 10:
        return ki18nc("Julian month 10 - KLocale::LongName", "October").toString(languages);
    case 11:
        return ki18nc("Julian month 11 - KLocale::LongName", "November").toString(languages);
    case 12:
        return ki18nc("Julian month 12 - KLocale::LongName", "December").toString(languages);
    default:
        return QString();
    }
}

QString KCalendarSystemJulianPrivate::weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const
{
    QStringList languages = locale()->languageList();

    if (format == KLocale::NarrowName) {
        switch (weekDay) {
        case 1:
            return ki18nc("Julian weekday 1 - KLocale::NarrowName ", "M").toString(languages);
        case 2:
            return ki18nc("Julian weekday 2 - KLocale::NarrowName ", "T").toString(languages);
        case 3:
            return ki18nc("Julian weekday 3 - KLocale::NarrowName ", "W").toString(languages);
        case 4:
            return ki18nc("Julian weekday 4 - KLocale::NarrowName ", "T").toString(languages);
        case 5:
            return ki18nc("Julian weekday 5 - KLocale::NarrowName ", "F").toString(languages);
        case 6:
            return ki18nc("Julian weekday 6 - KLocale::NarrowName ", "S").toString(languages);
        case 7:
            return ki18nc("Julian weekday 7 - KLocale::NarrowName ", "S").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName  || format == KLocale:: ShortNumber) {
        switch (weekDay) {
        case 1:
            return ki18nc("Julian weekday 1 - KLocale::ShortName", "Mon").toString(languages);
        case 2:
            return ki18nc("Julian weekday 2 - KLocale::ShortName", "Tue").toString(languages);
        case 3:
            return ki18nc("Julian weekday 3 - KLocale::ShortName", "Wed").toString(languages);
        case 4:
            return ki18nc("Julian weekday 4 - KLocale::ShortName", "Thu").toString(languages);
        case 5:
            return ki18nc("Julian weekday 5 - KLocale::ShortName", "Fri").toString(languages);
        case 6:
            return ki18nc("Julian weekday 6 - KLocale::ShortName", "Sat").toString(languages);
        case 7:
            return ki18nc("Julian weekday 7 - KLocale::ShortName", "Sun").toString(languages);
        default: return QString();
        }
    }

    switch (weekDay) {
    case 1:
        return ki18nc("Julian weekday 1 - KLocale::LongName", "Monday").toString(languages);
    case 2:
        return ki18nc("Julian weekday 2 - KLocale::LongName", "Tuesday").toString(languages);
    case 3:
        return ki18nc("Julian weekday 3 - KLocale::LongName", "Wednesday").toString(languages);
    case 4:
        return ki18nc("Julian weekday 4 - KLocale::LongName", "Thursday").toString(languages);
    case 5:
        return ki18nc("Julian weekday 5 - KLocale::LongName", "Friday").toString(languages);
    case 6:
        return ki18nc("Julian weekday 6 - KLocale::LongName", "Saturday").toString(languages);
    case 7:
        return ki18nc("Julian weekday 7 - KLocale::LongName", "Sunday").toString(languages);
    default:
        return QString();
    }
}


KCalendarSystemJulian::KCalendarSystemJulian(const KSharedConfig::Ptr config, const KLocale *locale)
                     : KCalendarSystem(*new KCalendarSystemJulianPrivate(this), config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemJulian::KCalendarSystemJulian(KCalendarSystemJulianPrivate &dd,
                                             const KSharedConfig::Ptr config, const KLocale *locale)
                     : KCalendarSystem(dd, config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemJulian::~KCalendarSystemJulian()
{
}

QString KCalendarSystemJulian::calendarType() const
{
    return QLatin1String("julian");
}

KLocale::CalendarSystem KCalendarSystemJulian::calendarSystem() const
{
    return KLocale::JulianCalendar;
}

QDate KCalendarSystemJulian::epoch() const
{
    return QDate::fromJulianDay(1721426);
}

QDate KCalendarSystemJulian::earliestValidDate() const
{
    // 1 Jan 4712 BC, no year zero, cant be 4713BC due to error in QDate that day 0 is not valid
    // and we really need the first in each year to be valid for the date maths
    return QDate::fromJulianDay(366);
}

QDate KCalendarSystemJulian::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    // 31 Dec 9999 AD, no year zero
    return QDate::fromJulianDay(5373557);
}

QString KCalendarSystemJulian::monthName(int month, int year, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(month, year, format);
}

QString KCalendarSystemJulian::monthName(const QDate &date, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(date, format);
}

QString KCalendarSystemJulian::weekDayName(int weekDay, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(weekDay, format);
}

QString KCalendarSystemJulian::weekDayName(const QDate &date, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(date, format);
}

bool KCalendarSystemJulian::isLunar() const
{
    return false;
}

bool KCalendarSystemJulian::isLunisolar() const
{
    return false;
}

bool KCalendarSystemJulian::isSolar() const
{
    return true;
}

bool KCalendarSystemJulian::isProleptic() const
{
    return true;
}

bool KCalendarSystemJulian::julianDayToDate(qint64 jd, int &year, int &month, int &day) const
{
    // Formula from The Calendar FAQ by Claus Tondering
    // http://www.tondering.dk/claus/cal/node3.html#SECTION003161000000000000000
    // NOTE: Coded from scratch from mathematical formulas, not copied from
    // the Boost licensed source code

    int b = 0;
    int c = jd + 32082;
    int d = ((4 * c) + 3) / 1461;
    int e = c - ((1461 * d) / 4);
    int m = ((5 * e) + 2) / 153;
    day = e - (((153 * m) + 2) / 5) + 1;
    month = m + 3 - (12 * (m / 10));
    year = (100 * b) + d - 4800 + (m / 10);

    // If year is -ve then is BC.  In Julian there is no year 0, but the maths
    // is easier if we pretend there is, so internally year of 0 = 1BC = -1 outside
    if (year < 1) {
        year = year - 1;
    }

    return true;
}

bool KCalendarSystemJulian::dateToJulianDay(int year, int month, int day, qint64 &jd) const
{
    // Formula from The Calendar FAQ by Claus Tondering
    // http://www.tondering.dk/claus/cal/node3.html#SECTION003161000000000000000
    // NOTE: Coded from scratch from mathematical formulas, not copied from
    // the Boost licensed source code

    // If year is -ve then is BC.  In Julian there is no year 0, but the maths
    // is easier if we pretend there is, so internally year of -1 = 1BC = 0 internally
    int y;
    if (year < 1) {
        y = year + 1;
    } else {
        y = year;
    }

    int a = (14 - month) / 12;
    y = y + 4800 - a;
    int m = month + (12 * a) - 3;

    jd = day
         + (((153 * m) + 2) / 5)
         + (365 * y)
         + (y / 4)
         - 32083;

    return true;
}
