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
#include "kcalendarsystemgregorianproleptic_p.h"

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

    KCalendarSystemGregorianProleptic *gregorian;
};

// Shared d pointer base class definitions

KCalendarSystemIndianNationalPrivate::KCalendarSystemIndianNationalPrivate( KCalendarSystemIndianNational *q )
                                     :KCalendarSystemPrivate( q ),
                                      gregorian( new KCalendarSystemGregorianProleptic )
{
}

KCalendarSystemIndianNationalPrivate::~KCalendarSystemIndianNationalPrivate()
{
    delete gregorian;
}

void KCalendarSystemIndianNationalPrivate::initDefaultEraList()
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

    if ( month >= 2 || month <= 6  ) {
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
    //so add 78 years to get Gregorian year and call Gregorian implementation
    return gregorian->isLeapYear( year + 78 );
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
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemIndianNational::KCalendarSystemIndianNational( const KSharedConfig::Ptr config, const KLocale *locale )
                             : KCalendarSystem( *new KCalendarSystemIndianNationalPrivate( this ), config, locale ),
                               dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemIndianNational::KCalendarSystemIndianNational( KCalendarSystemIndianNationalPrivate &dd,
                                                              const KSharedConfig::Ptr config,
                                                              const KLocale *locale )
                             : KCalendarSystem( dd, config, locale ),
                               dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
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
    Q_D( const KCalendarSystemIndianNational );

    // The calendar is closely synchronized to the Gregorian Calendar, always starting on the same day
    // We can use this and the regular sequence of days in months to do a simple conversion by finding
    // what day in the Gregorian year the Julian Day number is, converting this to the day in the
    // Indian year and subtracting off the required number of months and days to get the final date

    // Calculate the Day of Year in the Gregorian calendar
    // Need to use julianDayToDate() and dateToJulianDay() to calculate instead of QDate::fromJulianDay()
    // and dayOfYear() to avoid the Gregorian year > 9999 validation check
    int gregorianYear, gregorianMonth, gregorianDay, jdGregorianFirstDayOfYear;
    d->gregorian->julianDayToDate( jd, gregorianYear, gregorianMonth, gregorianDay );
    d->gregorian->dateToJulianDay( gregorianYear, 1, 1, jdGregorianFirstDayOfYear );
    int gregorianDayOfYear = jd - jdGregorianFirstDayOfYear + 1;

    // There is a fixed 78 year difference between year numbers, but the years do not exactly match up,
    // there is a fixed 80 day difference between the first day of the year, if the Gregorian day of
    // the year is 80 or less then the equivalent Indian day actually falls in the preceding  year
    if ( gregorianDayOfYear > 80 ) {
        year = gregorianYear - 78;
    } else {
        year = gregorianYear - 79;
    }

    // If it is a leap year then the first month has 31 days, otherwise 30.
    int daysInMonth1;
    if ( isLeapYear( year ) ) {
        daysInMonth1 = 31;
    } else {
        daysInMonth1 = 30;
    }

    // The Indian year always starts 80 days after the Gregorian year, calculate the Indian day of
    // the year, taking into account if it falls into the previous Gregorian year
    int indianDayOfYear;
    if ( gregorianDayOfYear > 80 ) {
        indianDayOfYear = gregorianDayOfYear - 80;
    } else {
        indianDayOfYear = gregorianDayOfYear + ( daysInMonth1 + ( 5 * 31 ) + ( 6 * 30 ) ) - 80;
    }

    // Then simply remove the whole months from the day of the year and you are left with the day of month
    if ( indianDayOfYear <= daysInMonth1 ) {
        month = 1;
        day = indianDayOfYear;
    } else if ( indianDayOfYear <= ( daysInMonth1 + ( 5 * 31 ) ) ) {
        month = ( ( indianDayOfYear - daysInMonth1 - 1 ) / 31 ) + 2;
        day = indianDayOfYear - daysInMonth1 - ( ( month - 2 ) * 31 );
    } else {
        month = ( ( indianDayOfYear - daysInMonth1 - ( 5 * 31 ) - 1 ) / 30 ) + 7;
        day = indianDayOfYear - daysInMonth1 - ( 5 * 31 ) - ( ( month - 7 ) * 30 );
    }

    return true;
}

bool KCalendarSystemIndianNational::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    Q_D( const KCalendarSystemIndianNational );

    // The calendar is closely synchronized to the Gregorian Calendar, always starting on the same day
    // We can use this and the regular sequence of days in months to do a simple conversion by finding
    // the Julian Day number of the first day of the year and adding on the required number of months
    // and days to get the final Julian Day number

    int jdFirstDayOfYear;
    int daysInMonth1;

    // Calculate the jd of 1 Chaitra for this year and how many days are in Chaitra this year
    // If a Leap Year, then 1 Chaitra == 21 March of the Gregorian year and Chaitra has 31 days
    // If not a Leap Year, then 1 Chaitra == 22 March of the Gregorian year and Chaitra has 30 days
    // Need to use dateToJulianDay() to calculate instead of setDate() to avoid the year 9999 validation
    if ( isLeapYear( year ) ) {
        d->gregorian->dateToJulianDay( year + 78, 3, 21, jdFirstDayOfYear );
        daysInMonth1 = 31;
    } else {
        d->gregorian->dateToJulianDay( year + 78, 3, 22, jdFirstDayOfYear );
        daysInMonth1 = 30;
    }

    // Add onto the jd of the first day of the year the number of days required
    // Calculate the number of days in the months before the required month
    // Then add on the required days
    // The first month has 30 or 31 days depending on if it is a Leap Year (determined above)
    // The second to sixth months have 31 days each
    // The seventh to twelth months have 30 days each
    // Note: could be expressed more efficiently, but I think this is clearer
    if ( month == 1) {
        jd = jdFirstDayOfYear             // Start with jd of first day of year
           + day                          // Add on required day in required month
           - 1;                           // Adjust for fact we double count 1 Chaitra
    } else if ( month <= 6 ){
        jd = jdFirstDayOfYear             // Start with jd of first day of year
           + daysInMonth1                 // Add on days in month 1
           + ( ( month - 2 ) * 31 )       // Add on days after month 1 up to month before required month
           + day                          // Add on required day in required month
           - 1;                           // Adjust for fact we double count 1 Chaitra
    } else { // month > 6
        jd = jdFirstDayOfYear             // Start with jd of first day of year
           + daysInMonth1                 // Add on days in month 1
           + ( 5 * 31 )                   // Add on days in months 2 to 6
           + ( ( month - 7 ) * 30 )       // Add on days after month 6 up to month before required month
           + day                          // Add on required day in required month
           - 1;                           // Adjust for fact we double count 1 Chaitra
    }

    return true;
}
