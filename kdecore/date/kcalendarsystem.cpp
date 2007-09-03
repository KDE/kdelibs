/*
    Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
    Copyright (c) 2002 Hans Petter Bieker <bieker@kde.org>
    Copyright (c) 2007 John Layt <john@layt.net>
 
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

#include "kcalendarsystem.h"

#include "kglobal.h"

#include <QtCore/QDateTime>

#include "kcalendarsystemgregorian.h"
#include "kcalendarsystemhebrew.h"
#include "kcalendarsystemhijri.h"
#include "kcalendarsystemjalali.h"

KCalendarSystem *KCalendarSystem::create( const QString &calendarType, const KLocale *locale )
{
    if ( calendarType == "hebrew" ) {
        return new KCalendarSystemHebrew( locale );
    }

    if ( calendarType == "hijri" ) {
        return new KCalendarSystemHijri( locale );
    }

    if ( calendarType == "gregorian" ) {
        return new KCalendarSystemGregorian( locale );
    }

    if ( calendarType == "jalali" ) {
        return new KCalendarSystemJalali( locale );
    }

    // ### HPB: Should it really be a default here?
    return new KCalendarSystemGregorian( locale );
}

QStringList KCalendarSystem::calendarSystems()
{
    QStringList lst;

    lst.append( "hebrew" );
    lst.append( "hijri" );
    lst.append( "gregorian" );
    lst.append( "jalali" );

    return lst;
}

QString KCalendarSystem::calendarLabel( const QString &calendarType )
{
    if ( calendarType == "gregorian" ) {
        return ki18nc( "@item Calendar system", "Gregorian" ).toString( KGlobal::locale() );
    }

    if ( calendarType == "hebrew" ) {
        return ki18nc( "@item Calendar system", "Hebrew" ).toString( KGlobal::locale() );
    }

    if ( calendarType == "hijri" ) {
        return ki18nc("@item Calendar system", "Hijri").toString( KGlobal::locale());
    }

    if ( calendarType == "jalali" ) {
        return ki18nc( "@item Calendar system", "Jalali" ).toString( KGlobal::locale() );
    }

    return ki18nc( "@item Calendar system", "Invalid Calendar Type" ).toString( KGlobal::locale() );
}


class KCalendarSystemPrivate
{
public:
    KCalendarSystemPrivate( KCalendarSystem *q ): q( q )
    {
    }

    ~KCalendarSystemPrivate()
    {
    }

    KCalendarSystem *q;

    bool setAnyDate( QDate &date, int year, int month, int day ) const;

    int stringToInteger( const QString &sNum, int &iLength );

    const KLocale *locale;
};

// Allows us to set dates outside publically valid range, USE WITH CARE!!!!
bool KCalendarSystemPrivate::setAnyDate( QDate &date, int year, int month, int day ) const
{
    int jd;
    q->dateToJulianDay( year, month, day, jd );
    date = QDate::fromJulianDay( jd );
    return true;
}

int KCalendarSystemPrivate::stringToInteger( const QString &sNum, int &iLength )
{
    int iPos = 0;
    int result = 0;

    for ( ; sNum.length() > iPos && sNum.at( iPos ).isDigit(); iPos++ ) {
        result *= 10;
        result += sNum.at( iPos ).digitValue();
    }
    iLength = iPos;

    return result;
}

KCalendarSystem::KCalendarSystem( const KLocale *locale ) : d( new KCalendarSystemPrivate( this ) )
{
    d->locale = locale;
}

KCalendarSystem::~KCalendarSystem()
{
    delete d;
}

// This method MUST be re-implemented in any new Calendar System
QDate KCalendarSystem::epoch() const
{
    // Earliest valid QDate
    return QDate::fromJulianDay( 1 );
}

QDate KCalendarSystem::earliestValidDate() const
{
    return epoch();
}

// This method MUST be re-implemented in any new Calendar System
QDate KCalendarSystem::latestValidDate() const
{
    // Default to Gregorian 9999-12-31
    return QDate::fromJulianDay( 5373484 );
}

// This method MUST be re-implemented in any new Calendar System
bool KCalendarSystem::isValid( int y, int month, int day ) const
{
    // Default to true Gregorian

    if ( y < year( earliestValidDate() ) || y > year( latestValidDate() ) ) {
        return false;
    }

    if ( month < 1 || month > 12 ) {
        return false;
    }

    if ( month == 2 ) {
        if ( isLeapYear( y ) ) {
            return ( day >= 1 && day <= 29 );
        } else {
            return ( day >= 1 && day <= 28 );
        }
    }

    if ( month == 4 || month == 6 || month == 9 || month == 11  ) {
        return ( day >= 1 && day <= 30 );
    }

    return ( day >= 1 && day <= 31 );
}

bool KCalendarSystem::isValid( const QDate &date ) const
{
    if ( date.isNull() || date < earliestValidDate() || date > latestValidDate() ) {
        return false;
    }
    return true;
}

bool KCalendarSystem::setDate( QDate &date, int year, int month, int day ) const
{
    if ( isValid( year, month, day ) ) {
        int jd;
        dateToJulianDay( year, month, day, jd );
        date = QDate::fromJulianDay( jd );
        return true;
    }

    return false;
}

// Deprecated
bool KCalendarSystem::setYMD( QDate &date, int year, int month, int day ) const
{
    return setDate( date, year, month, day );
}

int KCalendarSystem::year( const QDate &date ) const
{
    if ( isValid( date ) ) {
        int year, month, day;

        julianDayToDate( date.toJulianDay(), year, month, day );

        return year;
    }

    return 0;  // How do you denote invalid year when we support -ve years?
}

int KCalendarSystem::month( const QDate &date ) const
{
    if ( isValid( date ) ) {
        int year, month, day;

        julianDayToDate( date.toJulianDay(), year, month, day );

        return month;
    }

    return 0;
}

int KCalendarSystem::day( const QDate &date ) const
{
    if ( isValid( date ) ) {
        int year, month, day;

        julianDayToDate( date.toJulianDay(), year, month, day );

        return day;
    }

    return 0;
}

QDate KCalendarSystem::addYears( const QDate &date, int numYears ) const
{
    if ( isValid( date ) ) {

        int originalYear, originalMonth, originalDay;
        int newYear, newMonth, newDay;
        QDate firstOfNewMonth, newDate;

        julianDayToDate( date.toJulianDay(), originalYear, originalMonth, originalDay );

        newYear = originalYear + numYears;
        newMonth = originalMonth;

        //Adjust day number if new month has fewer days than old month
        if ( setDate( firstOfNewMonth, newYear, newMonth, 1 ) ) {
            int daysInNewMonth = daysInMonth( firstOfNewMonth );
            newDay = ( daysInNewMonth < originalDay ) ? daysInNewMonth : originalDay;

            if ( setDate( newDate, newYear, newMonth, newDay ) ) {
                return newDate;
            }
        }

    }

    //Is QDate's way of saying is invalid
    return QDate::fromJulianDay( 0 );
}

QDate KCalendarSystem::addMonths( const QDate &date, int numMonths ) const
{
    if ( isValid( date ) ) {

        int originalYear, originalMonth, originalDay;
        int newYear, newMonth, newDay;
        int monthsInOriginalYear, daysInNewMonth;
        QDate firstOfNewMonth, newDate;

        julianDayToDate( date.toJulianDay(), originalYear, originalMonth, originalDay );

        monthsInOriginalYear = monthsInYear( date );

        newYear = originalYear + ( ( originalMonth + numMonths ) / monthsInOriginalYear );
        newMonth = ( originalMonth + numMonths ) % monthsInOriginalYear;

        if ( newMonth == 0 ) {
            newYear = newYear - 1;
            newMonth = monthsInOriginalYear;
        }
        if ( newMonth < 0 ) {
            newYear = newYear - 1;
            newMonth = newMonth + monthsInOriginalYear;
        }

        //Adjust day number if new month has fewer days than old month
        if ( setDate( firstOfNewMonth, newYear, newMonth, 1 ) ) {
            daysInNewMonth = daysInMonth( firstOfNewMonth );
            newDay = ( daysInNewMonth < originalDay ) ? daysInNewMonth : originalDay;

            if ( setDate( newDate, newYear, newMonth, newDay ) ) {
                return newDate;
            }
        }

    }

    //Is QDate's way of saying is invalid
    return QDate::fromJulianDay( 0 );
}

QDate KCalendarSystem::addDays( const QDate &date, int numDays ) const
{
    // QDate only holds a uint and has no boundary checking in addDays(), so we need to check
    if ( isValid( date ) && (long) date.toJulianDay() + (long) numDays > 0 ) {
        // QDate adds straight to jd
        QDate temp = date.addDays( numDays );
        if ( isValid( temp ) ) {
            return temp;
        }
    }

    //Is QDate's way of saying is invalid
    return QDate::fromJulianDay( 0 );
}

int KCalendarSystem::monthsInYear( const QDate &date ) const
{
    // Last day of this year = first day of next year minus 1 day
    // Use setAnyDate() to allow correct calculation in last valid year

    if ( isValid( date ) ) {
        QDate firstDayOfNextYear;
        d->setAnyDate( firstDayOfNextYear, year( date ) + 1, 1, 1 );
        QDate lastDayOfThisYear = addDays( firstDayOfNextYear, -1 );
        return month( lastDayOfThisYear );
    }

    return -1;
}

int KCalendarSystem::weeksInYear( const QDate &date ) const
{
    if ( isValid( date ) ) {
        return weeksInYear( year( date ) );
    }
    return -1;
}

// ISO compliant week numbering, not traditional number, rename in KDE5 to isoWeeksInYear()
int KCalendarSystem::weeksInYear( int year ) const
{
    // Last day of this year = first day of next year minus 1 day
    // Use setAnyDate() to allow correct calculation in last valid year

    if ( isValid( year, 1, 1 ) ) {
        QDate firstDayOfNextYear;
        d->setAnyDate( firstDayOfNextYear, year + 1, 1, 1 );
        QDate lastDayOfThisYear = addDays( firstDayOfNextYear, -1 );

        int lastWeekInThisYear = weekNumber( lastDayOfThisYear );

        // If the last day of the year is in the first week of next year use the week before
        if ( lastWeekInThisYear == 1 ) {
            lastDayOfThisYear = lastDayOfThisYear.addDays( -7 );
            lastWeekInThisYear = weekNumber( lastDayOfThisYear );
        }

        return lastWeekInThisYear;
    }

    return -1;
}

int KCalendarSystem::daysInYear( const QDate &date ) const
{
    // Days in year = jd of first day of next year minus jd of first day of this year
    // Use setAnyDate() to allow correct calculation in last valid year

    if ( isValid( date ) ) {
        QDate firstDayOfThisYear, firstDayOfNextYear;

        setDate( firstDayOfThisYear, year( date ), 1, 1 );
        d->setAnyDate( firstDayOfNextYear, year( date ) + 1, 1, 1 );

        return ( firstDayOfNextYear.toJulianDay() - firstDayOfThisYear.toJulianDay() );
    }

    return -1;
}

int KCalendarSystem::daysInMonth( const QDate &date ) const
{
    // Days In Month = jd of first day of next month minus jd of first day of this month
    // Use setAnyDate() to allow correct calculation in last valid year

    if ( isValid( date ) ) {
        QDate firstDayOfThisMonth, firstDayOfNextMonth;

        int thisYear = year( date );
        int thisMonth = month( date );

        setDate( firstDayOfThisMonth, thisYear, thisMonth, 1 );

        //check if next month falls in next year
        if ( thisMonth < monthsInYear( date ) ) {
            setDate( firstDayOfNextMonth, thisYear, thisMonth + 1, 1 );
        } else {
            d->setAnyDate( firstDayOfNextMonth, thisYear + 1, 1, 1 );
        }

        return ( firstDayOfNextMonth.toJulianDay() - firstDayOfThisMonth.toJulianDay() );
    }

    return -1;
}

int KCalendarSystem::daysInWeek( const QDate &date ) const
{
    Q_UNUSED( date );
    return 7;
}

int KCalendarSystem::dayOfYear( const QDate &date ) const
{
    //Take the jd of the given date, and subtract the jd of the first day of that year

    if ( isValid( date ) ) {
        QDate firstDayOfYear;

        if ( setDate( firstDayOfYear, year( date ), 1, 1 ) ) {
            return ( date.toJulianDay() - firstDayOfYear.toJulianDay() + 1 );
        }
    }

    return -1;
}

int KCalendarSystem::dayOfWeek( const QDate &date ) const
{
    // Makes assumption that Julian Day 0 was day 1 of week
    // This is true for Julian/Gregorian calendar with jd 0 being Monday
    // We add 1 for ISO compliant numbering for 7 day week
    // Assumes we've never skipped weekdays
    if ( isValid( date ) ) {
        return ( ( date.toJulianDay() % daysInWeek( date ) ) + 1 );
    }

    return -1;
}

// ISO compliant week numbering, not traditional number, rename in KDE5 to isoWeekNumber()
// JPL still need to fully clean up here
int KCalendarSystem::weekNumber( const QDate &date, int *yearNum ) const
{
    if ( isValid( date ) ) {
        QDate firstDayWeek1, lastDayOfYear;
        int y = year( date );
        int week;
        int weekDay1, dayOfWeek1InYear;

        // let's guess 1st day of 1st week
        setDate( firstDayWeek1, y, 1, 1 );
        weekDay1 = dayOfWeek( firstDayWeek1 );

        // iso 8601: week 1  is the first containing thursday and week starts on monday
        if ( weekDay1 > 4 /*Thursday*/ ) {
            firstDayWeek1 = addDays( firstDayWeek1 , daysInWeek( date ) - weekDay1 + 1 ); // next monday
        }

        dayOfWeek1InYear = dayOfYear( firstDayWeek1 );

        // our date in prev year's week
        if ( dayOfYear( date ) < dayOfWeek1InYear ) { 
            if ( yearNum ) {
                *yearNum = y - 1;
            }
            return weeksInYear( y - 1 );
        }

        // let's check if its last week belongs to next year
        d->setAnyDate( lastDayOfYear, y + 1, 1, 1 );
        lastDayOfYear = addDays( lastDayOfYear, -1 );
        // if our date is in last week && 1st week in next year has thursday
        if ( ( dayOfYear( date ) >= daysInYear( date ) - dayOfWeek( lastDayOfYear ) + 1 )
             && dayOfWeek( lastDayOfYear ) < 4 ) {
            if ( yearNum ) {
                * yearNum = y + 1;
            }
             week = 1;
        } else {
            // To calculate properly the number of weeks from day a to x let's make a day 1 of week
            if( weekDay1 < 5 ) {
                firstDayWeek1 = addDays( firstDayWeek1, -( weekDay1 - 1 ) );
            }

            week = firstDayWeek1.daysTo( date ) / daysInWeek( date ) + 1;
        }

        return week;
    }

    return -1;
}

// This method MUST be reimplemented in any derived Calendar Systems
bool KCalendarSystem::isLeapYear( int year ) const
{
    // Default to pure Gregorian

    if ( year % 4 == 0 ) {
        if ( year % 100 != 0 ) {
            return true;
        } else if ( year % 400 == 0 ) {
            return true;
        }
    }
    return false;
}

bool KCalendarSystem::isLeapYear( const QDate &date ) const
{
    return isLeapYear( year( date ) );
}

QString KCalendarSystem::monthName( const QDate &date, MonthNameFormat format ) const
{
    if ( isValid( date ) ) {
        return monthName( month( date ), year( date ), format );
    }

    return QString();
}

QString KCalendarSystem::weekDayName( const QDate &date, WeekDayNameFormat format ) const
{
    if ( isValid( date ) ) {
        return weekDayName( dayOfWeek( date ), format );
    }

    return QString();
}

QString KCalendarSystem::yearString( const QDate &date, StringFormat format ) const
{
    if ( isValid( date ) ) {
        QString result;

        result.setNum( year( date ) );
        if ( format == ShortFormat && result.length() == 4 ) {
            result = result.right( 2 );
        }

        return result;
    }

    return QString();
}

QString KCalendarSystem::monthString( const QDate &date, StringFormat format ) const
{
    if ( isValid( date ) ) {
        QString result;

        result.setNum( month( date ) );
        if ( format == LongFormat && result.length() == 1 ) {
            result.prepend( QLatin1Char( '0' ) );
        }

        return result;
    }

    return QString();
}

QString KCalendarSystem::dayString( const QDate &date, StringFormat format ) const
{
    if ( isValid( date ) ) {
        QString result;

        result.setNum( day( date ) );
        if ( format == LongFormat && result.length() == 1 ) {
            result.prepend( QLatin1Char( '0' ) );
        }

        return result;
    }

    return QString();
}

int KCalendarSystem::yearStringToInteger( const QString &yearString, int &iLength ) const
{
    return d->stringToInteger( yearString, iLength );
}

int KCalendarSystem::monthStringToInteger( const QString &monthString, int &iLength ) const
{
    return d->stringToInteger( monthString, iLength );
}

int KCalendarSystem::dayStringToInteger( const QString &dayString, int &iLength ) const
{
    return d->stringToInteger( dayString, iLength );
}

QString KCalendarSystem::formatDate( const QDate &date, KLocale::DateFormat format ) const
{
    return locale()->formatDate( date, format );
}

QDate KCalendarSystem::readDate( const QString &str, bool *ok ) const
{
    return locale()->readDate( str, ok );
}

QDate KCalendarSystem::readDate( const QString &intstr, const QString &fmt, bool *ok ) const
{
    return locale()->readDate( intstr, fmt, ok );
}

QDate KCalendarSystem::readDate( const QString &str, KLocale::ReadDateFlags flags, bool *ok ) const
{
    return locale()->readDate( str, flags, ok );
}

int KCalendarSystem::weekStartDay() const
{
    return locale()->weekStartDay();
}

// Fake version using QDate, each Calendar System MUST implement the correct version for themselves
// The implementation MUST NOT do validity checking on date ranges, all calls to this function MUST
// instead be wrapped in validity checks, as sometimes we want this to work outside the public valid
// range, i.e. to allow us to internally set dates of 1/1/10000 which are not publically valid but
// are required for internal maths
bool KCalendarSystem::julianDayToDate( int jd, int &year, int &month, int &day ) const
{
    QDate date = QDate::fromJulianDay( jd );

    if ( date.isValid() ) {
        year = date.year();
        month = date.month();
        day = date.day();
    }

    return date.isValid();
}

// Fake version using QDate, each Calendar System MUST implement the correct version for themselves
// The implementation MUST NOT do validity checking on date ranges, all calls to this function MUST
// instead be wrapped in validity checks, as sometimes we want this to work outside the public valid
// range, i.e. to allow us to internally set dates of 1/1/10000 which are not publically valid but
// are required for internal maths
bool KCalendarSystem::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    QDate date;

    if ( date.setDate( year, month, day ) ) {
        jd = date.toJulianDay();
        return true;
    }

    return false;
}

const KLocale * KCalendarSystem::locale() const
{
    if ( d->locale ) {
        return d->locale;
    }

    return KGlobal::locale();
}
