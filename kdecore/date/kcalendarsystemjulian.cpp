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

#include "kcalendarsystemjulian_p.h"
#include "kcalendarsystemprivate_p.h"

#include "kdebug.h"
#include "klocale.h"
#include "kglobal.h"
#include "kconfiggroup.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

class KCalendarSystemJulianPrivate : public KCalendarSystemPrivate
{
public:
    explicit KCalendarSystemJulianPrivate( KCalendarSystemJulian *q );

    virtual ~KCalendarSystemJulianPrivate();

    // Virtual methods each calendar system must re-implement
    virtual void initDefaultEraList();
    virtual int monthsInYear( int year ) const;
    virtual int daysInMonth( int year, int month ) const;
    virtual int daysInYear( int year ) const;
    virtual int daysInWeek() const;
    virtual bool isLeapYear( int year ) const;
    virtual bool hasLeapMonths() const;
    virtual bool hasYearZero() const;
    virtual int maxDaysInWeek() const;
    virtual int maxMonthsInYear() const;
    virtual int earliestValidYear() const;
    virtual int latestValidYear() const;

    bool m_useCommonEra;
};

// Shared d pointer base class definitions

KCalendarSystemJulianPrivate::KCalendarSystemJulianPrivate( KCalendarSystemJulian *q )
                             :KCalendarSystemPrivate( q )
{
}

KCalendarSystemJulianPrivate::~KCalendarSystemJulianPrivate()
{
}

void KCalendarSystemJulianPrivate::initDefaultEraList()
{
    QString name, shortName, format;

    KConfigGroup cg( config(), QString( "KCalendarSystem %1" ).arg( q->calendarType() ) );
    m_useCommonEra = cg.readEntry( "UseCommonEra", false );

    if ( m_useCommonEra ) {
        name = i18nc( "Calendar Era: Julian Common Era, years < 0, LongFormat", "Before Common Era" );
        shortName = i18nc( "Calendar Era: Julian Common Era, years < 0, ShortFormat", "BCE" );
    } else {
        name = i18nc( "Calendar Era: Julian Christian Era, years < 0, LongFormat", "Before Christ" );
        shortName = i18nc( "Calendar Era: Julian Christian Era, years < 0, ShortFormat", "BC" );
    }
    format = i18nc( "(kdedt-format) Julian, BC, full era year format used for %EY, e.g. 2000 BC", "%Ey %EC" );
    addEra( '-', 1, q->epoch().addDays( -1 ), -1, q->earliestValidDate(), name, shortName, format );

    if ( m_useCommonEra ) {
        name = i18nc( "Calendar Era: Julian Common Era, years > 0, LongFormat", "Common Era" );
        shortName = i18nc( "Calendar Era: Julian Common Era, years > 0, ShortFormat", "CE" );
    } else {
        name = i18nc( "Calendar Era: Julian Christian Era, years > 0, LongFormat", "Anno Domini" );
        shortName = i18nc( "Calendar Era: Julian Christian Era, years > 0, ShortFormat", "AD" );
    }
    format = i18nc( "(kdedt-format) Julian, AD, full era year format used for %EY, e.g. 2000 AD", "%Ey %EC" );
    addEra( '+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format );
}

int KCalendarSystemJulianPrivate::monthsInYear( int year ) const
{
    Q_UNUSED( year )
    return 12;
}

int KCalendarSystemJulianPrivate::daysInMonth( int year, int month ) const
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

int KCalendarSystemJulianPrivate::daysInYear( int year ) const
{
    if ( isLeapYear( year ) ) {
        return 366;
    } else {
        return 365;
    }
}

int KCalendarSystemJulianPrivate::daysInWeek() const
{
    return 7;
}

bool KCalendarSystemJulianPrivate::isLeapYear( int year ) const
{
    if ( year < 1 ) {
        year = year + 1;
    }

    if ( year % 4 == 0 ) {
        return true;
    }

    return false;
}

bool KCalendarSystemJulianPrivate::hasLeapMonths() const
{
    return false;
}

bool KCalendarSystemJulianPrivate::hasYearZero() const
{
    return false;
}

int KCalendarSystemJulianPrivate::maxDaysInWeek() const
{
    return 7;
}

int KCalendarSystemJulianPrivate::maxMonthsInYear() const
{
    return 12;
}

int KCalendarSystemJulianPrivate::earliestValidYear() const
{
    return -4712;
}

int KCalendarSystemJulianPrivate::latestValidYear() const
{
    return 9999;
}


KCalendarSystemJulian::KCalendarSystemJulian( const KLocale *locale )
                     : KCalendarSystem( *new KCalendarSystemJulianPrivate( this ), KSharedConfig::Ptr(), locale ),
                       dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemJulian::KCalendarSystemJulian( const KSharedConfig::Ptr config, const KLocale *locale )
                     : KCalendarSystem( *new KCalendarSystemJulianPrivate( this ), config, locale ),
                       dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemJulian::KCalendarSystemJulian( KCalendarSystemJulianPrivate &dd,
                                              const KSharedConfig::Ptr config, const KLocale *locale )
                     : KCalendarSystem( dd, config, locale ),
                       dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemJulian::~KCalendarSystemJulian()
{
    delete dont_use;
}

QString KCalendarSystemJulian::calendarType() const
{
    return QLatin1String( "julian" );
}

QDate KCalendarSystemJulian::epoch() const
{
    return QDate::fromJulianDay( 1721426 );
}

QDate KCalendarSystemJulian::earliestValidDate() const
{
    // 1 Jan 4712 BC, no year zero, cant be 4713BC due to error in QDate that day 0 is not valid
    // and we really need the first in each year to be valid for the date maths
    return QDate::fromJulianDay( 366 );
}

QDate KCalendarSystemJulian::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    // 31 Dec 9999 AD, no year zero
    return QDate::fromJulianDay( 5373557 );
}

bool KCalendarSystemJulian::isValid( int year, int month, int day ) const
{
    return KCalendarSystem::isValid( year, month, day );
}

bool KCalendarSystemJulian::isValid( const QDate &date ) const
{
    return KCalendarSystem::isValid( date );
}

bool KCalendarSystemJulian::setDate( QDate &date, int year, int month, int day ) const
{
    return KCalendarSystem::setDate( date, year, month, day );
}

// Deprecated
bool KCalendarSystemJulian::setYMD( QDate &date, int y, int m, int d ) const
{
    return KCalendarSystem::setDate( date, y, m, d );
}

int KCalendarSystemJulian::year( const QDate &date ) const
{
    return KCalendarSystem::year( date );
}

int KCalendarSystemJulian::month( const QDate &date ) const
{
    return KCalendarSystem::month( date );
}

int KCalendarSystemJulian::day( const QDate &date ) const
{
    return KCalendarSystem::day( date );
}

QDate KCalendarSystemJulian::addYears( const QDate &date, int nyears ) const
{
    return KCalendarSystem::addYears( date, nyears );
}

QDate KCalendarSystemJulian::addMonths( const QDate &date, int nmonths ) const
{
    return KCalendarSystem::addMonths( date, nmonths );
}

QDate KCalendarSystemJulian::addDays( const QDate &date, int ndays ) const
{
    return KCalendarSystem::addDays( date, ndays );
}

int KCalendarSystemJulian::monthsInYear( const QDate &date ) const
{
    return KCalendarSystem::monthsInYear( date );
}

int KCalendarSystemJulian::weeksInYear( const QDate &date ) const
{
    return KCalendarSystem::weeksInYear( date );
}

int KCalendarSystemJulian::weeksInYear( int year ) const
{
    return KCalendarSystem::weeksInYear( year );
}

int KCalendarSystemJulian::daysInYear( const QDate &date ) const
{
    return KCalendarSystem::daysInYear( date );
}

int KCalendarSystemJulian::daysInMonth( const QDate &date ) const
{
    return KCalendarSystem::daysInMonth( date );
}

int KCalendarSystemJulian::daysInWeek( const QDate &date ) const
{
    return KCalendarSystem::daysInWeek( date );
}

int KCalendarSystemJulian::dayOfYear( const QDate &date ) const
{
    return KCalendarSystem::dayOfYear( date );
}

int KCalendarSystemJulian::dayOfWeek( const QDate &date ) const
{
    return KCalendarSystem::dayOfWeek( date );
}

int KCalendarSystemJulian::weekNumber( const QDate &date, int * yearNum ) const
{
    return KCalendarSystem::weekNumber( date, yearNum );
}

bool KCalendarSystemJulian::isLeapYear( int year ) const
{
    return KCalendarSystem::isLeapYear( year );
}

bool KCalendarSystemJulian::isLeapYear( const QDate &date ) const
{
    return KCalendarSystem::isLeapYear( date );
}

QString KCalendarSystemJulian::monthName( int month, int year, MonthNameFormat format ) const
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

QString KCalendarSystemJulian::monthName( const QDate &date, MonthNameFormat format ) const
{
    return KCalendarSystem::monthName( date, format );
}


QString KCalendarSystemJulian::weekDayName( int weekDay, WeekDayNameFormat format ) const
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

QString KCalendarSystemJulian::weekDayName( const QDate &date, WeekDayNameFormat format ) const
{
    return KCalendarSystem::weekDayName( date, format );
}

QString KCalendarSystemJulian::yearString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::yearString( pDate, format );
}

QString KCalendarSystemJulian::monthString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::monthString( pDate, format );
}

QString KCalendarSystemJulian::dayString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::dayString( pDate, format );
}

int KCalendarSystemJulian::yearStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::yearStringToInteger( sNum, iLength );
}

int KCalendarSystemJulian::monthStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::monthStringToInteger( sNum, iLength );
}

int KCalendarSystemJulian::dayStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::dayStringToInteger( sNum, iLength );
}

QString KCalendarSystemJulian::formatDate( const QDate &date, KLocale::DateFormat format ) const
{
    return KCalendarSystem::formatDate( date, format );
}

QDate KCalendarSystemJulian::readDate( const QString &str, bool *ok ) const
{
    return KCalendarSystem::readDate( str, ok );
}

QDate KCalendarSystemJulian::readDate( const QString &intstr, const QString &fmt, bool *ok ) const
{
    return KCalendarSystem::readDate( intstr, fmt, ok );
}

QDate KCalendarSystemJulian::readDate( const QString &str, KLocale::ReadDateFlags flags, bool *ok ) const
{
    return KCalendarSystem::readDate( str, flags, ok );
}

int KCalendarSystemJulian::weekStartDay() const
{
    return KCalendarSystem::weekStartDay();
}

int KCalendarSystemJulian::weekDayOfPray() const
{
    return 7; // sunday
}

bool KCalendarSystemJulian::isLunar() const
{
    return false;
}

bool KCalendarSystemJulian::isLunisolar() const
{
    return false;
}

bool KCalendarSystemJulian::isSolar() const
{
    return true;
}

bool KCalendarSystemJulian::isProleptic() const
{
    return true;
}

bool KCalendarSystemJulian::julianDayToDate( int jd, int &year, int &month, int &day ) const
{
    // Formula from The Calendar FAQ by Claus Tondering
    // http://www.tondering.dk/claus/cal/node3.html#SECTION003161000000000000000
    // NOTE: Coded from scratch from mathematical formulas, not copied from
    // the Boost licensed source code

    int b = 0;
    int c = jd + 32082;
    int d = ( ( 4 * c ) + 3 ) / 1461;
    int e = c - ( ( 1461 * d ) / 4 );
    int m = ( ( 5 * e ) + 2 ) / 153;
    day = e - ( ( (153 * m ) + 2 ) / 5 ) + 1;
    month = m + 3 - ( 12 * ( m / 10 ) );
    year = ( 100 * b ) + d - 4800 + ( m / 10 );

    // If year is -ve then is BC.  In Julian there is no year 0, but the maths
    // is easier if we pretend there is, so internally year of 0 = 1BC = -1 outside
    if ( year < 1 ) {
        year = year - 1;
    }

    return true;
}

bool KCalendarSystemJulian::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    // Formula from The Calendar FAQ by Claus Tondering
    // http://www.tondering.dk/claus/cal/node3.html#SECTION003161000000000000000
    // NOTE: Coded from scratch from mathematical formulas, not copied from
    // the Boost licensed source code

    // If year is -ve then is BC.  In Julian there is no year 0, but the maths
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
         - 32083;

    return true;
}
