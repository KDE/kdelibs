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

#include "kcalendarsystemthai_p.h"
#include "kcalendarsystemgregorianprivate_p.h"

#include "klocale.h"
#include "klocalizedstring.h"

#include <QtCore/QDate>

//Reuse the Gregorian private implementation
class KCalendarSystemThaiPrivate : public KCalendarSystemGregorianPrivate
{
public:
    explicit KCalendarSystemThaiPrivate(KCalendarSystemThai *q);
    virtual ~KCalendarSystemThaiPrivate();

    virtual void loadDefaultEraList();
    virtual int daysInMonth(int year, int month) const;
    virtual int daysInYear(int year) const;
    virtual bool isLeapYear(int year) const;
    virtual bool hasYearZero() const;
    virtual int earliestValidYear() const;
};

//Override only a few of the Gregorian private methods

KCalendarSystemThaiPrivate::KCalendarSystemThaiPrivate(KCalendarSystemThai *q)
                          : KCalendarSystemGregorianPrivate(q)
{
}

KCalendarSystemThaiPrivate::~KCalendarSystemThaiPrivate()
{
}

void KCalendarSystemThaiPrivate::loadDefaultEraList()
{
    QString name, shortName, format;

    name = i18nc("Calendar Era: Thai Buddhist Era, years > 0, LongFormat", "Buddhist Era");
    shortName = i18nc("Calendar Era: Thai Buddhist Era, years > 0, ShortFormat", "BE");
    format = i18nc("(kdedt-format) Thai, BE, full era year format used for %EY, e.g. 2000 BE", "%Ey %EC");
    addEra('+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format);
}

int KCalendarSystemThaiPrivate::daysInMonth(int year, int month) const
{
    return KCalendarSystemGregorianPrivate::daysInMonth(year - 543, month);
}

int KCalendarSystemThaiPrivate::daysInYear(int year) const
{
    return KCalendarSystemGregorianPrivate::daysInYear(year - 543);
}

bool KCalendarSystemThaiPrivate::isLeapYear(int year) const
{
    return KCalendarSystemGregorianPrivate::isLeapYear(year - 543);
}

bool KCalendarSystemThaiPrivate::hasYearZero() const
{
    return true;
}

int KCalendarSystemThaiPrivate::earliestValidYear() const
{
    return 0;
}


KCalendarSystemThai::KCalendarSystemThai(const KSharedConfig::Ptr config, const KLocale *locale)
                   : KCalendarSystemGregorian(*new KCalendarSystemThaiPrivate(this), config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemThai::KCalendarSystemThai(KCalendarSystemThaiPrivate &dd,
                                         const KSharedConfig::Ptr config, const KLocale *locale)
                   : KCalendarSystemGregorian(dd, config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemThai::~KCalendarSystemThai()
{
}

QString KCalendarSystemThai::calendarType() const
{
    return QLatin1String("thai");
}

KLocale::CalendarSystem KCalendarSystemThai::calendarSystem() const
{
    return KLocale::ThaiCalendar;
}

QDate KCalendarSystemThai::epoch() const
{
    // 0000-01-01 = 0544-01-01 BC Gregorian = 0544-01-07 BC Julian
    return QDate::fromJulianDay(1522734);
}

QDate KCalendarSystemThai::earliestValidDate() const
{
    return epoch();
}

QDate KCalendarSystemThai::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    // 9999-12-31 = 9456-12-31 AD Gregorian
    return QDate::fromJulianDay(5175158);
}

bool KCalendarSystemThai::isLunar() const
{
    return KCalendarSystemGregorian::isLunar();
}

bool KCalendarSystemThai::isLunisolar() const
{
    return KCalendarSystemGregorian::isLunisolar();
}

bool KCalendarSystemThai::isSolar() const
{
    return KCalendarSystemGregorian::isSolar();
}

bool KCalendarSystemThai::isProleptic() const
{
    return false;
}

bool KCalendarSystemThai::julianDayToDate(qint64 jd, int &year, int &month, int &day) const
{
    bool result = KCalendarSystemGregorian::julianDayToDate(jd, year, month, day);
    year = year + 543;
    return result;
}

bool KCalendarSystemThai::dateToJulianDay(int year, int month, int day, qint64 &jd) const
{
    return KCalendarSystemGregorian::dateToJulianDay(year - 543, month, day, jd);
}

