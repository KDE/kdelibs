/*
    Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
    Copyright (c) 2002-2003 Hans Petter Bieker <bieker@kde.org>
    Copyright 2007, 2010 John Layt <john@layt.net>

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

// Derived QDate kde calendar class

#include "kcalendarsystemqdate_p.h"
#include "kcalendarsystemprivate_p.h"
#include "kcalendarera_p.h"

#include "kdebug.h"
#include "klocale.h"
#include "kconfiggroup.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

class KCalendarSystemQDatePrivate : public KCalendarSystemPrivate
{
public:
    explicit KCalendarSystemQDatePrivate(KCalendarSystemQDate *q);

    virtual ~KCalendarSystemQDatePrivate();

    // Virtual methods each calendar system must re-implement
    virtual void loadDefaultEraList();
    virtual int monthsInYear(int year) const;
    virtual int daysInMonth(int year, int month) const;
    virtual int daysInYear(int year) const;
    virtual int daysInWeek() const;
    virtual bool isLeapYear(int year) const;
    virtual bool hasLeapMonths() const;
    virtual bool hasYearZero() const;
    virtual int maxDaysInWeek() const;
    virtual int maxMonthsInYear() const;
    virtual int earliestValidYear() const;
    virtual int latestValidYear() const;
    virtual QString monthName(int month, int year, KLocale::DateTimeComponentFormat format, bool possessive) const;
    virtual QString weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const;

    bool m_useCommonEra;
};

// Shared d pointer implementations

KCalendarSystemQDatePrivate::KCalendarSystemQDatePrivate(KCalendarSystemQDate *q)
                           : KCalendarSystemPrivate(q),
                             m_useCommonEra(false)

{
}

KCalendarSystemQDatePrivate::~KCalendarSystemQDatePrivate()
{
}

void KCalendarSystemQDatePrivate::loadDefaultEraList()
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

int KCalendarSystemQDatePrivate::monthsInYear(int year) const
{
    Q_UNUSED(year)
    return 12;
}

int KCalendarSystemQDatePrivate::daysInMonth(int year, int month) const
{
    QDate tempDate(year, month, 1);
    return tempDate.daysInMonth();
}

int KCalendarSystemQDatePrivate::daysInYear(int year) const
{
    QDate tempDate(year, 1, 1);
    return tempDate.daysInYear();
}

int KCalendarSystemQDatePrivate::daysInWeek() const
{
    return 7;
}

bool KCalendarSystemQDatePrivate::isLeapYear(int year) const
{
    return QDate::isLeapYear(year);
}

bool KCalendarSystemQDatePrivate::hasLeapMonths() const
{
    return false;
}

bool KCalendarSystemQDatePrivate::hasYearZero() const
{
    return false;
}

int KCalendarSystemQDatePrivate::maxDaysInWeek() const
{
    return 7;
}

int KCalendarSystemQDatePrivate::maxMonthsInYear() const
{
    return 12;
}

int KCalendarSystemQDatePrivate::earliestValidYear() const
{
    return -4712;
}

int KCalendarSystemQDatePrivate::latestValidYear() const
{
    return 9999;
}

QString KCalendarSystemQDatePrivate::monthName(int month, int year, KLocale::DateTimeComponentFormat format, bool possessive) const
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

QString KCalendarSystemQDatePrivate::weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const
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


KCalendarSystemQDate::KCalendarSystemQDate(const KSharedConfig::Ptr config, const KLocale *locale)
                    : KCalendarSystem(*new KCalendarSystemQDatePrivate(this), config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemQDate::KCalendarSystemQDate(KCalendarSystemQDatePrivate &dd, const KSharedConfig::Ptr config, const KLocale *locale)
                    : KCalendarSystem(dd, config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemQDate::~KCalendarSystemQDate()
{
}

QString KCalendarSystemQDate::calendarType() const
{
    return QLatin1String("gregorian");
}

KLocale::CalendarSystem KCalendarSystemQDate::calendarSystem() const
{
    return KLocale::QDateCalendar;
}

QDate KCalendarSystemQDate::epoch() const
{
    // 1 Jan 1 AD in Julian
    return QDate::fromJulianDay(1721424);
}

QDate KCalendarSystemQDate::earliestValidDate() const
{
    // 1 Jan 4712 BC, no year zero, cant be 4713BC due to error in QDate that day 0 is not valid
    // and we really need the first in each year to be valid for the date maths
    return QDate::fromJulianDay(366);
}

QDate KCalendarSystemQDate::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widets support > 9999
    // In Gregorian this is 9999-12-31, which is  is jd 5373484
    // Can't call setDate( 9999, 12, 31 ) as it creates circular reference!
    return QDate::fromJulianDay(5373484);
}

int KCalendarSystemQDate::year(const QDate &date) const
{
    return date.year();
}

int KCalendarSystemQDate::month(const QDate &date) const
{
    return date.month();
}

int KCalendarSystemQDate::day(const QDate &date) const
{
    return date.day();
}

int KCalendarSystemQDate::dayOfYear(const QDate &date) const
{
    return date.dayOfYear();
}

int KCalendarSystemQDate::dayOfWeek(const QDate &date) const
{
    return date.dayOfWeek();
}

QString KCalendarSystemQDate::monthName(int month, int year, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(month, year, format);
}

QString KCalendarSystemQDate::monthName(const QDate &date, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(date, format);
}

QString KCalendarSystemQDate::weekDayName(int weekDay, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(weekDay, format);
}

QString KCalendarSystemQDate::weekDayName(const QDate &date, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(date, format);
}

bool KCalendarSystemQDate::isLunar() const
{
    return false;
}

bool KCalendarSystemQDate::isLunisolar() const
{
    return false;
}

bool KCalendarSystemQDate::isSolar() const
{
    return true;
}

bool KCalendarSystemQDate::isProleptic() const
{
    return false;
}

bool KCalendarSystemQDate::julianDayToDate(qint64 jd, int &year, int &month, int &day) const
{
    QDate date = QDate::fromJulianDay(jd);

    date.getDate(&year, &month, &day);

    return date.isValid();
}

bool KCalendarSystemQDate::dateToJulianDay(int year, int month, int day, qint64 &jd) const
{
    QDate date;

    date.setDate(year, month, day);
    jd = date.toJulianDay();

    return date.isValid();
}
