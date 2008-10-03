/*
    Copyright (c) 2003 Hans Petter Bieker <bieker@kde.org>
    Copyright (c) 2007 John Layt <john@layt.net>
        Calendar conversion routines based on Hdate v6, by Amos
        Shapir 1978 (rev. 1985, 1992)

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

// Derived hebrew kde calendar class

#include "kcalendarsystemhebrew.h"

#include "kdebug.h"
#include "klocale.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

//===========================================================================
//  This section holds the old Hebrew <=> jd <=> Hebrew conversion code
//  This is here to allow comparison testing to confirm old and new match
//===========================================================================

static int hebrewDaysElapsed( int y );

static QString num2heb( int num, bool includeMillenium );

class h_date
{
public:
    int hd_day;
    int hd_mon;
    int hd_year;
    int hd_dw;
    int hd_flg;
};

/*
 * compute general date structure from hebrew date
 */
static class h_date * hebrewToGregorian( int y, int m, int d )
{
    static class h_date h;
    int s;

    y -= 3744;
    s = hebrewDaysElapsed( y );
    d += s;
    s = hebrewDaysElapsed( y + 1 ) - s;    /* length of year */

    if ( s > 365 && m > 6 ) {
        --m;
        d += 30;
    }
    d += ( 59 * ( m - 1 ) + 1 ) / 2;  /* regular months */
    /* special cases */
    if ( s % 10 > 4 && m > 2 ) {  /* long Heshvan */
        d++;
    }
    if ( s % 10 < 4 && m > 3 ) {  /* short Kislev */
        d--;
    }
    // ### HPB: Broken in leap years
    //if (s > 365 && m > 6)  /* leap year */
    //  d += 30;
    d -= 6002;

    y = ( d + 36525 ) * 4 / 146097 - 1;
    d -= y / 4 * 146097 + ( y % 4 ) * 36524;
    y *= 100;

    /* compute year */
    s = ( d + 366 )*4 / 1461 - 1;
    d -= s / 4*1461 + ( s % 4 )*365;
    y += s;
    /* compute month */
    m = ( d + 245 )*12 / 367 - 7;
    d -= m*367 / 12 - 30;
    if ( ++m >= 12 ) {
        m -= 12;
        y++;
    }
    h.hd_day = d;
    h.hd_mon = m;
    h.hd_year = y;
    return( &h );
}

/*
 * compute date structure from no. of days since 1 Tishrei 3744
 */
static class h_date * gregorianToHebrew( int y, int m, int d )
{
    static class h_date h;
    int s;

    if ( ( m -= 2 ) <= 0 ) {
        m += 12;
        y--;
    }
    /* no. of days, Julian calendar */
    d += 365*y + y / 4 + 367*m / 12 + 5968;
    /* Gregorian calendar */
    d -= y / 100 - y / 400 - 2;
    h.hd_dw = ( d + 1 ) % 7;

    /* compute the year */
    y += 16;
    s = hebrewDaysElapsed( y );
    m = hebrewDaysElapsed( y + 1 );
    while( d >= m ) {  /* computed year was underestimated */
        s = m;
        y++;
        m = hebrewDaysElapsed( y + 1 );
    }
    d -= s;
    s = m - s;  /* size of current year */
    y += 3744;

    h.hd_flg = s % 10 - 4;

    /* compute day and month */
    if ( d >= s - 236 ) {  /* last 8 months are regular */
        d -= s - 236;
        m = d * 2 / 59;
        d -= ( m * 59 + 1 ) / 2;
        m += 4;
        if ( s > 365 && m <= 5 ) {  /* Adar of Meuberet */
            m += 8;
        }
    } else {
        /* first 4 months have 117-119 days */
        s = 114 + s % 10;
        m = d * 4 / s;
        d -= ( m * s + 3 ) / 4;
    }

    h.hd_day = d;
    h.hd_mon = m;
    h.hd_year = y;
    return( &h );
}

static QString num2heb( int num, bool includeMillenium )
{
    const QChar decade[] = {
                               0x05D8, 0x05D9, 0x05DB, 0x05DC, 0x05DE,
                               0x05E0, 0x05E1, 0x05E2, 0x05E4, 0x05E6
                           };
    QString result;

    if ( num < 1 || num > 9999 ) {
        return QString::number( num );
    }

    if ( num >= 1000 ) {
        if ( includeMillenium || num % 1000 == 0 )
            result += QChar( 0x05D0 - 1 + num / 1000 );
        num %= 1000;
    }

    if ( num >= 100 ) {
        while ( num >= 500 ) {
            result += QChar( 0x05EA );
            num -= 400;
        }
        result += QChar( 0x05E7 - 1 + num / 100 );
        num %= 100;
    }

    if ( num >= 10 ) {
        if ( num == 15 || num == 16 )
            num -= 9;
        result += decade[num / 10];
        num %= 10;
    }

    if ( num > 0 ) {
        result += QChar( 0x05D0 - 1 + num );
    }

    if ( result.length() == 1 ) {
        result += '\'';
    } else {
        result.insert( result.length() - 1, '\"' );
    }

    return result;
}

static int heb2num( const QString &str, int &iLength )
{
    QChar c;
    QString s = str;
    int result = 0;
    iLength = 0;
    int decadeValues[14] = {10, 20, 20, 30, 40, 40, 50,
                            50, 60, 70, 80, 80, 90, 90};

    int pos;
    for ( pos = 0 ; pos < s.length() ; pos++ ) {
        c = s[pos];
        if ( s.length() > pos && ( s[pos + 1] == QChar( '\'' ) ||
                                   s[pos + 1] == QChar( '\"' ) ) ) {
            iLength++;
            s.remove( pos + 1, 1 );
        }

        if ( c >= QChar( 0x05D0 ) && c <= QChar( 0x05D7 ) ) {
            if ( s.length() > pos && s[pos + 1] >= QChar( 0x05D0 ) &&
                    s[pos + 1] <= QChar( 0x05EA ) ) {
                result += ( c.unicode() - 0x05D0 + 1 ) * 1000;
            } else {
                result += c.unicode() - 0x05D0 + 1;
            }
        } else if ( c == QChar( 0x05D8 ) ) {
            if ( s.length() > pos && s[pos + 1] >= QChar( 0x05D0 ) &&
                    s[pos + 1] <= QChar( 0x05EA ) && s[pos + 1] != QChar( 0x05D5 ) &&
                    s[pos + 1] != QChar( 0x05D6 ) ) {
                result += 9000;
            } else {
                result += 9;
            }
        } else if ( c >= QChar( 0x05D9 ) && c <= QChar( 0x05E6 ) ) {
            if ( s.length() > pos && s[pos + 1] >= QChar( 0x05D9 ) ) {
                return -1;
            } else {
                result += decadeValues[c.unicode() - 0x05D9];
            }
        } else if ( c >= QChar( 0x05E7 ) && c <= QChar( 0x05EA ) ) {
            result += ( c.unicode() - 0x05E7 + 1 ) * 100;
        } else {
            break;
        }
    }

    iLength += pos;

    return result;
}

/* constants, in 1/18th of minute */
static const int HOUR = 1080;
static const int DAY = 24*HOUR;
static const int WEEK = 7*DAY;
#define M(h,p) ((h)*HOUR+p)
#define MONTH (DAY+M(12,793))

/**
 * @internal
 * no. of days in y years
 */
static int hebrewDaysElapsed( int y )
{
    int m, nm, dw, s, l;

    l = y * 7 + 1;  // no. of leap months
    m = y * 12 + l / 19;  // total no. of months
    l %= 19;
    nm = m * MONTH + M( 1 + 6, 779 ); // molad new year 3744 (16BC) + 6 hours
    s = m * 28 + nm / DAY - 2;

    nm %= WEEK;
    dw = nm / DAY;
    nm %= DAY;

    // special cases of Molad Zaken
    if ( (l < 12 && dw == 3 && nm >= M( 9 + 6, 204 )) ||
            (l < 7 && dw == 2 && nm >= M( 15 + 6, 589 )) ) {
        s++, dw++;
    }

    /* ADU */
    if ( dw == 1 || dw == 4 || dw == 6 ) {
        s++;
    }
    return s;
}

/**
 * @internal
 * true if long Cheshvan
 */
static int long_cheshvan( int year )
{
    QDate first, last;
    class h_date *gd;

    gd = hebrewToGregorian( year, 1, 1 );
    first.setYMD( gd->hd_year, gd->hd_mon + 1, gd->hd_day + 1 );

    gd = hebrewToGregorian( year + 1, 1, 1 );
    last.setYMD( gd->hd_year, gd->hd_mon + 1, gd->hd_day + 1 );

    return ( first.daysTo( last ) % 10 == 5 );
}

/**
 * @internal
 * true if short Kislev
 */
static int short_kislev( int year )
{
    QDate first, last;
    class h_date * gd;

    gd = hebrewToGregorian( year, 1, 1 );
    first.setYMD( gd->hd_year, gd->hd_mon + 1, gd->hd_day + 1 );

    gd = hebrewToGregorian( year + 1, 1, 1 );
    last.setYMD( gd->hd_year, gd->hd_mon + 1, gd->hd_day + 1 );

    return ( first.daysTo( last ) % 10 == 3 );
}

static bool is_leap_year( int year )
{
    return ( ( ( ( 7 * year ) + 1 ) % 19 ) < 7 );
}

// Ok
static class h_date *toHebrew( const QDate &date )
{
    class h_date *sd;

    sd = gregorianToHebrew( date.year(), date.month(), date.day() );
    ++sd->hd_mon;
    ++sd->hd_day;

    return sd;
}

//===========================================================================
//  End of old code
//===========================================================================

class KCalendarSystemHebrewPrivate
{
public:
    int characterOfYear( int year ) const;
    int julianDayOfTishri1( int year ) const;
    int yearOfJulianDay( int jd ) const;
    int daysPreceedingMonth( int character, int month ) const;

    /**
     * Gets the number of days in a month for a given date
     *
     * @param year given year
     * @param mon month number
     * @return number of days in month
     */
    int hndays( int year, int mon ) const;
};

int KCalendarSystemHebrewPrivate::daysPreceedingMonth( int character, int month ) const
{
    // FIXME move to Private class header as static when figure out how to do it without compiler errors :-)
    int daysPreceedingMonthTable[6][13] =
        {
            {0, 30, 59, 88, 117, 147, 176, 206, 235, 265, 294, 324, },
            {0, 30, 59, 89, 118, 148, 177, 207, 236, 266, 295, 325, },
            {0, 30, 60, 90, 119, 149, 178, 208, 237, 267, 296, 326, },
            {0, 30, 59, 88, 117, 147, 177, 206, 236, 265, 295, 324, 354},
            {0, 30, 59, 89, 118, 148, 178, 207, 237, 266, 296, 325, 355},
            {0, 30, 60, 90, 119, 149, 179, 209, 238, 267, 297, 326, 356},
        };

    if ( character >= 1 && character <= 6  && month >= 1 && month <= 13 ) {
        return daysPreceedingMonthTable[character-1][month-1];
    }
    return -911;
}

int KCalendarSystemHebrewPrivate::characterOfYear( int year ) const
{
    int jdTishri1ThisYear = julianDayOfTishri1( year );
    int jdTishri1NextYear = julianDayOfTishri1( year + 1 );
    int K = jdTishri1NextYear - jdTishri1ThisYear - 352
            - 27 * ( ( 7 * year + 13 ) % 19 ) / 12;
    return K;
}

// Hatcher formula G
int KCalendarSystemHebrewPrivate::julianDayOfTishri1( int year ) const
{
    // if calc of t overflows, have alternative formula for step
    long t = 31524 + 765433 * ( ( 235 * year - 234 ) / 19 );
    int d = t / 25920;
    int t1 = t % 25920;
    int w = 1 + ( d % 7 );
    int E = ( ( 7 * year + 13 ) % 19 ) / 12;
    int E1 = ( ( 7 * year + 6 ) % 19 ) / 12;
    if (  t1 >= 19940 ||
            ( t1 >= 9924 && w == 3 && E == 0 ) ||
            ( t1 >= 16788 && w == 2 && E == 0 && E1 == 1 ) ) {
        d = d + 1;
    }
    int jd = d + ( ( ( d + 5 ) % 7 ) % 2 ) + 347997;
    return jd;
}

// Hatcher formula H
int KCalendarSystemHebrewPrivate::yearOfJulianDay( int jd ) const
{
    long M = ( 25920 * ( jd - 347996 ) ) / 765433 + 1;
    int year = 19 * ( M / 235 ) + ( 19 * ( M % 2356 ) - 2 ) / 235 + 1;
    if ( julianDayOfTishri1( year ) > jd ) {
        year = year - 1;
    }
    return year;
}

int KCalendarSystemHebrewPrivate::hndays( int mon, int year ) const
{
    if ( mon == 6 && is_leap_year( year ) ) {
        mon = 13; /*Adar I*/
    } else if ( mon == 7 && is_leap_year( year ) ) {
        mon = 14; /*Adar II*/
    } else if ( mon > 7 && is_leap_year( year ) ) {
        mon--; //Because of Adar II
    }

    if( mon == 8 /*IYYAR*/ || mon == 10 /*TAMUZ*/ ||
            mon == 12 /*ELUL*/ || mon == 4 /*TEVET*/ ||
            mon == 14 /*ADAR 2*/ ||
            ( mon == 6 /*ADAR*/ && !is_leap_year( year ) ) ||
            ( mon ==  2 /*CHESHVAN*/ && !long_cheshvan( year ) ) ||
            ( mon == 3 /*KISLEV*/ && short_kislev( year ) ) ) {
        return 29;
    } else {
        return 30;
    }
}

// Ok
KCalendarSystemHebrew::KCalendarSystemHebrew( const KLocale * locale )
        : KCalendarSystem( locale ), d( new KCalendarSystemHebrewPrivate )
{
}

// Ok
KCalendarSystemHebrew::~KCalendarSystemHebrew()
{
    delete d;
}

QString KCalendarSystemHebrew::calendarType() const
{
    return QLatin1String( "hebrew" );
}

QDate KCalendarSystemHebrew::epoch() const
{
    return QDate::fromJulianDay( 347998 );
}

QDate KCalendarSystemHebrew::earliestValidDate() const
{
    return KCalendarSystem::earliestValidDate();
}

QDate KCalendarSystemHebrew::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widets support > 9999
    // Last day of Hebrew year 9999 is 9999-12-29
    // Which in Gregorian is 6239-09-25
    // Which is jd xxxx FIXME Find out jd and use that instead
    // Can't call setDate( 9999, 12, 29 ) as it creates circular reference!
    return QDate( 6239, 9, 25 );
}

bool KCalendarSystemHebrew::isValid( int y, int month, int day ) const
{
    // taken from setYMD below, adapted to use new methods
    if ( y < year( earliestValidDate() ) || y > year( latestValidDate() ) ) {
        return false;
    }
    if ( month < 1 || month > ( isLeapYear( y ) ? 13 : 12 ) ) {
        return false;
    }
    if ( day < 1 || day > d->hndays( month, y ) ) {
        return false;
    }
    return true;
}

bool KCalendarSystemHebrew::isValid( const QDate &date ) const
{
    return KCalendarSystem::isValid( date );
}

bool KCalendarSystemHebrew::setDate( QDate &date, int year, int month, int day ) const
{
    return KCalendarSystem::setDate( date, year, month, day );
}

// Deprecated
bool KCalendarSystemHebrew::setYMD( QDate & date, int y, int m, int day ) const
{
    // range checks
    // Removed deleted minValidYear and maxValidYear methods
    // Still use minimum of 1753 for now due to QDate using Julian calendar before then
    // Later change to following once new methods validated
    // if ( y < year( earliestValidDate() ) || y > year( latestValidDate() ) )
    if ( y < year( QDate( 1753, 1, 1 ) ) || y > 9999 ) {
        return false;
    }

    if( m < 1 || m > ( is_leap_year( y ) ? 13 : 12 ) ) {
        return false;
    }

    if( day < 1 || day > d->hndays( m, y ) ) {
        return false;
    }

    class h_date * gd = hebrewToGregorian( y, m, day );

    return date.setYMD( gd->hd_year, gd->hd_mon + 1, gd->hd_day + 1 );
}

int KCalendarSystemHebrew::year( const QDate &date ) const
{
    class h_date * sd = toHebrew( date );
    return sd->hd_year;
}

int KCalendarSystemHebrew::month( const QDate &date ) const
{
    class h_date * sd = toHebrew( date );

    int month = sd->hd_mon;
    if ( is_leap_year( sd->hd_year ) ) {
        if( month == 13 /*AdarI*/ ) {
            month = 6;
        } else if( month == 14 /*AdarII*/ ) {
            month = 7;
        } else if ( month > 6 && month < 13 ) {
            ++month;
        }
    }

    return month;
}

int KCalendarSystemHebrew::day( const QDate &date ) const
{
    class h_date * sd = toHebrew( date );

    return sd->hd_day;
}

QDate KCalendarSystemHebrew::addYears( const QDate &date, int nyears ) const
{
    QDate result = date;
    int y = year( date ) + nyears;

    setYMD( result, y, month( date ), day( date ) );

    return result;
}

QDate KCalendarSystemHebrew::addMonths( const QDate &date, int nmonths ) const
{
    QDate result = date;

    while ( nmonths > 0 ) {
        result = addDays( result, daysInMonth( result ) );
        --nmonths;
    }

    while ( nmonths < 0 ) {
        // get the number of days in the previous month to be consistent with
        // addMonths where nmonths > 0
        int nDaysInMonth = daysInMonth( addDays( result, -day( result ) ) );
        result = addDays( result, -nDaysInMonth );
        ++nmonths;
    }

    return result;
}

QDate KCalendarSystemHebrew::addDays( const QDate &date, int ndays ) const
{
    return date.addDays( ndays );
}

int KCalendarSystemHebrew::monthsInYear( const QDate &date ) const
{
    if ( is_leap_year( year( date ) ) ) {
        return 13;
    } else {
        return 12;
    }
}

int KCalendarSystemHebrew::weeksInYear( const QDate &date ) const
{
    return KCalendarSystem::weeksInYear( date );
}

// Ok
int KCalendarSystemHebrew::weeksInYear( int year ) const
{
    QDate temp;
    setYMD( temp, year, 1, 1 );  // don't pass an uninitialized QDate to
    // monthsInYear in the next call
    setYMD( temp, year, monthsInYear( temp ), d->hndays( monthsInYear( temp ), year ) );

    int nWeekNumber = weekNumber( temp );
    // last week belongs to next year
    if( nWeekNumber == 1 ) {
        temp = temp.addDays( -7 );
        nWeekNumber = weekNumber( temp );
    }

    return nWeekNumber;
}

int KCalendarSystemHebrew::daysInYear( const QDate &date ) const
{
    QDate first, last;

    setYMD( first, year( date ), 1, 1 ); // 1 Tishrey
    setYMD( last, year( date ) + 1, 1, 1 ); // 1 Tishrey the year later

    return first.daysTo( last );
}

int KCalendarSystemHebrew::daysInMonth( const QDate &date ) const
{
    return d->hndays( month( date ), year( date ) );
}

int KCalendarSystemHebrew::daysInWeek( const QDate &date ) const
{
    return KCalendarSystem::daysInWeek( date );
}

int KCalendarSystemHebrew::dayOfYear( const QDate &date ) const
{
    QDate first;

    setYMD( first, year( date ), 1, 1 );

    return first.daysTo( date ) + 1;
}

int KCalendarSystemHebrew::dayOfWeek( const QDate &date ) const
{
    class h_date * sd = toHebrew( date );
    if ( sd->hd_dw == 0 ) {
        return 7;
    } else {
        return ( sd->hd_dw );
    }
}

int KCalendarSystemHebrew::weekNumber( const QDate &date, int *yearNum ) const
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
    if ( weekDay1 > 4 /*Thursday*/ ) {
        firstDayWeek1 = addDays( firstDayWeek1 , 7 - weekDay1 + 1 ); // next monday
    }

    dayOfWeek1InYear = dayOfYear( firstDayWeek1 );

    // if our date in prev year's week
    if ( dayOfYear( date ) < dayOfWeek1InYear ) {
        if ( yearNum )
            * yearNum = y - 1;
        return weeksInYear( y - 1 );
    }

    // let's check if its last week belongs to next year
    setYMD( lastDayOfYear, y + 1, 1, 1 );
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

        week = firstDayWeek1.daysTo( date ) / 7 + 1;
    }

    return week;
}

bool KCalendarSystemHebrew::isLeapYear( int year ) const
{
    // from is_leap_year above
    return ( ( ( ( 7 * year ) + 1 ) % 19 ) < 7 );
}

bool KCalendarSystemHebrew::isLeapYear( const QDate &date ) const
{
    return QDate::isLeapYear( year( date ) );
}

// ### Fixme
// JPL Fix what?
// Ask translators for short fomats of month names!
QString KCalendarSystemHebrew::monthName( int month, int year, MonthNameFormat format ) const
{
    if ( month < 1 ) {
        return QString();
    }

    if ( is_leap_year( year ) && month > 13 ) {
        return QString();
    } else if ( month > 12 ) {
        return QString();
    }

    // We must map conversion algorithm month index to real index
    if( month == 6 && is_leap_year( year ) ) {
        month = 13; /*Adar I*/
    } else if ( month == 7 && is_leap_year( year ) ) {
        month = 14; /*Adar II*/
    } else if ( month > 7 && is_leap_year( year ) ) {
        month--; //Because of Adar II
    }

    if ( format == ShortNamePossessive || format == LongNamePossessive ) {
        switch( month ) {
        case 1:
            return ki18n( "of Tishrey" ).toString( locale() );
        case 2:
            return ki18n( "of Heshvan" ).toString( locale() );
        case 3:
            return ki18n( "of Kislev" ).toString( locale() );
        case 4:
            return ki18n( "of Tevet" ).toString( locale() );
        case 5:
            return ki18n( "of Shvat" ).toString( locale() );
        case 6:
            return ki18n( "of Adar" ).toString( locale() );
        case 7:
            return ki18n( "of Nisan" ).toString( locale() );
        case 8:
            return ki18n( "of Iyar" ).toString( locale() );
        case 9:
            return ki18n( "of Sivan" ).toString( locale() );
        case 10:
            return ki18n( "of Tamuz" ).toString( locale() );
        case 11:
            return ki18n( "of Av" ).toString( locale() );
        case 12:
            return ki18n( "of Elul" ).toString( locale() );
        case 13:
            return ki18n( "of Adar I" ).toString( locale() );
        case 14:
            return ki18n( "of Adar II" ).toString( locale() );
        default:
            return QString();
        }
    }

    switch( month ) {
    case 1:
        return ki18n( "Tishrey" ).toString( locale() );
    case 2:
        return ki18n( "Heshvan" ).toString( locale() );
    case 3:
        return ki18n( "Kislev" ).toString( locale() );
    case 4:
        return ki18n( "Tevet" ).toString( locale() );
    case 5:
        return ki18n( "Shvat" ).toString( locale() );
    case 6:
        return ki18n( "Adar" ).toString( locale() );
    case 7:
        return ki18n( "Nisan" ).toString( locale() );
    case 8:
        return ki18n( "Iyar" ).toString( locale() );
    case 9:
        return ki18n( "Sivan" ).toString( locale() );
    case 10:
        return ki18n( "Tamuz" ).toString( locale() );
    case 11:
        return ki18n( "Av" ).toString( locale() );
    case 12:
        return ki18n( "Elul" ).toString( locale() );
    case 13:
        return ki18n( "Adar I" ).toString( locale() );
    case 14:
        return ki18n( "Adar II" ).toString( locale() );
    default:
        return QString();
    }
}

QString KCalendarSystemHebrew::monthName( const QDate& date, MonthNameFormat format ) const
{
    return monthName( month( date ), year( date ), format );
}

QString KCalendarSystemHebrew::weekDayName( int weekDay, WeekDayNameFormat format ) const
{
    // Use Western day names for now as that's what the old version did,
    // but wouldn't it be better to use the right Hebrew names like Shabbat?
    // Could make it switchable by adding new enums to WeekDayFormat, e.g. ShortNameWestern?
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

QString KCalendarSystemHebrew::weekDayName( const QDate &date, WeekDayNameFormat format ) const
{
    return weekDayName( dayOfWeek( date ), format );
}

QString KCalendarSystemHebrew::yearString( const QDate &pDate, StringFormat format ) const
{
    QString sResult;

    // Only use hebrew numbers if the hebrew setting is selected
    if ( locale()->language() == QLatin1String( "he" ) ) {
        if ( format == ShortFormat ) {
            sResult = num2heb( year( pDate ), false );
        }
    } else {
        sResult = KCalendarSystem::yearString( pDate, format );
    }

    return sResult;
}

QString KCalendarSystemHebrew::monthString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::monthString( pDate, format );
}

QString KCalendarSystemHebrew::dayString( const QDate &pDate, StringFormat format ) const
{
    QString sResult;

    // Only use hebrew numbers if the hebrew setting is selected
    if ( locale()->language() == QLatin1String( "he" ) ) {
        sResult = num2heb( day( pDate ), false );
    } else {
        sResult = KCalendarSystem::dayString( pDate, format );
    }

    return sResult;
}

int KCalendarSystemHebrew::yearStringToInteger( const QString &sNum, int &iLength ) const
{
    int iResult;

    if ( locale()->language() == "he" ) {
        iResult = heb2num( sNum, iLength );
    } else {
        iResult = KCalendarSystem::yearStringToInteger( sNum, iLength );
    }

    if ( iResult < 1000 ) {
        iResult += 5000; // assume we're in the 6th millenium (y6k bug)
    }

    return iResult;
}

int KCalendarSystemHebrew::monthStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::monthStringToInteger( sNum, iLength );
}

int KCalendarSystemHebrew::dayStringToInteger( const QString &sNum, int &iLength ) const
{
    int iResult;

    if ( locale()->language() == "he" ) {
        iResult = heb2num( sNum, iLength );
    } else {
        iResult = KCalendarSystem::yearStringToInteger( sNum, iLength );
    }

    return iResult;
}

QString KCalendarSystemHebrew::formatDate( const QDate &date, KLocale::DateFormat format ) const
{
    return KCalendarSystem::formatDate( date, format );
}

QDate KCalendarSystemHebrew::readDate( const QString &str, bool *ok ) const
{
    return KCalendarSystem::readDate( str, ok );
}

QDate KCalendarSystemHebrew::readDate( const QString &intstr, const QString &fmt, bool *ok ) const
{
    return KCalendarSystem::readDate( intstr, fmt, ok );
}

QDate KCalendarSystemHebrew::readDate( const QString &str, KLocale::ReadDateFlags flags, bool *ok ) const
{
    return KCalendarSystem::readDate( str, flags, ok );
}

int KCalendarSystemHebrew::weekDayOfPray() const
{
    return 6; // saturday
}

int KCalendarSystemHebrew::weekStartDay() const
{
    return KCalendarSystem::weekStartDay();
}

bool KCalendarSystemHebrew::isLunar() const
{
    return false;
}

bool KCalendarSystemHebrew::isLunisolar() const
{
    return true;
}

bool KCalendarSystemHebrew::isSolar() const
{
    return false;
}

bool KCalendarSystemHebrew::isProleptic() const
{
    return false;
}

bool KCalendarSystemHebrew::julianDayToDate( int jd, int &year, int &month, int &day ) const
{
    if ( jd >= earliestValidDate().toJulianDay() && jd <= latestValidDate().toJulianDay() ) {
        // Hatcher formula I.  Fix me!
        year = d->yearOfJulianDay( jd );
        int jdTishri1ThisYear = d->julianDayOfTishri1( year );
        int dayInYear = jd - jdTishri1ThisYear + 1;
        int K = d->characterOfYear( year );
        int n = dayInYear / 30;
        if ( dayInYear > d->daysPreceedingMonth( K, n + 1 ) ) {
            month = n + 1;
        } else {
            if ( dayInYear > d->daysPreceedingMonth( K, n + 1 ) ) {
                month = n;
            } else {
                month = n - 1;
            }
        }
        day = dayInYear - d->daysPreceedingMonth( K, month );
        return true;
    }
    return false;
}

bool KCalendarSystemHebrew::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    // From Hatcher formula J.  Fix me!
    if ( isValid( year, month, day ) ) {
        int jdTishri1ThisYear = d->julianDayOfTishri1( year );
        jd = jdTishri1ThisYear
             + d->daysPreceedingMonth( d->characterOfYear( year ), month )
             + day - 1;
        return true;
    }
    return false;
}
