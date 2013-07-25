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

#ifndef KCALENDARSYSTEMETHIOPIAN_H
#define KCALENDARSYSTEMETHIOPIAN_H

#include "kcalendarsystemcoptic_p.h"

class KCalendarSystemEthiopianPrivate;

/**
 * @internal
 * This is the Ethiopian calendar implementation which is the Coptic calendar
 * but with a differenct Epoch and month/weekday names
 *
 * @b license GNU-LGPL v.2 or later
 *
 * @see KLocale,KCalendarSystem
 *
 * @author John Layt <john@layt.net>
 */
class KCalendarSystemEthiopian: public KCalendarSystemCoptic
{
public:
    explicit KCalendarSystemEthiopian(const KSharedConfig::Ptr config, const KLocale *locale);
    virtual ~KCalendarSystemEthiopian();

    virtual QString calendarType() const;
    virtual KLocale::CalendarSystem calendarSystem() const;

    virtual QDate epoch() const;
    virtual QDate earliestValidDate() const;
    virtual QDate latestValidDate() const;

    virtual QString monthName(int month, int year, MonthNameFormat format = LongName) const;
    virtual QString monthName(const QDate &date, MonthNameFormat format = LongName) const;

    virtual QString weekDayName(int weekDay, WeekDayNameFormat format = LongDayName) const;
    virtual QString weekDayName(const QDate &date, WeekDayNameFormat format = LongDayName) const;

    virtual bool isLunar() const;
    virtual bool isLunisolar() const;
    virtual bool isSolar() const;
    virtual bool isProleptic() const;

protected:
    virtual bool julianDayToDate(qint64 jd, int &year, int &month, int &day) const;
    virtual bool dateToJulianDay(int year, int month, int day, qint64 &jd) const;
    KCalendarSystemEthiopian(KCalendarSystemEthiopianPrivate &dd, const KSharedConfig::Ptr config, const KLocale *locale);

private:
    Q_DECLARE_PRIVATE(KCalendarSystemEthiopian)
};

#endif // KCALENDARSYSTEMETHIOPIAN_H
