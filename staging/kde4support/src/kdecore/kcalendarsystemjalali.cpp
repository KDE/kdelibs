/*
    Copyright (C) 2002-2003 Arash Bijanzadeh  and FarsiKDE Project <www.farsikde.org>
    Contact: Arash Bijanzadeh <a.bijanzadeh@linuxiran.org>
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

/*
    This is an implementation of the artithmetic Persian calendar using the
    Birashk algorithm with adjustments to always be correct in the period 1244
    and 1530 (1865 to 2152 Gregorian).

    In future this will be replaced with the correct astronomical calendar.
*/

#include "kcalendarsystemjalali_p.h"
#include "kcalendarsystemprivate_p.h"

#include <QtCore/QDate>

#include <klocalizedstring.h>

class KCalendarSystemJalaliPrivate : public KCalendarSystemPrivate
{
public:
    explicit KCalendarSystemJalaliPrivate(KCalendarSystemJalali *q);

    virtual ~KCalendarSystemJalaliPrivate();

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
};

// Shared d pointer base class definitions

KCalendarSystemJalaliPrivate::KCalendarSystemJalaliPrivate(KCalendarSystemJalali *q)
                            : KCalendarSystemPrivate(q)
{
}

KCalendarSystemJalaliPrivate::~KCalendarSystemJalaliPrivate()
{
}

void KCalendarSystemJalaliPrivate::loadDefaultEraList()
{
    QString name, shortName, format;
    // Islamic Era (Hijri), Anno Persico.
    name = i18nc("Calendar Era: Jalali Islamic Era, years > 0, LongFormat", "Anno Persico");
    shortName = i18nc("Calendar Era: Jalali Islamic Era, years > 0, ShortFormat", "AP");
    format = i18nc("(kdedt-format) Jalali, AP, full era year format used for %EY, e.g. 2000 AP", "%Ey %EC");
    addEra('+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format);
}

int KCalendarSystemJalaliPrivate::monthsInYear(int year) const
{
    Q_UNUSED(year)
    return 12;
}

int KCalendarSystemJalaliPrivate::daysInMonth(int year, int month) const
{
    if (month == 12) {
        if (isLeapYear(year)) {
            return 30;
        } else {
            return 29;
        }
    }

    if (month <= 6) {
        return 31;
    }

    return 30;
}

int KCalendarSystemJalaliPrivate::daysInYear(int year) const
{
    if (isLeapYear(year)) {
        return 366;
    } else {
        return 365;
    }
}

bool KCalendarSystemJalaliPrivate::isLeapYear(int year) const
{
    // From formilab Public Domain code http://www.fourmilab.ch/documents/calendar/
    // Use Birashk algorithm as it matches the to/from jd code below

    // Birashk algorithm is incorrect in two years in period AP 1244 to 1531,
    // 1403/1404 and 1436/1437, and so catch them here first
    if (year == 1403 || year == 1436) {
        return true;
    } else if (year == 1404 || year == 1437) {
        return false;
    }

    if (year >= 0) {
        year = year - 474;
    } else {
        year = year - 473;
    }

    if ((((((year % 2820) + 474) + 38) * 682) % 2816) < 682) {
        return true;
    } else {
        return false;
    }
}

bool KCalendarSystemJalaliPrivate::hasLeapMonths() const
{
    return false;
}

bool KCalendarSystemJalaliPrivate::hasYearZero() const
{
    return false;
}

int KCalendarSystemJalaliPrivate::maxMonthsInYear() const
{
    return 12;
}

int KCalendarSystemJalaliPrivate::earliestValidYear() const
{
    return 1244;
}

int KCalendarSystemJalaliPrivate::latestValidYear() const
{
    return 1530;
}

QString KCalendarSystemJalaliPrivate::monthName(int month, int year, KLocale::DateTimeComponentFormat format, bool possessive) const
{
    Q_UNUSED(year);

    QStringList languages = locale()->languageList();

    if (format == KLocale::NarrowName) {
        switch (month) {
        case 1:
            return ki18nc("Jalali month 1 - KLocale::NarrowName",  "F").toString(languages);
        case 2:
            return ki18nc("Jalali month 2 - KLocale::NarrowName",  "O").toString(languages);
        case 3:
            return ki18nc("Jalali month 3 - KLocale::NarrowName",  "K").toString(languages);
        case 4:
            return ki18nc("Jalali month 4 - KLocale::NarrowName",  "T").toString(languages);
        case 5:
            return ki18nc("Jalali month 5 - KLocale::NarrowName",  "M").toString(languages);
        case 6:
            return ki18nc("Jalali month 6 - KLocale::NarrowName",  "S").toString(languages);
        case 7:
            return ki18nc("Jalali month 7 - KLocale::NarrowName",  "M").toString(languages);
        case 8:
            return ki18nc("Jalali month 8 - KLocale::NarrowName",  "A").toString(languages);
        case 9:
            return ki18nc("Jalali month 9 - KLocale::NarrowName",  "A").toString(languages);
        case 10:
            return ki18nc("Jalali month 10 - KLocale::NarrowName", "D").toString(languages);
        case 11:
            return ki18nc("Jalali month 11 - KLocale::NarrowName", "B").toString(languages);
        case 12:
            return ki18nc("Jalali month 12 - KLocale::NarrowName", "E").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Jalali month 1 - KLocale::ShortName Possessive",  "of Far").toString(languages);
        case 2:
            return ki18nc("Jalali month 2 - KLocale::ShortName Possessive",  "of Ord").toString(languages);
        case 3:
            return ki18nc("Jalali month 3 - KLocale::ShortName Possessive",  "of Kho").toString(languages);
        case 4:
            return ki18nc("Jalali month 4 - KLocale::ShortName Possessive",  "of Tir").toString(languages);
        case 5:
            return ki18nc("Jalali month 5 - KLocale::ShortName Possessive",  "of Mor").toString(languages);
        case 6:
            return ki18nc("Jalali month 6 - KLocale::ShortName Possessive",  "of Sha").toString(languages);
        case 7:
            return ki18nc("Jalali month 7 - KLocale::ShortName Possessive",  "of Meh").toString(languages);
        case 8:
            return ki18nc("Jalali month 8 - KLocale::ShortName Possessive",  "of Aba").toString(languages);
        case 9:
            return ki18nc("Jalali month 9 - KLocale::ShortName Possessive",  "of Aza").toString(languages);
        case 10:
            return ki18nc("Jalali month 10 - KLocale::ShortName Possessive", "of Dei").toString(languages);
        case 11:
            return ki18nc("Jalali month 11 - KLocale::ShortName Possessive", "of Bah").toString(languages);
        case 12:
            return ki18nc("Jalali month 12 - KLocale::ShortName Possessive", "of Esf").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && !possessive) {
        switch (month) {
        case 1:
            return ki18nc("Jalali month 1 - KLocale::ShortName",  "Far").toString(languages);
        case 2:
            return ki18nc("Jalali month 2 - KLocale::ShortName",  "Ord").toString(languages);
        case 3:
            return ki18nc("Jalali month 3 - KLocale::ShortName",  "Kho").toString(languages);
        case 4:
            return ki18nc("Jalali month 4 - KLocale::ShortName",  "Tir").toString(languages);
        case 5:
            return ki18nc("Jalali month 5 - KLocale::ShortName",  "Mor").toString(languages);
        case 6:
            return ki18nc("Jalali month 6 - KLocale::ShortName",  "Sha").toString(languages);
        case 7:
            return ki18nc("Jalali month 7 - KLocale::ShortName",  "Meh").toString(languages);
        case 8:
            return ki18nc("Jalali month 8 - KLocale::ShortName",  "Aba").toString(languages);
        case 9:
            return ki18nc("Jalali month 9 - KLocale::ShortName",  "Aza").toString(languages);
        case 10:
            return ki18nc("Jalali month 10 - KLocale::ShortName", "Dei").toString(languages);
        case 11:
            return ki18nc("Jalali month 11 - KLocale::ShortName", "Bah").toString(languages);
        case 12:
            return ki18nc("Jalali month 12 - KLocale::ShortName", "Esf").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::LongName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Jalali month 1 - KLocale::LongName Possessive",  "of Farvardin").toString(languages);
        case 2:
            return ki18nc("Jalali month 2 - KLocale::LongName Possessive",  "of Ordibehesht").toString(languages);
        case 3:
            return ki18nc("Jalali month 3 - KLocale::LongName Possessive",  "of Khordad").toString(languages);
        case 4:
            return ki18nc("Jalali month 4 - KLocale::LongName Possessive",  "of Tir").toString(languages);
        case 5:
            return ki18nc("Jalali month 5 - KLocale::LongName Possessive",  "of Mordad").toString(languages);
        case 6:
            return ki18nc("Jalali month 6 - KLocale::LongName Possessive",  "of Shahrivar").toString(languages);
        case 7:
            return ki18nc("Jalali month 7 - KLocale::LongName Possessive",  "of Mehr").toString(languages);
        case 8:
            return ki18nc("Jalali month 8 - KLocale::LongName Possessive",  "of Aban").toString(languages);
        case 9:
            return ki18nc("Jalali month 9 - KLocale::LongName Possessive",  "of Azar").toString(languages);
        case 10:
            return ki18nc("Jalali month 10 - KLocale::LongName Possessive", "of Dei").toString(languages);
        case 11:
            return ki18nc("Jalali month 11 - KLocale::LongName Possessive", "of Bahman").toString(languages);
        case 12:
            return ki18nc("Jalali month 12 - KLocale::LongName Possessive", "of Esfand").toString(languages);
        default:
            return QString();
        }
    }

    // Default to LongName
    switch (month) {
    case 1:
        return ki18nc("Jalali month 1 - KLocale::LongName",  "Farvardin").toString(languages);
    case 2:
        return ki18nc("Jalali month 2 - KLocale::LongName",  "Ordibehesht").toString(languages);
    case 3:
        return ki18nc("Jalali month 3 - KLocale::LongName",  "Khordad").toString(languages);
    case 4:
        return ki18nc("Jalali month 4 - KLocale::LongName",  "Tir").toString(languages);
    case 5:
        return ki18nc("Jalali month 5 - KLocale::LongName",  "Mordad").toString(languages);
    case 6:
        return ki18nc("Jalali month 6 - KLocale::LongName",  "Shahrivar").toString(languages);
    case 7:
        return ki18nc("Jalali month 7 - KLocale::LongName",  "Mehr").toString(languages);
    case 8:
        return ki18nc("Jalali month 8 - KLocale::LongName",  "Aban").toString(languages);
    case 9:
        return ki18nc("Jalali month 9 - KLocale::LongName",  "Azar").toString(languages);
    case 10:
        return ki18nc("Jalali month 10 - KLocale::LongName", "Dei").toString(languages);
    case 11:
        return ki18nc("Jalali month 11 - KLocale::LongName", "Bahman").toString(languages);
    case 12:
        return ki18nc("Jalali month 12 - KLocale::LongName", "Esfand").toString(languages);
    default:
        return QString();
    }
}

QString KCalendarSystemJalaliPrivate::weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const
{
    QStringList languages = locale()->languageList();

    if (format == KLocale::NarrowName) {
        switch (weekDay) {
        case 1:
            return ki18nc("Jalali weekday 1 - KLocale::NarrowName ", "2").toString(languages);
        case 2:
            return ki18nc("Jalali weekday 2 - KLocale::NarrowName ", "3").toString(languages);
        case 3:
            return ki18nc("Jalali weekday 3 - KLocale::NarrowName ", "4").toString(languages);
        case 4:
            return ki18nc("Jalali weekday 4 - KLocale::NarrowName ", "5").toString(languages);
        case 5:
            return ki18nc("Jalali weekday 5 - KLocale::NarrowName ", "J").toString(languages);
        case 6:
            return ki18nc("Jalali weekday 6 - KLocale::NarrowName ", "S").toString(languages);
        case 7:
            return ki18nc("Jalali weekday 7 - KLocale::NarrowName ", "1").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName  || format == KLocale:: ShortNumber) {
        switch (weekDay) {
        case 1:
            return ki18nc("Jalali weekday 1 - KLocale::ShortName", "2sh").toString(languages);
        case 2:
            return ki18nc("Jalali weekday 2 - KLocale::ShortName", "3sh").toString(languages);
        case 3:
            return ki18nc("Jalali weekday 3 - KLocale::ShortName", "4sh").toString(languages);
        case 4:
            return ki18nc("Jalali weekday 4 - KLocale::ShortName", "5sh").toString(languages);
        case 5:
            return ki18nc("Jalali weekday 5 - KLocale::ShortName", "Jom").toString(languages);
        case 6:
            return ki18nc("Jalali weekday 6 - KLocale::ShortName", "Shn").toString(languages);
        case 7:
            return ki18nc("Jalali weekday 7 - KLocale::ShortName", "1sh").toString(languages);
        default: return QString();
        }
    }

    switch (weekDay) {
    case 1:
        return ki18nc("Jalali weekday 1 - KLocale::LongName", "Do shanbe").toString(languages);
    case 2:
        return ki18nc("Jalali weekday 2 - KLocale::LongName", "Se shanbe").toString(languages);
    case 3:
        return ki18nc("Jalali weekday 3 - KLocale::LongName", "Chahar shanbe").toString(languages);
    case 4:
        return ki18nc("Jalali weekday 4 - KLocale::LongName", "Panj shanbe").toString(languages);
    case 5:
        return ki18nc("Jalali weekday 5 - KLocale::LongName", "Jumee").toString(languages);
    case 6:
        return ki18nc("Jalali weekday 6 - KLocale::LongName", "Shanbe").toString(languages);
    case 7:
        return ki18nc("Jalali weekday 7 - KLocale::LongName", "Yek-shanbe").toString(languages);
    default:
        return QString();
    }
}


KCalendarSystemJalali::KCalendarSystemJalali(const KSharedConfig::Ptr config, const KLocale *locale)
                     : KCalendarSystem(*new KCalendarSystemJalaliPrivate(this), config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemJalali::KCalendarSystemJalali(KCalendarSystemJalaliPrivate &dd,
                                             const KSharedConfig::Ptr config, const KLocale *locale)
                     : KCalendarSystem(dd, config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemJalali::~KCalendarSystemJalali()
{
}

QString KCalendarSystemJalali::calendarType() const
{
    return QLatin1String("jalali");
}

KLocale::CalendarSystem KCalendarSystemJalali::calendarSystem() const
{
    return KLocale::JalaliCalendar;
}

QDate KCalendarSystemJalali::epoch() const
{
    // 19 March 622 in the Julian calendar
    return QDate::fromJulianDay(1948321);
}

QDate KCalendarSystemJalali::earliestValidDate() const
{
    // Using the Birashk formula which is accurate in period AP 1244 to 1530 (AD 1865 to 2152)
    // 1244-01-01 Jalali 1865-03-21 Gregorian
    return QDate::fromJulianDay(2402317);
}

QDate KCalendarSystemJalali::latestValidDate() const
{
    // Using the Birashk formula which is accurate in period AP 1244 to 1530 (AD 1865 to 2152)
    // 1530-12-29 Jalali 2152-03-19 Gregorian
    return QDate::fromJulianDay(2507140);
}

QString KCalendarSystemJalali::monthName(int month, int year, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(month, year, format);
}

QString KCalendarSystemJalali::monthName(const QDate &date, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(date, format);
}

QString KCalendarSystemJalali::weekDayName(int weekDay, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(weekDay, format);
}

QString KCalendarSystemJalali::weekDayName(const QDate &date, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(date, format);
}

bool KCalendarSystemJalali::isLunar() const
{
    return false;
}

bool KCalendarSystemJalali::isLunisolar() const
{
    return false;
}

bool KCalendarSystemJalali::isSolar() const
{
    return true;
}

bool KCalendarSystemJalali::isProleptic() const
{
    return false;
}

bool KCalendarSystemJalali::julianDayToDate(qint64 jd, int &year, int &month, int &day) const
{
    // Birashk algorithm is incorrect in two years in period AP 1244 to 1531.
    // This results in a leap day being added to the end of 1404 instead of 1403
    // and to the end of 1437 instead of 1436.  Check for these dates first and
    // return accordingly.  Relies on later use of dateToJulianDay() to correctly
    // calculate firstDayOfYear in 1404 and 1437, so no other adjustments needed.
    if (jd == 2460755) {
        year = 1403;
        month = 12;
        day = 30;
        return true;
    }
    if (jd == 2472808) {
        year = 1436;
        month = 12;
        day = 30;
        return true;
    }

    // From original KDE3 code, source unknown?  Unable to contact author or committer to confirm
    // Matches Fermilab code, EMACS and D&R so check for PD source, likely Birashk's book

    qint64 jdCycleStart;
    int daysSinceCycleStart;
    int cycle;
    int dayInCycle;
    int yearInCycle;
    dateToJulianDay(475, 1, 1, jdCycleStart);
    daysSinceCycleStart = jd - jdCycleStart;
    cycle = daysSinceCycleStart / 1029983;
    dayInCycle = daysSinceCycleStart % 1029983;
    if (dayInCycle == 1029982) {
        yearInCycle = 2820;
    } else {
        int aux1 = dayInCycle / 366;
        int aux2 = dayInCycle % 366;
        yearInCycle = (((2134 * aux1) + (2816 * aux2) + 2815) / 1028522) + aux1 + 1;
    }
    year = yearInCycle + (2820 * cycle) + 474;
    if (year <= 0) {
        year = year - 1;
    }

    qint64 firstDayOfYear;
    dateToJulianDay(year, 1, 1, firstDayOfYear);
    int dayinYear = jd - firstDayOfYear + 1;
    if (dayinYear <= 186) {
        month = ((dayinYear - 1) / 31) + 1;
        day = dayinYear - ((month - 1) * 31);
    } else {
        month = ((dayinYear - 7) / 30) + 1;
        day = dayinYear - ((month - 1) * 30) - 6;
    }

    return true;
}

bool KCalendarSystemJalali::dateToJulianDay(int year, int month, int day, qint64 &jd) const
{
    Q_D(const KCalendarSystemJalali);

    // Birashk algorithm is incorrect in two years in period AP 1244 to 1531.
    // This results in a leap day being added to the end of 1404 instead of 1403
    // and to the end of 1437 instead of 1436.  Thus all dates in 1404 and 1437
    // are off by 1 JD.  Check for these dates first and adjust accordingly.
    if (year == 1403 && month == 12 && day == 30) {
        jd = 2460755;
        return true;
    }
    if (year == 1436 && month == 12 && day == 30) {
        jd = 2472808;
        return true;
    }
    if (year == 1404 || year == 1437) {
        if (month < 12 && day + 1 > d->daysInMonth(year, month)) {
            day = 1;
            month = month + 1;
        } else {
            day = day + 1;
        }
    }

    // From original KDE3 code, source unknown?  Unable to contact author or committer to confirm
    // Matches Fermilab code, EMACS and D&R so check for PD source, likely Birashk's book
    int epbase;
    long epyear;
    long monthDays;

    if (year >= 0) {
        epbase = year - 474;
    } else {
        epbase = year - 473;
    }

    epyear = 474 + (epbase % 2820);

    if (month <= 7) {
        monthDays = (month - 1) * 31;
    } else {
        monthDays = ((month - 1) * 30) + 6;
    }

    jd = (epoch().toJulianDay() - 1) +            // days before epoch
         (epyear - 1) * 365 +                     // normal days in previous years
         (((epyear * 682) - 110) / 2816) +        // leap days in previous years
         (epbase / 2820) * 1029983 +
         monthDays +                              // days in previous months this year
         day;                                     // days in this month

    return true;
}
