/*
    Copyright (c) 2002-2003 Carlos Moro <cfmoro@correo.uniovi.es>
    Copyright (c) 2002-2003 Hans Petter Bieker <bieker@kde.org>
    Copyright 2007, 2008, 2009, 2010 John Layt <john@layt.net>

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

#include "kcalendarsystemhijri_p.h"
#include "kcalendarsystemprivate_p.h"

#include <QtCore/QDate>

class KCalendarSystemHijriPrivate : public KCalendarSystemPrivate
{
public:
    explicit KCalendarSystemHijriPrivate( KCalendarSystemHijri *q );

    virtual ~KCalendarSystemHijriPrivate();

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
};

// Shared d pointer base class definitions

KCalendarSystemHijriPrivate::KCalendarSystemHijriPrivate( KCalendarSystemHijri *q )
                            :KCalendarSystemPrivate( q )
{
}

KCalendarSystemHijriPrivate::~KCalendarSystemHijriPrivate()
{
}

void KCalendarSystemHijriPrivate::initDefaultEraList()
{
    QString name, shortName, format;
    // Islamic Era, Anno Hegirae, "Year of the Hijra".
    name = i18nc( "Calendar Era: Hijri Islamic Era, years > 0, LongFormat", "Anno Hegirae" );
    shortName = i18nc( "Calendar Era: Hijri Islamic Era, years > 0, ShortFormat", "AH" );
    format = i18nc( "(kdedt-format) Hijri, AH, full era year format used for %EY, e.g. 2000 AH", "%Ey %EC" );
    addEra( '+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format );
}

int KCalendarSystemHijriPrivate::monthsInYear( int year ) const
{
    Q_UNUSED( year )
    return 12;
}

int KCalendarSystemHijriPrivate::daysInMonth( int year, int month ) const
{
    if ( month == 12 && isLeapYear( year ) ) {
        return 30;
    }

    if ( month % 2 == 0 ) { // Even number months have 29 days
        return 29;
    } else {  // Odd number months have 30 days
        return 30;
    }
}

int KCalendarSystemHijriPrivate::daysInYear( int year ) const
{
    if ( isLeapYear( year ) ) {
        return 355;
    } else {
        return 354;
    }
}

int KCalendarSystemHijriPrivate::daysInWeek() const
{
    return 7;
}

bool KCalendarSystemHijriPrivate::isLeapYear( int year ) const
{
    // Years 2, 5, 7, 10, 13, 16, 18, 21, 24, 26, 29 of the 30 year cycle

    /*
    The following C++ code is translated from the Lisp code
    in ``Calendrical Calculations'' by Nachum Dershowitz and
    Edward M. Reingold, Software---Practice & Experience,
    vol. 20, no. 9 (September, 1990), pp. 899--928.

    This code is in the public domain, but any use of it
    should publically acknowledge its source.
    */

    if ( ( ( ( 11 * year ) + 14 ) % 30 ) < 11 ) {
        return true;
    } else {
        return false;
    }

    // The following variations will be implemented in separate classes in 4.5
    // May be cleaner to formally define using a case statement switch on (year % 30)

    // Variation used by Bar Habraeus / Graves / Birashk / Some Microsoft products
    // Years 2, 5, 7, 10, 13, 15, 18, 21, 24, 26, 29 of the 30 year cycle
    // if ( ( ( ( 11 * year ) + 15 ) % 30 ) < 11 ) {

    // Variation used by Bohras / Sahifa with epoch 15 July 622 jd = 1948440
    // Years 2, 5, 8, 10, 13, 16, 19, 21, 24, 27, 29 of the 30 year cycle
    // if ( ( ( ( 11 * year ) + 1 ) % 30 ) < 11 ) {
}

bool KCalendarSystemHijriPrivate::hasLeapMonths() const
{
    return false;
}

bool KCalendarSystemHijriPrivate::hasYearZero() const
{
    return false;
}

int KCalendarSystemHijriPrivate::maxDaysInWeek() const
{
    return 7;
}

int KCalendarSystemHijriPrivate::maxMonthsInYear() const
{
    return 12;
}

int KCalendarSystemHijriPrivate::earliestValidYear() const
{
    return 1;
}

int KCalendarSystemHijriPrivate::latestValidYear() const
{
    return 9999;
}


KCalendarSystemHijri::KCalendarSystemHijri( const KLocale *locale )
                    : KCalendarSystem( *new KCalendarSystemHijriPrivate( this ), KSharedConfig::Ptr(), locale ),
                      dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemHijri::KCalendarSystemHijri( const KSharedConfig::Ptr config, const KLocale *locale )
                    : KCalendarSystem( *new KCalendarSystemHijriPrivate( this ), config, locale ),
                      dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemHijri::KCalendarSystemHijri( KCalendarSystemHijriPrivate &dd,
                                            const KSharedConfig::Ptr config, const KLocale *locale )
                    : KCalendarSystem( dd, config, locale ),
                      dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemHijri::~KCalendarSystemHijri()
{
    delete dont_use;
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
    return epoch();
}

QDate KCalendarSystemHijri::latestValidDate() const
{
    // Set to last day of year 9999
    // Last day of Hijri year 9999 is 9999-12-29
    return QDate::fromJulianDay( 5491751 );
}

bool KCalendarSystemHijri::isValid( int year, int month, int day ) const
{
    return KCalendarSystem::isValid( year, month, day );
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
bool KCalendarSystemHijri::setYMD( QDate &date, int year, int month, int day ) const
{
    return KCalendarSystem::setYMD( date, year, month, day );
}

int KCalendarSystemHijri::year( const QDate &date ) const
{
    return KCalendarSystem::year( date );
}

int KCalendarSystemHijri::month( const QDate &date ) const
{
    return KCalendarSystem::month( date );
}

int KCalendarSystemHijri::day( const QDate &date ) const
{
    return KCalendarSystem::day( date );
}

QDate KCalendarSystemHijri::addYears( const QDate &date, int nyears ) const
{
    return KCalendarSystem::addYears( date, nyears );
}

QDate KCalendarSystemHijri::addMonths( const QDate &date, int nmonths ) const
{
    return KCalendarSystem::addMonths( date, nmonths );
}

QDate KCalendarSystemHijri::addDays( const QDate &date, int ndays ) const
{
    return KCalendarSystem::addDays( date, ndays );
}

int KCalendarSystemHijri::monthsInYear( const QDate &date ) const
{
    return KCalendarSystem::monthsInYear( date );
}

int KCalendarSystemHijri::weeksInYear( const QDate &date ) const
{
    return KCalendarSystem::weeksInYear( date );
}

int KCalendarSystemHijri::weeksInYear( int year ) const
{
    return KCalendarSystem::weeksInYear( year );
}

int KCalendarSystemHijri::daysInYear( const QDate &date ) const
{
    return KCalendarSystem::daysInYear( date );
}

int KCalendarSystemHijri::daysInMonth( const QDate &date ) const
{
    return KCalendarSystem::daysInMonth( date );
}

int KCalendarSystemHijri::daysInWeek( const QDate &date ) const
{
    return KCalendarSystem::daysInWeek( date );
}

int KCalendarSystemHijri::dayOfYear( const QDate &date ) const
{
    return KCalendarSystem::dayOfYear( date );
}

int KCalendarSystemHijri::dayOfWeek( const QDate &date ) const
{
    return KCalendarSystem::dayOfWeek( date );
}

int KCalendarSystemHijri::weekNumber( const QDate &date, int *yearNum ) const
{
    return KCalendarSystem::weekNumber( date, yearNum );
}

bool KCalendarSystemHijri::isLeapYear( int year ) const
{
    return KCalendarSystem::isLeapYear( year );
}

bool KCalendarSystemHijri::isLeapYear( const QDate &date ) const
{
    return KCalendarSystem::isLeapYear( date );
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
    return 5; // Friday
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
    Q_D( const KCalendarSystemHijri );

    /*
    The following C++ code is translated from the Lisp code
    in ``Calendrical Calculations'' by Nachum Dershowitz and
    Edward M. Reingold, Software---Practice & Experience,
    vol. 20, no. 9 (September, 1990), pp. 899--928.

    This code is in the public domain, but any use of it
    should publically acknowledge its source.
    */

    // Search forward year by year from approximate year
    year = ( jd - epoch().toJulianDay() ) / 355;
    int testJd;
    dateToJulianDay( year, 12, d->daysInMonth( year, 12 ), testJd );
    while ( jd > testJd ) {
        year++;
        dateToJulianDay( year, 12, d->daysInMonth( year, 12 ), testJd );
    }

    // Search forward month by month from Muharram
    month = 1;
    dateToJulianDay( year, month, d->daysInMonth( year, month ), testJd );
    while ( jd > testJd ) {
        month++;
        dateToJulianDay( year, month, d->daysInMonth( year, month ), testJd );
    }

    dateToJulianDay( year, month, 1, testJd );
    day = jd - testJd + 1;

    return true;

    // Alternative implementations

    // More recent editions of "Calendrical Calculations" by Dershowitz & Reingold have a more
    // efficient direct calculation without recusrion, but this cannot be used due to licensing

    /*
    Formula from "Explanatory Supplement to the Astronomical Almanac" 2006, derived from Fliegel & Van Flandern 1968
    int L = jd - epoch().toJulianDay() + 10632;
    int N = ( L - 1 ) / 10631;
    L = L - 10631 * N + 354;
    int J = ( ( 10985 - L ) / 5316 ) x ( ( 50* L ) / 17719 ) + ( L / 5670 ) * ( ( 43 * L ) / 15238 );
    L = L - ( ( 30 - J ) / 15 ) * ( ( 17719 * J ) / 50 ) - ( J / 16 ) * ( ( 15238 * J ) / 43 ) + 29;
    year = ( 30 * N ) + J - 30;
    month = ( 24 * L ) / 709;
    day = L - ( ( 709 * month ) / 24 );
    */

    /*
    Formula from Fourmilab website
    jd = Math.floor(jd) + 0.5;
    year = Math.floor(((30 * (jd - epoch().toJulianDay())) + 10646) / 10631);
    month = qMin(12, Math.ceil((jd - (29 + islamic_to_jd(year, 1, 1))) / 29.5) + 1);
    day = (jd - islamic_to_jd(year, month, 1)) + 1;
    */
}

bool KCalendarSystemHijri::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    /*
    The following C++ code is translated from the Lisp code
    in ``Calendrical Calculations'' by Nachum Dershowitz and
    Edward M. Reingold, Software---Practice & Experience,
    vol. 20, no. 9 (September, 1990), pp. 899--928.

    This code is in the public domain, but any use of it
    should publically acknowledge its source.
    */

    jd = epoch().toJulianDay() - 1 +   // days before start of calendar
         ( year - 1 ) * 354 +          // non-leap days in prior years
         ( 3 + ( 11 * year ) ) / 30 +  // leap days in prior years
         29 * ( month - 1 ) +          // days so far...
         month / 2          +          //            ...this year
         day;                          // days so far this month

    return true;

    // Alternative implementations

    /*
    Formula from "Explanatory Supplement to the Astronomical Almanac" 2006, derived from Fliegel & Van Flandern 1968
    jd = ( 3 + ( 11 * year ) ) / 30 + 354 * year + 30 * month - ( month - 1 ) / 2 + day + epoch().toJulianDay() - 385;
    */
}
