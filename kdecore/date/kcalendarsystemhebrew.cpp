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

#include "kcalendarsystemhebrew_p.h"
#include "kcalendarsystemprivate_p.h"

#include "kdebug.h"
#include "klocale.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

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

// Ok
static class h_date *toHebrew( const QDate &date )
{
    class h_date *sd;

    sd = gregorianToHebrew( date.year(), date.month(), date.day() );
    ++sd->hd_mon;
    ++sd->hd_day;

    return sd;
}

class KCalendarSystemHebrewPrivate : public KCalendarSystemPrivate
{
public:
    KCalendarSystemHebrewPrivate( KCalendarSystemHebrew *q ) : KCalendarSystemPrivate( q )
    {
    }

    virtual ~KCalendarSystemHebrewPrivate()
    {
    }

    virtual bool isLeapYear( int year ) const;

    virtual int monthNumberToMonthIndex( int year, int month ) const;

    virtual int monthsInYear( int year ) const;

    virtual int daysInMonth( int year, int month ) const;
};

bool KCalendarSystemHebrewPrivate::isLeapYear( int year ) const
{
    return ( ( ( ( 7 * year ) + 1 ) % 19 ) < 7 );
}

int KCalendarSystemHebrewPrivate::monthNumberToMonthIndex( int year, int month ) const
{
    if ( q->isLeapYear( year ) ) {
        if ( month == 6 ) {
            return 13;        // Adar I
        } else if ( month == 7 ) {
            return 14;        // Adar II
        } else if ( month > 7 ) {
            return month - 1; // Because of Adar II
        }
    }

    return month;
}

int KCalendarSystemHebrewPrivate::monthsInYear( int year ) const
{
    if ( isLeapYear( year ) ) {
        return 13;
    } else {
        return 12;
    }
}

int KCalendarSystemHebrewPrivate::daysInMonth( int year, int month ) const
{
    int mi = monthNumberToMonthIndex( year, month );

    if ( mi == 2 && long_cheshvan( year ) ) {
        return 30;
    }

    if ( mi == 3 && short_kislev( year ) ) {
        return 29;
    }

    if ( mi % 2 == 0 ) {
        return 29;
    } else {
        return 30;
    }
}

KCalendarSystemHebrew::KCalendarSystemHebrew( const KLocale * locale )
                      : KCalendarSystem( *new KCalendarSystemHebrewPrivate( this ), locale ),
                        dont_use( 0 )
{
    setMaxMonthsInYear(13);
}

KCalendarSystemHebrew::KCalendarSystemHebrew( KCalendarSystemHebrewPrivate &dd, const KLocale * locale )
                      : KCalendarSystem( dd, locale ),
                        dont_use( 0 )
{
    setMaxMonthsInYear(13);
}

KCalendarSystemHebrew::~KCalendarSystemHebrew()
{
    delete dont_use;
}

QString KCalendarSystemHebrew::calendarType() const
{
    return QLatin1String( "hebrew" );
}

QDate KCalendarSystemHebrew::epoch() const
{
    // Hebrew 0001-01-01 (Gregorian -3760-09-07, Julian -3761-10-07)
    return QDate::fromJulianDay( 347998 );
}

QDate KCalendarSystemHebrew::earliestValidDate() const
{
    // Current formulas using direct Gregorian <-> Hebrew conversion using Qt
    // will return invalid results prior to the Gregorian switchover in 1582
    // Next valid Hebrew year starts 5344-01-01 (Gregorian 1583-09-17)
    return QDate::fromJulianDay( 2299498 );
}

QDate KCalendarSystemHebrew::latestValidDate() const
{
    // Testing shows current formulas only work up to 8119-13-29 (Gregorian 4359-10-07)
    return QDate::fromJulianDay( 3313431 );
}

bool KCalendarSystemHebrew::isValid( int year, int month, int day ) const
{
    Q_D( const KCalendarSystemHebrew );

    if ( year < 5344 || year > 8119 ) {
        return false;
    }

    if ( month < 1 || month > d->monthsInYear( year ) ) {
        return false;
    }

    if ( day < 1 || day > d->daysInMonth( year, month ) ) {
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
    return setYMD( date, year, month, day );
}

// Deprecated
bool KCalendarSystemHebrew::setYMD( QDate &date, int year, int month, int day ) const
{
    if ( !isValid( year, month, day) ) {
        return false;
    }

    class h_date * gd = hebrewToGregorian( year, month, day );

    return date.setYMD( gd->hd_year, gd->hd_mon + 1, gd->hd_day + 1 );
}

int KCalendarSystemHebrew::year( const QDate &date ) const
{
    if ( !isValid( date ) ) {
        return -1;
    }

    class h_date * sd = toHebrew( date );
    return sd->hd_year;
}

int KCalendarSystemHebrew::month( const QDate &date ) const
{
    if ( !isValid( date ) ) {
        return -1;
    }

    class h_date * sd = toHebrew( date );

    int month = sd->hd_mon;
    if ( isLeapYear( sd->hd_year ) ) {
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
    if ( !isValid( date ) ) {
        return -1;
    }

    class h_date * sd = toHebrew( date );

    return sd->hd_day;
}

QDate KCalendarSystemHebrew::addYears( const QDate &date, int nyears ) const
{
    if ( !isValid( date ) ) {
        return QDate();
    }

    QDate result = date;
    int y = year( date ) + nyears;

    setYMD( result, y, month( date ), day( date ) );

    return result;
}

QDate KCalendarSystemHebrew::addMonths( const QDate &date, int nmonths ) const
{
    if ( !isValid( date ) ) {
        return QDate();
    }

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
    if ( !isValid( date ) ) {
        return QDate();
    }

    return date.addDays( ndays );
}

int KCalendarSystemHebrew::monthsInYear( const QDate &date ) const
{
    Q_D( const KCalendarSystemHebrew );

    if ( !isValid( date ) ) {
        return -1;
    }

    return d->monthsInYear( year( date ) );
}

int KCalendarSystemHebrew::weeksInYear( const QDate &date ) const
{
    return KCalendarSystem::weeksInYear( date );
}

int KCalendarSystemHebrew::weeksInYear( int year ) const
{
    return KCalendarSystem::weeksInYear( year );
}

int KCalendarSystemHebrew::daysInYear( const QDate &date ) const
{
    if ( !isValid( date ) ) {
        return -1;
    }

    int y = year( date );

    // Last valid year, as setYmd() won't allow calculating following year
    if ( y == 8119 ) {
        return 385;
    }

    QDate first, last;
    setYMD( first, y, 1, 1 ); // 1 Tishrey
    setYMD( last, y + 1, 1, 1 ); // 1 Tishrey the year later

    return first.daysTo( last );
}

int KCalendarSystemHebrew::daysInMonth( const QDate &date ) const
{
    Q_D( const KCalendarSystemHebrew );

    if ( !isValid( date ) ) {
        return -1;
    }

    return d->daysInMonth( year( date ), month( date ) );
}

int KCalendarSystemHebrew::daysInWeek( const QDate &date ) const
{
    Q_UNUSED( date );

    return 7;
}

int KCalendarSystemHebrew::dayOfYear( const QDate &date ) const
{
    if ( !isValid( date ) ) {
        return -1;
    }

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
    Q_D( const KCalendarSystemHebrew );
    return d->isLeapYear( year );
}

bool KCalendarSystemHebrew::isLeapYear( const QDate &date ) const
{
    return KCalendarSystem::isLeapYear( date );
}

// ### Fixme
// JPL Fix what?
// Ask translators for short fomats of month names!
QString KCalendarSystemHebrew::monthName( int month, int year, MonthNameFormat format ) const
{
    Q_D( const KCalendarSystemHebrew );

    if ( month < 1 || month > d->monthsInYear( year ) ) {
        return QString();
    }

    // We must map month number to month index
    int monthIndex = d->monthNumberToMonthIndex( year, month );

    if ( format == ShortNamePossessive || format == LongNamePossessive ) {
        switch( monthIndex ) {
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

    switch( monthIndex ) {
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
    return 6; // Saturday
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
    Q_UNUSED(jd);
    year = -1;
    month = -1;
    day = -1;
    return false;
}

bool KCalendarSystemHebrew::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    Q_UNUSED(year);
    Q_UNUSED(month);
    Q_UNUSED(day);
    jd = -1;
    return false;
}
