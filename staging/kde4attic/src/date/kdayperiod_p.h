/*
    Copyright (c) 2010 John Layt <john@layt.net>

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

#ifndef KDAYPERIOD_H
#define KDAYPERIOD_H

#include <QtCore/QSharedDataPointer>

#include "klocale.h"
#include <kde4attic_export.h>

class QDate;
class QString;
class QTime;

class KDayPeriodPrivate;

/**
 * @internal
 * @since 4.6
 *
 * This class is internal for now but may later be exported if required.
 *
 * This is a class to implement the CLDR Day Period Rules.
 *
 * Most commonly this will be used to implement support for the 12 hour clock
 * e.g. 00:00:00 to 11:59:59.999 is AM and 12:00:00 to 23:59:59.999 is PM.
 * However CLDR Day Period Rules cater for cultures where the day may be divided
 * into more periods than just AM/PM.
 * See http://www.unicode.org/reports/tr35/tr35-15.html#DayPeriodRules
 *
 * @b license GNU-LGPL v.2 or later
 *
 * @see KLocale
 *
 * @author John Layt <john@layt.net>
 */
class KDayPeriod
{
public:
    /**
     * Constructs a KDayPeriod for a given time period
     *
     * @param periodCode the unique code for the period
     * @param longName the KLocale::LongName of the Day Period
     * @param shortName the KLocale::ShortName of the Day Period
     * @param narrowName the KLocale::NarrowName of the Day Period
     * @param periodStart the inclusive Start Time of the Day Period
     * @param periodEnd the inclusive End Time of the Day Period
     * @param offsetFromStart offset of hour in period from the periodStart
     * @param offsetIfZero if hour is 0, what should it be converted to
     */
    KDayPeriod(const QString &periodCode,
               const QString &longName,
               const QString &shortName,
               const QString &narrowName,
               const QTime &periodStart,
               const QTime &periodEnd,
               int offsetFromStart,
               int offsetIfZero);

    /**
     * Constructs a null KDayPeriod
     */
    explicit KDayPeriod();

    /**
     * Copy Constructor
     *
     * @param rhs KDayPeriod to copy
     *
     */
    KDayPeriod(const KDayPeriod &rhs);

    /**
     * Destructor.
     */
    virtual ~KDayPeriod();

    /**
     * Assignment operator
     *
     * @param rhs KDayPeriod to assign
     *
     */
    KDayPeriod& operator=(const KDayPeriod &rhs);

    /**
     * Return the Period Code
     *
     * @return the Period Code
     */
    QString periodCode() const;

    /**
     * Return the time the Period starts at
     *
     * @return the time the Period starts
     */
    QTime periodStart() const;

    /**
     * Return the time the Period ends at
     *
     * @return the time the Period ends
     */
    QTime periodEnd() const;

    /**
     * Return translated Period Name in the required format
     * e.g. Ante Meridian, AM or A
     *
     * @param format the name format to return
     * @return the Period Name
     */
    QString periodName(KLocale::DateTimeComponentFormat format = KLocale::ShortName) const;

    /**
     * Calculate and return the hour in the Day Period for a given 24h time.
     *
     * For example, 17:00 would return 5 in the PM period of the standard 12 hour clock
     *
     * @param time the time to return the hour for
     * @return the Hour in the Day Period
     */
    int hourInPeriod(const QTime &time) const;

    /**
     * Calculate and return the 24hr time for a given hms in the Day Period
     *
     * For example, 5 in the PM period of the standard 12 hour clock would return 17:00
     *
     * @param hourInPeriod the hour in the day period
     * @param minute the minute in the hour
     * @param second the second in the minute
     * @param millisecond the millisecond in the second
     * @return the Time in the 24hr clock
     */
    QTime time(int hourInPeriod, int minute, int second, int millisecond = 0) const;

    /**
     * Return if the Day Period is valid
     *
     * @return if the Day Period is valid
     */
    bool isValid() const;

    /**
     * Return if a given time is in the Day Period
     *
     * @return if the time is valid in the Day Period
     */
    bool isValid(const QTime &time) const;

private:
    QSharedDataPointer<KDayPeriodPrivate> d;
};

#endif // KDAYPERIOD_H
