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

// Derived gregorian kde calendar class

#include "kcalendarsystemgregorianproleptic_p.h"
#include "kcalendarsystemgregorianprolepticprivate_p.h"
#include "kcalendarera_p.h"

#include "kdebug.h"
#include "klocale.h"
#include "kglobal.h"
#include "kconfiggroup.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

// Shared d pointer base class definitions

KCalendarSystemGregorianProlepticPrivate::KCalendarSystemGregorianProlepticPrivate( KCalendarSystemGregorianProleptic *q )
                                         :KCalendarSystemPrivate( q )
{
}

KCalendarSystemGregorianProlepticPrivate::~KCalendarSystemGregorianProlepticPrivate()
{
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
void KCalendarSystemGregorianProlepticPrivate::initDefaultEraList()
{
    QString name, shortName, format;

    KConfigGroup cg( config(), QString( "KCalendarSystem %1" ).arg( q->calendarType() ) );
    m_useCommonEra = cg.readEntry( "UseCommonEra", false );

    if ( m_useCommonEra ) {
        name = i18nc( "Calendar Era: Gregorian Common Era, years < 0, LongFormat", "Before Common Era" );
        shortName = i18nc( "Calendar Era: Gregorian Common Era, years < 0, ShortFormat", "BCE" );
    } else {
        name = i18nc( "Calendar Era: Gregorian Christian Era, years < 0, LongFormat", "Before Christ" );
        shortName = i18nc( "Calendar Era: Gregorian Christian Era, years < 0, ShortFormat", "BC" );
    }
    format = i18nc( "(kdedt-format) Gregorian, BC, full era year format used for %EY, e.g. 2000 BC", "%Ey %EC" );
    addEra( '-', 1, q->epoch().addDays( -1 ), -1, q->earliestValidDate(), name, shortName, format );

    if ( m_useCommonEra ) {
        name = i18nc( "Calendar Era: Gregorian Common Era, years > 0, LongFormat", "Common Era" );
        shortName = i18nc( "Calendar Era: Gregorian Common Era, years > 0, ShortFormat", "CE" );
    } else {
        name = i18nc( "Calendar Era: Gregorian Christian Era, years > 0, LongFormat", "Anno Domini" );
        shortName = i18nc( "Calendar Era: Gregorian Christian Era, years > 0, ShortFormat", "AD" );
    }
    format = i18nc( "(kdedt-format) Gregorian, AD, full era year format used for %EY, e.g. 2000 AD", "%Ey %EC" );
    addEra( '+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format );
}

int KCalendarSystemGregorianProlepticPrivate::monthsInYear( int year ) const
{
    Q_UNUSED( year )
    return 12;
}

int KCalendarSystemGregorianProlepticPrivate::daysInMonth( int year, int month ) const
{
    if ( month == 2 ) {
        if ( isLeapYear( year ) ) {
            return 29;
        } else {
            return 28;
        }
    }

    if ( month == 4 || month == 6 || month == 9 || month == 11 ) {
        return 30;
    }

    return 31;
}

int KCalendarSystemGregorianProlepticPrivate::daysInYear( int year ) const
{
    if ( isLeapYear( year ) ) {
        return 366;
    } else {
        return 365;
    }
}

int KCalendarSystemGregorianProlepticPrivate::daysInWeek() const
{
    return 7;
}

bool KCalendarSystemGregorianProlepticPrivate::isLeapYear( int year ) const
{
    if ( !hasYearZero() && year < 1 ) {
        year = year + 1;
    }

    if ( year % 4 == 0 ) {
        if ( year % 100 != 0 ) {
            return true;
        } else if ( year % 400 == 0 ) {
            return true;
        }
    }

    return false;
}

bool KCalendarSystemGregorianProlepticPrivate::hasLeapMonths() const
{
    return false;
}

bool KCalendarSystemGregorianProlepticPrivate::hasYearZero() const
{
    return false;
}

int KCalendarSystemGregorianProlepticPrivate::maxDaysInWeek() const
{
    return 7;
}

int KCalendarSystemGregorianProlepticPrivate::maxMonthsInYear() const
{
    return 12;
}

int KCalendarSystemGregorianProlepticPrivate::earliestValidYear() const
{
    return -4713;
}

int KCalendarSystemGregorianProlepticPrivate::latestValidYear() const
{
    return 9999;
}


KCalendarSystemGregorianProleptic::KCalendarSystemGregorianProleptic( const KLocale *locale )
                                 : KCalendarSystem( *new KCalendarSystemGregorianProlepticPrivate( this ), KSharedConfig::Ptr(), locale ),
                                 dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemGregorianProleptic::KCalendarSystemGregorianProleptic( const KSharedConfig::Ptr config,
                                                                      const KLocale *locale )
                                 : KCalendarSystem( *new KCalendarSystemGregorianProlepticPrivate( this ), config, locale ),
                                   dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemGregorianProleptic::KCalendarSystemGregorianProleptic( KCalendarSystemGregorianProlepticPrivate &dd,
                                                                      const KSharedConfig::Ptr config,
                                                                      const KLocale *locale )
                                 : KCalendarSystem( dd, config, locale ),
                                   dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemGregorianProleptic::~KCalendarSystemGregorianProleptic()
{
    delete dont_use;
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
    // Gregorian 1 Jan 4713 BC, no year zero
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
    return KCalendarSystem::isValid( year, month, day );
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
bool KCalendarSystemGregorianProleptic::setYMD( QDate &date, int year, int month, int day ) const
{
    return KCalendarSystem::setYMD( date, year, month, day );
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
    return KCalendarSystem::monthsInYear( date );
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
    return KCalendarSystem::daysInYear( date );
}

int KCalendarSystemGregorianProleptic::daysInMonth( const QDate &date ) const
{
    return KCalendarSystem::daysInMonth( date );
}

int KCalendarSystemGregorianProleptic::daysInWeek( const QDate &date ) const
{
    return KCalendarSystem::daysInWeek( date );
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
    return KCalendarSystem::isLeapYear( year );
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
    Q_D( const KCalendarSystemGregorianProleptic );

    // Formula from The Calendar FAQ by Claus Tondering
    // http://www.tondering.dk/claus/cal/node3.html#SECTION003161000000000000000
    // NOTE: Coded from scratch from mathematical formulas, not copied from
    // the Boost licensed source code

    int a = jd + 32044;
    int b = ( ( 4 * a ) + 3 ) / 146097;
    int c = a - ( ( 146097 * b ) / 4 );
    int dd = ( ( 4 * c ) + 3 ) / 1461;
    int e = c - ( ( 1461 * dd ) / 4 );
    int m = ( ( 5 * e ) + 2 ) / 153;
    day = e - ( ( (153 * m ) + 2 ) / 5 ) + 1;
    month = m + 3 - ( 12 * ( m / 10 ) );
    year = ( 100 * b ) + dd - 4800 + ( m / 10 );

    // If year is -ve then is BC.  In Gregorian there is no year 0, but the maths
    // is easier if we pretend there is, so internally year of 0 = 1BC = -1 outside
    // Check for Year 0 support as some Gregorian based calendars do have it, e.g. Thai and ISO
    if ( !d->hasYearZero() && year < 1 ) {
        year = year - 1;
    }
    return true;
}

bool KCalendarSystemGregorianProleptic::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    Q_D( const KCalendarSystemGregorianProleptic );

    // Formula from The Calendar FAQ by Claus Tondering
    // http://www.tondering.dk/claus/cal/node3.html#SECTION003161000000000000000
    // NOTE: Coded from scratch from mathematical formulas, not copied from
    // the Boost licensed source code

    // If year is -ve then is BC.  In Gregorian there is no year 0, but the maths
    // is easier if we pretend there is, so internally year of -1 = 1BC = 0 internally
    // Check for Year 0 support as some Gregorian based calendars do have it, e.g. Thai and ISO
    int y;
    if ( !d->hasYearZero() && year < 1 ) {
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
