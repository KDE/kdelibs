/*
    Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
    Copyright (c) 2002 Hans Petter Bieker <bieker@kde.org>
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

#ifndef KCALENDARSYSTEMQDATE_H
#define KCALENDARSYSTEMQDATE_H

#include "kcalendarsystem.h"

class KCalendarSystemQDatePrivate;

/**
 * @internal
 * This is the QDate calendar implementation which combines the Julian and Gregorian
 * calculations into a single calendar.  It should always be used by default in Qt
 * applications for consistency.
 *
 * @b license GNU-LGPL v.2+
 *
 * @see KLocale,KCalendarSystem
 *
 * @author John Layt <john@layt.net>
 */
class KCalendarSystemQDate: public KCalendarSystem
{
public:
    explicit KCalendarSystemQDate(const KLocale *locale = 0);
    explicit KCalendarSystemQDate(const KSharedConfig::Ptr config, const KLocale *locale = 0);
    virtual ~KCalendarSystemQDate();

    virtual QString calendarType() const;

    virtual QDate epoch() const;
    virtual QDate earliestValidDate() const;
    virtual QDate latestValidDate() const;
    virtual bool isValid(int year, int month, int day) const;
    virtual bool isValid(const QDate &date) const;

    virtual int year(const QDate &date) const;
    virtual int month(const QDate &date) const;
    virtual int day(const QDate &date) const;

    virtual int daysInYear(const QDate &date) const;
    virtual int daysInMonth(const QDate &date) const;

    virtual int dayOfYear(const QDate &date) const;
    virtual int dayOfWeek(const QDate &date) const;

    virtual bool isLeapYear(int year) const;
    virtual bool isLeapYear(const QDate &date) const;

    virtual QString monthName(int month, int year, MonthNameFormat format = LongName) const;
    virtual QString monthName(const QDate &date, MonthNameFormat format = LongName) const;

    virtual QString weekDayName(int weekDay, WeekDayNameFormat format = LongDayName) const;
    virtual QString weekDayName(const QDate &date, WeekDayNameFormat format = LongDayName) const;

    virtual int weekDayOfPray() const;

    virtual bool isLunar() const;
    virtual bool isLunisolar() const;
    virtual bool isSolar() const;
    virtual bool isProleptic() const;

protected:
    virtual bool julianDayToDate(int jd, int &year, int &month, int &day) const;
    virtual bool dateToJulianDay(int year, int month, int day, int &jd) const;
    KCalendarSystemQDate(KCalendarSystemQDatePrivate &dd,
                             const KSharedConfig::Ptr config = KSharedConfig::Ptr(),
                             const KLocale *locale = 0);

private:
    Q_DECLARE_PRIVATE(KCalendarSystemQDate)
};

#endif
