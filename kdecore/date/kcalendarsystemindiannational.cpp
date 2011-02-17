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

#include "kcalendarsystemindiannational_p.h"
#include "kcalendarsystemprivate_p.h"

#include "kdebug.h"
#include "klocale.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

class KCalendarSystemIndianNationalPrivate : public KCalendarSystemPrivate
{
public:
    explicit KCalendarSystemIndianNationalPrivate( KCalendarSystemIndianNational *q );

    virtual ~KCalendarSystemIndianNationalPrivate();

    // Virtual methods each calendar system must re-implement
    virtual KLocale::CalendarSystem calendarSystem() const;
    virtual void loadDefaultEraList();
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
};

// Shared d pointer base class definitions

KCalendarSystemIndianNationalPrivate::KCalendarSystemIndianNationalPrivate( KCalendarSystemIndianNational *q )
                                     :KCalendarSystemPrivate( q )
{
}

KCalendarSystemIndianNationalPrivate::~KCalendarSystemIndianNationalPrivate()
{
}

KLocale::CalendarSystem KCalendarSystemIndianNationalPrivate::calendarSystem() const
{
    return KLocale::IndianNationalCalendar;
}

void KCalendarSystemIndianNationalPrivate::loadDefaultEraList()
{
    QString name, shortName, format;
    // Saka Era
    name = i18nc( "Calendar Era: Indian National Saka Era, years > 0, LongFormat", "Saka Era" );
    shortName = i18nc( "Calendar Era: Indian National Saka Era, years > 0, ShortFormat", "SE" );
    format = i18nc( "(kdedt-format) Indian National, SE, full era year format used for %EY, e.g. 2000 SE", "%Ey %EC" );
    addEra( '+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format );
}

int KCalendarSystemIndianNationalPrivate::monthsInYear( int year ) const
{
    Q_UNUSED( year )
    return 12;
}

int KCalendarSystemIndianNationalPrivate::daysInMonth( int year, int month ) const
{
    if ( month == 1 ) {
        if ( isLeapYear( year ) ) {
            return 31;
        } else {
            return 30;
        }
    }

    if ( month >= 2 && month <= 6  ) {
        return 31;
    }

    return 30;
}

int KCalendarSystemIndianNationalPrivate::daysInYear( int year ) const
{
    if ( isLeapYear( year ) ) {
        return 366;
    } else {
        return 365;
    }
}

int KCalendarSystemIndianNationalPrivate::daysInWeek() const
{
    return 7;
}

bool KCalendarSystemIndianNationalPrivate::isLeapYear( int year ) const
{
    //Uses same rule as Gregorian, and is explicitly synchronized to Gregorian
    //so add 78 years to get Gregorian year and apply Gregorian calculation
    year = year + 78;
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

bool KCalendarSystemIndianNationalPrivate::hasLeapMonths() const
{
    return false;
}

bool KCalendarSystemIndianNationalPrivate::hasYearZero() const
{
    return true;
}

int KCalendarSystemIndianNationalPrivate::maxDaysInWeek() const
{
    return 7;
}

int KCalendarSystemIndianNationalPrivate::maxMonthsInYear() const
{
    return 12;
}

int KCalendarSystemIndianNationalPrivate::earliestValidYear() const
{
    return 0;
}

int KCalendarSystemIndianNationalPrivate::latestValidYear() const
{
    return 9999;
}


KCalendarSystemIndianNational::KCalendarSystemIndianNational( const KLocale *locale )
                             : KCalendarSystem( *new KCalendarSystemIndianNationalPrivate( this ), KSharedConfig::Ptr(), locale ),
                               dont_use( 0 )
{
    d_ptr->loadConfig( calendarType() );
}

KCalendarSystemIndianNational::KCalendarSystemIndianNational( const KSharedConfig::Ptr config, const KLocale *locale )
                             : KCalendarSystem( *new KCalendarSystemIndianNationalPrivate( this ), config, locale ),
                               dont_use( 0 )
{
    d_ptr->loadConfig( calendarType() );
}

KCalendarSystemIndianNational::KCalendarSystemIndianNational( KCalendarSystemIndianNationalPrivate &dd,
                                                              const KSharedConfig::Ptr config,
                                                              const KLocale *locale )
                             : KCalendarSystem( dd, config, locale ),
                               dont_use( 0 )
{
    d_ptr->loadConfig( calendarType() );
}

KCalendarSystemIndianNational::~KCalendarSystemIndianNational()
{
    delete dont_use;
}

QString KCalendarSystemIndianNational::calendarType() const
{
    return QLatin1String( "indian-national" );
}

QDate KCalendarSystemIndianNational::epoch() const
{
    //0000-01-01, has Year 0.
    //0078-03-22 AD Gregorian / 0078-03-24 AD Julian
    return QDate::fromJulianDay( 1749994 );
}

QDate KCalendarSystemIndianNational::earliestValidDate() const
{
    //0000-01-01, has Year 0.
    //0078-03-22 AD Gregorian / 0078-03-24 AD Julian
    //Don't do proleptic yet, need to check
    return QDate::fromJulianDay( 1749630 );
}

QDate KCalendarSystemIndianNational::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    //9999-12-30
    //10078-03-21 AD Gregorian
    return QDate::fromJulianDay( 5402054 );
}

bool KCalendarSystemIndianNational::isValid( int year, int month, int day ) const
{
    return KCalendarSystem::isValid( year, month, day );
}

bool KCalendarSystemIndianNational::isValid( const QDate &date ) const
{
    return KCalendarSystem::isValid( date );
}

bool KCalendarSystemIndianNational::setDate( QDate &date, int year, int month, int day ) const
{
    return KCalendarSystem::setDate( date, year, month, day );
}

// Deprecated
bool KCalendarSystemIndianNational::setYMD( QDate &date, int y, int m, int d ) const
{
    return KCalendarSystem::setDate( date, y, m, d );
}

int KCalendarSystemIndianNational::year( const QDate &date ) const
{
    return KCalendarSystem::year( date );
}

int KCalendarSystemIndianNational::month( const QDate &date ) const
{
    return KCalendarSystem::month( date );
}

int KCalendarSystemIndianNational::day( const QDate &date ) const
{
    return KCalendarSystem::day( date );
}

QDate KCalendarSystemIndianNational::addYears( const QDate &date, int nyears ) const
{
    return KCalendarSystem::addYears( date, nyears );
}

QDate KCalendarSystemIndianNational::addMonths( const QDate &date, int nmonths ) const
{
    return KCalendarSystem::addMonths( date, nmonths );
}

QDate KCalendarSystemIndianNational::addDays( const QDate &date, int ndays ) const
{
    return KCalendarSystem::addDays( date, ndays );
}

int KCalendarSystemIndianNational::monthsInYear( const QDate &date ) const
{
    return KCalendarSystem::monthsInYear( date );
}

int KCalendarSystemIndianNational::weeksInYear( const QDate &date ) const
{
    return KCalendarSystem::weeksInYear( date );
}

int KCalendarSystemIndianNational::weeksInYear( int year ) const
{
    return KCalendarSystem::weeksInYear( year );
}

int KCalendarSystemIndianNational::daysInYear( const QDate &date ) const
{
    return KCalendarSystem::daysInYear( date );
}

int KCalendarSystemIndianNational::daysInMonth( const QDate &date ) const
{
    return KCalendarSystem::daysInMonth( date );
}

int KCalendarSystemIndianNational::daysInWeek( const QDate &date ) const
{
    return KCalendarSystem::daysInWeek( date );
}

int KCalendarSystemIndianNational::dayOfYear( const QDate &date ) const
{
    return KCalendarSystem::dayOfYear( date );
}

int KCalendarSystemIndianNational::dayOfWeek( const QDate &date ) const
{
    return KCalendarSystem::dayOfWeek( date );
}

int KCalendarSystemIndianNational::weekNumber( const QDate &date, int * yearNum ) const
{
    return KCalendarSystem::weekNumber( date, yearNum );
}

bool KCalendarSystemIndianNational::isLeapYear( int year ) const
{
    return KCalendarSystem::isLeapYear( year );
}

bool KCalendarSystemIndianNational::isLeapYear( const QDate &date ) const
{
    return KCalendarSystem::isLeapYear( date );
}

QString KCalendarSystemIndianNational::monthName( int month, int year, MonthNameFormat format ) const
{
    Q_UNUSED( year );

    if ( format == ShortNamePossessive ) {
        switch ( month ) {
        case 1:
            return ki18nc( "Indian National month 1 - ShortNamePossessive",  "of Cha" ).toString( locale() );
        case 2:
            return ki18nc( "Indian National month 2 - ShortNamePossessive",  "of Vai" ).toString( locale() );
        case 3:
            return ki18nc( "Indian National month 3 - ShortNamePossessive",  "of Jya" ).toString( locale() );
        case 4:
            return ki18nc( "Indian National month 4 - ShortNamePossessive",  "of Āsh" ).toString( locale() );
        case 5:
            return ki18nc( "Indian National month 5 - ShortNamePossessive",  "of Shr" ).toString( locale() );
        case 6:
            return ki18nc( "Indian National month 6 - ShortNamePossessive",  "of Bhā" ).toString( locale() );
        case 7:
            return ki18nc( "Indian National month 7 - ShortNamePossessive",  "of Āsw" ).toString( locale() );
        case 8:
            return ki18nc( "Indian National month 8 - ShortNamePossessive",  "of Kār" ).toString( locale() );
        case 9:
            return ki18nc( "Indian National month 9 - ShortNamePossessive",  "of Agr" ).toString( locale() );
        case 10:
            return ki18nc( "Indian National month 10 - ShortNamePossessive", "of Pau" ).toString( locale() );
        case 11:
            return ki18nc( "Indian National month 11 - ShortNamePossessive", "of Māg" ).toString( locale() );
        case 12:
            return ki18nc( "Indian National month 12 - ShortNamePossessive", "of Phā" ).toString( locale() );
        default:
            return QString();
        }
    }

    if ( format == LongNamePossessive ) {
        switch ( month ) {
        case 1:
            return ki18nc( "Indian National month 1 - LongNamePossessive",  "of Chaitra" ).toString( locale() );
        case 2:
            return ki18nc( "Indian National month 2 - LongNamePossessive",  "of Vaishākh" ).toString( locale() );
        case 3:
            return ki18nc( "Indian National month 3 - LongNamePossessive",  "of Jyaishtha" ).toString( locale() );
        case 4:
            return ki18nc( "Indian National month 4 - LongNamePossessive",  "of Āshādha" ).toString( locale() );
        case 5:
            return ki18nc( "Indian National month 5 - LongNamePossessive",  "of Shrāvana" ).toString( locale() );
        case 6:
            return ki18nc( "Indian National month 6 - LongNamePossessive",  "of Bhādrapad" ).toString( locale() );
        case 7:
            return ki18nc( "Indian National month 7 - LongNamePossessive",  "of Āshwin" ).toString( locale() );
        case 8:
            return ki18nc( "Indian National month 8 - LongNamePossessive",  "of Kārtik" ).toString( locale() );
        case 9:
            return ki18nc( "Indian National month 9 - LongNamePossessive",  "of Agrahayana" ).toString( locale() );
        case 10:
            return ki18nc( "Indian National month 10 - LongNamePossessive", "of Paush" ).toString( locale() );
        case 11:
            return ki18nc( "Indian National month 11 - LongNamePossessive", "of Māgh" ).toString( locale() );
        case 12:
            return ki18nc( "Indian National month 12 - LongNamePossessive", "of Phālgun" ).toString( locale() );
        default:
            return QString();
        }
    }

    if ( format == ShortName ) {
        switch ( month ) {
        case 1:
            return ki18nc( "Indian National month 1 - ShortName",  "Cha" ).toString( locale() );
        case 2:
            return ki18nc( "Indian National month 2 - ShortName",  "Vai" ).toString( locale() );
        case 3:
            return ki18nc( "Indian National month 3 - ShortName",  "Jya" ).toString( locale() );
        case 4:
            return ki18nc( "Indian National month 4 - ShortName",  "Āsh" ).toString( locale() );
        case 5:
            return ki18nc( "Indian National month 5 - ShortName",  "Shr" ).toString( locale() );
        case 6:
            return ki18nc( "Indian National month 6 - ShortName",  "Bhā" ).toString( locale() );
        case 7:
            return ki18nc( "Indian National month 7 - ShortName",  "Āsw" ).toString( locale() );
        case 8:
            return ki18nc( "Indian National month 8 - ShortName",  "Kār" ).toString( locale() );
        case 9:
            return ki18nc( "Indian National month 9 - ShortName",  "Agr" ).toString( locale() );
        case 10:
            return ki18nc( "Indian National month 10 - ShortName", "Pau" ).toString( locale() );
        case 11:
            return ki18nc( "Indian National month 11 - ShortName", "Māg" ).toString( locale() );
        case 12:
            return ki18nc( "Indian National month 12 - ShortName", "Phā" ).toString( locale() );
        default:
            return QString();
        }
    }

    // Default to LongName
    switch ( month ) {
    case 1:
        return ki18nc( "Indian National month 1 - LongName",  "Chaitra" ).toString( locale() );
    case 2:
        return ki18nc( "Indian National month 2 - LongName",  "Vaishākh" ).toString( locale() );
    case 3:
        return ki18nc( "Indian National month 3 - LongName",  "Jyaishtha" ).toString( locale() );
    case 4:
        return ki18nc( "Indian National month 4 - LongName",  "Āshādha" ).toString( locale() );
    case 5:
        return ki18nc( "Indian National month 5 - LongName",  "Shrāvana" ).toString( locale() );
    case 6:
        return ki18nc( "Indian National month 6 - LongName",  "Bhādrapad" ).toString( locale() );
    case 7:
        return ki18nc( "Indian National month 7 - LongName",  "Āshwin" ).toString( locale() );
    case 8:
        return ki18nc( "Indian National month 8 - LongName",  "Kārtik" ).toString( locale() );
    case 9:
        return ki18nc( "Indian National month 9 - LongName",  "Agrahayana" ).toString( locale() );
    case 10:
        return ki18nc( "Indian National month 10 - LongName", "Paush" ).toString( locale() );
    case 11:
        return ki18nc( "Indian National month 11 - LongName", "Māgh" ).toString( locale() );
    case 12:
        return ki18nc( "Indian National month 12 - LongName", "Phālgun" ).toString( locale() );
    default:
        return QString();
    }
}

QString KCalendarSystemIndianNational::monthName( const QDate &date, MonthNameFormat format ) const
{
    return KCalendarSystem::monthName( date, format );
}

QString KCalendarSystemIndianNational::weekDayName( int weekDay, WeekDayNameFormat format ) const
{
    if ( format == ShortDayName ) {
        switch ( weekDay ) {
        case 1:  return ki18nc( "Indian National weekday 1 - ShortDayName", "Som" ).toString( locale() );
        case 2:  return ki18nc( "Indian National weekday 2 - ShortDayName", "Mañ" ).toString( locale() );
        case 3:  return ki18nc( "Indian National weekday 3 - ShortDayName", "Bud" ).toString( locale() );
        case 4:  return ki18nc( "Indian National weekday 4 - ShortDayName", "Gur" ).toString( locale() );
        case 5:  return ki18nc( "Indian National weekday 5 - ShortDayName", "Suk" ).toString( locale() );
        case 6:  return ki18nc( "Indian National weekday 6 - ShortDayName", "San" ).toString( locale() );
        case 7:  return ki18nc( "Indian National weekday 7 - ShortDayName", "Rav" ).toString( locale() );
        default: return QString();
        }
    }

    switch ( weekDay ) {
    case 1:  return ki18nc( "Indian National weekday 1 - LongDayName", "Somavãra" ).toString( locale() );
    case 2:  return ki18nc( "Indian National weekday 2 - LongDayName", "Mañgalvã" ).toString( locale() );
    case 3:  return ki18nc( "Indian National weekday 3 - LongDayName", "Budhavãra" ).toString( locale() );
    case 4:  return ki18nc( "Indian National weekday 4 - LongDayName", "Guruvãra" ).toString( locale() );
    case 5:  return ki18nc( "Indian National weekday 5 - LongDayName", "Sukravãra" ).toString( locale() );
    case 6:  return ki18nc( "Indian National weekday 6 - LongDayName", "Sanivãra" ).toString( locale() );
    case 7:  return ki18nc( "Indian National weekday 7 - LongDayName", "Raviãra" ).toString( locale() );
    default: return QString();
    }
}

QString KCalendarSystemIndianNational::weekDayName( const QDate &date, WeekDayNameFormat format ) const
{
    return KCalendarSystem::weekDayName( date, format );
}

QString KCalendarSystemIndianNational::yearString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::yearString( pDate, format );
}

QString KCalendarSystemIndianNational::monthString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::monthString( pDate, format );
}

QString KCalendarSystemIndianNational::dayString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::dayString( pDate, format );
}

int KCalendarSystemIndianNational::yearStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::yearStringToInteger( sNum, iLength );
}

int KCalendarSystemIndianNational::monthStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::monthStringToInteger( sNum, iLength );
}

int KCalendarSystemIndianNational::dayStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::dayStringToInteger( sNum, iLength );
}

QString KCalendarSystemIndianNational::formatDate( const QDate &date, KLocale::DateFormat format ) const
{
    return KCalendarSystem::formatDate( date, format );
}

QDate KCalendarSystemIndianNational::readDate( const QString &str, bool *ok ) const
{
    return KCalendarSystem::readDate( str, ok );
}

QDate KCalendarSystemIndianNational::readDate( const QString &intstr, const QString &fmt, bool *ok ) const
{
    return KCalendarSystem::readDate( intstr, fmt, ok );
}

QDate KCalendarSystemIndianNational::readDate( const QString &str, KLocale::ReadDateFlags flags, bool *ok ) const
{
    return KCalendarSystem::readDate( str, flags, ok );
}

int KCalendarSystemIndianNational::weekStartDay() const
{
    return KCalendarSystem::weekStartDay();
}

int KCalendarSystemIndianNational::weekDayOfPray() const
{
    return 7; // JPL ???
}

bool KCalendarSystemIndianNational::isLunar() const
{
    return false;
}

bool KCalendarSystemIndianNational::isLunisolar() const
{
    return true;
}

bool KCalendarSystemIndianNational::isSolar() const
{
    return false;
}

bool KCalendarSystemIndianNational::isProleptic() const
{
    return false;
}

bool KCalendarSystemIndianNational::julianDayToDate( int jd, int &year, int &month, int &day ) const
{
    int L, N, I, J, D, M, Y;

    // "Explanatory Supplement to the Astronomical Almanac" 2006 section 12.94 pp 605-606
    // Originally from "Report of the Calendar Reform Committee" 1955
    L = jd + 68518;
    N = ( 4 * L ) / 146097;
    L = L - ( 146097 * N + 3 ) / 4;
    I = ( 4000 * ( L + 1 ) ) / 1461001;
    L = L - ( 1461 * I ) / 4 + 1;
    J = ( ( L - 1 ) / 31 ) * ( 1 - L / 185 ) + ( L / 185 ) * ( ( L - 156 ) / 30 + 5 ) - L / 366;
    D = L - 31 * J + ( ( J + 2 ) / 8 ) * ( J - 5 );
    L = J / 11;
    M = J + 2 - 12 * L;
    Y = 100 * ( N - 49 ) + L + I - 78;

    day = D;
    month = M;
    year = Y;

    return true;
}

bool KCalendarSystemIndianNational::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    int Y = year;
    int M = month;
    int D = day;

    // "Explanatory Supplement to the Astronomical Almanac" 2006 section 12.94 pp 605-606
    // Originally from "Report of the Calendar Reform Committee" 1955
    jd = 365 * Y
         + ( Y + 78 - 1 / M ) / 4
         + 31 * M
         - ( M + 9 ) / 11
         - ( M / 7 ) * ( M - 7 )
         - ( 3 * ( ( Y  + 78 - 1 / M ) / 100 + 1 ) ) / 4
         + D
         + 1749579;

    return true;
}
