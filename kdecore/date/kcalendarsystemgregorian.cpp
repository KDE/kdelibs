/*
    Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
    Copyright (c) 2002-2003 Hans Petter Bieker <bieker@kde.org>
    Copyright 2007, 2010 John Layt <john@layt.net>

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

#include "kcalendarsystemgregorian_p.h"
#include "kcalendarsystemprivate_p.h"
#include "kcalendarera_p.h"

#include "kdebug.h"
#include "klocale.h"
#include "kglobal.h"
#include "kconfiggroup.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

class KCalendarSystemGregorianPrivate : public KCalendarSystemPrivate
{
public:
    explicit KCalendarSystemGregorianPrivate( KCalendarSystemGregorian *q );

    virtual ~KCalendarSystemGregorianPrivate();

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
    virtual QString monthName( int month, int year, KLocale::DateTimeComponentFormat format, bool possessive ) const;
    virtual QString weekDayName( int weekDay, KLocale::DateTimeComponentFormat format ) const;

    bool m_useCommonEra;
};

// Shared d pointer implementations

KCalendarSystemGregorianPrivate::KCalendarSystemGregorianPrivate( KCalendarSystemGregorian *q )
                                :KCalendarSystemPrivate( q ),
                                 m_useCommonEra( false )

{
}

KCalendarSystemGregorianPrivate::~KCalendarSystemGregorianPrivate()
{
}

KLocale::CalendarSystem KCalendarSystemGregorianPrivate::calendarSystem() const
{
    return KLocale::QDateCalendar;
}

void KCalendarSystemGregorianPrivate::loadDefaultEraList()
{
    QString name, shortName, format;

    KConfigGroup lcg( config(), QString::fromLatin1( "Locale" ) );
    KConfigGroup cg = lcg.group( QString::fromLatin1( "KCalendarSystem %1" ).arg( q->calendarType() ) );
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

int KCalendarSystemGregorianPrivate::monthsInYear( int year ) const
{
    Q_UNUSED( year )
    return 12;
}

int KCalendarSystemGregorianPrivate::daysInMonth( int year, int month ) const
{
    QDate tempDate( year, month, 1 );
    return tempDate.daysInMonth();
}

int KCalendarSystemGregorianPrivate::daysInYear( int year ) const
{
    QDate tempDate( year, 1, 1 );
    return tempDate.daysInYear();
}

int KCalendarSystemGregorianPrivate::daysInWeek() const
{
    return 7;
}

bool KCalendarSystemGregorianPrivate::isLeapYear( int year ) const
{
    return QDate::isLeapYear( year );
}

bool KCalendarSystemGregorianPrivate::hasLeapMonths() const
{
    return false;
}

bool KCalendarSystemGregorianPrivate::hasYearZero() const
{
    return false;
}

int KCalendarSystemGregorianPrivate::maxDaysInWeek() const
{
    return 7;
}

int KCalendarSystemGregorianPrivate::maxMonthsInYear() const
{
    return 12;
}

int KCalendarSystemGregorianPrivate::earliestValidYear() const
{
    return -4712;
}

int KCalendarSystemGregorianPrivate::latestValidYear() const
{
    return 9999;
}

QString KCalendarSystemGregorianPrivate::monthName( int month, int year, KLocale::DateTimeComponentFormat format, bool possessive ) const
{
    Q_UNUSED( year );

    if ( format == KLocale::NarrowName ) {
        switch ( month ) {
        case 1:
            return ki18nc( "Gregorian month 1 - KLocale::NarrowName",  "J" ).toString( locale() );
        case 2:
            return ki18nc( "Gregorian month 2 - KLocale::NarrowName",  "F" ).toString( locale() );
        case 3:
            return ki18nc( "Gregorian month 3 - KLocale::NarrowName",  "M" ).toString( locale() );
        case 4:
            return ki18nc( "Gregorian month 4 - KLocale::NarrowName",  "A" ).toString( locale() );
        case 5:
            return ki18nc( "Gregorian month 5 - KLocale::NarrowName",  "M" ).toString( locale() );
        case 6:
            return ki18nc( "Gregorian month 6 - KLocale::NarrowName",  "J" ).toString( locale() );
        case 7:
            return ki18nc( "Gregorian month 7 - KLocale::NarrowName",  "J" ).toString( locale() );
        case 8:
            return ki18nc( "Gregorian month 8 - KLocale::NarrowName",  "A" ).toString( locale() );
        case 9:
            return ki18nc( "Gregorian month 9 - KLocale::NarrowName",  "S" ).toString( locale() );
        case 10:
            return ki18nc( "Gregorian month 10 - KLocale::NarrowName", "O" ).toString( locale() );
        case 11:
            return ki18nc( "Gregorian month 11 - KLocale::NarrowName", "N" ).toString( locale() );
        case 12:
            return ki18nc( "Gregorian month 12 - KLocale::NarrowName", "D" ).toString( locale() );
        default:
            return QString();
        }
    }

    if ( format == KLocale::ShortName && possessive ) {
        switch ( month ) {
        case 1:
            return ki18nc( "Gregorian month 1 - KLocale::ShortName Possessive",  "of Jan" ).toString( locale() );
        case 2:
            return ki18nc( "Gregorian month 2 - KLocale::ShortName Possessive",  "of Feb" ).toString( locale() );
        case 3:
            return ki18nc( "Gregorian month 3 - KLocale::ShortName Possessive",  "of Mar" ).toString( locale() );
        case 4:
            return ki18nc( "Gregorian month 4 - KLocale::ShortName Possessive",  "of Apr" ).toString( locale() );
        case 5:
            return ki18nc( "Gregorian month 5 - KLocale::ShortName Possessive",  "of May" ).toString( locale() );
        case 6:
            return ki18nc( "Gregorian month 6 - KLocale::ShortName Possessive",  "of Jun" ).toString( locale() );
        case 7:
            return ki18nc( "Gregorian month 7 - KLocale::ShortName Possessive",  "of Jul" ).toString( locale() );
        case 8:
            return ki18nc( "Gregorian month 8 - KLocale::ShortName Possessive",  "of Aug" ).toString( locale() );
        case 9:
            return ki18nc( "Gregorian month 9 - KLocale::ShortName Possessive",  "of Sep" ).toString( locale() );
        case 10:
            return ki18nc( "Gregorian month 10 - KLocale::ShortName Possessive", "of Oct" ).toString( locale() );
        case 11:
            return ki18nc( "Gregorian month 11 - KLocale::ShortName Possessive", "of Nov" ).toString( locale() );
        case 12:
            return ki18nc( "Gregorian month 12 - KLocale::ShortName Possessive", "of Dec" ).toString( locale() );
        default:
            return QString();
        }
    }

    if ( format == KLocale::ShortName && !possessive ) {
        switch ( month ) {
        case 1:
            return ki18nc( "Gregorian month 1 - KLocale::ShortName",  "Jan" ).toString( locale() );
        case 2:
            return ki18nc( "Gregorian month 2 - KLocale::ShortName",  "Feb" ).toString( locale() );
        case 3:
            return ki18nc( "Gregorian month 3 - KLocale::ShortName",  "Mar" ).toString( locale() );
        case 4:
            return ki18nc( "Gregorian month 4 - KLocale::ShortName",  "Apr" ).toString( locale() );
        case 5:
            return ki18nc( "Gregorian month 5 - KLocale::ShortName",  "May" ).toString( locale() );
        case 6:
            return ki18nc( "Gregorian month 6 - KLocale::ShortName",  "Jun" ).toString( locale() );
        case 7:
            return ki18nc( "Gregorian month 7 - KLocale::ShortName",  "Jul" ).toString( locale() );
        case 8:
            return ki18nc( "Gregorian month 8 - KLocale::ShortName",  "Aug" ).toString( locale() );
        case 9:
            return ki18nc( "Gregorian month 9 - KLocale::ShortName",  "Sep" ).toString( locale() );
        case 10:
            return ki18nc( "Gregorian month 10 - KLocale::ShortName", "Oct" ).toString( locale() );
        case 11:
            return ki18nc( "Gregorian month 11 - KLocale::ShortName", "Nov" ).toString( locale() );
        case 12:
            return ki18nc( "Gregorian month 12 - KLocale::ShortName", "Dec" ).toString( locale() );
        default:
            return QString();
        }
    }

    if ( format == KLocale::LongName && possessive ) {
        switch ( month ) {
        case 1:
            return ki18nc( "Gregorian month 1 - KLocale::LongName Possessive",  "of January"   ).toString( locale() );
        case 2:
            return ki18nc( "Gregorian month 2 - KLocale::LongName Possessive",  "of February"  ).toString( locale() );
        case 3:
            return ki18nc( "Gregorian month 3 - KLocale::LongName Possessive",  "of March"     ).toString( locale() );
        case 4:
            return ki18nc( "Gregorian month 4 - KLocale::LongName Possessive",  "of April"     ).toString( locale() );
        case 5:
            return ki18nc( "Gregorian month 5 - KLocale::LongName Possessive",  "of May"       ).toString( locale() );
        case 6:
            return ki18nc( "Gregorian month 6 - KLocale::LongName Possessive",  "of June"      ).toString( locale() );
        case 7:
            return ki18nc( "Gregorian month 7 - KLocale::LongName Possessive",  "of July"      ).toString( locale() );
        case 8:
            return ki18nc( "Gregorian month 8 - KLocale::LongName Possessive",  "of August"    ).toString( locale() );
        case 9:
            return ki18nc( "Gregorian month 9 - KLocale::LongName Possessive",  "of September" ).toString( locale() );
        case 10:
            return ki18nc( "Gregorian month 10 - KLocale::LongName Possessive", "of October"   ).toString( locale() );
        case 11:
            return ki18nc( "Gregorian month 11 - KLocale::LongName Possessive", "of November"  ).toString( locale() );
        case 12:
            return ki18nc( "Gregorian month 12 - KLocale::LongName Possessive", "of December"  ).toString( locale() );
        default:
            return QString();
        }
    }

    // Default to LongName
    switch ( month ) {
    case 1:
        return ki18nc( "Gregorian month 1 - KLocale::LongName",  "January"   ).toString( locale() );
    case 2:
        return ki18nc( "Gregorian month 2 - KLocale::LongName",  "February"  ).toString( locale() );
    case 3:
        return ki18nc( "Gregorian month 3 - KLocale::LongName",  "March"     ).toString( locale() );
    case 4:
        return ki18nc( "Gregorian month 4 - KLocale::LongName",  "April"     ).toString( locale() );
    case 5:
        return ki18nc( "Gregorian month 5 - KLocale::LongName",  "May"       ).toString( locale() );
    case 6:
        return ki18nc( "Gregorian month 6 - KLocale::LongName",  "June"      ).toString( locale() );
    case 7:
        return ki18nc( "Gregorian month 7 - KLocale::LongName",  "July"      ).toString( locale() );
    case 8:
        return ki18nc( "Gregorian month 8 - KLocale::LongName",  "August"    ).toString( locale() );
    case 9:
        return ki18nc( "Gregorian month 9 - KLocale::LongName",  "September" ).toString( locale() );
    case 10:
        return ki18nc( "Gregorian month 10 - KLocale::LongName", "October"   ).toString( locale() );
    case 11:
        return ki18nc( "Gregorian month 11 - KLocale::LongName", "November"  ).toString( locale() );
    case 12:
        return ki18nc( "Gregorian month 12 - KLocale::LongName", "December"  ).toString( locale() );
    default:
        return QString();
    }
}

QString KCalendarSystemGregorianPrivate::weekDayName( int weekDay, KLocale::DateTimeComponentFormat format ) const
{
    if ( format == KLocale::NarrowName ) {
        switch ( weekDay ) {
        case 1:
            return ki18nc( "Gregorian weekday 1 - KLocale::NarrowName ", "M" ).toString( locale() );
        case 2:
            return ki18nc( "Gregorian weekday 2 - KLocale::NarrowName ", "T" ).toString( locale() );
        case 3:
            return ki18nc( "Gregorian weekday 3 - KLocale::NarrowName ", "W" ).toString( locale() );
        case 4:
            return ki18nc( "Gregorian weekday 4 - KLocale::NarrowName ", "T" ).toString( locale() );
        case 5:
            return ki18nc( "Gregorian weekday 5 - KLocale::NarrowName ", "F" ).toString( locale() );
        case 6:
            return ki18nc( "Gregorian weekday 6 - KLocale::NarrowName ", "S" ).toString( locale() );
        case 7:
            return ki18nc( "Gregorian weekday 7 - KLocale::NarrowName ", "S" ).toString( locale() );
        default:
            return QString();
        }
    }

    if ( format == KLocale::ShortName  || format == KLocale:: ShortNumber ) {
        switch ( weekDay ) {
        case 1:
            return ki18nc( "Gregorian weekday 1 - KLocale::ShortName", "Mon" ).toString( locale() );
        case 2:
            return ki18nc( "Gregorian weekday 2 - KLocale::ShortName", "Tue" ).toString( locale() );
        case 3:
            return ki18nc( "Gregorian weekday 3 - KLocale::ShortName", "Wed" ).toString( locale() );
        case 4:
            return ki18nc( "Gregorian weekday 4 - KLocale::ShortName", "Thu" ).toString( locale() );
        case 5:
            return ki18nc( "Gregorian weekday 5 - KLocale::ShortName", "Fri" ).toString( locale() );
        case 6:
            return ki18nc( "Gregorian weekday 6 - KLocale::ShortName", "Sat" ).toString( locale() );
        case 7:
            return ki18nc( "Gregorian weekday 7 - KLocale::ShortName", "Sun" ).toString( locale() );
        default: return QString();
        }
    }

    switch ( weekDay ) {
    case 1:
        return ki18nc( "Gregorian weekday 1 - KLocale::LongName", "Monday"    ).toString( locale() );
    case 2:
        return ki18nc( "Gregorian weekday 2 - KLocale::LongName", "Tuesday"   ).toString( locale() );
    case 3:
        return ki18nc( "Gregorian weekday 3 - KLocale::LongName", "Wednesday" ).toString( locale() );
    case 4:
        return ki18nc( "Gregorian weekday 4 - KLocale::LongName", "Thursday"  ).toString( locale() );
    case 5:
        return ki18nc( "Gregorian weekday 5 - KLocale::LongName", "Friday"    ).toString( locale() );
    case 6:
        return ki18nc( "Gregorian weekday 6 - KLocale::LongName", "Saturday"  ).toString( locale() );
    case 7:
        return ki18nc( "Gregorian weekday 7 - KLocale::LongName", "Sunday"    ).toString( locale() );
    default:
        return QString();
    }
}


KCalendarSystemGregorian::KCalendarSystemGregorian( const KLocale *locale )
                        : KCalendarSystem( *new KCalendarSystemGregorianPrivate( this ), KSharedConfig::Ptr(), locale ),
                          dont_use( 0 )
{
    d_ptr->loadConfig( calendarType() );
}

KCalendarSystemGregorian::KCalendarSystemGregorian( const KSharedConfig::Ptr config, const KLocale *locale )
                        : KCalendarSystem( *new KCalendarSystemGregorianPrivate( this ), config, locale ),
                          dont_use( 0 )
{
    d_ptr->loadConfig( calendarType() );
}

KCalendarSystemGregorian::KCalendarSystemGregorian( KCalendarSystemGregorianPrivate &dd, const KSharedConfig::Ptr config, const KLocale *locale )
                        : KCalendarSystem( dd, config, locale ),
                          dont_use( 0 )
{
    d_ptr->loadConfig( calendarType() );
}

KCalendarSystemGregorian::~KCalendarSystemGregorian()
{
    delete dont_use;
}

QString KCalendarSystemGregorian::calendarType() const
{
    return QLatin1String( "gregorian" );
}

QDate KCalendarSystemGregorian::epoch() const
{
    // 1 Jan 1 AD in Julian
    return QDate::fromJulianDay( 1721424 );
}

QDate KCalendarSystemGregorian::earliestValidDate() const
{
    // 1 Jan 4712 BC, no year zero, cant be 4713BC due to error in QDate that day 0 is not valid
    // and we really need the first in each year to be valid for the date maths
    return QDate::fromJulianDay( 366 );
}

QDate KCalendarSystemGregorian::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widets support > 9999
    // In Gregorian this is 9999-12-31, which is  is jd 5373484
    // Can't call setDate( 9999, 12, 31 ) as it creates circular reference!
    return QDate::fromJulianDay( 5373484 );
}

bool KCalendarSystemGregorian::isValid( int year, int month, int day ) const
{
    // Limit to max year 9999 for now, QDate allows to be greater
    if ( year <= 9999 ) {
        return QDate::isValid( year, month, day );
    }

    return false;
}

bool KCalendarSystemGregorian::isValid( const QDate &date ) const
{
    return KCalendarSystem::isValid( date );
}

bool KCalendarSystemGregorian::setDate( QDate &date, int year, int month, int day ) const
{
    return KCalendarSystem::setDate( date, year, month, day );
}

// Deprecated
bool KCalendarSystemGregorian::setYMD( QDate &date, int y, int m, int d ) const
{
    return KCalendarSystem::setDate( date, y, m, d );
}

int KCalendarSystemGregorian::year( const QDate &date ) const
{
    return date.year();
}

int KCalendarSystemGregorian::month( const QDate &date ) const
{
    return date.month();
}

int KCalendarSystemGregorian::day( const QDate &date ) const
{
    return date.day();
}

QDate KCalendarSystemGregorian::addYears( const QDate &date, int nyears ) const
{
    return KCalendarSystem::addYears( date, nyears );
}

QDate KCalendarSystemGregorian::addMonths( const QDate &date, int nmonths ) const
{
    return KCalendarSystem::addMonths( date, nmonths );
}

QDate KCalendarSystemGregorian::addDays( const QDate &date, int ndays ) const
{
    return KCalendarSystem::addDays( date, ndays );
}

int KCalendarSystemGregorian::monthsInYear( const QDate &date ) const
{
    return KCalendarSystem::monthsInYear( date );
}

int KCalendarSystemGregorian::weeksInYear( const QDate &date ) const
{
    return KCalendarSystem::weeksInYear( date );
}

int KCalendarSystemGregorian::weeksInYear( int year ) const
{
    return KCalendarSystem::weeksInYear( year );
}

int KCalendarSystemGregorian::daysInYear( const QDate &date ) const
{
    return date.daysInYear();
}

int KCalendarSystemGregorian::daysInMonth( const QDate &date ) const
{
    return date.daysInMonth();
}

int KCalendarSystemGregorian::daysInWeek( const QDate &date ) const
{
    return KCalendarSystem::daysInWeek( date );
}

int KCalendarSystemGregorian::dayOfYear( const QDate &date ) const
{
    return date.dayOfYear();
}

int KCalendarSystemGregorian::dayOfWeek( const QDate &date ) const
{
    return date.dayOfWeek();
}

int KCalendarSystemGregorian::weekNumber( const QDate &date, int * yearNum ) const
{
    return KCalendarSystem::weekNumber( date, yearNum );
}

bool KCalendarSystemGregorian::isLeapYear( int year ) const
{
    return QDate::isLeapYear( year );
}

bool KCalendarSystemGregorian::isLeapYear( const QDate &date ) const
{
    return QDate::isLeapYear( date.year() );
}

QString KCalendarSystemGregorian::monthName( int month, int year, MonthNameFormat format ) const
{
    return KCalendarSystem::monthName( month, year, format );
}

QString KCalendarSystemGregorian::monthName( const QDate &date, MonthNameFormat format ) const
{
    return KCalendarSystem::monthName( date, format );
}

QString KCalendarSystemGregorian::weekDayName( int weekDay, WeekDayNameFormat format ) const
{
    return KCalendarSystem::weekDayName( weekDay, format );
}

QString KCalendarSystemGregorian::weekDayName( const QDate &date, WeekDayNameFormat format ) const
{
    return KCalendarSystem::weekDayName( date, format );
}

QString KCalendarSystemGregorian::yearString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::yearString( pDate, format );
}

QString KCalendarSystemGregorian::monthString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::monthString( pDate, format );
}

QString KCalendarSystemGregorian::dayString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystem::dayString( pDate, format );
}

int KCalendarSystemGregorian::yearStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::yearStringToInteger( sNum, iLength );
}

int KCalendarSystemGregorian::monthStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::monthStringToInteger( sNum, iLength );
}

int KCalendarSystemGregorian::dayStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystem::dayStringToInteger( sNum, iLength );
}

QString KCalendarSystemGregorian::formatDate( const QDate &date, KLocale::DateFormat format ) const
{
    return KCalendarSystem::formatDate( date, format );
}

QDate KCalendarSystemGregorian::readDate( const QString &str, bool *ok ) const
{
    return KCalendarSystem::readDate( str, ok );
}

QDate KCalendarSystemGregorian::readDate( const QString &intstr, const QString &fmt, bool *ok ) const
{
    return KCalendarSystem::readDate( intstr, fmt, ok );
}

QDate KCalendarSystemGregorian::readDate( const QString &str, KLocale::ReadDateFlags flags, bool *ok ) const
{
    return KCalendarSystem::readDate( str, flags, ok );
}

int KCalendarSystemGregorian::weekStartDay() const
{
    return KCalendarSystem::weekStartDay();
}

int KCalendarSystemGregorian::weekDayOfPray() const
{
    return 7; // sunday
}

bool KCalendarSystemGregorian::isLunar() const
{
    return false;
}

bool KCalendarSystemGregorian::isLunisolar() const
{
    return false;
}

bool KCalendarSystemGregorian::isSolar() const
{
    return true;
}

bool KCalendarSystemGregorian::isProleptic() const
{
    return false;
}

bool KCalendarSystemGregorian::julianDayToDate( int jd, int &year, int &month, int &day ) const
{
    QDate date = QDate::fromJulianDay( jd );

    date.getDate( &year, &month, &day );

    return date.isValid();
}

bool KCalendarSystemGregorian::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    QDate date;

    date.setDate( year, month, day );
    jd = date.toJulianDay();

    return date.isValid();
}
