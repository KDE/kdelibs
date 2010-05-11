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

#ifndef KCALENDARSYSTEMJULIAN_H
#define KCALENDARSYSTEMJULIAN_H

#include "kcalendarsystem.h"

class KCalendarSystemJulianPrivate;

/**
 * @internal
 * This is the Julian calendar implementation.
 *
 * WARNING: This is a proleptic version of the Julian calendar, it does not
 * reflect the historic marking of leap years prior to 4 AD and always
 * uses New Style years with 1 Jan for the start of the year.
 *
 * @b license GNU-LGPL v.2 or later
 *
 * @see KLocale,KCalendarSystem
 *
 * @author John Layt <john@layt.net>
 */
class KCalendarSystemJulian: public KCalendarSystem
{
public:
    explicit KCalendarSystemJulian( const KLocale *locale = 0 );
    explicit KCalendarSystemJulian( const KSharedConfig::Ptr config, const KLocale *locale = 0 );
    virtual ~KCalendarSystemJulian();

    virtual QString calendarType() const;

    virtual QDate epoch() const;
    virtual QDate earliestValidDate() const;
    virtual QDate latestValidDate() const;
    virtual bool isValid( int year, int month, int day ) const;
    virtual bool isValid( const QDate &date ) const;

    virtual bool setDate( QDate &date, int year, int month, int day ) const;
    /** @deprecated */
    virtual bool setYMD( QDate &date, int year, int month, int day ) const;

    virtual int year( const QDate &date ) const;
    virtual int month( const QDate &date ) const;
    virtual int day( const QDate &date ) const;

    virtual QDate addYears( const QDate &date, int nyears ) const;
    virtual QDate addMonths( const QDate &date, int nmonths ) const;
    virtual QDate addDays( const QDate &date, int ndays ) const;

    virtual int monthsInYear( const QDate &date ) const;
    virtual int weeksInYear( const QDate &date ) const;
    virtual int weeksInYear( int year ) const;
    virtual int daysInYear( const QDate &date ) const;
    virtual int daysInMonth( const QDate &date ) const;
    virtual int daysInWeek( const QDate &date ) const;

    virtual int dayOfYear( const QDate &date ) const;
    virtual int dayOfWeek( const QDate &date ) const;

    virtual int weekNumber( const QDate &date, int *yearNum = 0 ) const;

    virtual bool isLeapYear( int year ) const;
    virtual bool isLeapYear( const QDate &date ) const;

    virtual QString monthName( int month, int year, MonthNameFormat format = LongName ) const;
    virtual QString monthName( const QDate &date, MonthNameFormat format = LongName ) const;

    virtual QString weekDayName( int weekDay, WeekDayNameFormat format = LongDayName ) const;
    virtual QString weekDayName( const QDate &date, WeekDayNameFormat format = LongDayName ) const;

    virtual QString yearString( const QDate & pDate, StringFormat format = LongFormat ) const;
    virtual QString monthString( const QDate &pDate, StringFormat format = LongFormat ) const;
    virtual QString dayString( const QDate &pDate, StringFormat format = LongFormat ) const;

    virtual int yearStringToInteger( const QString &sNum, int &iLength ) const;
    virtual int monthStringToInteger( const QString &sNum, int &iLength ) const;
    virtual int dayStringToInteger( const QString &sNum, int &iLength ) const;

    virtual QString formatDate( const QDate &fromDate, KLocale::DateFormat toFormat = KLocale::LongDate ) const;

    virtual QDate readDate( const QString &str, bool *ok = 0 ) const;
    virtual QDate readDate( const QString &dateString, const QString &dateFormat, bool *ok = 0 ) const;
    virtual QDate readDate( const QString &str, KLocale::ReadDateFlags flags, bool *ok = 0 ) const;

    virtual int weekStartDay() const;
    virtual int weekDayOfPray () const;

    virtual bool isLunar() const;
    virtual bool isLunisolar() const;
    virtual bool isSolar() const;
    virtual bool isProleptic() const;

protected:
    virtual bool julianDayToDate( int jd, int &year, int &month, int &day ) const;
    virtual bool dateToJulianDay( int year, int month, int day, int &jd ) const;
    KCalendarSystemJulian( KCalendarSystemJulianPrivate &dd,
                           const KSharedConfig::Ptr config = KSharedConfig::Ptr(),
                           const KLocale *locale = 0 );

private:
    Q_DECLARE_PRIVATE( KCalendarSystemJulian )
    KCalendarSystemJulianPrivate * const dont_use; // KDE5 remove, use shared d
};

#endif // KCALENDARSYSTEMJULIAN_H
