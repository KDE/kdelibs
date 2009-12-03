/*
    Copyright (c) 2009 John Layt <john@layt.net>
 
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

// Derived gregorian kde calendar class

#include "kcalendarsystemgregorianproleptic_p.h"

#include "kdebug.h"
#include "klocale.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

KCalendarSystemGregorianProleptic::KCalendarSystemGregorianProleptic( const KLocale * locale )
                         : KCalendarSystem( locale ), d( 0 )
{
}

KCalendarSystemGregorianProleptic::~KCalendarSystemGregorianProleptic()
{
}

QString KCalendarSystemGregorianProleptic::calendarType() const
{
    return QLatin1String( "gregorian-proleptic" );
}

QDate KCalendarSystemGregorianProleptic::epoch() const
{
    return QDate::fromJulianDay( 1721426 );
}

QDate KCalendarSystemGregorianProleptic::earliestValidDate() const
{
    // 1 Jan 4713 BC, no year zero
    return QDate::fromJulianDay( 38 );
}

QDate KCalendarSystemGregorianProleptic::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    // In Gregorian this is 9999-12-31, which is  is jd 5373484
    // Can't call setDate( 9999, 12, 31 ) as it creates circular reference!
    return QDate::fromJulianDay( 5373484 );
}

bool KCalendarSystemGregorianProleptic::isValid( int year, int month, int day ) const
{
    if ( year < -4713 || year > 9999 || year == 0 ) {
        return false;
    }

    if ( month < 1 || month > 12 ) {
        return false;
    }

    if ( month == 2 ) {
        if ( isLeapYear( year ) ) {
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

bool KCalendarSystemGregorianProleptic::isValid( const QDate &date ) const
{
    return KCalendarSystem::isValid( date );
}

bool KCalendarSystemGregorianProleptic::setDate( QDate &date, int year, int month, int day ) const
{
    return KCalendarSystem::setDate( date, year, month, day );
}

// Deprecated
bool KCalendarSystemGregorianProleptic::setYMD( QDate &date, int y, int m, int d ) const
{
    return KCalendarSystem::setDate( date, y, m, d );
}

int KCalendarSystemGregorianProleptic::year( const QDate &date ) const
{
    return KCalendarSystem::year( date );
}

int KCalendarSystemGregorianProleptic::month( const QDate &date ) const
{
    return KCalendarSystem::month( date );
}

int KCalendarSystemGregorianProleptic::day( const QDate &date ) const
{
    return KCalendarSystem::day( date );
}

QDate KCalendarSystemGregorianProleptic::addYears( const QDate &date, int nyears ) const
{
    return KCalendarSystem::addYears( date, nyears );
}

QDate KCalendarSystemGregorianProleptic::addMonths( const QDate &date, int nmonths ) const
{
    return KCalendarSystem::addMonths( date, nmonths );
}

QDate KCalendarSystemGregorianProleptic::addDays( const QDate &date, int ndays ) const
{
    return KCalendarSystem::addDays( date, ndays );
}

int KCalendarSystemGregorianProleptic::monthsInYear( const QDate &date ) const
{
    Q_UNUSED( date )
    return 12;
}

int KCalendarSystemGregorianProleptic::weeksInYear( const QDate &date ) const
{
    return KCalendarSystem::weeksInYear( date );
}

int KCalendarSystemGregorianProleptic::weeksInYear( int year ) const
{
    return KCalendarSystem::weeksInYear( year );
}

int KCalendarSystemGregorianProleptic::daysInYear( const QDate &date ) const
{
    if ( !isValid( date ) ) {
        return -1;
    }

    if ( isLeapYear( date ) ) {
        return 366;
    } else {
        return 365;
    }
}

int KCalendarSystemGregorianProleptic::daysInMonth( const QDate &date ) const
{
    if ( !isValid( date ) ) {
        return -1;
    }

    int m = month( date );

    if ( m == 2 ) {
        if ( isLeapYear( year( date ) ) ) {
            return 29;
        } else {
            return 28;
        }
    }

    if ( m == 4 || m == 6 || m == 9 || m == 11 ) {
        return 30;
    }

    return 31;
}

int KCalendarSystemGregorianProleptic::daysInWeek( const QDate &date ) const
{
    Q_UNUSED( date );
    return 7;
}

int KCalendarSystemGregorianProleptic::dayOfYear( const QDate &date ) const
{
    return KCalendarSystem::dayOfYear( date );
}

int KCalendarSystemGregorianProleptic::dayOfWeek( const QDate &date ) const
{
    return KCalendarSystem::dayOfWeek( date );
}

int KCalendarSystemGregorianProleptic::weekNumber( const QDate &date, int * yearNum ) const
{
    return KCalendarSystem::weekNumber( date, yearNum );
}

bool KCalendarSystemGregorianProleptic::isLeapYear( int year ) const
{
    int y;

    if ( year < 1 ) {
        y = year + 1;
    } else {
        y = year;
    }

    if ( y % 4 == 0 ) {
        if ( y % 100 != 0 ) {
            return true;
        } else if ( y % 400 == 0 ) {
            return true;
        }
    }
    return false;
}

bool KCalendarSystemGregorianProleptic::isLeapYear( const QDate &date ) const
{
    return KCalendarSystem::isLeapYear( date );
}

QString KCalendarSystemGregorianProleptic::monthName( int month, int year, MonthNameFormat format ) const
{
    Q_UNUSED( year );

    if ( format == ShortNamePossessive ) {
        switch ( month ) {
        case 1:
            return ki18nc( "of January",   "of Jan" ).toString( locale() );
        case 2:
            return ki18nc( "of February",  "of Feb" ).toString( locale() );
        case 3:
            return ki18nc( "of March",     "of Mar" ).toString( locale() );
        case 4:
            return ki18nc( "of April",     "of Apr" ).toString( locale() );
        case 5:
            return ki18nc( "of May short", "of May" ).toString( locale() );
        case 6:
            return ki18nc( "of June",      "of Jun" ).toString( locale() );
        case 7:
            return ki18nc( "of July",      "of Jul" ).toString( locale() );
        case 8:
            return ki18nc( "of August",    "of Aug" ).toString( locale() );
        case 9:
            return ki18nc( "of September", "of Sep" ).toString( locale() );
        case 10:
            return ki18nc( "of October",   "of Oct" ).toString( locale() );
        case 11:
            return ki18nc( "of November",  "of Nov" ).toString( locale() );
        case 12:
            return ki18nc( "of December",  "of Dec" ).toString( locale() );
        default:
            return QString();
        }
    }

    if ( format == LongNamePossessive ) {
        switch ( month ) {
        case 1:
            return ki18n( "of January" ).toString( locale() );
        case 2:
            return ki18n( "of February" ).toString( locale() );
        case 3:
            return ki18n( "of March" ).toString( locale() );
        case 4:
            return ki18n( "of April" ).toString( locale() );
        case 5:
            return ki18nc( "of May long", "of May" ).toString( locale() );
        case 6:
            return ki18n( "of June" ).toString( locale() );
        case 7:
            return ki18n( "of July" ).toString( locale() );
        case 8:
            return ki18n( "of August" ).toString( locale() );
        case 9:
            return ki18n( "of September" ).toString( locale() );
        case 10:
            return ki18n( "of October" ).toString( locale() );
        case 11:
            return ki18n( "of November" ).toString( locale() );
        case 12:
            return ki18n( "of December" ).toString( locale() );
        default:
            return QString();
        }
    }

    if ( format == ShortName ) {
        switch ( month ) {
        case 1:
            return ki18nc( "January", "Jan" ).toString( locale() );
        case 2:
            return ki18nc( "February", "Feb" ).toString( locale() );
        case 3:
            return ki18nc( "March", "Mar" ).toString( locale() );
        case 4:
            return ki18nc( "April", "Apr" ).toString( locale() );
        case 5:
            return ki18nc( "May short", "May" ).toString( locale() );
        case 6:
            return ki18nc( "June", "Jun" ).toString( locale() );
        case 7:
            return ki18nc( "July", "Jul" ).toString( locale() );
        case 8:
            return ki18nc( "August", "Aug" ).toString( locale() );
        case 9:
            return ki18nc( "September", "Sep" ).toString( locale() );
        case 10:
            return ki18nc( "October", "Oct" ).toString( locale() );
        case 11:
            return ki18nc( "November", "Nov" ).toString( locale() );
        case 12:
            return ki18nc( "December", "Dec" ).toString( locale() );
        default:
            return QString();
        }
    }

    // Default to LongName
    switch ( month ) {
    case 1:
        return ki18n( "January" ).toString( locale() );
    case 2:
        return ki18n( "February" ).toString( locale() );
    case 3:
        return ki18nc( "March long", "March" ).toString( locale() );
    case 4:
        return ki18n( "April" ).toString( locale() );
    case 5:
        return ki18nc( "May long", "May" ).toString( locale() );
    case 6:
        return ki18n( "June" ).toString( locale() );
    case 7:
        return ki18n( "July" ).toString( locale() );
    case 8:
        return ki18nc( "August long", "August" ).toString( locale() );
    case 9:
        return ki18n( "September" ).toString( locale() );
    case 10:
        return ki18n( "October" ).toString( locale() );
    case 11:
        return ki18n( "November" ).toString( locale() );
    case 12:
        return ki18n( "December" ).toString( locale() );
    default:
        return QString();
    }
}

QString KCalendarSystemGregorianProleptic::monthName( const QDate &date, MonthNameFormat format ) const
{
    return KCalendarSystem::monthName( date, format );
}


QString KCalendarSystemGregorianProleptic::weekDayName( int weekDay, WeekDayNameFormat format ) const
{
    if ( format == ShortDayName ) {
        switch ( weekDay ) {
        case 1:  return ki18nc( "Monday",    "Mon" ).toString( locale() );
        case 2:  return ki18nc( "Tuesday",   "Tue" ).toString( locale() );
        case 3:  return ki18nc( "Wednesday", "Wed" ).toString( locale() );
        case 4:  return ki18nc( "Thursday",  "Thu" ).toString( locale() );
        case 5:  return ki18nc( "Friday",    "Fri" ).toString( locale() );
        case 6:  return ki18nc( "Saturday",  "Sat" ).toString( locale() );
        case 7:  return ki18nc( "Sunday",    "Sun" ).toString( locale() );
        default: return QString();
        }
    }

    switch ( weekDay ) {
    case 1:  return ki18n( "Monday" ).toString( locale() );
    case 2:  return ki18n( "Tuesday" ).toString( locale() );
    case 3:  return ki18n( "Wednesday" ).toString( locale() );
    case 4:  return ki18n( "Thursday" ).toString( locale() );
    case 5:  return ki18n( "Friday" ).toString( locale() );
    case 6:  return ki18n( "Saturday" ).toString( locale() );
    case 7:  return ki18n( "Sunday" ).toString( locale() );
    default: return QString();
    }
}

QString KCalendarSystemGregorianProleptic::weekDayName( const QDate &date, WeekDayNameFormat format ) const
{
    return KCalendarSystem::weekDayName( date, format );
}

QString KCalendarSystemGregorianProleptic::yearString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::yearString( pDate, format );
}

QString KCalendarSystemGregorianProleptic::monthString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::monthString( pDate, format );
}

QString KCalendarSystemGregorianProleptic::dayString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::dayString( pDate, format );
}

int KCalendarSystemGregorianProleptic::yearStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::yearStringToInteger( sNum, iLength );
}

int KCalendarSystemGregorianProleptic::monthStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::monthStringToInteger( sNum, iLength );
}

int KCalendarSystemGregorianProleptic::dayStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::dayStringToInteger( sNum, iLength );
}

QString KCalendarSystemGregorianProleptic::formatDate( const QDate &date, KLocale::DateFormat format ) const
{
    return KCalendarSystem::formatDate( date, format );
}

QDate KCalendarSystemGregorianProleptic::readDate( const QString &str, bool *ok ) const
{
    return KCalendarSystem::readDate( str, ok );
}

QDate KCalendarSystemGregorianProleptic::readDate( const QString &intstr, const QString &fmt, bool *ok ) const
{
    return KCalendarSystem::readDate( intstr, fmt, ok );
}

QDate KCalendarSystemGregorianProleptic::readDate( const QString &str, KLocale::ReadDateFlags flags, bool *ok ) const
{
    return KCalendarSystem::readDate( str, flags, ok );
}

int KCalendarSystemGregorianProleptic::weekStartDay() const
{
    return KCalendarSystem::weekStartDay();
}

int KCalendarSystemGregorianProleptic::weekDayOfPray() const
{
    return 7; // sunday
}

bool KCalendarSystemGregorianProleptic::isLunar() const
{
    return false;
}

bool KCalendarSystemGregorianProleptic::isLunisolar() const
{
    return false;
}

bool KCalendarSystemGregorianProleptic::isSolar() const
{
    return true;
}

bool KCalendarSystemGregorianProleptic::isProleptic() const
{
    return true;
}

bool KCalendarSystemGregorianProleptic::julianDayToDate( int jd, int &year, int &month, int &day ) const
{
    // Formula from The Calendar FAQ by Claus Tondering
    // http://www.tondering.dk/claus/cal/node3.html#SECTION003161000000000000000
    // NOTE: Coded from scratch from mathematical formulas, not copied from
    // the Boost licensed source code

    int a = jd + 32044;
    int b = ( ( 4 * a ) + 3 ) / 146097;
    int c = a - ( ( 146097 * b ) / 4 );
    int d = ( ( 4 * c ) + 3 ) / 1461;
    int e = c - ( ( 1461 * d ) / 4 );
    int m = ( ( 5 * e ) + 2 ) / 153;
    day = e - ( ( (153 * m ) + 2 ) / 5 ) + 1;
    month = m + 3 - ( 12 * ( m / 10 ) );
    year = ( 100 * b ) + d - 4800 + ( m / 10 );

    // If year is -ve then is BC.  In Gregorian there is no year 0, but the maths
    // is easier if we pretend there is, so internally year of 0 = 1BC = -1 outside
    if ( year < 1 ) {
        year = year - 1;
    }

    return true;
}

bool KCalendarSystemGregorianProleptic::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    // Formula from The Calendar FAQ by Claus Tondering
    // http://www.tondering.dk/claus/cal/node3.html#SECTION003161000000000000000
    // NOTE: Coded from scratch from mathematical formulas, not copied from
    // the Boost licensed source code

    // If year is -ve then is BC.  In Gregorian there is no year 0, but the maths
    // is easier if we pretend there is, so internally year of -1 = 1BC = 0 internally
    int y;
    if ( year < 1 ) {
        y = year + 1;
    } else {
        y = year;
    }

    int a = ( 14 - month ) / 12;
    y = y + 4800 - a;
    int m = month + ( 12 * a ) - 3;

    jd = day
         + ( ( ( 153 * m ) + 2 ) / 5 )
         + ( 365 * y )
         + ( y / 4 )
         - ( y / 100 )
         + ( y / 400 )
         - 32045;

    return true;
}
