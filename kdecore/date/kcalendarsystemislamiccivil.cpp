/*
    Copyright (c) 2002-2003 Carlos Moro <cfmoro@correo.uniovi.es>
    Copyright (c) 2002-2003 Hans Petter Bieker <bieker@kde.org>
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

#include "kcalendarsystemislamiccivil_p.h"
#include "kcalendarsystemprivate_p.h"

#include <QtCore/QDate>

#include <klocalizedstring.h>

class KCalendarSystemIslamicCivilPrivate : public KCalendarSystemPrivate
{
public:
    explicit KCalendarSystemIslamicCivilPrivate(KCalendarSystemIslamicCivil *q);

    virtual ~KCalendarSystemIslamicCivilPrivate();

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

KCalendarSystemIslamicCivilPrivate::KCalendarSystemIslamicCivilPrivate(KCalendarSystemIslamicCivil *q)
                                  : KCalendarSystemPrivate(q)
{
}

KCalendarSystemIslamicCivilPrivate::~KCalendarSystemIslamicCivilPrivate()
{
}

void KCalendarSystemIslamicCivilPrivate::loadDefaultEraList()
{
    QString name, shortName, format;
    // Islamic Era, Anno Hegirae, "Year of the Hijra".
    name = i18nc("Calendar Era: Hijri Islamic Era, years > 0, LongFormat", "Anno Hegirae");
    shortName = i18nc("Calendar Era: Hijri Islamic Era, years > 0, ShortFormat", "AH");
    format = i18nc("(kdedt-format) Hijri, AH, full era year format used for %EY, e.g. 2000 AH", "%Ey %EC");
    addEra('+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format);
}

int KCalendarSystemIslamicCivilPrivate::monthsInYear(int year) const
{
    Q_UNUSED(year)
    return 12;
}

int KCalendarSystemIslamicCivilPrivate::daysInMonth(int year, int month) const
{
    if (month == 12 && isLeapYear(year)) {
        return 30;
    }

    if (month % 2 == 0) {   // Even number months have 29 days
        return 29;
    } else {  // Odd number months have 30 days
        return 30;
    }
}

int KCalendarSystemIslamicCivilPrivate::daysInYear(int year) const
{
    if (isLeapYear(year)) {
        return 355;
    } else {
        return 354;
    }
}

bool KCalendarSystemIslamicCivilPrivate::isLeapYear(int year) const
{
    // Years 2, 5, 7, 10, 13, 16, 18, 21, 24, 26, 29 of the 30 year cycle

    /*
    The following C++ code is translated from the Lisp code
    in ``Calendrical Calculations'' by Nachum Dershowitz and
    Edward M. Reingold, Software---Practice & Experience,
    vol. 20, no. 9 (September, 1990), pp. 899--928.

    This code is in the public domain, but any use of it
    should publically acknowledge its source.
    */

    if ((((11 * year) + 14) % 30) < 11) {
        return true;
    } else {
        return false;
    }

    // The following variations will be implemented in separate classes in 4.5
    // May be cleaner to formally define using a case statement switch on (year % 30)

    // Variation used by Bar Habraeus / Graves / Birashk / Some Microsoft products
    // Years 2, 5, 7, 10, 13, 15, 18, 21, 24, 26, 29 of the 30 year cycle
    // if ( ( ( ( 11 * year ) + 15 ) % 30 ) < 11 ) {

    // Variation used by Bohras / Sahifa with epoch 15 July 622 jd = 1948440
    // Years 2, 5, 8, 10, 13, 16, 19, 21, 24, 27, 29 of the 30 year cycle
    // if ( ( ( ( 11 * year ) + 1 ) % 30 ) < 11 ) {
}

bool KCalendarSystemIslamicCivilPrivate::hasLeapMonths() const
{
    return false;
}

bool KCalendarSystemIslamicCivilPrivate::hasYearZero() const
{
    return false;
}

int KCalendarSystemIslamicCivilPrivate::maxMonthsInYear() const
{
    return 12;
}

int KCalendarSystemIslamicCivilPrivate::earliestValidYear() const
{
    return 1;
}

int KCalendarSystemIslamicCivilPrivate::latestValidYear() const
{
    return 9999;
}

QString KCalendarSystemIslamicCivilPrivate::monthName(int month, int year, KLocale::DateTimeComponentFormat format, bool possessive) const
{
    Q_UNUSED(year);

    QStringList languages = locale()->languageList();

    if (format == KLocale::NarrowName) {
        switch (month) {
        case 1:
            return ki18nc("Hijri month 1 - KLocale::NarrowName",  "M").toString(languages);
        case 2:
            return ki18nc("Hijri month 2 - KLocale::NarrowName",  "S").toString(languages);
        case 3:
            return ki18nc("Hijri month 3 - KLocale::NarrowName",  "A").toString(languages);
        case 4:
            return ki18nc("Hijri month 4 - KLocale::NarrowName",  "T").toString(languages);
        case 5:
            return ki18nc("Hijri month 5 - KLocale::NarrowName",  "A").toString(languages);
        case 6:
            return ki18nc("Hijri month 6 - KLocale::NarrowName",  "T").toString(languages);
        case 7:
            return ki18nc("Hijri month 7 - KLocale::NarrowName",  "R").toString(languages);
        case 8:
            return ki18nc("Hijri month 8 - KLocale::NarrowName",  "S").toString(languages);
        case 9:
            return ki18nc("Hijri month 9 - KLocale::NarrowName",  "R").toString(languages);
        case 10:
            return ki18nc("Hijri month 10 - KLocale::NarrowName", "S").toString(languages);
        case 11:
            return ki18nc("Hijri month 11 - KLocale::NarrowName", "Q").toString(languages);
        case 12:
            return ki18nc("Hijri month 12 - KLocale::NarrowName", "H").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Hijri month 1 - KLocale::ShortName Possessive",  "of Muh").toString(languages);
        case 2:
            return ki18nc("Hijri month 2 - KLocale::ShortName Possessive",  "of Saf").toString(languages);
        case 3:
            return ki18nc("Hijri month 3 - KLocale::ShortName Possessive",  "of R.A").toString(languages);
        case 4:
            return ki18nc("Hijri month 4 - KLocale::ShortName Possessive",  "of R.T").toString(languages);
        case 5:
            return ki18nc("Hijri month 5 - KLocale::ShortName Possessive",  "of J.A").toString(languages);
        case 6:
            return ki18nc("Hijri month 6 - KLocale::ShortName Possessive",  "of J.T").toString(languages);
        case 7:
            return ki18nc("Hijri month 7 - KLocale::ShortName Possessive",  "of Raj").toString(languages);
        case 8:
            return ki18nc("Hijri month 8 - KLocale::ShortName Possessive",  "of Sha").toString(languages);
        case 9:
            return ki18nc("Hijri month 9 - KLocale::ShortName Possessive",  "of Ram").toString(languages);
        case 10:
            return ki18nc("Hijri month 10 - KLocale::ShortName Possessive", "of Shw").toString(languages);
        case 11:
            return ki18nc("Hijri month 11 - KLocale::ShortName Possessive", "of Qid").toString(languages);
        case 12:
            return ki18nc("Hijri month 12 - KLocale::ShortName Possessive", "of Hij").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && !possessive) {
        switch (month) {
        case 1:
            return ki18nc("Hijri month 1 - KLocale::ShortName",  "Muh").toString(languages);
        case 2:
            return ki18nc("Hijri month 2 - KLocale::ShortName",  "Saf").toString(languages);
        case 3:
            return ki18nc("Hijri month 3 - KLocale::ShortName",  "R.A").toString(languages);
        case 4:
            return ki18nc("Hijri month 4 - KLocale::ShortName",  "R.T").toString(languages);
        case 5:
            return ki18nc("Hijri month 5 - KLocale::ShortName",  "J.A").toString(languages);
        case 6:
            return ki18nc("Hijri month 6 - KLocale::ShortName",  "J.T").toString(languages);
        case 7:
            return ki18nc("Hijri month 7 - KLocale::ShortName",  "Raj").toString(languages);
        case 8:
            return ki18nc("Hijri month 8 - KLocale::ShortName",  "Sha").toString(languages);
        case 9:
            return ki18nc("Hijri month 9 - KLocale::ShortName",  "Ram").toString(languages);
        case 10:
            return ki18nc("Hijri month 10 - KLocale::ShortName", "Shw").toString(languages);
        case 11:
            return ki18nc("Hijri month 11 - KLocale::ShortName", "Qid").toString(languages);
        case 12:
            return ki18nc("Hijri month 12 - KLocale::ShortName", "Hij").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::LongName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Hijri month 1 - KLocale::LongName Possessive",  "of Muharram").toString(languages);
        case 2:
            return ki18nc("Hijri month 2 - KLocale::LongName Possessive",  "of Safar").toString(languages);
        case 3:
            return ki18nc("Hijri month 3 - KLocale::LongName Possessive",  "of Rabi` al-Awal").toString(languages);
        case 4:
            return ki18nc("Hijri month 4 - KLocale::LongName Possessive",  "of Rabi` al-Thaani").toString(languages);
        case 5:
            return ki18nc("Hijri month 5 - KLocale::LongName Possessive",  "of Jumaada al-Awal").toString(languages);
        case 6:
            return ki18nc("Hijri month 6 - KLocale::LongName Possessive",  "of Jumaada al-Thaani").toString(languages);
        case 7:
            return ki18nc("Hijri month 7 - KLocale::LongName Possessive",  "of Rajab").toString(languages);
        case 8:
            return ki18nc("Hijri month 8 - KLocale::LongName Possessive",  "of Sha`ban").toString(languages);
        case 9:
            return ki18nc("Hijri month 9 - KLocale::LongName Possessive",  "of Ramadan").toString(languages);
        case 10:
            return ki18nc("Hijri month 10 - KLocale::LongName Possessive", "of Shawwal").toString(languages);
        case 11:
            return ki18nc("Hijri month 11 - KLocale::LongName Possessive", "of Thu al-Qi`dah").toString(languages);
        case 12:
            return ki18nc("Hijri month 12 - KLocale::LongName Possessive", "of Thu al-Hijjah").toString(languages);
        default:
            return QString();
        }
    }

    // Default to LongName
    switch (month) {
    case 1:
        return ki18nc("Hijri month 1 - KLocale::LongName",  "Muharram").toString(languages);
    case 2:
        return ki18nc("Hijri month 2 - KLocale::LongName",  "Safar").toString(languages);
    case 3:
        return ki18nc("Hijri month 3 - KLocale::LongName",  "Rabi` al-Awal").toString(languages);
    case 4:
        return ki18nc("Hijri month 4 - KLocale::LongName",  "Rabi` al-Thaani").toString(languages);
    case 5:
        return ki18nc("Hijri month 5 - KLocale::LongName",  "Jumaada al-Awal").toString(languages);
    case 6:
        return ki18nc("Hijri month 6 - KLocale::LongName",  "Jumaada al-Thaani").toString(languages);
    case 7:
        return ki18nc("Hijri month 7 - KLocale::LongName",  "Rajab").toString(languages);
    case 8:
        return ki18nc("Hijri month 8 - KLocale::LongName",  "Sha`ban").toString(languages);
    case 9:
        return ki18nc("Hijri month 9 - KLocale::LongName",  "Ramadan").toString(languages);
    case 10:
        return ki18nc("Hijri month 10 - KLocale::LongName", "Shawwal").toString(languages);
    case 11:
        return ki18nc("Hijri month 11 - KLocale::LongName", "Thu al-Qi`dah").toString(languages);
    case 12:
        return ki18nc("Hijri month 12 - KLocale::LongName", "Thu al-Hijjah").toString(languages);
    default:
        return QString();
    }
}

QString KCalendarSystemIslamicCivilPrivate::weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const
{
    QStringList languages = locale()->languageList();

    if (format == KLocale::NarrowName) {
        switch (weekDay) {
        case 1:
            return ki18nc("Hijri weekday 1 - KLocale::NarrowName ", "I").toString(languages);
        case 2:
            return ki18nc("Hijri weekday 2 - KLocale::NarrowName ", "T").toString(languages);
        case 3:
            return ki18nc("Hijri weekday 3 - KLocale::NarrowName ", "A").toString(languages);
        case 4:
            return ki18nc("Hijri weekday 4 - KLocale::NarrowName ", "K").toString(languages);
        case 5:
            return ki18nc("Hijri weekday 5 - KLocale::NarrowName ", "J").toString(languages);
        case 6:
            return ki18nc("Hijri weekday 6 - KLocale::NarrowName ", "S").toString(languages);
        case 7:
            return ki18nc("Hijri weekday 7 - KLocale::NarrowName ", "A").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName  || format == KLocale:: ShortNumber) {
        switch (weekDay) {
        case 1:
            return ki18nc("Hijri weekday 1 - KLocale::ShortName", "Ith").toString(languages);
        case 2:
            return ki18nc("Hijri weekday 2 - KLocale::ShortName", "Thl").toString(languages);
        case 3:
            return ki18nc("Hijri weekday 3 - KLocale::ShortName", "Arb").toString(languages);
        case 4:
            return ki18nc("Hijri weekday 4 - KLocale::ShortName", "Kha").toString(languages);
        case 5:
            return ki18nc("Hijri weekday 5 - KLocale::ShortName", "Jum").toString(languages);
        case 6:
            return ki18nc("Hijri weekday 6 - KLocale::ShortName", "Sab").toString(languages);
        case 7:
            return ki18nc("Hijri weekday 7 - KLocale::ShortName", "Ahd").toString(languages);
        default: return QString();
        }
    }

    switch (weekDay) {
    case 1:
        return ki18nc("Hijri weekday 1 - KLocale::LongName", "Yaum al-Ithnain").toString(languages);
    case 2:
        return ki18nc("Hijri weekday 2 - KLocale::LongName", "Yau al-Thulatha").toString(languages);
    case 3:
        return ki18nc("Hijri weekday 3 - KLocale::LongName", "Yaum al-Arbi'a").toString(languages);
    case 4:
        return ki18nc("Hijri weekday 4 - KLocale::LongName", "Yaum al-Khamees").toString(languages);
    case 5:
        return ki18nc("Hijri weekday 5 - KLocale::LongName", "Yaum al-Jumma").toString(languages);
    case 6:
        return ki18nc("Hijri weekday 6 - KLocale::LongName", "Yaum al-Sabt").toString(languages);
    case 7:
        return ki18nc("Hijri weekday 7 - KLocale::LongName", "Yaum al-Ahad").toString(languages);
    default:
        return QString();
    }
}


KCalendarSystemIslamicCivil::KCalendarSystemIslamicCivil(const KSharedConfig::Ptr config, const KLocale *locale)
                           : KCalendarSystem(*new KCalendarSystemIslamicCivilPrivate(this), config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemIslamicCivil::KCalendarSystemIslamicCivil(KCalendarSystemIslamicCivilPrivate &dd,
                                                         const KSharedConfig::Ptr config, const KLocale *locale)
                           : KCalendarSystem(dd, config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemIslamicCivil::~KCalendarSystemIslamicCivil()
{
}

QString KCalendarSystemIslamicCivil::calendarType() const
{
    return QLatin1String("hijri");
}

KLocale::CalendarSystem KCalendarSystemIslamicCivil::calendarSystem() const
{
    return KLocale::IslamicCivilCalendar;
}

QDate KCalendarSystemIslamicCivil::epoch() const
{
    // 16 July 622 in the Julian calendar
    return QDate::fromJulianDay(1948440);
}

QDate KCalendarSystemIslamicCivil::earliestValidDate() const
{
    return epoch();
}

QDate KCalendarSystemIslamicCivil::latestValidDate() const
{
    // Set to last day of year 9999
    // Last day of Islamic Civil year 9999 is 9999-12-29
    return QDate::fromJulianDay(5491751);
}

QString KCalendarSystemIslamicCivil::monthName(int month, int year, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(month, year, format);
}

QString KCalendarSystemIslamicCivil::monthName(const QDate &date, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(date, format);
}

QString KCalendarSystemIslamicCivil::weekDayName(int weekDay, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(weekDay, format);
}

QString KCalendarSystemIslamicCivil::weekDayName(const QDate &date, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(date, format);
}

bool KCalendarSystemIslamicCivil::isLunar() const
{
    return true;
}

bool KCalendarSystemIslamicCivil::isLunisolar() const
{
    return false;
}

bool KCalendarSystemIslamicCivil::isSolar() const
{
    return false;
}

bool KCalendarSystemIslamicCivil::isProleptic() const
{
    return false;
}

bool KCalendarSystemIslamicCivil::julianDayToDate(qint64 jd, int &year, int &month, int &day) const
{
    Q_D(const KCalendarSystemIslamicCivil);

    /*
    The following C++ code is translated from the Lisp code
    in ``Calendrical Calculations'' by Nachum Dershowitz and
    Edward M. Reingold, Software---Practice & Experience,
    vol. 20, no. 9 (September, 1990), pp. 899--928.

    This code is in the public domain, but any use of it
    should publically acknowledge its source.
    */

    // Search forward year by year from approximate year
    year = (jd - epoch().toJulianDay()) / 355;
    qint64 testJd;
    dateToJulianDay(year, 12, d->daysInMonth(year, 12), testJd);
    while (jd > testJd) {
        year++;
        dateToJulianDay(year, 12, d->daysInMonth(year, 12), testJd);
    }

    // Search forward month by month from Muharram
    month = 1;
    dateToJulianDay(year, month, d->daysInMonth(year, month), testJd);
    while (jd > testJd) {
        month++;
        dateToJulianDay(year, month, d->daysInMonth(year, month), testJd);
    }

    dateToJulianDay(year, month, 1, testJd);
    day = jd - testJd + 1;

    return true;

    // Alternative implementations

    // More recent editions of "Calendrical Calculations" by Dershowitz & Reingold have a more
    // efficient direct calculation without recusrion, but this cannot be used due to licensing

    /*
    Formula from "Explanatory Supplement to the Astronomical Almanac" 2006, derived from Fliegel & Van Flandern 1968
    int L = jd - epoch().toJulianDay() + 10632;
    int N = ( L - 1 ) / 10631;
    L = L - 10631 * N + 354;
    int J = ( ( 10985 - L ) / 5316 ) x ( ( 50* L ) / 17719 ) + ( L / 5670 ) * ( ( 43 * L ) / 15238 );
    L = L - ( ( 30 - J ) / 15 ) * ( ( 17719 * J ) / 50 ) - ( J / 16 ) * ( ( 15238 * J ) / 43 ) + 29;
    year = ( 30 * N ) + J - 30;
    month = ( 24 * L ) / 709;
    day = L - ( ( 709 * month ) / 24 );
    */

    /*
    Formula from Fourmilab website
    jd = Math.floor(jd) + 0.5;
    year = Math.floor(((30 * (jd - epoch().toJulianDay())) + 10646) / 10631);
    month = qMin(12, Math.ceil((jd - (29 + islamic_to_jd(year, 1, 1))) / 29.5) + 1);
    day = (jd - islamic_to_jd(year, month, 1)) + 1;
    */
}

bool KCalendarSystemIslamicCivil::dateToJulianDay(int year, int month, int day, qint64 &jd) const
{
    /*
    The following C++ code is translated from the Lisp code
    in ``Calendrical Calculations'' by Nachum Dershowitz and
    Edward M. Reingold, Software---Practice & Experience,
    vol. 20, no. 9 (September, 1990), pp. 899--928.

    This code is in the public domain, but any use of it
    should publically acknowledge its source.
    */

    jd = epoch().toJulianDay() - 1 +   // days before start of calendar
         (year - 1) * 354 +            // non-leap days in prior years
         (3 + (11 * year)) / 30 +      // leap days in prior years
         29 * (month - 1) +            // days so far...
         month / 2          +          //            ...this year
         day;                          // days so far this month

    return true;

    // Alternative implementations

    /*
    Formula from "Explanatory Supplement to the Astronomical Almanac" 2006, derived from Fliegel & Van Flandern 1968
    jd = ( 3 + ( 11 * year ) ) / 30 + 354 * year + 30 * month - ( month - 1 ) / 2 + day + epoch().toJulianDay() - 385;
    */
}
