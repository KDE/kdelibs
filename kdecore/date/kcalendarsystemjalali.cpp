/*
    Copyright (C) 2002-2003 Arash Bijanzadeh  and FarsiKDE Project <www.farsikde.org>
    Contact: Arash Bijanzadeh <a.bijanzadeh@linuxiran.org>
    Copyright (c) 2007 John Layt <john@layt.net>

    This program is part of FarsiKDE

    FarsiKDE is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    FarsiKDE is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */


#include "kcalendarsystemjalali.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>
#include <math.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <stdio.h>


//===========================================================================
//  This section holds the old Jalali <=> jd <=> Gregorian conversion code
//  Delete once conversion to new code complete and fully tested
//===========================================================================

static const int  gMonthDay[2][13] =
    {
        {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    };

static const    int     jMonthDay[2][13] =
    {
        {0, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29},
        {0, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30},
    };

typedef struct
{
    int day;
    int mon;
    int year;
} SDATE;

// converting funcs from

static int Ceil( float number )
{
    int ret;

    if( number > 0 ) {
        number += 0.5;
    }
    ret = ( int ) number;

    return ret;
}

static long jalali_jdn( int year, int month, int day )
{
    const long PERSIAN_EPOCH = 1948321; /* The JDN of 1 Farvardin 1*/
    int epbase;
    long epyear;
    long mdays;
    long jdn;

    epbase = year - 474;
    epyear = 474 + ( epbase % 2820 );
    if ( month <= 7 ) {
        mdays = ( month - 1 ) * 31;
    } else {
        mdays = ( month - 1 ) * 30 + 6;
    }
    jdn = day + mdays ;
    jdn += ( ( ( epyear * 682 ) - 110 ) / 2816 ) ;
    jdn += ( epyear - 1 ) * 365;
    jdn += ( epbase / 2820 ) * 1029983 ;
    jdn += ( PERSIAN_EPOCH - 1 );

    return jdn;
}


static SDATE jdn_jalali( long jdn )
{
    static SDATE ret;
    int day, month, year;
    int iYear, iMonth, iDay;
    int depoch;
    int cycle;
    int cyear;
    int ycycle;
    int aux1, aux2;
    int yday;

    day = 1;
    month = 1;
    year = 475;
    depoch = jdn - jalali_jdn( year, month, day );
    cycle = ( int ) ( depoch / 1029983 );
    cyear = depoch % 1029983;
    if( cyear == 1029982 ) {
        ycycle = 2820;
    } else {
        aux1 = cyear / 366;
        aux2 = cyear % 366;
        ycycle = ( ( ( 2134 * aux1 ) + ( 2816 * aux2 ) + 2815 ) / 1028522 ) + aux1 + 1;
    }
    iYear = ycycle + ( 2820 * cycle ) + 474;
    if ( iYear <= 0 ) {
        iYear = iYear - 1;
    }
    year = iYear;
    yday = ( jdn - jalali_jdn( year, month, day ) ) + 1;
    if( yday <= 186 ) {
        iMonth = Ceil( ( yday - 1 ) / 31 );
    } else {
        iMonth = Ceil( ( yday - 7 ) / 30 );
    }
    iMonth++;
    month = iMonth;
    iDay = ( jdn - jalali_jdn( year, month, day ) ) + 1;
    ret.day = iDay;
    ret.mon = iMonth;
    ret.year = iYear;

    return ret;
}

static long civil_jdn( int year, int month, int day )
{
    long jdn = ( ( 1461 * ( year + 4800 + ( ( month - 14 ) / 12 ) ) ) / 4 )
               + ( ( 367 * ( month - 2 - 12 * ( ( ( month - 14 ) / 12 ) ) ) ) / 12 )
               - ( ( 3 * ( ( ( year + 4900 + ( ( month - 14 ) / 12 ) ) / 100 ) ) ) / 4 )
               + day - 32075;

    return jdn;
}

static SDATE jdn_civil( long jdn )
{
    long l, n, i, j;
    static SDATE ret;
    int iday, imonth, iyear;

    l = jdn + 68569;
    n = ( ( 4 * l ) / 146097 );
    l = l - ( ( 146097 * n + 3 ) / 4 );
    i = ( ( 4000 * ( l + 1 ) ) / 1461001 );
    l = l - ( ( 1461 * i ) / 4 ) + 31;
    j = ( ( 80 * l ) / 2447 );
    iday = l - ( ( 2447 * j ) / 80 );
    l = ( j / 11 );
    imonth = j + 2 - 12 * l;
    iyear = 100 * ( n - 49 ) + i + l;
    ret.day = iday;
    ret.mon = imonth;
    ret.year = iyear;

    return ( ret );
}

static SDATE *jalaliToGregorian( int y, int m, int d )
{
    static SDATE sd;

    long jday = jalali_jdn( y, m, d );
    sd = jdn_civil( jday );

    return ( &sd );
}

static SDATE *gregorianToJalali( int y, int m, int d )
{
    static SDATE sd;

    long   jdn = civil_jdn( y, m, d );//QDate::gregorianToJulian(y, m, d);
    sd = jdn_jalali( jdn );

    return( &sd );
}

static void gregorianToJalali( const QDate &date, int *pYear, int *pMonth, int *pDay )
{
    SDATE * sd;

    sd = gregorianToJalali( date.year(), date.month(), date.day() );
    if ( pYear ) {
        * pYear = sd->year;
    }
    if ( pMonth ) {
        * pMonth = sd->mon;
    }
    if ( pDay ) {
        * pDay = sd->day;
    }
}

// End of converting functions

static int isJalaliLeap( int year )
{
    int     tmp;

    tmp = year % 33;
    if ( tmp == 1 || tmp == 5 || tmp == 9 || tmp == 13 || tmp == 17 || tmp == 22 || tmp == 26 || tmp == 30 ) {
        return 1;
    } else {
        return 0;
    }
}

static int hndays( int m, int y )
{
    return jMonthDay[isJalaliLeap( y )][m];
}


//===========================================================================


KCalendarSystemJalali::KCalendarSystemJalali( const KLocale * locale )
                      : KCalendarSystem( locale ), d( 0 )
{
}

KCalendarSystemJalali::~KCalendarSystemJalali()
{
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
    // 19 March 622 in the Julian calendar
    return KCalendarSystem::earliestValidDate();
}

QDate KCalendarSystemJalali::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widets support > 9999
    // Last day of Jalali year 9999 is 9999-12-29
    // Which in Gregorian is 10621-03-17
    // Which is jd xxxx FIXME Find out jd and use that instead
    // Can't call setDate() as it creates circular reference!
    return QDate( 10621, 3, 17 );
}

bool KCalendarSystemJalali::isValid( int y, int month, int day ) const
{
    // taken from setYMD below, adapted to use new methods
    if ( y < year( earliestValidDate() ) || y > year( latestValidDate() ) ) {
        return false;
    }

    if ( month < 1 || month > 12 ) {  // Fix me!
        return false;
    }

    if ( day < 1 || day > hndays( month, y ) ) {
        return false;
    }

    return true;
}

bool KCalendarSystemJalali::isValid( const QDate &date ) const
{
    return KCalendarSystem::isValid( date );
}

bool KCalendarSystemJalali::setDate( QDate &date, int year, int month, int day ) const
{
    return KCalendarSystem::setDate( date, year, month, day );
}

//MAY BE BUGGY
bool KCalendarSystemJalali::setYMD( QDate &date, int y, int m, int d ) const
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

    if ( d < 1 || d > hndays( m, y ) ) {
        return false;
    }

    SDATE  *gd = jalaliToGregorian( y, m, d );

    return date.setYMD( gd->year, gd->mon, gd->day );
}

int KCalendarSystemJalali::year( const QDate &date ) const
{
    kDebug( 5400 ) << "Jalali year...";
    int y;

    gregorianToJalali( date, &y, 0, 0 );

    return y;
}

int KCalendarSystemJalali::month ( const QDate& date ) const

{
    kDebug( 5400 ) << "Jalali month...";
    int m;

    gregorianToJalali( date, 0 , &m, 0 );

    return m;
}

int KCalendarSystemJalali::day( const QDate &date ) const
{
    kDebug( 5400 ) << "Jalali day...";
    int d;

    gregorianToJalali( date, 0, 0, &d );

    return d;
}

QDate KCalendarSystemJalali::addYears( const QDate &date, int nyears ) const
{
    QDate result = date;

    int y = year( date ) + nyears;
    setYMD( result, y, month( date ), day( date ) );

    return result;
}

QDate KCalendarSystemJalali::addMonths( const QDate &date, int nmonths ) const
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

QDate KCalendarSystemJalali::addDays( const QDate &date, int ndays ) const
{
    return date.addDays( ndays );
}

int KCalendarSystemJalali::monthsInYear( const QDate &date ) const
{
    Q_UNUSED( date )

    return 12;
}

int KCalendarSystemJalali::weeksInYear( const QDate &date ) const
{
    return KCalendarSystem::weeksInYear( date );
}

int KCalendarSystemJalali::weeksInYear( int year ) const

{
    Q_UNUSED( year );
// couldn't understand it!
    return 52;
}

int KCalendarSystemJalali::daysInYear( const QDate &date ) const
{
    Q_UNUSED( date );
    int result;
//SDATE *sd = gregorianToJalali(year(date),month(date),day(date));
//if (isJalaliLeap(sd->year))
    result = 366;
//else
// result=365;
    return result;
}

int KCalendarSystemJalali::daysInMonth( const QDate &date ) const
{
    SDATE * sd = gregorianToJalali( date.year(), date.month(), date.day() );
    return hndays( sd->mon, sd->year );
}

int KCalendarSystemJalali::daysInWeek( const QDate &date ) const
{
    return KCalendarSystem::daysInWeek( date );
}

//NOT TESTED YET
int KCalendarSystemJalali::dayOfYear( const QDate &date ) const
{
    QDate first;
    setYMD( first, year( date ), 1, 1 );

    return first.daysTo( date ) + 1;
}

int KCalendarSystemJalali::dayOfWeek( const QDate &date ) const
{
    //same same I think?!
    return date.dayOfWeek();

}

int KCalendarSystemJalali::weekNumber( const QDate &date, int *yearNum ) const
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
    if ( weekDay1 > 4 /*Thursday*/ )
        firstDayWeek1 = addDays( firstDayWeek1 , 7 - weekDay1 + 1 ); // next monday

    dayOfWeek1InYear = dayOfYear( firstDayWeek1 );

    // if our date in prev year's week
    if ( dayOfYear( date ) < dayOfWeek1InYear ) {
        if ( yearNum ) {
            * yearNum = y - 1;
        }
        return weeksInYear( y - 1 );
    }
    // let' check if its last week belongs to next year
    setYMD( lastDayOfYear, y, 12, hndays( 12, y ) );
    // if our date is in last week && 1st week in next year has thursday
    if ( ( dayOfYear( date ) >= daysInYear( date ) - dayOfWeek( lastDayOfYear ) + 1 )
            && dayOfWeek( lastDayOfYear ) < 4 ) {
        if ( yearNum ) {
            * yearNum = y + 1;
        }
        week = 1;
    } else {
        week = firstDayWeek1.daysTo( date ) / 7 + 1;
    }

    return week;
}

bool KCalendarSystemJalali::isLeapYear( int year ) const
{
    // from isJalaliLeap above
    int     tmp;
    tmp = year % 33;
    if ( tmp == 1 || tmp == 5 || tmp == 9 || tmp == 13 || tmp == 17 || tmp == 22 || tmp == 26 || tmp == 30 ) {
        return true;
    } else {
        return false;
    }
}

bool KCalendarSystemJalali::isLeapYear( const QDate &date ) const
{
    return QDate::isLeapYear( year( date ) );
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
    // from jdn_jalali above.  Fix me!
    if ( jd >= earliestValidDate().toJulianDay() && jd <= latestValidDate().toJulianDay() ) {
        int y, m, d;
        int iYear, iMonth, iDay;
        int depoch;
        int cycle;
        int cyear;
        int ycycle;
        int aux1, aux2;
        int yday;
        d = 1;
        m = 1;
        y = 475;
        depoch = jd - jalali_jdn( y, m, d );
        cycle = ( int ) ( depoch / 1029983 );
        cyear = depoch % 1029983;
        if ( cyear == 1029982 ) {
            ycycle = 2820;
        } else {
            aux1 = cyear / 366;
            aux2 = cyear % 366;
            ycycle = ( ( ( 2134 * aux1 ) + ( 2816 * aux2 ) + 2815 ) / 1028522 ) + aux1 + 1;
        }
        iYear = ycycle + ( 2820 * cycle ) + 474;
        if ( iYear <= 0 ) {
            iYear = iYear - 1;
        }
        y = iYear;
        yday = ( jd - jalali_jdn( y, m, d ) ) + 1;
        if( yday <= 186 ) {
            iMonth = Ceil( ( yday - 1 ) / 31 );
        } else {
            iMonth = Ceil( ( yday - 7 ) / 30 );
        }
        iMonth++;
        m = iMonth;
        iDay = ( jd - jalali_jdn( y, m, d ) ) + 1;
        day = iDay;
        month = iMonth;
        year = iYear;
        return true;
    }
    return false;
}

bool KCalendarSystemJalali::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    // From jalali_jdn above.  Fix me!
    if ( isValid( year, month, day ) ) {
        int epbase;
        long epyear;
        long mdays;
        epbase = year - 474;
        epyear = 474 + ( epbase % 2820 );
        if ( month <= 7 ) {
            mdays = ( month - 1 ) * 31;
        } else {
            mdays = ( month - 1 ) * 30 + 6;
        }
        jd = day + mdays;
        jd += ( ( ( epyear * 682 ) - 110 ) / 2816 ) ;
        jd += ( epyear - 1 ) * 365;
        jd += ( epbase / 2820 ) * 1029983 ;
        jd += ( epoch().toJulianDay() - 1 );
        return true;
    }
    return false;
}
