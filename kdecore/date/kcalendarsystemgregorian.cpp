/*
    Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
    Copyright (c) 2002-2003 Hans Petter Bieker <bieker@kde.org>
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

// Derived gregorian kde calendar class
// Just a schema.

#include "kcalendarsystemgregorian.h"

#include "kdebug.h"
#include "klocale.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

KCalendarSystemGregorian::KCalendarSystemGregorian( const KLocale * locale )
                         : KCalendarSystem( locale ), d( 0 )
{
}

KCalendarSystemGregorian::~KCalendarSystemGregorian()
{
}

QString KCalendarSystemGregorian::calendarType() const
{
    return QLatin1String( "gregorian" );
}

QDate KCalendarSystemGregorian::epoch() const
{
    return QDate::fromJulianDay( 1721426 );
}

QDate KCalendarSystemGregorian::earliestValidDate() const
{
    return QDate::fromJulianDay( 1 );
}

QDate KCalendarSystemGregorian::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widets support > 9999
    // In Gregorian this is 9999-12-31, which is  is jd 5373484
    // Can't call setDate( 9999, 12, 31 ) as it creates circular reference!
    return QDate::fromJulianDay( 5373484 );
}

bool KCalendarSystemGregorian::isValid( int year, int month, int day ) const
{
    return QDate::isValid( year, month, day );
}

bool KCalendarSystemGregorian::isValid( const QDate &date ) const
{
    if ( date.isValid() && date >= earliestValidDate() && date <= latestValidDate() ) {
        return true;
    }

    return false;
}

bool KCalendarSystemGregorian::setDate( QDate &date, int year, int month, int day ) const
{
    return date.setDate( year, month, day );
}

QString KCalendarSystemGregorian::yearString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::yearString( pDate, format );
}

QString KCalendarSystemGregorian::monthString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::monthString( pDate, format );
}

int KCalendarSystemGregorian::monthStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::monthStringToInteger( sNum, iLength );
}

QString KCalendarSystemGregorian::dayString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::dayString( pDate, format );
}

int KCalendarSystemGregorian::dayStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::dayStringToInteger( sNum, iLength );
}

bool KCalendarSystemGregorian::isLeapYear( int year ) const
{
    return QDate::isLeapYear( year );
}

bool KCalendarSystemGregorian::isLeapYear( const QDate &date ) const
{
    return QDate::isLeapYear( date.year() );
}

QString KCalendarSystemGregorian::formatDate( const QDate &date, KLocale::DateFormat format ) const
{
    return KCalendarSystem::formatDate( date, format );
}

QDate KCalendarSystemGregorian::readDate( const QString &str, bool *ok ) const
{
    return KCalendarSystem::readDate( str, ok );
}

QDate KCalendarSystemGregorian::readDate( const QString &intstr, const QString &fmt, bool *ok ) const
{
    return KCalendarSystem::readDate( intstr, fmt, ok );
}

QDate KCalendarSystemGregorian::readDate( const QString &str, KLocale::ReadDateFlags flags, bool *ok ) const
{
    return KCalendarSystem::readDate( str, flags, ok );
}

int KCalendarSystemGregorian::daysInWeek( const QDate &date ) const
{
    Q_UNUSED( date );
    return 7;
}

int KCalendarSystemGregorian::weekStartDay() const
{
    return KCalendarSystem::weekStartDay();
}

bool KCalendarSystemGregorian::isProleptic() const
{
    return true;
}

int KCalendarSystemGregorian::weeksInYear( const QDate &date ) const
{
    return KCalendarSystem::weeksInYear( date );
}

int KCalendarSystemGregorian::year( const QDate &date ) const
{
    return date.year();
}

int KCalendarSystemGregorian::monthsInYear( const QDate &date ) const
{
    Q_UNUSED( date )

    return 12;
}

int KCalendarSystemGregorian::weeksInYear( int year ) const
{
    QDate temp;
    temp.setYMD( year, 12, 31 );

    // If the last day of the year is in the first week, we have to check the
    // week before
    if ( temp.weekNumber() == 1 ) {
        temp = temp.addDays( -7 );
    }

    return temp.weekNumber();
}

int KCalendarSystemGregorian::weekNumber( const QDate &date, int * yearNum ) const
{
    return date.weekNumber( yearNum );
}

QString KCalendarSystemGregorian::monthName( int month, int year, MonthNameFormat format ) const
{
    Q_UNUSED( year );

    if ( format == ShortNamePossessive ) {
        switch ( month ) {
        case 1:
            return ki18nc( "of January", "of Jan" ).toString( locale() );
        case 2:
            return ki18nc( "of February", "of Feb" ).toString( locale() );
        case 3:
            return ki18nc( "of March", "of Mar" ).toString( locale() );
        case 4:
            return ki18nc( "of April", "of Apr" ).toString( locale() );
        case 5:
            return ki18nc( "of May short", "of May" ).toString( locale() );
        case 6:
            return ki18nc( "of June", "of Jun" ).toString( locale() );
        case 7:
            return ki18nc( "of July", "of Jul" ).toString( locale() );
        case 8:
            return ki18nc( "of August", "of Aug" ).toString( locale() );
        case 9:
            return ki18nc( "of September", "of Sep" ).toString( locale() );
        case 10:
            return ki18nc( "of October", "of Oct" ).toString( locale() );
        case 11:
            return ki18nc( "of November", "of Nov" ).toString( locale() );
        case 12:
            return ki18nc( "of December", "of Dec" ).toString( locale() );
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
        return ki18n( "March" ).toString( locale() );
    case 4:
        return ki18n( "April" ).toString( locale() );
    case 5:
        return ki18nc( "May long", "May" ).toString( locale() );
    case 6:
        return ki18n( "June" ).toString( locale() );
    case 7:
        return ki18n( "July" ).toString( locale() );
    case 8:
        return ki18n( "August" ).toString( locale() );
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

QString KCalendarSystemGregorian::monthName( const QDate &date, MonthNameFormat format ) const
{
    return monthName( month( date ), year( date ), format );
}


// Deprecated
bool KCalendarSystemGregorian::setYMD( QDate &date, int y, int m, int d ) const
{
    return date.setDate( y, m, d );
}

QDate KCalendarSystemGregorian::addYears( const QDate &date, int nyears ) const
{
    return date.addYears( nyears );
}

QDate KCalendarSystemGregorian::addMonths( const QDate &date, int nmonths ) const
{
    return date.addMonths( nmonths );
}

QDate KCalendarSystemGregorian::addDays( const QDate &date, int ndays ) const
{
    return date.addDays( ndays );
}

QString KCalendarSystemGregorian::weekDayName( int weekDay, WeekDayNameFormat format ) const
{
    if ( format == ShortDayName ) {
        switch ( weekDay ) {
        case 1:  return ki18nc( "Monday", "Mon" ).toString( locale() );
        case 2:  return ki18nc( "Tuesday", "Tue" ).toString( locale() );
        case 3:  return ki18nc( "Wednesday", "Wed" ).toString( locale() );
        case 4:  return ki18nc( "Thursday", "Thu" ).toString( locale() );
        case 5:  return ki18nc( "Friday", "Fri" ).toString( locale() );
        case 6:  return ki18nc( "Saturday", "Sat" ).toString( locale() );
        case 7:  return ki18nc( "Sunday", "Sun" ).toString( locale() );
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

QString KCalendarSystemGregorian::weekDayName( const QDate &date, WeekDayNameFormat format ) const
{
    return weekDayName( dayOfWeek( date ), format );
}

int KCalendarSystemGregorian::dayOfWeek( const QDate &date ) const
{
    return date.dayOfWeek();
}

int KCalendarSystemGregorian::dayOfYear( const QDate &date ) const
{
    return date.dayOfYear();
}

int KCalendarSystemGregorian::daysInMonth( const QDate &date ) const
{
    return date.daysInMonth();
}

int KCalendarSystemGregorian::day( const QDate &date ) const
{
    return date.day();
}

int KCalendarSystemGregorian::month( const QDate &date ) const
{
    return date.month();
}

int KCalendarSystemGregorian::daysInYear( const QDate &date ) const
{
    return date.daysInYear();
}

int KCalendarSystemGregorian::weekDayOfPray() const
{
    return 7; // sunday
}

bool KCalendarSystemGregorian::isLunar() const
{
    return false;
}

bool KCalendarSystemGregorian::isLunisolar() const
{
    return false;
}

bool KCalendarSystemGregorian::isSolar() const
{
    return true;
}

int KCalendarSystemGregorian::yearStringToInteger( const QString &sNum, int &iLength ) const
{
    int iYear;
    iYear = KCalendarSystem::yearStringToInteger( sNum, iLength );

    // Qt treats a year in the range 0-100 as 1900-1999.
    // It is nicer for the user if we treat 0-68 as 2000-2068
    if ( iYear < 69 ) {
        iYear += 2000;
    } else if ( iYear < 100 ) {
        iYear += 1900;
    }

    return iYear;
}

bool KCalendarSystemGregorian::julianDayToDate( int jd, int &year, int &month, int &day ) const
{
    QDate date = QDate::fromJulianDay( jd );

    if ( date.isValid() ) {
        year = date.year();
        month = date.month();
        day = date.day();
    }

    return date.isValid();
}

bool KCalendarSystemGregorian::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    QDate date;

    if ( date.setDate( year, month, day ) ) {
        jd = date.toJulianDay();
        return true;
    }

    return false;
}
