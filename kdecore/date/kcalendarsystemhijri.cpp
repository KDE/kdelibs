/*
    Copyright (c) 2002-2003 Carlos Moro <cfmoro@correo.uniovi.es>
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

// Derived hijri kde calendar class

#include "kcalendarsystemhijri.h"

#include "kdebug.h"
#include "klocale.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>


//===========================================================================
//  This section holds the old Hijri <=> jd <=> Hijri conversion code
//  This is here to allow comparison testing to confirm old and new match
//===========================================================================

/*
    The following C++ code is translated from the Lisp code
    in ``Calendrical Calculations'' by Nachum Dershowitz and
    Edward M. Reingold, Software---Practice & Experience,
    vol. 20, no. 9 (September, 1990), pp. 899--928.
 
    This code is in the public domain, but any use of it
    should publically acknowledge its source.
 
    Classes GregorianDate, IslamicDate
 */

static int lastDayOfGregorianMonth( int month, int year )
{
    // Compute the last date of the month for the Gregorian calendar.

    switch ( month ) {
    case 2:
        if ( ( ( ( year % 4 ) == 0 ) && ( ( year % 100 ) != 0 ) )
                || ( ( year % 400 ) == 0 ) ) {
            return 29;
        } else {
            return 28;
        }
    case 4:
    case 6:
    case 9:
    case 11: return 30;
    default: return 31;
    }
}

class GregorianDate
{
private:
    int year;   // 1...
    int month;  // 1 == January, ..., 12 == December
    int day;    // 1..lastDayOfGregorianMonth(month, year)

public:
    GregorianDate( int m, int d, int y )
    {
        month = m; day = d; year = y;
    }

    // Computes the Gregorian date from the absolute date.
    GregorianDate( int d )
    {
        // Search forward year by year from approximate year
        year = d / 366;
        while ( d >= GregorianDate( 1, 1, year + 1 ) ) {
            year++;
        }
        // Search forward month by month from January
        month = 1;
        while ( d > GregorianDate( month, lastDayOfGregorianMonth( month, year ), year ) ) {
            month++;
        }
        day = d - GregorianDate( month, 1, year ) + 1;
    }

    // Computes the absolute date from the Gregorian date.
    operator int()
    {
        int N = day;           // days this month
        for ( int m = month - 1;  m > 0; m-- ) // days in prior months this year
            N = N + lastDayOfGregorianMonth( m, year );
        return
            ( N                    // days this year
              + 365 * ( year - 1 )   // days in previous years ignoring leap days
              + ( year - 1 ) / 4       // Julian leap days before this year...
              - ( year - 1 ) / 100     // ...minus prior century years...
              + ( year - 1 ) / 400 );   // ...plus prior years divisible by 400
    }

    int getMonth()
    {
        return month;
    }

    int getDay()
    {
        return day;
    }

    int getYear()
    {
        return year;
    }

};

static int IslamicLeapYear( int year )
{
// True if year is an Islamic leap year

    if ( ( ( ( 11 * year ) + 14 ) % 30 ) < 11 ) {
        return 1;
    } else {
        return 0;
    }
}

static const int IslamicEpoch = 227014; // Absolute date of start of
// Islamic calendar

static int lastDayOfIslamicMonth( int month, int year )
{
// Last day in month during year on the Islamic calendar.

    if ( ( ( month % 2 ) == 1 ) || ( ( month == 12 ) && IslamicLeapYear( year ) ) ) {
        return 30;
    } else {
        return 29;
    }
}

class IslamicDate
{
private:
    int year;   // 1...
    int month;  // 1..13 (12 in a common year)
    int day;    // 1..lastDayOfIslamicMonth(month,year)

public:
    IslamicDate( int m, int d, int y )
    {
        month = m; day = d; year = y;
    }

    // Computes the Islamic date from the absolute date.
    IslamicDate( int d )
    {
        if ( d <= IslamicEpoch ) { // Date is pre-Islamic
            month = 0;
            day = 0;
            year = 0;
        } else {
            // Search forward year by year from approximate year
            year = ( d - IslamicEpoch ) / 355;
            while ( d >= IslamicDate( 1, 1, year + 1 ) ) {
                year++;
            }
            // Search forward month by month from Muharram
            month = 1;
            while ( d > IslamicDate( month, lastDayOfIslamicMonth( month, year ), year ) ) {
                month++;
            }
            day = d - IslamicDate( month, 1, year ) + 1;
        }
    }

    // Computes the absolute date from the Islamic date.
    operator int()
    {
        return ( day                          // days so far this month
                 + 29 * ( month - 1 )         // days so far...
                 + month / 2                  //            ...this year
                 + 354 * ( year - 1 )         // non-leap days in prior years
                 + ( 3 + ( 11 * year ) ) / 30 // leap days in prior years
                 + IslamicEpoch );            // days before start of calendar
    }

    int getMonth()
    {
        return month;
    }

    int getDay()
    {
        return day;
    }

    int getYear()
    {
        return year;
    }

};

static void gregorianToHijri( const QDate & date, int *pYear, int *pMonth, int * pDay )
{
    GregorianDate gregorian( date.month(), date.day(), date.year() );
    int absolute = gregorian;

    IslamicDate islamic( absolute );

    if ( pYear ) {
        * pYear = islamic.getYear();
    }

    if ( pMonth ) {
        * pMonth = islamic.getMonth();
    }

    if ( pDay ) {
        * pDay = islamic.getDay();
    }
}

//===========================================================================
//  End of old code
//===========================================================================

KCalendarSystemHijri::KCalendarSystemHijri( const KLocale * locale )
                     : KCalendarSystem( locale ), d( 0 )
{
}

KCalendarSystemHijri::~KCalendarSystemHijri()
{
}

QString KCalendarSystemHijri::calendarType() const
{
    return QLatin1String( "hijri" );
}

QDate KCalendarSystemHijri::epoch() const
{
    // 16 July 622 in the Julian calendar
    return QDate::fromJulianDay( 1948440 );
}

QDate KCalendarSystemHijri::earliestValidDate() const
{
    return KCalendarSystem::earliestValidDate();
}

QDate KCalendarSystemHijri::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    // Last day of Hijri year 9999 is 9999-12-29
    // Which in Gregorian is 10323-10-21
    // Which is jd xxxx FIXME Find out jd and use that instead
    // Can't call setDate( 9999, 12, 29 ) as it creates circular reference!
    return QDate( 10323, 10, 21 );
}

bool KCalendarSystemHijri::isValid( int y, int month, int day ) const
{
    // taken from setYMD below, adapted to use new methods
    if ( y < year( earliestValidDate() ) || y > year( latestValidDate() ) ) {
        return false;
    }

    if ( month < 1 || month > 12 ) {  // FIXME use monthsInYear
        return false;
    }

    if ( day < 1 || day > lastDayOfIslamicMonth( month, y ) ) {
        return false;
    }

    return true;
}

bool KCalendarSystemHijri::isValid( const QDate &date ) const
{
    return KCalendarSystem::isValid( date );
}

bool KCalendarSystemHijri::setDate( QDate &date, int year, int month, int day ) const
{
    return KCalendarSystem::setDate( date, year, month, day );
}

// Deprecated
bool KCalendarSystemHijri::setYMD( QDate &date, int y, int m, int d ) const
{
    // range checks
    // Removed deleted minValidYear and maxValidYear methods
    // Still use minimum of 1753 gregorain for now due to QDate using Julian calendar before then
    // Later change to following once new methods validated
    // if ( y < year( earliestValidDate() ) || y > year( latestValidDate() ) )
    if ( y < year( QDate( 1753, 1, 1 ) ) || y > 9999 ) {
        return false;
    }

    if ( m < 1 || m > 12 ) {
        return false;
    }

    if ( d < 1 || d > lastDayOfIslamicMonth( m, y ) ) {
        return false;
    }

    IslamicDate islamic ( m, d, y );
    int absolute = islamic;
    GregorianDate gregorian( absolute );

    return date.setYMD( gregorian.getYear(), gregorian.getMonth(), gregorian.getDay() );
}

int KCalendarSystemHijri::year( const QDate &date ) const
{
    int y;

    gregorianToHijri( date, &y, 0, 0 );

    return y;
}

int KCalendarSystemHijri::month( const QDate &date ) const
{
    int m;
    gregorianToHijri( date, 0, &m, 0 );
    return m;
}

int KCalendarSystemHijri::day( const QDate &date ) const
{
    int d;

    gregorianToHijri( date, 0, 0, &d );

    return d;
}

QDate KCalendarSystemHijri::addYears( const QDate &date, int nyears ) const
{
    QDate result = date;

    int y = year( date ) + nyears;
    setYMD( result, y, month( date ), day( date ) );

    return result;
}

QDate KCalendarSystemHijri::addMonths( const QDate &date, int nmonths ) const
{
    QDate result = date;
    int m = month( date );
    int y = year( date );

    if ( nmonths < 0 ) {
        m += 12;
        y -= 1;
    }

    --m; // this only works if we start counting at zero
    m += nmonths;
    y += m / 12;
    m %= 12;
    ++m;

    setYMD( result, y, m, day( date ) );

    return result;
}

QDate KCalendarSystemHijri::addDays( const QDate &date, int ndays ) const
{
    return date.addDays( ndays );
}

int KCalendarSystemHijri::monthsInYear( const QDate &date ) const
{
    Q_UNUSED( date )

    return 12;
}

int KCalendarSystemHijri::weeksInYear( const QDate &date ) const
{
    return KCalendarSystem::weeksInYear( date );
}

int KCalendarSystemHijri::weeksInYear( int year ) const
{
    QDate temp;
    setYMD( temp, year, 12, lastDayOfIslamicMonth( 12, year ) );

    // If the last day of the year is in the first week, we have to check the
    // week before
    if ( weekNumber( temp ) == 1 ) {
        temp = addDays( temp, -7 );
    }

    return weekNumber( temp );
}

int KCalendarSystemHijri::daysInYear( const QDate &date ) const
{
    QDate first, last;

    setYMD( first, year( date ), 1, 1 );
    setYMD( last, year( date ) + 1, 1, 1 );

    return first.daysTo( last );
}

int KCalendarSystemHijri::daysInMonth( const QDate &date ) const
{
    int y, m;

    gregorianToHijri( date, &y, &m, 0 );

    return lastDayOfIslamicMonth( m, y );
}

int KCalendarSystemHijri::daysInWeek( const QDate &date ) const
{
    return KCalendarSystem::daysInWeek( date );
}

int KCalendarSystemHijri::dayOfYear( const QDate &date ) const
{
    QDate first;

    setYMD( first, year( date ), 1, 1 );

    return first.daysTo( date ) + 1;
}

int KCalendarSystemHijri::dayOfWeek( const QDate &date ) const
{
    return date.dayOfWeek(); // same as gregorian
}

int KCalendarSystemHijri::weekNumber( const QDate &date, int *yearNum ) const
{
    QDate firstDayWeek1, lastDayOfYear;
    int y = year( date );
    int week;
    int weekDay1, dayOfWeek1InYear;

    // let's guess 1st day of 1st week
    setYMD( firstDayWeek1, y, 1, 1 );
    weekDay1 = dayOfWeek( firstDayWeek1 );

    // iso 8601: week 1  is the first containing thursday and week starts on
    // monday
    if ( weekDay1 > 4 ) {
        firstDayWeek1 = addDays( firstDayWeek1 , 7 - weekDay1 + 1 ); // next monday
    }

    dayOfWeek1InYear = dayOfYear( firstDayWeek1 );

    // if our date in prev year's week
    if ( dayOfYear( date ) < dayOfWeek1InYear ) {
        if ( yearNum ) {
            * yearNum = y - 1;
        }
        return weeksInYear( y - 1 );
    }

    // let' check if its last week belongs to next year
    setYMD( lastDayOfYear, y, 12, lastDayOfIslamicMonth( 12, y ) );
    // our date is in last week && 1st week in next year has thursday
    if ( ( dayOfYear( date ) >= daysInYear( date ) - dayOfWeek( lastDayOfYear ) + 1 )
            && dayOfWeek( lastDayOfYear ) < 4 ) {
        if ( yearNum ) {
            * yearNum = y + 1;
        }
        week = 1;
    } else {
        if ( weekDay1 < 5 ) {
            firstDayWeek1 = addDays( firstDayWeek1, - ( weekDay1 - 1 ) );
        }

        week = firstDayWeek1.daysTo( date ) / 7 + 1;
    }

    return week;
}

bool KCalendarSystemHijri::isLeapYear( int year ) const
{
    // Taken from IslamicLeapYear above
    if ( ( ( ( 11 * year ) + 14 ) % 30 ) < 11 ) {
        return true;
    } else {
        return false;
    }
}

bool KCalendarSystemHijri::isLeapYear( const QDate &date ) const
{
    return KCalendarSystem::isLeapYear( year( date ) );
}

QString KCalendarSystemHijri::monthName( int month, int year, MonthNameFormat format ) const
{
    Q_UNUSED( year );

    if ( format == ShortNamePossessive ) {
        switch ( month ) {
        case 1:
            return ki18n( "of Muharram" ).toString( locale() );
        case 2:
            return ki18n( "of Safar" ).toString( locale() );
        case 3:
            return ki18n( "of R. Awal" ).toString( locale() );
        case 4:
            return ki18n( "of R. Thaani" ).toString( locale() );
        case 5:
            return ki18n( "of J. Awal" ).toString( locale() );
        case 6:
            return ki18n( "of J. Thaani" ).toString( locale() );
        case 7:
            return ki18n( "of Rajab" ).toString( locale() );
        case 8:
            return ki18n( "of Sha`ban" ).toString( locale() );
        case 9:
            return ki18n( "of Ramadan" ).toString( locale() );
        case 10:
            return ki18n( "of Shawwal" ).toString( locale() );
        case 11:
            return ki18n( "of Qi`dah" ).toString( locale() );
        case 12:
            return ki18n( "of Hijjah" ).toString( locale() );
        default:
            return QString();
        }
    }

    if ( format == LongNamePossessive ) {
        switch ( month ) {
        case 1:
            return ki18n( "of Muharram" ).toString( locale() );
        case 2:
            return ki18n( "of Safar" ).toString( locale() );
        case 3:
            return ki18n( "of Rabi` al-Awal" ).toString( locale() );
        case 4:
            return ki18n( "of Rabi` al-Thaani" ).toString( locale() );
        case 5:
            return ki18n( "of Jumaada al-Awal" ).toString( locale() );
        case 6:
            return ki18n( "of Jumaada al-Thaani" ).toString( locale() );
        case 7:
            return ki18n( "of Rajab" ).toString( locale() );
        case 8:
            return ki18n( "of Sha`ban" ).toString( locale() );
        case 9:
            return ki18n( "of Ramadan" ).toString( locale() );
        case 10:
            return ki18n( "of Shawwal" ).toString( locale() );
        case 11:
            return ki18n( "of Thu al-Qi`dah" ).toString( locale() );
        case 12:
            return ki18n( "of Thu al-Hijjah" ).toString( locale() );
        default:
            return QString();
        }
    }

    if ( format == ShortName ) {
        switch ( month ) {
        case 1:
            return ki18n( "Muharram" ).toString( locale() );
        case 2:
            return ki18n( "Safar" ).toString( locale() );
        case 3:
            return ki18n( "R. Awal" ).toString( locale() );
        case 4:
            return ki18n( "R. Thaani" ).toString( locale() );
        case 5:
            return ki18n( "J. Awal" ).toString( locale() );
        case 6:
            return ki18n( "J. Thaani" ).toString( locale() );
        case 7:
            return ki18n( "Rajab" ).toString( locale() );
        case 8:
            return ki18n( "Sha`ban" ).toString( locale() );
        case 9:
            return ki18n( "Ramadan" ).toString( locale() );
        case 10:
            return ki18n( "Shawwal" ).toString( locale() );
        case 11:
            return ki18n( "Qi`dah" ).toString( locale() );
        case 12:
            return ki18n( "Hijjah" ).toString( locale() );
        default:
            return QString();
        }
    }

    // LongName
    switch ( month ) {
    case 1:
        return ki18n( "Muharram" ).toString( locale() );
    case 2:
        return ki18n( "Safar" ).toString( locale() );
    case 3:
        return ki18n( "Rabi` al-Awal" ).toString( locale() );
    case 4:
        return ki18n( "Rabi` al-Thaani" ).toString( locale() );
    case 5:
        return ki18n( "Jumaada al-Awal" ).toString( locale() );
    case 6:
        return ki18n( "Jumaada al-Thaani" ).toString( locale() );
    case 7:
        return ki18n( "Rajab" ).toString( locale() );
    case 8:
        return ki18n( "Sha`ban" ).toString( locale() );
    case 9:
        return ki18n( "Ramadan" ).toString( locale() );
    case 10:
        return ki18n( "Shawwal" ).toString( locale() );
    case 11:
        return ki18n( "Thu al-Qi`dah" ).toString( locale() );
    case 12:
        return ki18n( "Thu al-Hijjah" ).toString( locale() );
    default:
        return QString();
    }
}

QString KCalendarSystemHijri::monthName( const QDate &date, MonthNameFormat format ) const
{
    return monthName( month( date ), year( date ), format );
}

QString KCalendarSystemHijri::weekDayName( int weekDay, WeekDayNameFormat format ) const
{
    if ( format == ShortDayName ) {
        switch ( weekDay ) {
        case 1:
            return ki18n( "Ith" ).toString( locale() );
        case 2:
            return ki18n( "Thl" ).toString( locale() );
        case 3:
            return ki18n( "Arb" ).toString( locale() );
        case 4:
            return ki18n( "Kha" ).toString( locale() );
        case 5:
            return ki18n( "Jum" ).toString( locale() );
        case 6:
            return ki18n( "Sab" ).toString( locale() );
        case 7:
            return ki18n( "Ahd" ).toString( locale() );
        default:
            return QString();
        }
    }

    // Default to LongDayName format
    switch ( weekDay ) {
    case 1:
        return ki18n( "Yaum al-Ithnain" ).toString( locale() );
    case 2:
        return ki18n( "Yau al-Thulatha" ).toString( locale() );
    case 3:
        return ki18n( "Yaum al-Arbi'a" ).toString( locale() );
    case 4:
        return ki18n( "Yaum al-Khamees" ).toString( locale() );
    case 5:
        return ki18n( "Yaum al-Jumma" ).toString( locale() );
    case 6:
        return ki18n( "Yaum al-Sabt" ).toString( locale() );
    case 7:
        return ki18n( "Yaum al-Ahad" ).toString( locale() );
    default:
        return QString();
    }
}

QString KCalendarSystemHijri::weekDayName( const QDate &date, WeekDayNameFormat format ) const
{
    return weekDayName( dayOfWeek( date ), format );
}

QString KCalendarSystemHijri::yearString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::yearString( pDate, format );
}

QString KCalendarSystemHijri::monthString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::monthString( pDate, format );
}

QString KCalendarSystemHijri::dayString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::dayString( pDate, format );
}

int KCalendarSystemHijri::yearStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::yearStringToInteger( sNum, iLength );
}

int KCalendarSystemHijri::monthStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::monthStringToInteger( sNum, iLength );
}

int KCalendarSystemHijri::dayStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::dayStringToInteger( sNum, iLength );
}

QString KCalendarSystemHijri::formatDate( const QDate &date, KLocale::DateFormat format ) const
{
    return KCalendarSystem::formatDate( date, format );
}

QDate KCalendarSystemHijri::readDate( const QString &str, bool *ok ) const
{
    return KCalendarSystem::readDate( str, ok );
}

QDate KCalendarSystemHijri::readDate( const QString &intstr, const QString &fmt, bool *ok ) const
{
    return KCalendarSystem::readDate( intstr, fmt, ok );
}

QDate KCalendarSystemHijri::readDate( const QString &str, KLocale::ReadDateFlags flags, bool *ok ) const
{
    return KCalendarSystem::readDate( str, flags, ok );
}

int KCalendarSystemHijri::weekStartDay() const
{
    return KCalendarSystem::weekStartDay();
}

int KCalendarSystemHijri::weekDayOfPray() const
{
    return 5; // friday
}

bool KCalendarSystemHijri::isLunar() const
{
    return true;
}

bool KCalendarSystemHijri::isLunisolar() const
{
    return false;
}

bool KCalendarSystemHijri::isSolar() const
{
    return false;
}

bool KCalendarSystemHijri::isProleptic() const
{
    return false;
}

bool KCalendarSystemHijri::julianDayToDate( int jd, int &year, int &month, int &day ) const
{
    // From IslamicDate above.  Check me!
    if ( jd >= earliestValidDate().toJulianDay() && jd <= latestValidDate().toJulianDay() ) {
        // Search forward year by year from approximate year
        year = ( jd - epoch().toJulianDay() ) / 355;
        while ( jd >= IslamicDate( 1, 1, year + 1 ) ) {
            year++;
        }
        // Search forward month by month from Muharram
        month = 1;
        while ( jd > IslamicDate( month, lastDayOfIslamicMonth( month, year ), year ) ) {
            month++;
        }
        day = jd - IslamicDate( month, 1, year ) + 1;
        return true;
    }

    return false;
}

bool KCalendarSystemHijri::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    // From IslamicDate above.  Check me!
    if ( isValid( year, month, day ) ) {
        jd =  ( day                           // days so far this month
                + 29 * ( month - 1 )          // days so far...
                + month / 2                   //            ...this year
                + 354 * ( year - 1 )          // non-leap days in prior years
                + ( 3 + ( 11 * year ) ) / 30  // leap days in prior years
                + epoch().toJulianDay() );                  // days before start of calendar
        return true;
    }

    return false;
}

