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

#include "kcalendarsystemcoptic_p.h"
#include "kcalendarsystemprivate_p.h"
#include "kcalendarsystemcopticprivate_p.h"

#include "kdebug.h"
#include "klocale.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

// Shared d pointer implementations

KCalendarSystemCopticPrivate::KCalendarSystemCopticPrivate(KCalendarSystemCoptic *q)
                            : KCalendarSystemPrivate(q)
{
}

KCalendarSystemCopticPrivate::~KCalendarSystemCopticPrivate()
{
}

KLocale::CalendarSystem KCalendarSystemCopticPrivate::calendarSystem() const
{
    return KLocale::CopticCalendar;
}

void KCalendarSystemCopticPrivate::loadDefaultEraList()
{
    QString name, shortName, format;
    // AM for Anno Martyrum or "Year of the Martyrs"
    name = i18nc("Calendar Era: Coptic Era of Martyrs, years > 0, LongFormat", "Anno Martyrum");
    shortName = i18nc("Calendar Era: Coptic Era of Martyrs, years > 0, ShortFormat", "AM");
    format = i18nc("(kdedt-format) Coptic, AM, full era year format used for %EY, e.g. 2000 AM", "%Ey %EC");
    addEra('+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format);
}

int KCalendarSystemCopticPrivate::monthsInYear(int year) const
{
    Q_UNUSED(year)
    return 13;
}

int KCalendarSystemCopticPrivate::daysInMonth(int year, int month) const
{
    if (month == 13) {
        if (isLeapYear(year)) {
            return 6;
        } else {
            return 5;
        }
    }

    return 30;
}

int KCalendarSystemCopticPrivate::daysInYear(int year) const
{
    if (isLeapYear(year)) {
        return 366;
    } else {
        return 365;
    }
}

int KCalendarSystemCopticPrivate::daysInWeek() const
{
    return 7;
}

bool KCalendarSystemCopticPrivate::isLeapYear(int year) const
{
    //Uses same rule as Julian but offset by 1 year with year 3 being first leap year
    if (year < 1) {
        year = year + 2;
    } else {
        year = year + 1;
    }

    if (year % 4 == 0) {
        return true;
    }
    return false;
}

bool KCalendarSystemCopticPrivate::hasLeapMonths() const
{
    return false;
}

bool KCalendarSystemCopticPrivate::hasYearZero() const
{
    return false;
}

int KCalendarSystemCopticPrivate::maxDaysInWeek() const
{
    return 7;
}

int KCalendarSystemCopticPrivate::maxMonthsInYear() const
{
    return 13;
}

int KCalendarSystemCopticPrivate::earliestValidYear() const
{
    return 1;
}

int KCalendarSystemCopticPrivate::latestValidYear() const
{
    return 9999;
}

// Names taken from Bohairic dialect transliterations in http://www.copticheritage.org/parameters/copticheritage/calendar/The_Coptic_Calendar.pdf
// These differ from the transliterations found on Wikipedia http://en.wikipedia.org/wiki/Coptic_calendar
// These differ from the Sahidic dialect transliterations used in Dershowitz & Reingold which went out of use in the 11th centuary
// These differ from the Arabic transliterations found on Wikipedia
// These differ from the transliterations used in Mac OSX 10.6 Snow Leopard
// The Boharic was initially chosen as this is the dialect apparantly in 'common' use in the Coptic Church.
// But it could be argued the Arabic forms should be used as they are in 'common' usage in Eqypt
// And where did the rest come from?
//
//    Boharic         Wikipedia Copt   D&R Sahidic     Wikipedia Arab  Mac OSX
//    --------------  ---------------  --------------  --------------  --------------
//  * Thoout          Thout            Thoout          Tout            Tout
//  * Paope           Paopi            Paope           Baba            Baba
//  * Hathor          Hathor           Athōr           Hatour          Hatour
//  * Kiahk           Koiak            Koiak           Kiahk           Kiahk
//  * Tobe            Tobi             Tōbe            Touba           Toba
//  * Meshir          Meshir           Meshir          Amshir          Amshir
//  * Paremhotep      Paremhat         Paremotep       Baramhat        Baramhat
//  * Parmoute        Paremoude        Paremoute       Baramouda       Baramouda
//  * Pashons         Pashons          Pashons         Bashans         Bashans
//  * Paone           Paoni            Paōne           Ba'ouna         Paona
//  * Epep            Epip             Epēp            Abib            Epep
//  * Mesore          Mesori           Mesorē          Mesra           Mesra
//  * Kouji nabot     Pi Kogi Enavot   Epagomenē                       Nasie
//  *
QString KCalendarSystemCopticPrivate::monthName(int month, int year, KLocale::DateTimeComponentFormat format, bool possessive) const
{
    Q_UNUSED(year);

    if (format == KLocale::NarrowName) {
        switch (month) {
        case 1:
            return ki18nc("Coptic month 1 - KLocale::NarrowName",  "T").toString(locale());
        case 2:
            return ki18nc("Coptic month 2 - KLocale::NarrowName",  "P").toString(locale());
        case 3:
            return ki18nc("Coptic month 3 - KLocale::NarrowName",  "H").toString(locale());
        case 4:
            return ki18nc("Coptic month 4 - KLocale::NarrowName",  "K").toString(locale());
        case 5:
            return ki18nc("Coptic month 5 - KLocale::NarrowName",  "T").toString(locale());
        case 6:
            return ki18nc("Coptic month 6 - KLocale::NarrowName",  "M").toString(locale());
        case 7:
            return ki18nc("Coptic month 7 - KLocale::NarrowName",  "P").toString(locale());
        case 8:
            return ki18nc("Coptic month 8 - KLocale::NarrowName",  "P").toString(locale());
        case 9:
            return ki18nc("Coptic month 9 - KLocale::NarrowName",  "P").toString(locale());
        case 10:
            return ki18nc("Coptic month 10 - KLocale::NarrowName", "P").toString(locale());
        case 11:
            return ki18nc("Coptic month 11 - KLocale::NarrowName", "E").toString(locale());
        case 12:
            return ki18nc("Coptic month 12 - KLocale::NarrowName", "M").toString(locale());
        case 13:
            return ki18nc("Coptic month 13 - KLocale::NarrowName", "K").toString(locale());
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Coptic month 1 - KLocale::ShortName Possessive",  "of Tho").toString(locale());
        case 2:
            return ki18nc("Coptic month 2 - KLocale::ShortName Possessive",  "of Pao").toString(locale());
        case 3:
            return ki18nc("Coptic month 3 - KLocale::ShortName Possessive",  "of Hat").toString(locale());
        case 4:
            return ki18nc("Coptic month 4 - KLocale::ShortName Possessive",  "of Kia").toString(locale());
        case 5:
            return ki18nc("Coptic month 5 - KLocale::ShortName Possessive",  "of Tob").toString(locale());
        case 6:
            return ki18nc("Coptic month 6 - KLocale::ShortName Possessive",  "of Mes").toString(locale());
        case 7:
            return ki18nc("Coptic month 7 - KLocale::ShortName Possessive",  "of Par").toString(locale());
        case 8:
            return ki18nc("Coptic month 8 - KLocale::ShortName Possessive",  "of Pam").toString(locale());
        case 9:
            return ki18nc("Coptic month 9 - KLocale::ShortName Possessive",  "of Pas").toString(locale());
        case 10:
            return ki18nc("Coptic month 10 - KLocale::ShortName Possessive", "of Pan").toString(locale());
        case 11:
            return ki18nc("Coptic month 11 - KLocale::ShortName Possessive", "of Epe").toString(locale());
        case 12:
            return ki18nc("Coptic month 12 - KLocale::ShortName Possessive", "of Meo").toString(locale());
        case 13:
            return ki18nc("Coptic month 13 - KLocale::ShortName Possessive", "of Kou").toString(locale());
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName && !possessive) {
        switch (month) {
        case 1:
            return ki18nc("Coptic month 1 - KLocale::ShortName",  "Tho").toString(locale());
        case 2:
            return ki18nc("Coptic month 2 - KLocale::ShortName",  "Pao").toString(locale());
        case 3:
            return ki18nc("Coptic month 3 - KLocale::ShortName",  "Hat").toString(locale());
        case 4:
            return ki18nc("Coptic month 4 - KLocale::ShortName",  "Kia").toString(locale());
        case 5:
            return ki18nc("Coptic month 5 - KLocale::ShortName",  "Tob").toString(locale());
        case 6:
            return ki18nc("Coptic month 6 - KLocale::ShortName",  "Mes").toString(locale());
        case 7:
            return ki18nc("Coptic month 7 - KLocale::ShortName",  "Par").toString(locale());
        case 8:
            return ki18nc("Coptic month 8 - KLocale::ShortName",  "Pam").toString(locale());
        case 9:
            return ki18nc("Coptic month 9 - KLocale::ShortName",  "Pas").toString(locale());
        case 10:
            return ki18nc("Coptic month 10 - KLocale::ShortName", "Pan").toString(locale());
        case 11:
            return ki18nc("Coptic month 11 - KLocale::ShortName", "Epe").toString(locale());
        case 12:
            return ki18nc("Coptic month 12 - KLocale::ShortName", "Meo").toString(locale());
        case 13:
            return ki18nc("Coptic month 12 - KLocale::ShortName", "Kou").toString(locale());
        default:
            return QString();
        }
    }

    if (format == KLocale::LongName && possessive) {
        switch (month) {
        case 1:
            return ki18nc("Coptic month 1 - KLocale::LongName Possessive",  "of Thoout").toString(locale());
        case 2:
            return ki18nc("Coptic month 2 - KLocale::LongName Possessive",  "of Paope").toString(locale());
        case 3:
            return ki18nc("Coptic month 3 - KLocale::LongName Possessive",  "of Hathor").toString(locale());
        case 4:
            return ki18nc("Coptic month 4 - KLocale::LongName Possessive",  "of Kiahk").toString(locale());
        case 5:
            return ki18nc("Coptic month 5 - KLocale::LongName Possessive",  "of Tobe").toString(locale());
        case 6:
            return ki18nc("Coptic month 6 - KLocale::LongName Possessive",  "of Meshir").toString(locale());
        case 7:
            return ki18nc("Coptic month 7 - KLocale::LongName Possessive",  "of Paremhotep").toString(locale());
        case 8:
            return ki18nc("Coptic month 8 - KLocale::LongName Possessive",  "of Parmoute").toString(locale());
        case 9:
            return ki18nc("Coptic month 9 - KLocale::LongName Possessive",  "of Pashons").toString(locale());
        case 10:
            return ki18nc("Coptic month 10 - KLocale::LongName Possessive", "of Paone").toString(locale());
        case 11:
            return ki18nc("Coptic month 11 - KLocale::LongName Possessive", "of Epep").toString(locale());
        case 12:
            return ki18nc("Coptic month 12 - KLocale::LongName Possessive", "of Mesore").toString(locale());
        case 13:
            return ki18nc("Coptic month 12 - KLocale::LongName Possessive", "of Kouji nabot").toString(locale());
        default:
            return QString();
        }
    }

    // Default to LongName
    switch (month) {
    case 1:
        return ki18nc("Coptic month 1 - KLocale::LongName",  "Thoout").toString(locale());
    case 2:
        return ki18nc("Coptic month 2 - KLocale::LongName",  "Paope").toString(locale());
    case 3:
        return ki18nc("Coptic month 3 - KLocale::LongName",  "Hathor").toString(locale());
    case 4:
        return ki18nc("Coptic month 4 - KLocale::LongName",  "Kiahk").toString(locale());
    case 5:
        return ki18nc("Coptic month 5 - KLocale::LongName",  "Tobe").toString(locale());
    case 6:
        return ki18nc("Coptic month 6 - KLocale::LongName",  "Meshir").toString(locale());
    case 7:
        return ki18nc("Coptic month 7 - KLocale::LongName",  "Paremhotep").toString(locale());
    case 8:
        return ki18nc("Coptic month 8 - KLocale::LongName",  "Parmoute").toString(locale());
    case 9:
        return ki18nc("Coptic month 9 - KLocale::LongName",  "Pashons").toString(locale());
    case 10:
        return ki18nc("Coptic month 10 - KLocale::LongName", "Paone").toString(locale());
    case 11:
        return ki18nc("Coptic month 11 - KLocale::LongName", "Epep").toString(locale());
    case 12:
        return ki18nc("Coptic month 12 - KLocale::LongName", "Mesore").toString(locale());
    case 13:
        return ki18nc("Coptic month 12 - KLocale::LongName", "Kouji nabot").toString(locale());
    default:
        return QString();
    }
}

// Names taken from from the Sahidic dialect transliterations used in Dershowitz & Reingold which went out of use in the 11th centuary
// Boharic or Arabic transliterations would be preferred but none could be found
QString KCalendarSystemCopticPrivate::weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const
{
    if (format == KLocale::NarrowName) {
        switch (weekDay) {
        case 1:
            return ki18nc("Coptic weekday 1 - KLocale::NarrowName", "P").toString(locale());
        case 2:
            return ki18nc("Coptic weekday 2 - KLocale::NarrowName", "P").toString(locale());
        case 3:
            return ki18nc("Coptic weekday 3 - KLocale::NarrowName", "P").toString(locale());
        case 4:
            return ki18nc("Coptic weekday 4 - KLocale::NarrowName", "P").toString(locale());
        case 5:
            return ki18nc("Coptic weekday 5 - KLocale::NarrowName", "P").toString(locale());
        case 6:
            return ki18nc("Coptic weekday 6 - KLocale::NarrowName", "P").toString(locale());
        case 7:
            return ki18nc("Coptic weekday 7 - KLocale::NarrowName", "T").toString(locale());
        default:
            return QString();
        }
    }

    if (format == KLocale::ShortName  || format == KLocale:: ShortNumber) {
        switch (weekDay) {
        case 1:
            return ki18nc("Coptic weekday 1 - KLocale::ShortName", "Pes").toString(locale());
        case 2:
            return ki18nc("Coptic weekday 2 - KLocale::ShortName", "Psh").toString(locale());
        case 3:
            return ki18nc("Coptic weekday 3 - KLocale::ShortName", "Pef").toString(locale());
        case 4:
            return ki18nc("Coptic weekday 4 - KLocale::ShortName", "Pti").toString(locale());
        case 5:
            return ki18nc("Coptic weekday 5 - KLocale::ShortName", "Pso").toString(locale());
        case 6:
            return ki18nc("Coptic weekday 6 - KLocale::ShortName", "Psa").toString(locale());
        case 7:
            return ki18nc("Coptic weekday 7 - KLocale::ShortName", "Tky").toString(locale());
        default:
            return QString();
        }
    }

    switch (weekDay) {
    case 1:
        return ki18nc("Coptic weekday 1 - KLocale::LongName", "Pesnau").toString(locale());
    case 2:
        return ki18nc("Coptic weekday 2 - KLocale::LongName", "Pshoment").toString(locale());
    case 3:
        return ki18nc("Coptic weekday 3 - KLocale::LongName", "Peftoou").toString(locale());
    case 4:
        return ki18nc("Coptic weekday 4 - KLocale::LongName", "Ptiou").toString(locale());
    case 5:
        return ki18nc("Coptic weekday 5 - KLocale::LongName", "Psoou").toString(locale());
    case 6:
        return ki18nc("Coptic weekday 6 - KLocale::LongName", "Psabbaton").toString(locale());
    case 7:
        return ki18nc("Coptic weekday 7 - KLocale::LongName", "Tkyriakē").toString(locale());
    default:
        return QString();
    }
}


KCalendarSystemCoptic::KCalendarSystemCoptic(const KLocale *locale)
                     : KCalendarSystem(*new KCalendarSystemCopticPrivate(this), KSharedConfig::Ptr(), locale),
                       dont_use(0)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemCoptic::KCalendarSystemCoptic(const KSharedConfig::Ptr config, const KLocale *locale)
                     : KCalendarSystem(*new KCalendarSystemCopticPrivate(this), config, locale),
                       dont_use(0)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemCoptic::KCalendarSystemCoptic(KCalendarSystemCopticPrivate &dd,
                                             const KSharedConfig::Ptr config, const KLocale *locale)
                     : KCalendarSystem(dd, config, locale),
                       dont_use(0)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemCoptic::~KCalendarSystemCoptic()
{
    delete dont_use;
}

QString KCalendarSystemCoptic::calendarType() const
{
    return QLatin1String("coptic");
}

QDate KCalendarSystemCoptic::epoch() const
{
    //0001-01-01, no Year 0.
    //0284-08-29 AD Julian
    return QDate::fromJulianDay(1825030);
}

QDate KCalendarSystemCoptic::earliestValidDate() const
{
    //0001-01-01, no Year 0.
    //0284-08-29 AD Julian
    return QDate::fromJulianDay(1825030);
}

QDate KCalendarSystemCoptic::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    //9999-12-30
    //10283-08-29 AD Julian
    return QDate::fromJulianDay(5477164);
}

bool KCalendarSystemCoptic::isValid(int year, int month, int day) const
{
    return KCalendarSystem::isValid(year, month, day);
}

bool KCalendarSystemCoptic::isValid(const QDate &date) const
{
    return KCalendarSystem::isValid(date);
}

bool KCalendarSystemCoptic::setDate(QDate &date, int year, int month, int day) const
{
    return KCalendarSystem::setDate(date, year, month, day);
}

// Deprecated
bool KCalendarSystemCoptic::setYMD(QDate &date, int y, int m, int d) const
{
    return KCalendarSystem::setDate(date, y, m, d);
}

int KCalendarSystemCoptic::year(const QDate &date) const
{
    return KCalendarSystem::year(date);
}

int KCalendarSystemCoptic::month(const QDate &date) const
{
    return KCalendarSystem::month(date);
}

int KCalendarSystemCoptic::day(const QDate &date) const
{
    return KCalendarSystem::day(date);
}

QDate KCalendarSystemCoptic::addYears(const QDate &date, int nyears) const
{
    return KCalendarSystem::addYears(date, nyears);
}

QDate KCalendarSystemCoptic::addMonths(const QDate &date, int nmonths) const
{
    return KCalendarSystem::addMonths(date, nmonths);
}

QDate KCalendarSystemCoptic::addDays(const QDate &date, int ndays) const
{
    return KCalendarSystem::addDays(date, ndays);
}

int KCalendarSystemCoptic::monthsInYear(const QDate &date) const
{
    return KCalendarSystem::monthsInYear(date);
}

int KCalendarSystemCoptic::weeksInYear(const QDate &date) const
{
    return KCalendarSystem::weeksInYear(date);
}

int KCalendarSystemCoptic::weeksInYear(int year) const
{
    return KCalendarSystem::weeksInYear(year);
}

int KCalendarSystemCoptic::daysInYear(const QDate &date) const
{
    return KCalendarSystem::daysInYear(date);
}

int KCalendarSystemCoptic::daysInMonth(const QDate &date) const
{
    return KCalendarSystem::daysInMonth(date);
}

int KCalendarSystemCoptic::daysInWeek(const QDate &date) const
{
    return KCalendarSystem::daysInWeek(date);
}

int KCalendarSystemCoptic::dayOfYear(const QDate &date) const
{
    return KCalendarSystem::dayOfYear(date);
}

int KCalendarSystemCoptic::dayOfWeek(const QDate &date) const
{
    return KCalendarSystem::dayOfWeek(date);
}

int KCalendarSystemCoptic::weekNumber(const QDate &date, int * yearNum) const
{
    return KCalendarSystem::weekNumber(date, yearNum);
}

bool KCalendarSystemCoptic::isLeapYear(int year) const
{
    return KCalendarSystem::isLeapYear(year);
}

bool KCalendarSystemCoptic::isLeapYear(const QDate &date) const
{
    return KCalendarSystem::isLeapYear(date);
}

QString KCalendarSystemCoptic::monthName(int month, int year, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(month, year, format);
}

QString KCalendarSystemCoptic::monthName(const QDate &date, MonthNameFormat format) const
{
    return KCalendarSystem::monthName(date, format);
}

QString KCalendarSystemCoptic::weekDayName(int weekDay, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(weekDay, format);
}

QString KCalendarSystemCoptic::weekDayName(const QDate &date, WeekDayNameFormat format) const
{
    return KCalendarSystem::weekDayName(date, format);
}

QString KCalendarSystemCoptic::yearString(const QDate &pDate, StringFormat format) const
{
    return KCalendarSystem::yearString(pDate, format);
}

QString KCalendarSystemCoptic::monthString(const QDate &pDate, StringFormat format) const
{
    return KCalendarSystem::monthString(pDate, format);
}

QString KCalendarSystemCoptic::dayString(const QDate &pDate, StringFormat format) const
{
    return KCalendarSystem::dayString(pDate, format);
}

int KCalendarSystemCoptic::yearStringToInteger(const QString &sNum, int &iLength) const
{
    return KCalendarSystem::yearStringToInteger(sNum, iLength);
}

int KCalendarSystemCoptic::monthStringToInteger(const QString &sNum, int &iLength) const
{
    return KCalendarSystem::monthStringToInteger(sNum, iLength);
}

int KCalendarSystemCoptic::dayStringToInteger(const QString &sNum, int &iLength) const
{
    return KCalendarSystem::dayStringToInteger(sNum, iLength);
}

QString KCalendarSystemCoptic::formatDate(const QDate &date, KLocale::DateFormat format) const
{
    return KCalendarSystem::formatDate(date, format);
}

QDate KCalendarSystemCoptic::readDate(const QString &str, bool *ok) const
{
    return KCalendarSystem::readDate(str, ok);
}

QDate KCalendarSystemCoptic::readDate(const QString &intstr, const QString &fmt, bool *ok) const
{
    return KCalendarSystem::readDate(intstr, fmt, ok);
}

QDate KCalendarSystemCoptic::readDate(const QString &str, KLocale::ReadDateFlags flags, bool *ok) const
{
    return KCalendarSystem::readDate(str, flags, ok);
}

int KCalendarSystemCoptic::weekStartDay() const
{
    return KCalendarSystem::weekStartDay();
}

int KCalendarSystemCoptic::weekDayOfPray() const
{
    return 7;
}

bool KCalendarSystemCoptic::isLunar() const
{
    return false;
}

bool KCalendarSystemCoptic::isLunisolar() const
{
    return false;
}

bool KCalendarSystemCoptic::isSolar() const
{
    return true;
}

bool KCalendarSystemCoptic::isProleptic() const
{
    return false;
}

bool KCalendarSystemCoptic::julianDayToDate(int jd, int &year, int &month, int &day) const
{
    //The Coptic calendar has 12 months of 30 days, a 13th month of 5 or 6 days,
    //and a leap year every 4th year without fail that falls on the last day of
    //the year, starting from year 3.

    //Use a fake year 0 for our epoch instead of the real epoch in year 1. This is because year 3
    //is the first leap year and a pattern of 365/365/366/365 is hard to calculate, instead a
    //pattern of 365/365/365/366 with the leap day the very last day makes the maths easier.

    //Day number in the fake epoch, 0 indexed
    int dayInEpoch = jd - (epoch().toJulianDay() - 365);
    //How many full 4 year leap cycles have been completed, 1461 = (365*3)+366
    int leapCyclesCompleted = dayInEpoch / 1461;
    //Which year are we in the current 4 year leap cycle, 0 indexed
    //Need the qMin as day 366 of 4th year of cycle returns following year (max 3 as 0 index)
    int yearInCurrentLeapCycle = qMin(3, (dayInEpoch % 1461) / 365);
    //Calculate the year
    year = (leapCyclesCompleted * 4) + yearInCurrentLeapCycle;
    //Days since the fake epoch up to 1st day of this year
    int daysBeforeThisYear = (year * 365) + (year / 4);
    //Gives the day number in this year, 0 indexed
    int dayOfThisYear = dayInEpoch -  daysBeforeThisYear;
    //Then just calculate month and day from that based on regular 30 day months
    month = ((dayOfThisYear) / 30) + 1;
    day = dayOfThisYear - ((month - 1) * 30) + 1;

    // If year is -ve then is BC.  In Coptic there is no year 0, but the maths
    // is easier if we pretend there is, so internally year of 0 = 1BC = -1 outside
    if (year < 1) {
        year = year - 1;
    }

    return true;
}

bool KCalendarSystemCoptic::dateToJulianDay(int year, int month, int day, int &jd) const
{
    //The Coptic calendar has 12 months of 30 days, a 13th month of 5 or 6 days,
    //and a leap year every 4th year without fail that falls on the last day of
    //the year, starting from year 3.  This simple repeating pattern makes calculating
    // a jd the simple process taking the epoch jd and adding on the years months and
    //days required.

    // If year is -ve then is 'BC'.  In Coptic there is no year 0, but the maths
    // is easier if we pretend there is, so internally year of -1 = 1BC = 0 internally
    int y;
    if (year < 1) {
        y = year + 1;
    } else {
        y = year;
    }

    jd = epoch().toJulianDay() - 1    // jd of day before Epoch
         + ((y - 1) * 365)              // Add all normal days in years preceding
         + (y / 4)                      // Add all leap days in years preceding
         + ((month - 1) * 30)           // Add days this year in months preceding
         + day;                         // Add days in this month

    return true;
}
