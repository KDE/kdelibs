/*
    Copyright (C) 2002-2003 Arash Bijanzadeh  and FarsiKDE Project <www.farsikde.org>
    Contact: Arash Bijanzadeh <a.bijanzadeh@linuxiran.org>
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

/*
    This is an implementation of the artithmetic Persian calendar using the
    Birashk algorithm with adjustments to always be correct in the period 1244
    and 1530 (1865 to 2152 Gregorian).

    In future this will be replaced with the correct astronomical calendar.
*/

#include "kcalendarsystemjalali_p.h"
#include "kcalendarsystemprivate_p.h"

#include <QtCore/QDate>

class KCalendarSystemJalaliPrivate : public KCalendarSystemPrivate
{
public:
    explicit KCalendarSystemJalaliPrivate( KCalendarSystemJalali *q );

    virtual ~KCalendarSystemJalaliPrivate();

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

KCalendarSystemJalaliPrivate::KCalendarSystemJalaliPrivate( KCalendarSystemJalali *q )
                             :KCalendarSystemPrivate( q )
{
}

KCalendarSystemJalaliPrivate::~KCalendarSystemJalaliPrivate()
{
}

void KCalendarSystemJalaliPrivate::initDefaultEraList()
{
    QString name, shortName, format;
    // Islamic Era (Hijri), Anno Persico.
    name = i18nc( "Calendar Era: Jalali Islamic Era, years > 0, LongFormat", "Anno Persico" );
    shortName = i18nc( "Calendar Era: Jalali Islamic Era, years > 0, ShortFormat", "AP" );
    format = i18nc( "(kdedt-format) Jalali, AP, full era year format used for %EY, e.g. 2000 AP", "%Ey %EC" );
    addEra( '+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format );
}

int KCalendarSystemJalaliPrivate::monthsInYear( int year ) const
{
    Q_UNUSED( year )
    return 12;
}

int KCalendarSystemJalaliPrivate::daysInMonth( int year, int month ) const
{
    if ( month == 12 ) {
        if ( isLeapYear( year ) ) {
            return 30;
        } else {
            return 29;
        }
    }

    if ( month <= 6  ) {
        return 31;
    }

    return 30;
}

int KCalendarSystemJalaliPrivate::daysInYear( int year ) const
{
    if ( isLeapYear( year ) ) {
        return 366;
    } else {
        return 365;
    }
}

int KCalendarSystemJalaliPrivate::daysInWeek() const
{
    return 7;
}

bool KCalendarSystemJalaliPrivate::isLeapYear( int year ) const
{
    // From formilab Public Domain code http://www.fourmilab.ch/documents/calendar/
    // Use Birashk algorithm as it matches the to/from jd code below

    // Birashk algorithm is incorrect in two years in period AP 1244 to 1531,
    // 1403/1404 and 1436/1437, and so catch them here first
    if ( year == 1403 || year == 1436 ) {
        return true;
    } else if ( year == 1404 || year == 1437 ) {
        return false;
    }

    if ( year >= 0 ) {
        year = year - 474;
    } else {
        year = year - 473;
    }

    if ( ( ( ( ( ( year % 2820 ) + 474 ) + 38 ) * 682 ) % 2816 ) < 682 ) {
        return true;
    } else {
        return false;
    }
}

bool KCalendarSystemJalaliPrivate::hasLeapMonths() const
{
    return false;
}

bool KCalendarSystemJalaliPrivate::hasYearZero() const
{
    return false;
}

int KCalendarSystemJalaliPrivate::maxDaysInWeek() const
{
    return 7;
}

int KCalendarSystemJalaliPrivate::maxMonthsInYear() const
{
    return 12;
}

int KCalendarSystemJalaliPrivate::earliestValidYear() const
{
    return 1244;
}

int KCalendarSystemJalaliPrivate::latestValidYear() const
{
    return 1530;
}


KCalendarSystemJalali::KCalendarSystemJalali( const KLocale *locale )
                     : KCalendarSystem( *new KCalendarSystemJalaliPrivate( this ), KSharedConfig::Ptr(), locale ),
                       dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemJalali::KCalendarSystemJalali( const KSharedConfig::Ptr config, const KLocale *locale )
                     : KCalendarSystem( *new KCalendarSystemJalaliPrivate( this ), config, locale ),
                       dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemJalali::KCalendarSystemJalali( KCalendarSystemJalaliPrivate &dd,
                                              const KSharedConfig::Ptr config, const KLocale *locale )
                     : KCalendarSystem( dd, config, locale ),
                       dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemJalali::~KCalendarSystemJalali()
{
    delete dont_use;
}

QString KCalendarSystemJalali::calendarType() const
{
    return QLatin1String( "jalali" );
}

QDate KCalendarSystemJalali::epoch() const
{
    // 19 March 622 in the Julian calendar
    return QDate::fromJulianDay( 1948321 );
}

QDate KCalendarSystemJalali::earliestValidDate() const
{
    // Using the Birashk formula which is accurate in period AP 1244 to 1530 (AD 1865 to 2152)
    // 1244-01-01 Jalali 1865-03-21 Gregorian
    return QDate::fromJulianDay( 2402317 );
}

QDate KCalendarSystemJalali::latestValidDate() const
{
    // Using the Birashk formula which is accurate in period AP 1244 to 1530 (AD 1865 to 2152)
    // 1530-12-29 Jalali 2152-03-19 Gregorian
    return QDate::fromJulianDay( 2507140 );
}

bool KCalendarSystemJalali::isValid( int year, int month, int day ) const
{
    return KCalendarSystem::isValid( year, month, day );
}

bool KCalendarSystemJalali::isValid( const QDate &date ) const
{
    return KCalendarSystem::isValid( date );
}

bool KCalendarSystemJalali::setDate( QDate &date, int year, int month, int day ) const
{
    return KCalendarSystem::setDate( date, year, month, day );
}

// Deprecated
bool KCalendarSystemJalali::setYMD( QDate &date, int year, int month, int day ) const
{
    return KCalendarSystem::setYMD( date, year, month, day );
}

int KCalendarSystemJalali::year( const QDate &date ) const
{
    return KCalendarSystem::year( date );
}

int KCalendarSystemJalali::month ( const QDate& date ) const

{
    return KCalendarSystem::month( date );
}

int KCalendarSystemJalali::day( const QDate &date ) const
{
    return KCalendarSystem::day( date );
}

QDate KCalendarSystemJalali::addYears( const QDate &date, int nyears ) const
{
    return KCalendarSystem::addYears( date, nyears );
}

QDate KCalendarSystemJalali::addMonths( const QDate &date, int nmonths ) const
{
    return KCalendarSystem::addMonths( date, nmonths );
}

QDate KCalendarSystemJalali::addDays( const QDate &date, int ndays ) const
{
    return KCalendarSystem::addDays( date, ndays );
}

int KCalendarSystemJalali::monthsInYear( const QDate &date ) const
{
    return KCalendarSystem::monthsInYear( date );
}

int KCalendarSystemJalali::weeksInYear( const QDate &date ) const
{
    return KCalendarSystem::weeksInYear( date );
}

int KCalendarSystemJalali::weeksInYear( int year ) const
{
    return KCalendarSystem::weeksInYear( year );
}

int KCalendarSystemJalali::daysInYear( const QDate &date ) const
{
    return KCalendarSystem::daysInYear( date );
}

int KCalendarSystemJalali::daysInMonth( const QDate &date ) const
{
    return KCalendarSystem::daysInMonth( date );
}

int KCalendarSystemJalali::daysInWeek( const QDate &date ) const
{
    return KCalendarSystem::daysInWeek( date );
}

int KCalendarSystemJalali::dayOfYear( const QDate &date ) const
{
    return KCalendarSystem::dayOfYear( date );
}

int KCalendarSystemJalali::dayOfWeek( const QDate &date ) const
{
    return KCalendarSystem::dayOfWeek( date );
}

int KCalendarSystemJalali::weekNumber( const QDate &date, int *yearNum ) const
{
    return KCalendarSystem::weekNumber( date, yearNum );
}

bool KCalendarSystemJalali::isLeapYear( int year ) const
{
    return KCalendarSystem::isLeapYear( year );
}

bool KCalendarSystemJalali::isLeapYear( const QDate &date ) const
{
    return KCalendarSystem::isLeapYear( date );
}

QString KCalendarSystemJalali::monthName( int month, int year, MonthNameFormat format )  const
{
    Q_UNUSED( year );

    if ( format == ShortNamePossessive ) {
        switch ( month ) {
        case 1:
            return ki18nc( "of Farvardin short",   "of Far" ).toString( locale() );
        case 2:
            return ki18nc( "of Ordibehesht short", "of Ord" ).toString( locale() );
        case 3:
            return ki18nc( "of Khordad short",     "of Kho" ).toString( locale() );
        case 4:
            return ki18nc( "of Tir short",         "of Tir" ).toString( locale() );
        case 5:
            return ki18nc( "of Mordad short",      "of Mor" ).toString( locale() );
        case 6:
            return ki18nc( "of Shahrivar short",   "of Sha" ).toString( locale() );
        case 7:
            return ki18nc( "of Mehr short",        "of Meh" ).toString( locale() );
        case 8:
            return ki18nc( "of Aban short",        "of Aba" ).toString( locale() );
        case 9:
            return ki18nc( "of Azar short",        "of Aza" ).toString( locale() );
        case 10:
            return ki18nc( "of Dei short",         "of Dei" ).toString( locale() );
        case 11:
            return ki18nc( "of Bahman short",      "of Bah" ).toString( locale() );
        case 12:
            return ki18nc( "of Esfand short",      "of Esf" ).toString( locale() );
        default:
            return QString();
        }
    }

    if ( format == ShortName ) {
        switch ( month ) {
        case 1:
            return ki18nc( "Farvardin short",   "Far" ).toString( locale() );
        case 2:
            return ki18nc( "Ordibehesht short", "Ord" ).toString( locale() );
        case 3:
            return ki18nc( "Khordad short",     "Kho" ).toString( locale() );
        case 4:
            return ki18nc( "Tir short",         "Tir" ).toString( locale() );
        case 5:
            return ki18nc( "Mordad short",      "Mor" ).toString( locale() );
        case 6:
            return ki18nc( "Shahrivar short",   "Sha" ).toString( locale() );
        case 7:
            return ki18nc( "Mehr short",        "Meh" ).toString( locale() );
        case 8:
            return ki18nc( "Aban short",        "Aba" ).toString( locale() );
        case 9:
            return ki18nc( "Azar short",        "Aza" ).toString( locale() );
        case 10:
            return ki18nc( "Dei short",         "Dei" ).toString( locale() );
        case 11:
            return ki18nc( "Bahman short",      "Bah" ).toString( locale() );
        case 12:
            return ki18nc( "Esfand",            "Esf" ).toString( locale() );
        default:
            return QString();
        }
    }

    if ( format == LongNamePossessive ) {
        switch ( month ) {
        case 1:
            return ki18n( "of Farvardin" ).toString( locale() );
        case 2:
            return ki18n( "of Ordibehesht" ).toString( locale() );
        case 3:
            return ki18n( "of Khordad" ).toString( locale() );
        case 4:
            return ki18nc( "of Tir long", "of Tir" ).toString( locale() );
        case 5:
            return ki18n( "of Mordad" ).toString( locale() );
        case 6:
            return ki18n( "of Shahrivar" ).toString( locale() );
        case 7:
            return ki18n( "of Mehr" ).toString( locale() );
        case 8:
            return ki18n( "of Aban" ).toString( locale() );
        case 9:
            return ki18n( "of Azar" ).toString( locale() );
        case 10:
            return ki18nc( "of Dei long", "of Dei" ).toString( locale() );
        case 11:
            return ki18n( "of Bahman" ).toString( locale() );
        case 12:
            return ki18n( "of Esfand" ).toString( locale() );
        default:
            return QString();
        }
    }

    // Default to LongName format
    switch ( month ) {
    case 1:
        return ki18n( "Farvardin" ).toString( locale() );
    case 2:
        return ki18n( "Ordibehesht" ).toString( locale() );
    case 3:
        return ki18n( "Khordad" ).toString( locale() );
    case 4:
        return ki18nc( "Tir long", "Tir" ).toString( locale() );
    case 5:
        return ki18n( "Mordad" ).toString( locale() );
    case 6:
        return ki18n( "Shahrivar" ).toString( locale() );
    case 7:
        return ki18n( "Mehr" ).toString( locale() );
    case 8:
        return ki18n( "Aban" ).toString( locale() );
    case 9:
        return ki18n( "Azar" ).toString( locale() );
    case 10:
        return ki18nc( "Dei long", "Dei" ).toString( locale() );
    case 11:
        return ki18n( "Bahman" ).toString( locale() );
    case 12:
        return ki18n( "Esfand" ).toString( locale() );
    default:
        return QString();
    }
}

QString KCalendarSystemJalali::monthName( const QDate &date, MonthNameFormat format ) const
{
    return monthName( month( date ), year( date ), format );
}

QString KCalendarSystemJalali::weekDayName( int day, WeekDayNameFormat format ) const
{
    if ( format == ShortDayName ) {
        switch ( day ) {
        case 1:
            return ki18nc( "Do shanbe short",     "2sh" ).toString( locale() );
        case 2:
            return ki18nc( "Se shanbe short",     "3sh" ).toString( locale() );
        case 3:
            return ki18nc( "Chahar shanbe short", "4sh" ).toString( locale() );
        case 4:
            return ki18nc( "Panj shanbe short",   "5sh" ).toString( locale() );
        case 5:
            return ki18nc( "Jumee short",         "Jom" ).toString( locale() );
        case 6:
            return ki18nc( "Shanbe short",        "shn" ).toString( locale() );
        case 7:
            return ki18nc( "Yek-shanbe short",    "1sh" ).toString( locale() );
        default:
            return QString();
        }
    }

    // Default to ShortDayName format
    switch ( day ) {
    case 1:
        return ki18n( "Do shanbe" ).toString( locale() );
    case 2:
        return ki18n( "Se shanbe" ).toString( locale() );
    case 3:
        return ki18n( "Chahar shanbe" ).toString( locale() );
    case 4:
        return ki18n( "Panj shanbe" ).toString( locale() );
    case 5:
        return ki18n( "Jumee" ).toString( locale() );
    case 6:
        return ki18n( "Shanbe" ).toString( locale() );
    case 7:
        return ki18n( "Yek-shanbe" ).toString( locale() );
    default:
        return QString();
    }
}

QString KCalendarSystemJalali::weekDayName( const QDate &date, WeekDayNameFormat format ) const
{
    return weekDayName( dayOfWeek( date ), format );
}

QString KCalendarSystemJalali::yearString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::yearString( pDate, format );
}

QString KCalendarSystemJalali::monthString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::monthString( pDate, format );
}

QString KCalendarSystemJalali::dayString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::dayString( pDate, format );
}

int KCalendarSystemJalali::yearStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::yearStringToInteger( sNum, iLength );
}

int KCalendarSystemJalali::monthStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::monthStringToInteger( sNum, iLength );
}

int KCalendarSystemJalali::dayStringToInteger( const QString & sNum, int & iLength ) const
{
    return KCalendarSystem::dayStringToInteger( sNum, iLength );
}

QString KCalendarSystemJalali::formatDate( const QDate &date, KLocale::DateFormat format ) const
{
    return KCalendarSystem::formatDate( date, format );
}

QDate KCalendarSystemJalali::readDate( const QString &str, bool *ok ) const
{
    return KCalendarSystem::readDate( str, ok );
}

QDate KCalendarSystemJalali::readDate( const QString &intstr, const QString &fmt, bool *ok ) const
{
    return KCalendarSystem::readDate( intstr, fmt, ok );
}

QDate KCalendarSystemJalali::readDate( const QString &str, KLocale::ReadDateFlags flags, bool *ok ) const
{
    return KCalendarSystem::readDate( str, flags, ok );
}

int KCalendarSystemJalali::weekStartDay() const
{
    return KCalendarSystem::weekStartDay();
}

int KCalendarSystemJalali::weekDayOfPray() const
{
    return 5; // friday
}

bool KCalendarSystemJalali::isLunar() const
{
    return false;
}

bool KCalendarSystemJalali::isLunisolar() const
{
    return false;
}

bool KCalendarSystemJalali::isSolar() const
{
    return true;
}

bool KCalendarSystemJalali::isProleptic() const
{
    return false;
}

bool KCalendarSystemJalali::julianDayToDate( int jd, int &year, int &month, int &day ) const
{
    // Birashk algorithm is incorrect in two years in period AP 1244 to 1531.
    // This results in a leap day being added to the end of 1404 instead of 1403
    // and to the end of 1437 instead of 1436.  Check for these dates first and
    // return accordingly.  Relies on later use of dateToJulianDay() to correctly
    // calculate firstDayOfYear in 1404 and 1437, so no other adjustments needed.
    if ( jd == 2460755 ) {
        year = 1403;
        month = 12;
        day = 30;
        return true;
    }
    if ( jd == 2472808 ) {
        year = 1436;
        month = 12;
        day = 30;
        return true;
    }

    // From original KDE3 code, source unknown?  Unable to contact author or committer to confirm
    // Matches Fermilab code, EMACS and D&R so check for PD source, likely Birashk's book

    int jdCycleStart;
    int daysSinceCycleStart;
    int cycle;
    int dayInCycle;
    int yearInCycle;
    dateToJulianDay( 475, 1, 1, jdCycleStart );
    daysSinceCycleStart = jd - jdCycleStart;
    cycle = daysSinceCycleStart / 1029983;
    dayInCycle = daysSinceCycleStart % 1029983;
    if ( dayInCycle == 1029982 ) {
        yearInCycle = 2820;
    } else {
        int aux1 = dayInCycle / 366;
        int aux2 = dayInCycle % 366;
        yearInCycle = ( ( ( 2134 * aux1 ) + ( 2816 * aux2 ) + 2815 ) / 1028522 ) + aux1 + 1;
    }
    year = yearInCycle + ( 2820 * cycle ) + 474;
    if ( year <= 0 ) {
        year = year - 1;
    }

    int firstDayOfYear;
    dateToJulianDay( year, 1, 1, firstDayOfYear );
    int dayinYear = jd - firstDayOfYear + 1;
    if( dayinYear <= 186 ) {
        month = ( ( dayinYear - 1 ) / 31 ) + 1;
        day = dayinYear - ( ( month - 1 ) * 31 );
    } else {
        month = ( ( dayinYear - 7 ) / 30 ) + 1;
        day = dayinYear - ( ( month - 1 ) * 30 ) - 6;
    }

    return true;
}

bool KCalendarSystemJalali::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    Q_D( const KCalendarSystemJalali );

    // Birashk algorithm is incorrect in two years in period AP 1244 to 1531.
    // This results in a leap day being added to the end of 1404 instead of 1403
    // and to the end of 1437 instead of 1436.  Thus all dates in 1404 and 1437
    // are off by 1 JD.  Check for these dates first and adjust accordingly.
    if ( year == 1403 && month == 12 && day == 30 ) {
        jd = 2460755;
        return true;
    }
    if ( year == 1436 && month == 12 && day == 30 ) {
        jd = 2472808;
        return true;
    }
    if ( year == 1404 || year == 1437 ) {
        if ( month < 12 && day + 1 > d->daysInMonth( year, month ) ) {
            day = 1;
            month = month + 1;
        } else {
            day = day + 1;
        }
    }

    // From original KDE3 code, source unknown?  Unable to contact author or committer to confirm
    // Matches Fermilab code, EMACS and D&R so check for PD source, likely Birashk's book
    int epbase;
    long epyear;
    long monthDays;

    if ( year >= 0 ) {
        epbase = year - 474;
    } else {
        epbase = year - 473;
    }

    epyear = 474 + ( epbase % 2820 );

    if ( month <= 7 ) {
        monthDays = ( month - 1 ) * 31;
    } else {
        monthDays = ( ( month - 1 ) * 30 ) + 6;
    }

    jd = ( epoch().toJulianDay() - 1 ) +          // days before epoch
         ( epyear - 1 ) * 365 +                   // normal days in previous years
         ( ( ( epyear * 682 ) - 110 ) / 2816 ) +  // leap days in previous years
         ( epbase / 2820 ) * 1029983 +
         monthDays +                              // days in previous months this year
         day;                                     // days in this month

    return true;
}
