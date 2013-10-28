/*
    Copyright 2010 John Layt <john@layt.net>

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

#include "kcalendarsystemminguo_p.h"
#include "kcalendarsystemgregorianprivate_p.h"

#include "klocale.h"
#include "klocalizedstring.h"

#include <QtCore/QDate>

//Reuse the Gregorian private implementation
class KCalendarSystemMinguoPrivate : public KCalendarSystemGregorianPrivate
{
public:
    explicit KCalendarSystemMinguoPrivate(KCalendarSystemMinguo *q);
    virtual ~KCalendarSystemMinguoPrivate();

    virtual void loadDefaultEraList();
    virtual int daysInMonth(int year, int month) const;
    virtual int daysInYear(int year) const;
    virtual bool isLeapYear(int year) const;
    virtual int earliestValidYear() const;
};

//Override only a few of the Gregorian private methods

KCalendarSystemMinguoPrivate::KCalendarSystemMinguoPrivate(KCalendarSystemMinguo *q)
                            : KCalendarSystemGregorianPrivate(q)
{
}

KCalendarSystemMinguoPrivate::~KCalendarSystemMinguoPrivate()
{
}

void KCalendarSystemMinguoPrivate::loadDefaultEraList()
{
    QString name, shortName, format;

    name = i18nc("Calendar Era: Taiwan Republic of China Era, years > 0, LongFormat", "Republic of China Era");
    shortName = i18nc("Calendar Era: Taiwan Republic of China Era, years > 0, ShortFormat", "ROC");
    format = i18nc("(kdedt-format) Taiwan, ROC, full era year format used for %EY, e.g. ROC 99", "%EC %Ey");
    addEra('+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format);
}

int KCalendarSystemMinguoPrivate::daysInMonth(int year, int month) const
{
    return KCalendarSystemGregorianPrivate::daysInMonth(year + 1911, month);
}

int KCalendarSystemMinguoPrivate::daysInYear(int year) const
{
    return KCalendarSystemGregorianPrivate::daysInYear(year + 1911);
}

bool KCalendarSystemMinguoPrivate::isLeapYear(int year) const
{
    return KCalendarSystemGregorianPrivate::isLeapYear(year + 1911);
}

int KCalendarSystemMinguoPrivate::earliestValidYear() const
{
    return 1;
}


KCalendarSystemMinguo::KCalendarSystemMinguo(const KSharedConfig::Ptr config, const KLocale *locale)
                     : KCalendarSystemGregorian(*new KCalendarSystemMinguoPrivate(this), config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemMinguo::KCalendarSystemMinguo(KCalendarSystemMinguoPrivate &dd,
                                             const KSharedConfig::Ptr config, const KLocale *locale)
                     : KCalendarSystemGregorian(dd, config, locale)
{
    d_ptr->loadConfig(calendarType());
}

KCalendarSystemMinguo::~KCalendarSystemMinguo()
{
}

QString KCalendarSystemMinguo::calendarType() const
{
    return QLatin1String("minguo");
}

KLocale::CalendarSystem KCalendarSystemMinguo::calendarSystem() const
{
    return KLocale::MinguoCalendar;
}

QDate KCalendarSystemMinguo::epoch() const
{
    // 0001-01-01 = 1912-01-01 AD Gregorian
    return QDate::fromJulianDay(2419403);
}

QDate KCalendarSystemMinguo::earliestValidDate() const
{
    return epoch();
}

QDate KCalendarSystemMinguo::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    // 9999-12-31 = 11910-12-31 AD Gregorian
    return QDate::fromJulianDay(6071462);
}

QString KCalendarSystemMinguo::monthName(int month, int year, MonthNameFormat format) const
{
    return KCalendarSystemGregorian::monthName(month, year, format);
}

QString KCalendarSystemMinguo::monthName(const QDate &date, MonthNameFormat format) const
{
    return KCalendarSystemGregorian::monthName(date, format);
}

QString KCalendarSystemMinguo::weekDayName(int weekDay, WeekDayNameFormat format) const
{
    return KCalendarSystemGregorian::weekDayName(weekDay, format);
}

QString KCalendarSystemMinguo::weekDayName(const QDate &date, WeekDayNameFormat format) const
{
    return KCalendarSystemGregorian::weekDayName(date, format);
}

bool KCalendarSystemMinguo::isLunar() const
{
    return KCalendarSystemGregorian::isLunar();
}

bool KCalendarSystemMinguo::isLunisolar() const
{
    return KCalendarSystemGregorian::isLunisolar();
}

bool KCalendarSystemMinguo::isSolar() const
{
    return KCalendarSystemGregorian::isSolar();
}

bool KCalendarSystemMinguo::isProleptic() const
{
    return false;
}

bool KCalendarSystemMinguo::julianDayToDate(qint64 jd, int &year, int &month, int &day) const
{
    bool result = KCalendarSystemGregorian::julianDayToDate(jd, year, month, day);
    year = year - 1911;
    return result;
}

bool KCalendarSystemMinguo::dateToJulianDay(int year, int month, int day, qint64 &jd) const
{
    return KCalendarSystemGregorian::dateToJulianDay(year + 1911, month, day, jd);
}

