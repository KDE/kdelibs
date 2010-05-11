/*
    Copyright 2010 John Layt <john@layt.net>

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

#include "kcalendarsystemminguo_p.h"
#include "kcalendarsystemgregorianprolepticprivate_p.h"

#include "kdebug.h"
#include "klocale.h"

#include <QtCore/QDate>

//Reuse the Gregorian Proleptic private implementation
class KCalendarSystemMinguoPrivate : public KCalendarSystemGregorianProlepticPrivate
{
public:
    explicit KCalendarSystemMinguoPrivate( KCalendarSystemMinguo *q );
    virtual ~KCalendarSystemMinguoPrivate();

    virtual void initDefaultEraList();
    virtual bool isLeapYear( int year ) const;
    virtual int earliestValidYear() const;
};

//Override only a few of the Gregorian Proleptic private methods

KCalendarSystemMinguoPrivate::KCalendarSystemMinguoPrivate( KCalendarSystemMinguo *q )
                              : KCalendarSystemGregorianProlepticPrivate( q )
{
}

KCalendarSystemMinguoPrivate::~KCalendarSystemMinguoPrivate()
{
}

void KCalendarSystemMinguoPrivate::initDefaultEraList()
{
    QString name, shortName, format;

    name = i18nc( "Calendar Era: Taiwan Republic of China Era, years > 0, LongFormat", "Republic of China Era" );
    shortName = i18nc( "Calendar Era: Taiwan Republic of China Era, years > 0, ShortFormat", "ROC" );
    format = i18nc( "(kdedt-format) Taiwan, ROC, full era year format used for %EY, e.g. ROC 99", "%EC %Ey" );
    addEra( '+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format );
}

bool KCalendarSystemMinguoPrivate::isLeapYear( int year ) const
{
    return KCalendarSystemGregorianProlepticPrivate::isLeapYear( year + 1911 );
}

int KCalendarSystemMinguoPrivate::earliestValidYear() const
{
    return 1;
}


KCalendarSystemMinguo::KCalendarSystemMinguo( const KLocale *locale )
                     : KCalendarSystemGregorianProleptic( *new KCalendarSystemMinguoPrivate( this ), KSharedConfig::Ptr(), locale )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemMinguo::KCalendarSystemMinguo( const KSharedConfig::Ptr config, const KLocale *locale )
                     : KCalendarSystemGregorianProleptic( *new KCalendarSystemMinguoPrivate( this ), config, locale )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemMinguo::KCalendarSystemMinguo( KCalendarSystemMinguoPrivate &dd,
                                              const KSharedConfig::Ptr config, const KLocale *locale )
                     : KCalendarSystemGregorianProleptic( dd, config, locale )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemMinguo::~KCalendarSystemMinguo()
{
}

QString KCalendarSystemMinguo::calendarType() const
{
    return QLatin1String( "minguo" );
}

QDate KCalendarSystemMinguo::epoch() const
{
    // 0001-01-01 = 1912-01-01 AD Gregorian
    return QDate::fromJulianDay( 2419403 );
}

QDate KCalendarSystemMinguo::earliestValidDate() const
{
    return epoch();
}

QDate KCalendarSystemMinguo::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    // 9999-12-31 = 11910-12-31 AD Gregorian
    return QDate::fromJulianDay( 6071462 );
}

bool KCalendarSystemMinguo::isValid( int year, int month, int day ) const
{
    return KCalendarSystemGregorianProleptic::isValid( year, month, day );
}

bool KCalendarSystemMinguo::isValid( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::isValid( date );
}

bool KCalendarSystemMinguo::setDate( QDate &date, int year, int month, int day ) const
{
    return KCalendarSystemGregorianProleptic::setDate( date, year, month, day );
}

// Deprecated
bool KCalendarSystemMinguo::setYMD( QDate &date, int y, int m, int d ) const
{
    return KCalendarSystemGregorianProleptic::setDate( date, y, m, d );
}

int KCalendarSystemMinguo::year( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::year( date );
}

int KCalendarSystemMinguo::month( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::month( date );
}

int KCalendarSystemMinguo::day( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::day( date );
}

QDate KCalendarSystemMinguo::addYears( const QDate &date, int nyears ) const
{
    return KCalendarSystemGregorianProleptic::addYears( date, nyears );
}

QDate KCalendarSystemMinguo::addMonths( const QDate &date, int nmonths ) const
{
    return KCalendarSystemGregorianProleptic::addMonths( date, nmonths );
}

QDate KCalendarSystemMinguo::addDays( const QDate &date, int ndays ) const
{
    return KCalendarSystemGregorianProleptic::addDays( date, ndays );
}

int KCalendarSystemMinguo::monthsInYear( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::monthsInYear( date );
}

int KCalendarSystemMinguo::weeksInYear( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::weeksInYear( date );
}

int KCalendarSystemMinguo::weeksInYear( int year ) const
{
    return KCalendarSystemGregorianProleptic::weeksInYear( year );
}

int KCalendarSystemMinguo::daysInYear( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::daysInYear( date );
}

int KCalendarSystemMinguo::daysInMonth( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::daysInMonth( date );
}

int KCalendarSystemMinguo::daysInWeek( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::daysInWeek( date );
}

int KCalendarSystemMinguo::dayOfYear( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::dayOfYear( date );
}

int KCalendarSystemMinguo::dayOfWeek( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::dayOfWeek( date );
}

int KCalendarSystemMinguo::weekNumber( const QDate &date, int * yearNum ) const
{
    return KCalendarSystemGregorianProleptic::weekNumber( date, yearNum );
}

bool KCalendarSystemMinguo::isLeapYear( int year ) const
{
    return KCalendarSystemGregorianProleptic::isLeapYear( year );
}

bool KCalendarSystemMinguo::isLeapYear( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::isLeapYear( date );
}

QString KCalendarSystemMinguo::monthName( int month, int year, MonthNameFormat format ) const
{
    return KCalendarSystemGregorianProleptic::monthName( month, year, format );
}

QString KCalendarSystemMinguo::monthName( const QDate &date, MonthNameFormat format ) const
{
    return KCalendarSystemGregorianProleptic::monthName( date, format );
}

QString KCalendarSystemMinguo::weekDayName( int weekDay, WeekDayNameFormat format ) const
{
    return KCalendarSystemGregorianProleptic::weekDayName( weekDay, format );
}

QString KCalendarSystemMinguo::weekDayName( const QDate &date, WeekDayNameFormat format ) const
{
    return KCalendarSystemGregorianProleptic::weekDayName( date, format );
}

QString KCalendarSystemMinguo::yearString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystemGregorianProleptic::yearString( pDate, format );
}

QString KCalendarSystemMinguo::monthString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystemGregorianProleptic::monthString( pDate, format );
}

QString KCalendarSystemMinguo::dayString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystemGregorianProleptic::dayString( pDate, format );
}

int KCalendarSystemMinguo::yearStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystemGregorianProleptic::yearStringToInteger( sNum, iLength );
}

int KCalendarSystemMinguo::monthStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystemGregorianProleptic::monthStringToInteger( sNum, iLength );
}

int KCalendarSystemMinguo::dayStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystemGregorianProleptic::dayStringToInteger( sNum, iLength );
}

QString KCalendarSystemMinguo::formatDate( const QDate &date, KLocale::DateFormat format ) const
{
    return KCalendarSystemGregorianProleptic::formatDate( date, format );
}

QDate KCalendarSystemMinguo::readDate( const QString &str, bool *ok ) const
{
    return KCalendarSystemGregorianProleptic::readDate( str, ok );
}

QDate KCalendarSystemMinguo::readDate( const QString &intstr, const QString &fmt, bool *ok ) const
{
    return KCalendarSystemGregorianProleptic::readDate( intstr, fmt, ok );
}

QDate KCalendarSystemMinguo::readDate( const QString &str, KLocale::ReadDateFlags flags, bool *ok ) const
{
    return KCalendarSystemGregorianProleptic::readDate( str, flags, ok );
}

int KCalendarSystemMinguo::weekStartDay() const
{
    return KCalendarSystemGregorianProleptic::weekStartDay();
}

int KCalendarSystemMinguo::weekDayOfPray() const
{
    return 7; // TODO JPL ???
}

bool KCalendarSystemMinguo::isLunar() const
{
    return KCalendarSystemGregorianProleptic::isLunar();
}

bool KCalendarSystemMinguo::isLunisolar() const
{
    return KCalendarSystemGregorianProleptic::isLunisolar();
}

bool KCalendarSystemMinguo::isSolar() const
{
    return KCalendarSystemGregorianProleptic::isSolar();
}

bool KCalendarSystemMinguo::isProleptic() const
{
    return false;
}

bool KCalendarSystemMinguo::julianDayToDate( int jd, int &year, int &month, int &day ) const
{
    bool result = KCalendarSystemGregorianProleptic::julianDayToDate( jd, year, month, day );
    year = year - 1911;
    return result;
}

bool KCalendarSystemMinguo::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    return KCalendarSystemGregorianProleptic::dateToJulianDay( year + 1911, month, day, jd );
}

