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

#ifndef KCALENDARSYSTEMGREGORIAN_H
#define KCALENDARSYSTEMGREGORIAN_H

#include "kcalendarsystem.h"

class KCalendarSystemGregorianPrivate;

/**
 * @internal
 * This is the pure Gregorian calendar implementation.
 *
 * Note: This is a proleptic version of the Gregorian calendar, it does not
 * copy the QDate method of using the Julian Calendar for dates before 15
 * October 1582.  This is the traditional proleptic Gregorian calendar
 * that does not have a year 0, unlike the ISO version which does.
 *
 * @b license GNU-LGPL v.2 or later
 *
 * @see KLocale,KCalendarSystem
 *
 * @author John Layt <john@layt.net>
 */
class KCalendarSystemGregorian: public KCalendarSystem
{
public:
    explicit KCalendarSystemGregorian(const KLocale *locale = 0);
    explicit KCalendarSystemGregorian(const KSharedConfig::Ptr config, const KLocale *locale = 0);
    virtual ~KCalendarSystemGregorian();

    virtual QString calendarType() const;

    virtual QDate epoch() const;
    virtual QDate earliestValidDate() const;
    virtual QDate latestValidDate() const;
    virtual bool isValid(int year, int month, int day) const;
    virtual bool isValid(const QDate &date) const;

    virtual bool isLeapYear(int year) const;
    virtual bool isLeapYear(const QDate &date) const;

    virtual QString monthName(int month, int year, MonthNameFormat format = LongName) const;
    virtual QString monthName(const QDate &date, MonthNameFormat format = LongName) const;

    virtual QString weekDayName(int weekDay, WeekDayNameFormat format = LongDayName) const;
    virtual QString weekDayName(const QDate &date, WeekDayNameFormat format = LongDayName) const;

    virtual int yearStringToInteger(const QString &sNum, int &iLength) const;

    virtual int weekDayOfPray() const;

    virtual bool isLunar() const;
    virtual bool isLunisolar() const;
    virtual bool isSolar() const;
    virtual bool isProleptic() const;

protected:
    virtual bool julianDayToDate(int jd, int &year, int &month, int &day) const;
    virtual bool dateToJulianDay(int year, int month, int day, int &jd) const;
    KCalendarSystemGregorian(KCalendarSystemGregorianPrivate &dd,
                                      const KSharedConfig::Ptr config = KSharedConfig::Ptr(),
                                      const KLocale *locale = 0);

private:
    friend class KCalendarSystemIndianNational;  // To allow access to julianDayToDate() and dateToJulianDay()
    Q_DECLARE_PRIVATE(KCalendarSystemGregorian)
};

#endif // KCALENDARSYSTEMGREGORIAN_H
