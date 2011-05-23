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

#ifndef KCALENDARSYSTEMISLAMICCIVIL_H
#define KCALENDARSYSTEMISLAMICCIVIL_H

#include "kcalendarsystem.h"

class KCalendarSystemIslamicCivilPrivate;

/**
 * @internal
 * This is the Islamic Civil calendar implementation.
 *
 * The Islamic or Hijri calendar is the traditional calendar used in the Middle
 * East.  This implementation is of the civil calculation that does not take
 * observed sunset into account and so may vary from actual dates by 1-2 days.
 *
 * @b license GNU-LGPL v2+
 *
 * @see KLocale,KCalendarSystem
 *
 * @author Carlos Moro <cfmoro@correo.uniovi.es>
 */
class KCalendarSystemIslamicCivil : public KCalendarSystem
{
public:
    explicit KCalendarSystemIslamicCivil(const KLocale *locale = 0);
    explicit KCalendarSystemIslamicCivil(const KSharedConfig::Ptr config, const KLocale *locale = 0);
    virtual ~KCalendarSystemIslamicCivil();

    virtual QString calendarType() const;

    virtual QDate epoch() const;
    virtual QDate earliestValidDate() const;
    virtual QDate latestValidDate() const;
    virtual bool isValid(int year, int month, int day) const;
    virtual bool isValid(const QDate &date) const;

    virtual bool setDate(QDate &date, int year, int month, int day) const;
    /** @deprecated */
    virtual bool setYMD(QDate &date, int year, int month, int day) const;

    virtual int year(const QDate &date) const;
    virtual int month(const QDate &date) const;
    virtual int day(const QDate &date) const;

    virtual QDate addYears(const QDate &date, int nyears) const;
    virtual QDate addMonths(const QDate &date, int nmonths) const;
    virtual QDate addDays(const QDate &date, int ndays) const;

    virtual int monthsInYear(const QDate &date) const;
    virtual int weeksInYear(const QDate &date) const;
    virtual int weeksInYear(int year) const;
    virtual int daysInYear(const QDate &date) const;
    virtual int daysInMonth(const QDate &date) const;
    virtual int daysInWeek(const QDate &date) const;

    virtual int dayOfYear(const QDate &date) const;
    virtual int dayOfWeek(const QDate &date) const;

    virtual int weekNumber(const QDate &date, int *yearNum = 0) const;

    virtual bool isLeapYear(int year) const;
    virtual bool isLeapYear(const QDate &date) const;

    virtual QString monthName(int month, int year, MonthNameFormat format = LongName) const;
    virtual QString monthName(const QDate &date, MonthNameFormat format = LongName) const;

    virtual QString weekDayName(int weekDay, WeekDayNameFormat format = LongDayName) const;
    virtual QString weekDayName(const QDate &date, WeekDayNameFormat format = LongDayName) const;

    virtual QString yearString(const QDate & pDate, StringFormat format = LongFormat) const;
    virtual QString monthString(const QDate &pDate, StringFormat format = LongFormat) const;
    virtual QString dayString(const QDate &pDate, StringFormat format = LongFormat) const;

    virtual int yearStringToInteger(const QString &sNum, int &iLength) const;
    virtual int monthStringToInteger(const QString &sNum, int &iLength) const;
    virtual int dayStringToInteger(const QString &sNum, int &iLength) const;

    virtual QString formatDate(const QDate &fromDate, KLocale::DateFormat toFormat = KLocale::LongDate) const;

    virtual QDate readDate(const QString &str, bool *ok = 0) const;
    virtual QDate readDate(const QString &dateString, const QString &dateFormat, bool *ok = 0) const;
    virtual QDate readDate(const QString &str, KLocale::ReadDateFlags flags, bool *ok = 0) const;

    virtual int weekStartDay() const;
    virtual int weekDayOfPray() const;

    virtual bool isLunar() const;
    virtual bool isLunisolar() const;
    virtual bool isSolar() const;
    virtual bool isProleptic() const;

protected:
    virtual bool julianDayToDate(int jd, int &year, int &month, int &day) const;
    virtual bool dateToJulianDay(int year, int month, int day, int &jd) const;
    KCalendarSystemIslamicCivil(KCalendarSystemIslamicCivilPrivate &dd,
                         const KSharedConfig::Ptr config = KSharedConfig::Ptr(),
                         const KLocale *locale = 0);

private:
    Q_DECLARE_PRIVATE(KCalendarSystemIslamicCivil)
    KCalendarSystemIslamicCivilPrivate * const dont_use; // KDE5 remove, use shared d
};

#endif // KCALENDARSYSTEMISLAMICCIVIL_H
