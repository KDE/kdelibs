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

#include "kcalendarsystemthai_p.h"
#include "kcalendarsystemgregorianprolepticprivate_p.h"

#include "kdebug.h"
#include "klocale.h"

#include <QtCore/QDate>

//Reuse the Gregorian Proleptic private implementation
class KCalendarSystemThaiPrivate : public KCalendarSystemGregorianProlepticPrivate
{
public:
    explicit KCalendarSystemThaiPrivate( KCalendarSystemThai *q );
    virtual ~KCalendarSystemThaiPrivate();

    virtual void initDefaultEraList();
    virtual bool isLeapYear( int year ) const;
    virtual bool hasYearZero() const;
    virtual int earliestValidYear() const;
};

//Override only a few of the Gregorian Proleptic private methods

KCalendarSystemThaiPrivate::KCalendarSystemThaiPrivate( KCalendarSystemThai *q )
                              : KCalendarSystemGregorianProlepticPrivate( q )
{
}

KCalendarSystemThaiPrivate::~KCalendarSystemThaiPrivate()
{
}

void KCalendarSystemThaiPrivate::initDefaultEraList()
{
    QString name, shortName, format;

    name = i18nc( "Calendar Era: Thai Buddhist Era, years > 0, LongFormat", "Buddhist Era" );
    shortName = i18nc( "Calendar Era: Thai Buddhist Era, years > 0, ShortFormat", "BE" );
    format = i18nc( "(kdedt-format) Thai, BE, full era year format used for %EY, e.g. 2000 BE", "%Ey %EC" );
    addEra( '+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format );
}

bool KCalendarSystemThaiPrivate::isLeapYear( int year ) const
{
    return KCalendarSystemGregorianProlepticPrivate::isLeapYear( year - 543 );
}

bool KCalendarSystemThaiPrivate::hasYearZero() const
{
    return true;
}

int KCalendarSystemThaiPrivate::earliestValidYear() const
{
    return 0;
}


KCalendarSystemThai::KCalendarSystemThai( const KLocale *locale )
                   : KCalendarSystemGregorianProleptic( *new KCalendarSystemThaiPrivate( this ), KSharedConfig::Ptr(), locale )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemThai::KCalendarSystemThai( const KSharedConfig::Ptr config, const KLocale *locale )
                   : KCalendarSystemGregorianProleptic( *new KCalendarSystemThaiPrivate( this ), config, locale )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemThai::KCalendarSystemThai( KCalendarSystemThaiPrivate &dd,
                                          const KSharedConfig::Ptr config, const KLocale *locale )
                   : KCalendarSystemGregorianProleptic( dd, config, locale )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemThai::~KCalendarSystemThai()
{
}

QString KCalendarSystemThai::calendarType() const
{
    return QLatin1String( "thai" );
}

QDate KCalendarSystemThai::epoch() const
{
    // 0000-01-01 = 0544-01-01 BC Gregorian = 0544-01-07 BC Julian
    return QDate::fromJulianDay( 1522734 );
}

QDate KCalendarSystemThai::earliestValidDate() const
{
    return epoch();
}

QDate KCalendarSystemThai::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    // 9999-12-31 = 9456-12-31 AD Gregorian
    return QDate::fromJulianDay( 5175158 );
}

bool KCalendarSystemThai::isValid( int year, int month, int day ) const
{
    return KCalendarSystemGregorianProleptic::isValid( year, month, day );
}

bool KCalendarSystemThai::isValid( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::isValid( date );
}

bool KCalendarSystemThai::setDate( QDate &date, int year, int month, int day ) const
{
    return KCalendarSystemGregorianProleptic::setDate( date, year, month, day );
}

// Deprecated
bool KCalendarSystemThai::setYMD( QDate &date, int y, int m, int d ) const
{
    return KCalendarSystemGregorianProleptic::setDate( date, y, m, d );
}

int KCalendarSystemThai::year( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::year( date );
}

int KCalendarSystemThai::month( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::month( date );
}

int KCalendarSystemThai::day( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::day( date );
}

QDate KCalendarSystemThai::addYears( const QDate &date, int nyears ) const
{
    return KCalendarSystemGregorianProleptic::addYears( date, nyears );
}

QDate KCalendarSystemThai::addMonths( const QDate &date, int nmonths ) const
{
    return KCalendarSystemGregorianProleptic::addMonths( date, nmonths );
}

QDate KCalendarSystemThai::addDays( const QDate &date, int ndays ) const
{
    return KCalendarSystemGregorianProleptic::addDays( date, ndays );
}

int KCalendarSystemThai::monthsInYear( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::monthsInYear( date );
}

int KCalendarSystemThai::weeksInYear( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::weeksInYear( date );
}

int KCalendarSystemThai::weeksInYear( int year ) const
{
    return KCalendarSystemGregorianProleptic::weeksInYear( year );
}

int KCalendarSystemThai::daysInYear( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::daysInYear( date );
}

int KCalendarSystemThai::daysInMonth( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::daysInMonth( date );
}

int KCalendarSystemThai::daysInWeek( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::daysInWeek( date );
}

int KCalendarSystemThai::dayOfYear( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::dayOfYear( date );
}

int KCalendarSystemThai::dayOfWeek( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::dayOfWeek( date );
}

int KCalendarSystemThai::weekNumber( const QDate &date, int * yearNum ) const
{
    return KCalendarSystemGregorianProleptic::weekNumber( date, yearNum );
}

bool KCalendarSystemThai::isLeapYear( int year ) const
{
    return KCalendarSystemGregorianProleptic::isLeapYear( year );
}

bool KCalendarSystemThai::isLeapYear( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::isLeapYear( date );
}

QString KCalendarSystemThai::monthName( int month, int year, MonthNameFormat format ) const
{
    return KCalendarSystemGregorianProleptic::monthName( month, year, format );
}

QString KCalendarSystemThai::monthName( const QDate &date, MonthNameFormat format ) const
{
    return KCalendarSystemGregorianProleptic::monthName( date, format );
}

QString KCalendarSystemThai::weekDayName( int weekDay, WeekDayNameFormat format ) const
{
    return KCalendarSystemGregorianProleptic::weekDayName( weekDay, format );
}

QString KCalendarSystemThai::weekDayName( const QDate &date, WeekDayNameFormat format ) const
{
    return KCalendarSystemGregorianProleptic::weekDayName( date, format );
}

QString KCalendarSystemThai::yearString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystemGregorianProleptic::yearString( pDate, format );
}

QString KCalendarSystemThai::monthString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystemGregorianProleptic::monthString( pDate, format );
}

QString KCalendarSystemThai::dayString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystemGregorianProleptic::dayString( pDate, format );
}

int KCalendarSystemThai::yearStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystemGregorianProleptic::yearStringToInteger( sNum, iLength );
}

int KCalendarSystemThai::monthStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystemGregorianProleptic::monthStringToInteger( sNum, iLength );
}

int KCalendarSystemThai::dayStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystemGregorianProleptic::dayStringToInteger( sNum, iLength );
}

QString KCalendarSystemThai::formatDate( const QDate &date, KLocale::DateFormat format ) const
{
    return KCalendarSystemGregorianProleptic::formatDate( date, format );
}

QDate KCalendarSystemThai::readDate( const QString &str, bool *ok ) const
{
    return KCalendarSystemGregorianProleptic::readDate( str, ok );
}

QDate KCalendarSystemThai::readDate( const QString &intstr, const QString &fmt, bool *ok ) const
{
    return KCalendarSystemGregorianProleptic::readDate( intstr, fmt, ok );
}

QDate KCalendarSystemThai::readDate( const QString &str, KLocale::ReadDateFlags flags, bool *ok ) const
{
    return KCalendarSystemGregorianProleptic::readDate( str, flags, ok );
}

int KCalendarSystemThai::weekStartDay() const
{
    return KCalendarSystemGregorianProleptic::weekStartDay();
}

int KCalendarSystemThai::weekDayOfPray() const
{
    return 7; // TODO JPL ???
}

bool KCalendarSystemThai::isLunar() const
{
    return KCalendarSystemGregorianProleptic::isLunar();
}

bool KCalendarSystemThai::isLunisolar() const
{
    return KCalendarSystemGregorianProleptic::isLunisolar();
}

bool KCalendarSystemThai::isSolar() const
{
    return KCalendarSystemGregorianProleptic::isSolar();
}

bool KCalendarSystemThai::isProleptic() const
{
    return false;
}

bool KCalendarSystemThai::julianDayToDate( int jd, int &year, int &month, int &day ) const
{
    bool result = KCalendarSystemGregorianProleptic::julianDayToDate( jd, year, month, day );
    year = year + 543;
    return result;
}

bool KCalendarSystemThai::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    return KCalendarSystemGregorianProleptic::dateToJulianDay( year - 543, month, day, jd );
}

