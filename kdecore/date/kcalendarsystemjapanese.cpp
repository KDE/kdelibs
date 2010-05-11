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

#include "kcalendarsystemjapanese_p.h"
#include "kcalendarsystemgregorianprolepticprivate_p.h"

#include "kdebug.h"
#include "klocale.h"

#include <QtCore/QDate>

//Reuse the Gregorian Proleptic private implementation
class KCalendarSystemJapanesePrivate : public KCalendarSystemGregorianProlepticPrivate
{
public:
    explicit KCalendarSystemJapanesePrivate( KCalendarSystemJapanese *q );
    virtual ~KCalendarSystemJapanesePrivate();

    virtual void initDefaultEraList();
    virtual int earliestValidYear() const;
};

//Override only a few of the Gregorian Proleptic private methods

KCalendarSystemJapanesePrivate::KCalendarSystemJapanesePrivate( KCalendarSystemJapanese *q )
                              : KCalendarSystemGregorianProlepticPrivate( q )
{
}

KCalendarSystemJapanesePrivate::~KCalendarSystemJapanesePrivate()
{
}

void KCalendarSystemJapanesePrivate::initDefaultEraList()
{
    QString name, shortName, format;

    // Nengō, Only do most recent for now, use AD for the rest.
    // Feel free to add more, but have mercy on the translators :-)

    name = i18nc( "Calendar Era: Gregorian Christian Era, years > 0, LongFormat", "Anno Domini" );
    shortName = i18nc( "Calendar Era: Gregorian Christian Era, years > 0, ShortFormat", "AD" );
    format = i18nc( "(kdedt-format) Gregorian, AD, full era year format used for %EY, e.g. 2000 AD", "%Ey %EC" );
    addEra( '+', 1, q->epoch(), 1, QDate( 1868, 9, 7 ), name, shortName, format );

    name = i18nc( "Calendar Era: Japanese Nengō, Meiji Era, LongFormat", "Meiji" );
    shortName = name;
    format = i18nc( "(kdedt-format) Japanese, Meiji, full era year format used for %EY, year = 1, e.g. Meiji 1", "%EC Gannen" );
    addEra( '+', 1, QDate( 1868, 9, 8 ), 1868, QDate( 1868, 12, 31 ), name, shortName, format );
    format = i18nc( "(kdedt-format) Japanese, Meiji, full era year format used for %EY, year > 1, e.g. Meiji 22", "%EC %Ey" );
    addEra( '+', 2, QDate( 1869, 1, 1 ), 1869, QDate( 1912, 7, 29 ), name, shortName, format );

    name = i18nc( "Calendar Era: Japanese Nengō, Taishō Era, LongFormat", "Taishō" );
    shortName = name;
    format = i18nc( "(kdedt-format) Japanese, Taishō, full era year format used for %EY, year = 1, e.g. Taishō 1", "%EC Gannen" );
    addEra( '+', 1, QDate( 1912, 7, 30 ), 1912, QDate( 1912, 12, 31 ), name, shortName, format );
    format = i18nc( "(kdedt-format) Japanese, Taishō, full era year format used for %EY, year > 1, e.g. Taishō 22", "%EC %Ey" );
    addEra( '+', 2, QDate( 1913, 1, 1 ), 1913, QDate( 1926, 12, 24 ), name, shortName, format );

    name = i18nc( "Calendar Era: Japanese Nengō, Shōwa Era, LongFormat", "Shōwa" );
    shortName = name;
    format = i18nc( "(kdedt-format) Japanese, Shōwa, full era year format used for %EY, year = 1, e.g. Shōwa 1", "%EC Gannen" );
    addEra( '+', 1, QDate( 1926, 12, 25 ), 1926, QDate( 1926, 12, 31 ), name, shortName, format );
    format = i18nc( "(kdedt-format) Japanese, Shōwa, full era year format used for %EY, year > 1, e.g. Shōwa 22", "%EC %Ey" );
    addEra( '+', 2, QDate( 1927, 1, 1 ), 1927, QDate( 1989, 1, 7 ), name, shortName, format );

    name = i18nc( "Calendar Era: Japanese Nengō, Heisei Era, LongFormat", "Heisei" );
    shortName = name;
    format = i18nc( "(kdedt-format) Japanese, Heisei, full era year format used for %EY, year = 1, e.g. Heisei 1", "%EC Gannen" );
    addEra( '+', 1, QDate( 1989, 1, 8 ), 1989, QDate( 1989, 12, 31 ), name, shortName, format );
    format = i18nc( "(kdedt-format) Japanese, Heisei, full era year format used for %EY, year > 1, e.g. Heisei 22", "%EC %Ey" );
    addEra( '+', 2, QDate( 1990, 1, 1 ), 1990, q->latestValidDate(), name, shortName, format );
}

int KCalendarSystemJapanesePrivate::earliestValidYear() const
{
    return 1;
}


KCalendarSystemJapanese::KCalendarSystemJapanese( const KLocale *locale )
                       : KCalendarSystemGregorianProleptic( *new KCalendarSystemJapanesePrivate( this ), KSharedConfig::Ptr(), locale )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemJapanese::KCalendarSystemJapanese( const KSharedConfig::Ptr config, const KLocale *locale )
                       : KCalendarSystemGregorianProleptic( *new KCalendarSystemJapanesePrivate( this ), config, locale )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemJapanese::KCalendarSystemJapanese( KCalendarSystemJapanesePrivate &dd,
                                                  const KSharedConfig::Ptr config, const KLocale *locale )
                       : KCalendarSystemGregorianProleptic( dd, config, locale )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemJapanese::~KCalendarSystemJapanese()
{
}

QString KCalendarSystemJapanese::calendarType() const
{
    return QLatin1String( "japanese" );
}

QDate KCalendarSystemJapanese::epoch() const
{
    // 0001-01-01 Gregorian for now
    return QDate::fromJulianDay( 1721426 );
}

QDate KCalendarSystemJapanese::earliestValidDate() const
{
    // 0001-01-01 Gregorian for now
    return QDate::fromJulianDay( 1721426 );
}

QDate KCalendarSystemJapanese::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    // 9999-12-31 Gregorian
    return QDate::fromJulianDay( 5373484 );
}

bool KCalendarSystemJapanese::isValid( int year, int month, int day ) const
{
    return KCalendarSystemGregorianProleptic::isValid( year, month, day );
}

bool KCalendarSystemJapanese::isValid( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::isValid( date );
}

bool KCalendarSystemJapanese::setDate( QDate &date, int year, int month, int day ) const
{
    return KCalendarSystemGregorianProleptic::setDate( date, year, month, day );
}

// Deprecated
bool KCalendarSystemJapanese::setYMD( QDate &date, int y, int m, int d ) const
{
    return KCalendarSystemGregorianProleptic::setDate( date, y, m, d );
}

int KCalendarSystemJapanese::year( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::year( date );
}

int KCalendarSystemJapanese::month( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::month( date );
}

int KCalendarSystemJapanese::day( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::day( date );
}

QDate KCalendarSystemJapanese::addYears( const QDate &date, int nyears ) const
{
    return KCalendarSystemGregorianProleptic::addYears( date, nyears );
}

QDate KCalendarSystemJapanese::addMonths( const QDate &date, int nmonths ) const
{
    return KCalendarSystemGregorianProleptic::addMonths( date, nmonths );
}

QDate KCalendarSystemJapanese::addDays( const QDate &date, int ndays ) const
{
    return KCalendarSystemGregorianProleptic::addDays( date, ndays );
}

int KCalendarSystemJapanese::monthsInYear( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::monthsInYear( date );
}

int KCalendarSystemJapanese::weeksInYear( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::weeksInYear( date );
}

int KCalendarSystemJapanese::weeksInYear( int year ) const
{
    return KCalendarSystemGregorianProleptic::weeksInYear( year );
}

int KCalendarSystemJapanese::daysInYear( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::daysInYear( date );
}

int KCalendarSystemJapanese::daysInMonth( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::daysInMonth( date );
}

int KCalendarSystemJapanese::daysInWeek( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::daysInWeek( date );
}

int KCalendarSystemJapanese::dayOfYear( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::dayOfYear( date );
}

int KCalendarSystemJapanese::dayOfWeek( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::dayOfWeek( date );
}

int KCalendarSystemJapanese::weekNumber( const QDate &date, int * yearNum ) const
{
    return KCalendarSystemGregorianProleptic::weekNumber( date, yearNum );
}

bool KCalendarSystemJapanese::isLeapYear( int year ) const
{
    return KCalendarSystemGregorianProleptic::isLeapYear( year );
}

bool KCalendarSystemJapanese::isLeapYear( const QDate &date ) const
{
    return KCalendarSystemGregorianProleptic::isLeapYear( date );
}

QString KCalendarSystemJapanese::monthName( int month, int year, MonthNameFormat format ) const
{
    return KCalendarSystemGregorianProleptic::monthName( month, year, format );
}

QString KCalendarSystemJapanese::monthName( const QDate &date, MonthNameFormat format ) const
{
    return KCalendarSystemGregorianProleptic::monthName( date, format );
}

QString KCalendarSystemJapanese::weekDayName( int weekDay, WeekDayNameFormat format ) const
{
    return KCalendarSystemGregorianProleptic::weekDayName( weekDay, format );
}

QString KCalendarSystemJapanese::weekDayName( const QDate &date, WeekDayNameFormat format ) const
{
    return KCalendarSystemGregorianProleptic::weekDayName( date, format );
}

QString KCalendarSystemJapanese::yearString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystemGregorianProleptic::yearString( pDate, format );
}

QString KCalendarSystemJapanese::monthString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystemGregorianProleptic::monthString( pDate, format );
}

QString KCalendarSystemJapanese::dayString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystemGregorianProleptic::dayString( pDate, format );
}

int KCalendarSystemJapanese::yearStringToInteger( const QString &sNum, int &iLength ) const
{
    QString gannen = i18nc( "Japanese year 1 of era", "Gannen" );
    if ( sNum.startsWith( gannen, Qt::CaseInsensitive ) ) {
        iLength = gannen.length();
        return 1;
    } else {
        return KCalendarSystemGregorianProleptic::yearStringToInteger( sNum, iLength );
    }
}

int KCalendarSystemJapanese::monthStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystemGregorianProleptic::monthStringToInteger( sNum, iLength );
}

int KCalendarSystemJapanese::dayStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystemGregorianProleptic::dayStringToInteger( sNum, iLength );
}

QString KCalendarSystemJapanese::formatDate( const QDate &date, KLocale::DateFormat format ) const
{
    return KCalendarSystemGregorianProleptic::formatDate( date, format );
}

QDate KCalendarSystemJapanese::readDate( const QString &str, bool *ok ) const
{
    return KCalendarSystemGregorianProleptic::readDate( str, ok );
}

QDate KCalendarSystemJapanese::readDate( const QString &intstr, const QString &fmt, bool *ok ) const
{
    return KCalendarSystemGregorianProleptic::readDate( intstr, fmt, ok );
}

QDate KCalendarSystemJapanese::readDate( const QString &str, KLocale::ReadDateFlags flags, bool *ok ) const
{
    return KCalendarSystemGregorianProleptic::readDate( str, flags, ok );
}

int KCalendarSystemJapanese::weekStartDay() const
{
    return KCalendarSystemGregorianProleptic::weekStartDay();
}

int KCalendarSystemJapanese::weekDayOfPray() const
{
    return 7; // TODO JPL ???
}

bool KCalendarSystemJapanese::isLunar() const
{
    return KCalendarSystemGregorianProleptic::isLunar();
}

bool KCalendarSystemJapanese::isLunisolar() const
{
    return KCalendarSystemGregorianProleptic::isLunisolar();
}

bool KCalendarSystemJapanese::isSolar() const
{
    return KCalendarSystemGregorianProleptic::isSolar();
}

bool KCalendarSystemJapanese::isProleptic() const
{
    return false;
}

bool KCalendarSystemJapanese::julianDayToDate( int jd, int &year, int &month, int &day ) const
{
    return KCalendarSystemGregorianProleptic::julianDayToDate( jd, year, month, day );
}

bool KCalendarSystemJapanese::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    return KCalendarSystemGregorianProleptic::dateToJulianDay( year, month, day, jd );
}
