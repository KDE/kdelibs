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

#include "kcalendarsystemindiannational_p.h"
#include "kcalendarsystemprivate_p.h"

#include "klocale.h"
#include "klocalizedstring.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

class KCalendarSystemIndianNationalPrivate : public KCalendarSystemPrivate
{
public:
    explicit KCalendarSystemIndianNationalPrivate(KCalendarSystemIndianNational *q);

    virtual ~KCalendarSystemIndianNationalPrivate();

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

KCalendarSystemIndianNationalPrivate::KCalendarSystemIndianNationalPrivate(KCalendarSystemIndianNational *q)
                                    : KCalendarSystemPrivate(q)
{
}

KCalendarSystemIndianNationalPrivate::~KCalendarSystemIndianNationalPrivate()
{
}

void KCalendarSystemIndianNationalPrivate::loadDefaultEraList()
{
    QString name, shortName, format;
    // Saka Era
    name = i18nc("Calendar Era: Indian National Saka Era, years > 0, LongFormat", "Saka Era");
    shortName = i18nc("Calendar Era: Indian National Saka Era, years > 0, ShortFormat", "SE");
    format = i18nc("(kdedt-format) Indian National, SE, full era year format used for %EY, e.g. 2000 SE", "%Ey %EC");
    addEra('+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format);
}

int KCalendarSystemIndianNationalPrivate::monthsInYear(int year) const
{
    Q_UNUSED(year)
    return 12;
}

int KCalendarSystemIndianNationalPrivate::daysInMonth(int year, int month) const
{
    if (month == 1) {
        if (isLeapYear(year)) {
            return 31;
        } else {
            return 30;
        }
    }

    if (month >= 2 && month <= 6) {
        return 31;
    }

    return 30;
}

int KCalendarSystemIndianNationalPrivate::daysInYear(int year) const
{
    if (isLeapYear(year)) {
        return 366;
    } else {
        return 365;
    }
}

bool KCalendarSystemIndianNationalPrivate::isLeapYear(int year) const
{
    //Uses same rule as Gregorian, and is explicitly synchronized to Gregorian
    //so add 78 years to get Gregorian year and apply Gregorian calculation
    year = year + 78;
    if (!hasYearZero() && year < 1) {
        year = year + 1;
    }

    if (year % 4 == 0) {
        if (year % 100 != 0) {
            return true;
        } else if (year % 400 == 0) {
            return true;
        }
    }

    return false;
}

bool KCalendarSystemIndianNationalPrivate::hasLeapMonths() const
{
    return false;
}

bool KCalendarSystemIndianNationalPrivate::hasYearZero() const
{
    return true;
}

int KCalendarSystemIndianNationalPrivate::maxMonthsInYear() const
{
    return 12;
}

int KCalendarSystemIndianNationalPrivate::earliestValidYear() const
{
    return 0;
}

int KCalendarSystemIndianNationalPrivate::latestValidYear() const
{
    return 9999;
}

QString KCalendarSystemIndianNationalPrivate::monthName(int month, int year, KLocale::DateTimeComponentFormat format, bool possessive) const
{
    Q_UNUSED(year);

    QStringList languages = locale()->languageList();

    if (format == KLocale::NarrowName) {
        switch (month) {
        case 1:
            return ki18nc("Indian National month 1 - KLocale::NarrowName",  "C").toString(languages);
        case 2:
            return ki18nc("Indian National month 2 - KLocale::NarrowName",  "V").toString(languages);
        case 3:
            return ki18nc("Indian National month 3 - KLocale::NarrowName",  "J").toString(languages);
        case 4:
            return ki18nc("Indian National month 4 - KLocale::NarrowName",  "Ā").toString(languages);
        case 5:
            return ki18nc("Indian National month 5 - KLocale::NarrowName",  "S").toString(languages);
        case 6:
            return ki18nc("Indian National month 6 - KLocale::NarrowName",  "B").toString(languages);
        case 7:
            return ki18nc("Indian National month 7 - KLocale::NarrowName",  "Ā").toString(languages);
        case 8:
            return ki18nc("Indian National month 8 - KLocale::NarrowName",  "K").toString(languages);
        case 9:
            return ki18nc("Indian National month 9 - KLocale::NarrowName",  "A").toString(languages);
        case 10:
            return ki18nc("Indian National month 10 - KLocale::NarrowName", "P").toString(languages);
        case 11:
            return ki18nc("Indian National month 11 - KLocale::NarrowName", "M").toString(languages);
        case 12:
            return ki18nc("Indian National month 12 - KLocale::NarrowName", "P").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Indian National month 1 - KLocale::ShortName Possessive",  "of Cha").toString(languages);
        case 2:
            return ki18nc("Indian National month 2 - KLocale::ShortName Possessive",  "of Vai").toString(languages);
        case 3:
            return ki18nc("Indian National month 3 - KLocale::ShortName Possessive",  "of Jya").toString(languages);
        case 4:
            return ki18nc("Indian National month 4 - KLocale::ShortName Possessive",  "of Āsh").toString(languages);
        case 5:
            return ki18nc("Indian National month 5 - KLocale::ShortName Possessive",  "of Shr").toString(languages);
        case 6:
            return ki18nc("Indian National month 6 - KLocale::ShortName Possessive",  "of Bhā").toString(languages);
        case 7:
            return ki18nc("Indian National month 7 - KLocale::ShortName Possessive",  "of Āsw").toString(languages);
        case 8:
            return ki18nc("Indian National month 8 - KLocale::ShortName Possessive",  "of Kār").toString(languages);
        case 9:
            return ki18nc("Indian National month 9 - KLocale::ShortName Possessive",  "of Agr").toString(languages);
        case 10:
            return ki18nc("Indian National month 10 - KLocale::ShortName Possessive", "of Pau").toString(languages);
        case 11:
            return ki18nc("Indian National month 11 - KLocale::ShortName Possessive", "of Māg").toString(languages);
        case 12:
            return ki18nc("Indian National month 12 - KLocale::ShortName Possessive", "of Phā").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && !possessive) {
        switch (month) {
        case 1:
            return ki18nc("Indian National month 1 - KLocale::ShortName",  "Cha").toString(languages);
        case 2:
            return ki18nc("Indian National month 2 - KLocale::ShortName",  "Vai").toString(languages);
        case 3:
            return ki18nc("Indian National month 3 - KLocale::ShortName",  "Jya").toString(languages);
        case 4:
            return ki18nc("Indian National month 4 - KLocale::ShortName",  "Āsh").toString(languages);
        case 5:
            return ki18nc("Indian National month 5 - KLocale::ShortName",  "Shr").toString(languages);
        case 6:
            return ki18nc("Indian National month 6 - KLocale::ShortName",  "Bhā").toString(languages);
        case 7:
            return ki18nc("Indian National month 7 - KLocale::ShortName",  "Āsw").toString(languages);
        case 8:
            return ki18nc("Indian National month 8 - KLocale::ShortName",  "Kār").toString(languages);
        case 9:
            return ki18nc("Indian National month 9 - KLocale::ShortName",  "Agr").toString(languages);
        case 10:
            return ki18nc("Indian National month 10 - KLocale::ShortName", "Pau").toString(languages);
        case 11:
            return ki18nc("Indian National month 11 - KLocale::ShortName", "Māg").toString(languages);
        case 12:
            return ki18nc("Indian National month 12 - KLocale::ShortName", "Phā").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::LongName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Indian National month 1 - KLocale::LongName Possessive",  "of Chaitra").toString(languages);
        case 2:
            return ki18nc("Indian National month 2 - KLocale::LongName Possessive",  "of Vaishākh").toString(languages);
        case 3:
            return ki18nc("Indian National month 3 - KLocale::LongName Possessive",  "of Jyaishtha").toString(languages);
        case 4:
            return ki18nc("Indian National month 4 - KLocale::LongName Possessive",  "of Āshādha").toString(languages);
        case 5:
            return ki18nc("Indian National month 5 - KLocale::LongName Possessive",  "of Shrāvana").toString(languages);
        case 6:
            return ki18nc("Indian National month 6 - KLocale::LongName Possessive",  "of Bhādrapad").toString(languages);
        case 7:
            return ki18nc("Indian National month 7 - KLocale::LongName Possessive",  "of Āshwin").toString(languages);
        case 8:
            return ki18nc("Indian National month 8 - KLocale::LongName Possessive",  "of Kārtik").toString(languages);
        case 9:
            return ki18nc("Indian National month 9 - KLocale::LongName Possessive",  "of Agrahayana").toString(languages);
        case 10:
            return ki18nc("Indian National month 10 - KLocale::LongName Possessive", "of Paush").toString(languages);
        case 11:
            return ki18nc("Indian National month 11 - KLocale::LongName Possessive", "of Māgh").toString(languages);
        case 12:
            return ki18nc("Indian National month 12 - KLocale::LongName Possessive", "of Phālgun").toString(languages);
        default:
            return QString();
        }
    }

    // Default to LongName
    switch (month) {
    case 1:
        return ki18nc("Indian National month 1 - KLocale::LongName",  "Chaitra").toString(languages);
    case 2:
        return ki18nc("Indian National month 2 - KLocale::LongName",  "Vaishākh").toString(languages);
    case 3:
        return ki18nc("Indian National month 3 - KLocale::LongName",  "Jyaishtha").toString(languages);
    case 4:
        return ki18nc("Indian National month 4 - KLocale::LongName",  "Āshādha").toString(languages);
    case 5:
        return ki18nc("Indian National month 5 - KLocale::LongName",  "Shrāvana").toString(languages);
    case 6:
        return ki18nc("Indian National month 6 - KLocale::LongName",  "Bhādrapad").toString(languages);
    case 7:
        return ki18nc("Indian National month 7 - KLocale::LongName",  "Āshwin").toString(languages);
    case 8:
        return ki18nc("Indian National month 8 - KLocale::LongName",  "Kārtik").toString(languages);
    case 9:
        return ki18nc("Indian National month 9 - KLocale::LongName",  "Agrahayana").toString(languages);
    case 10:
        return ki18nc("Indian National month 10 - KLocale::LongName", "Paush").toString(languages);
    case 11:
        return ki18nc("Indian National month 11 - KLocale::LongName", "Māgh").toString(languages);
    case 12:
        return ki18nc("Indian National month 12 - KLocale::LongName", "Phālgun").toString(languages);
    default:
        return QString();
    }
}

QString KCalendarSystemIndianNationalPrivate::weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const
{
    QStringList languages = locale()->languageList();

    if (format == KLocale::NarrowName) {
        switch (weekDay) {
        case 1:
            return ki18nc("Indian National weekday 1 - KLocale::NarrowName ", "S").toString(languages);
        case 2:
            return ki18nc("Indian National weekday 2 - KLocale::NarrowName ", "M").toString(languages);
        case 3:
            return ki18nc("Indian National weekday 3 - KLocale::NarrowName ", "B").toString(languages);
        case 4:
            return ki18nc("Indian National weekday 4 - KLocale::NarrowName ", "G").toString(languages);
        case 5:
            return ki18nc("Indian National weekday 5 - KLocale::NarrowName ", "S").toString(languages);
        case 6:
            return ki18nc("Indian National weekday 6 - KLocale::NarrowName ", "S").toString(languages);
        case 7:
            return ki18nc("Indian National weekday 7 - KLocale::NarrowName ", "R").toString(languages);
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName  || format == KLocale:: ShortNumber) {
        switch (weekDay) {
        case 1:
            return ki18nc("Indian National weekday 1 - KLocale::ShortName", "Som").toString(languages);
        case 2:
            return ki18nc("Indian National weekday 2 - KLocale::ShortName", "Mañ").toString(languages);
        case 3:
            return ki18nc("Indian National weekday 3 - KLocale::ShortName", "Bud").toString(languages);
        case 4:
            return ki18nc("Indian National weekday 4 - KLocale::ShortName", "Gur").toString(languages);
        case 5:
            return ki18nc("Indian National weekday 5 - KLocale::ShortName", "Suk").toString(languages);
        case 6:
            return ki18nc("Indian National weekday 6 - KLocale::ShortName", "San").toString(languages);
        case 7:
            return ki18nc("Indian National weekday 7 - KLocale::ShortName", "Rav").toString(languages);
        default: return QString();
        }
    }

    switch (weekDay) {
    case 1:
        return ki18nc("Indian National weekday 1 - KLocale::LongName", "Somavãra").toString(languages);
    case 2:
        return ki18nc("Indian National weekday 2 - KLocale::LongName", "Mañgalvã").toString(languages);
    case 3:
        return ki18nc("Indian National weekday 3 - KLocale::LongName", "Budhavãra").toString(languages);
    case 4:
        return ki18nc("Indian National weekday 4 - KLocale::LongName", "Guruvãra").toString(languages);
    case 5:
        return ki18nc("Indian National weekday 5 - KLocale::LongName", "Sukravãra").toString(languages);
    case 6:
        return ki18nc("Indian National weekday 6 - KLocale::LongName", "Sanivãra").toString(languages);
    case 7:
        return ki18nc("Indian National weekday 7 - KLocale::LongName", "Raviãra").toString(languages);
    default:
        return QString();
    }
}


KCalendarSystemIndianNational::KCalendarSystemIndianNational(const KSharedConfig::Ptr config, const KLocale *locale)
                             : KCalendarSystem(*new KCalendarSystemIndianNationalPrivate(this), config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemIndianNational::KCalendarSystemIndianNational(KCalendarSystemIndianNationalPrivate &dd,
                                                             const KSharedConfig::Ptr config,
                                                             const KLocale *locale)
                             : KCalendarSystem(dd, config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemIndianNational::~KCalendarSystemIndianNational()
{
}

QString KCalendarSystemIndianNational::calendarType() const
{
    return QLatin1String("indian-national");
}

KLocale::CalendarSystem KCalendarSystemIndianNational::calendarSystem() const
{
    return KLocale::IndianNationalCalendar;
}

QDate KCalendarSystemIndianNational::epoch() const
{
    //0000-01-01, has Year 0.
    //0078-03-22 AD Gregorian / 0078-03-24 AD Julian
    return QDate::fromJulianDay(1749994);
}

QDate KCalendarSystemIndianNational::earliestValidDate() const
{
    //0000-01-01, has Year 0.
    //0078-03-22 AD Gregorian / 0078-03-24 AD Julian
    //Don't do proleptic yet, need to check
    return QDate::fromJulianDay(1749630);
}

QDate KCalendarSystemIndianNational::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    //9999-12-30
    //10078-03-21 AD Gregorian
    return QDate::fromJulianDay(5402054);
}

QString KCalendarSystemIndianNational::monthName(int month, int year, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(month, year, format);
}

QString KCalendarSystemIndianNational::monthName(const QDate &date, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(date, format);
}

QString KCalendarSystemIndianNational::weekDayName(int weekDay, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(weekDay, format);
}

QString KCalendarSystemIndianNational::weekDayName(const QDate &date, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(date, format);
}

bool KCalendarSystemIndianNational::isLunar() const
{
    return false;
}

bool KCalendarSystemIndianNational::isLunisolar() const
{
    return true;
}

bool KCalendarSystemIndianNational::isSolar() const
{
    return false;
}

bool KCalendarSystemIndianNational::isProleptic() const
{
    return false;
}

bool KCalendarSystemIndianNational::julianDayToDate(qint64 jd, int &year, int &month, int &day) const
{
    int L, N, I, J, D, M, Y;

    // "Explanatory Supplement to the Astronomical Almanac" 2006 section 12.94 pp 605-606
    // Originally from "Report of the Calendar Reform Committee" 1955
    L = jd + 68518;
    N = (4 * L) / 146097;
    L = L - (146097 * N + 3) / 4;
    I = (4000 * (L + 1)) / 1461001;
    L = L - (1461 * I) / 4 + 1;
    J = ((L - 1) / 31) * (1 - L / 185) + (L / 185) * ((L - 156) / 30 + 5) - L / 366;
    D = L - 31 * J + ((J + 2) / 8) * (J - 5);
    L = J / 11;
    M = J + 2 - 12 * L;
    Y = 100 * (N - 49) + L + I - 78;

    day = D;
    month = M;
    year = Y;

    return true;
}

bool KCalendarSystemIndianNational::dateToJulianDay(int year, int month, int day, qint64 &jd) const
{
    int Y = year;
    int M = month;
    int D = day;

    // "Explanatory Supplement to the Astronomical Almanac" 2006 section 12.94 pp 605-606
    // Originally from "Report of the Calendar Reform Committee" 1955
    jd = 365 * Y
         + (Y + 78 - 1 / M) / 4
         + 31 * M
         - (M + 9) / 11
         - (M / 7) * (M - 7)
         - (3 * ((Y  + 78 - 1 / M) / 100 + 1)) / 4
         + D
         + 1749579;

    return true;
}
