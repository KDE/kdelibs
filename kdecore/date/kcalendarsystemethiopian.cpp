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

#include "kcalendarsystemethiopian_p.h"
#include "kcalendarsystemcopticprivate_p.h"

#include "kdebug.h"
#include "klocale.h"

#include <QtCore/QDate>
#include <QtCore/QCharRef>

//Reuse the Coptic private implementation
class KCalendarSystemEthiopianPrivate : public KCalendarSystemCopticPrivate
{
public:
    explicit KCalendarSystemEthiopianPrivate( KCalendarSystemEthiopian *q ) : KCalendarSystemCopticPrivate( q )
    {
    }

    virtual ~KCalendarSystemEthiopianPrivate()
    {
    }

    virtual void initDefaultEraList();
};

void KCalendarSystemEthiopianPrivate::initDefaultEraList()
{
    QString name, shortName, format;
    // Incarnation Era, Amätä Mehrät, "Year of Mercy".
    name = i18nc( "Calendar Era: Ethiopian Incarnation Era, years > 0, LongFormat", "Amata Mehrat" );
    shortName = i18nc( "Calendar Era: Ethiopian Incarnation Era, years > 0, ShortFormat", "AM" );
    format = i18nc( "(kdedt-format) Ethiopian, AM, full era year format used for %EY, e.g. 2000 AM", "%Ey %EC" );
    addEra( '+', 1, q->epoch(), 1, q->latestValidDate(), name, shortName, format );
}


KCalendarSystemEthiopian::KCalendarSystemEthiopian( const KLocale *locale )
                        : KCalendarSystemCoptic( *new KCalendarSystemEthiopianPrivate( this ), KSharedConfig::Ptr(), locale ),
                          dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemEthiopian::KCalendarSystemEthiopian( const KSharedConfig::Ptr config, const KLocale *locale )
                        : KCalendarSystemCoptic( *new KCalendarSystemEthiopianPrivate( this ), config, locale ),
                          dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemEthiopian::KCalendarSystemEthiopian( KCalendarSystemEthiopianPrivate &dd,
                                                    const KSharedConfig::Ptr config, const KLocale *locale )
                     : KCalendarSystemCoptic( dd, config, locale ),
                       dont_use( 0 )
{
    d_ptr->initialiseEraList( calendarType() );
}

KCalendarSystemEthiopian::~KCalendarSystemEthiopian()
{
    delete dont_use;
}

QString KCalendarSystemEthiopian::calendarType() const
{
    return QLatin1String( "ethiopian" );
}

QDate KCalendarSystemEthiopian::epoch() const
{
    //0001-01-01, no Year 0.
    //0008-08-29 AD Julian
    return QDate::fromJulianDay( 1724221 );
}

QDate KCalendarSystemEthiopian::earliestValidDate() const
{
    //0001-01-01, no Year 0.
    //0008-08-29 AD Julian
    return QDate::fromJulianDay( 1724221 );
}

QDate KCalendarSystemEthiopian::latestValidDate() const
{
    // Set to last day of year 9999 until confirm date formats & widgets support > 9999
    //9999-12-30
    //100008-08-29 AD Julian
    return QDate::fromJulianDay( 5376721 );
}

bool KCalendarSystemEthiopian::isValid( int year, int month, int day ) const
{
    return KCalendarSystemCoptic::isValid( year, month, day );
}

bool KCalendarSystemEthiopian::isValid( const QDate &date ) const
{
    return KCalendarSystemCoptic::isValid( date );
}

bool KCalendarSystemEthiopian::setDate( QDate &date, int year, int month, int day ) const
{
    return KCalendarSystemCoptic::setDate( date, year, month, day );
}

// Deprecated
bool KCalendarSystemEthiopian::setYMD( QDate &date, int y, int m, int d ) const
{
    return KCalendarSystemCoptic::setDate( date, y, m, d );
}

int KCalendarSystemEthiopian::year( const QDate &date ) const
{
    return KCalendarSystemCoptic::year( date );
}

int KCalendarSystemEthiopian::month( const QDate &date ) const
{
    return KCalendarSystemCoptic::month( date );
}

int KCalendarSystemEthiopian::day( const QDate &date ) const
{
    return KCalendarSystemCoptic::day( date );
}

QDate KCalendarSystemEthiopian::addYears( const QDate &date, int nyears ) const
{
    return KCalendarSystemCoptic::addYears( date, nyears );
}

QDate KCalendarSystemEthiopian::addMonths( const QDate &date, int nmonths ) const
{
    return KCalendarSystemCoptic::addMonths( date, nmonths );
}

QDate KCalendarSystemEthiopian::addDays( const QDate &date, int ndays ) const
{
    return KCalendarSystemCoptic::addDays( date, ndays );
}

int KCalendarSystemEthiopian::monthsInYear( const QDate &date ) const
{
    return KCalendarSystemCoptic::monthsInYear( date );
}

int KCalendarSystemEthiopian::weeksInYear( const QDate &date ) const
{
    return KCalendarSystemCoptic::weeksInYear( date );
}

int KCalendarSystemEthiopian::weeksInYear( int year ) const
{
    return KCalendarSystemCoptic::weeksInYear( year );
}

int KCalendarSystemEthiopian::daysInYear( const QDate &date ) const
{
    return KCalendarSystemCoptic::daysInYear( date );
}

int KCalendarSystemEthiopian::daysInMonth( const QDate &date ) const
{
    return KCalendarSystemCoptic::daysInMonth( date );
}

int KCalendarSystemEthiopian::daysInWeek( const QDate &date ) const
{
    return KCalendarSystemCoptic::daysInWeek( date );
}

int KCalendarSystemEthiopian::dayOfYear( const QDate &date ) const
{
    return KCalendarSystemCoptic::dayOfYear( date );
}

int KCalendarSystemEthiopian::dayOfWeek( const QDate &date ) const
{
    return KCalendarSystemCoptic::dayOfWeek( date );
}

int KCalendarSystemEthiopian::weekNumber( const QDate &date, int * yearNum ) const
{
    return KCalendarSystemCoptic::weekNumber( date, yearNum );
}

bool KCalendarSystemEthiopian::isLeapYear( int year ) const
{
    return KCalendarSystemCoptic::isLeapYear( year );
}

bool KCalendarSystemEthiopian::isLeapYear( const QDate &date ) const
{
    return KCalendarSystemCoptic::isLeapYear( date );
}

// Names taken from http://www.ethiopianembassy.at/dates_cycles.htm, alternative transliterations exist
QString KCalendarSystemEthiopian::monthName( int month, int year, MonthNameFormat format ) const
{
    Q_UNUSED( year );

    if ( format == ShortNamePossessive ) {
        switch ( month ) {
        case 1:
            return ki18nc( "Ethiopian month 1 - ShortNamePossessive",  "of Mes" ).toString( locale() );
        case 2:
            return ki18nc( "Ethiopian month 2 - ShortNamePossessive",  "of Teq" ).toString( locale() );
        case 3:
            return ki18nc( "Ethiopian month 3 - ShortNamePossessive",  "of Hed" ).toString( locale() );
        case 4:
            return ki18nc( "Ethiopian month 4 - ShortNamePossessive",  "of Tah" ).toString( locale() );
        case 5:
            return ki18nc( "Ethiopian month 5 - ShortNamePossessive",  "of Ter" ).toString( locale() );
        case 6:
            return ki18nc( "Ethiopian month 6 - ShortNamePossessive",  "of Yak" ).toString( locale() );
        case 7:
            return ki18nc( "Ethiopian month 7 - ShortNamePossessive",  "of Mag" ).toString( locale() );
        case 8:
            return ki18nc( "Ethiopian month 8 - ShortNamePossessive",  "of Miy" ).toString( locale() );
        case 9:
            return ki18nc( "Ethiopian month 9 - ShortNamePossessive",  "of Gen" ).toString( locale() );
        case 10:
            return ki18nc( "Ethiopian month 10 - ShortNamePossessive", "of Sen" ).toString( locale() );
        case 11:
            return ki18nc( "Ethiopian month 11 - ShortNamePossessive", "of Ham" ).toString( locale() );
        case 12:
            return ki18nc( "Ethiopian month 12 - ShortNamePossessive", "of Neh" ).toString( locale() );
        case 13:
            return ki18nc( "Ethiopian month 13 - ShortNamePossessive", "of Pag" ).toString( locale() );
        default:
            return QString();
        }
    }

    if ( format == LongNamePossessive ) {
    switch ( month ) {
        case 1:
            return ki18nc( "Ethiopian month 1 - LongNamePossessive",  "of Meskerem" ).toString( locale() );
        case 2:
            return ki18nc( "Ethiopian month 2 - LongNamePossessive",  "of Tequemt" ).toString( locale() );
        case 3:
            return ki18nc( "Ethiopian month 3 - LongNamePossessive",  "of Hedar" ).toString( locale() );
        case 4:
            return ki18nc( "Ethiopian month 4 - LongNamePossessive",  "of Tahsas" ).toString( locale() );
        case 5:
            return ki18nc( "Ethiopian month 5 - LongNamePossessive",  "of Ter" ).toString( locale() );
        case 6:
            return ki18nc( "Ethiopian month 6 - LongNamePossessive",  "of Yakatit" ).toString( locale() );
        case 7:
            return ki18nc( "Ethiopian month 7 - LongNamePossessive",  "of Magabit" ).toString( locale() );
        case 8:
            return ki18nc( "Ethiopian month 8 - LongNamePossessive",  "of Miyazya" ).toString( locale() );
        case 9:
            return ki18nc( "Ethiopian month 9 - LongNamePossessive",  "of Genbot" ).toString( locale() );
        case 10:
            return ki18nc( "Ethiopian month 10 - LongNamePossessive", "of Sene" ).toString( locale() );
        case 11:
            return ki18nc( "Ethiopian month 11 - LongNamePossessive", "of Hamle" ).toString( locale() );
        case 12:
            return ki18nc( "Ethiopian month 12 - LongNamePossessive", "of Nehase" ).toString( locale() );
        case 13:
            return ki18nc( "Ethiopian month 13 - LongNamePossessive", "of Pagumen" ).toString( locale() );
        default:
            return QString();
        }
    }

    if ( format == ShortName ) {
        switch ( month ) {
        case 1:
            return ki18nc( "Ethiopian month 1 - ShortName",  "Mes" ).toString( locale() );
        case 2:
            return ki18nc( "Ethiopian month 2 - ShortName",  "Teq" ).toString( locale() );
        case 3:
            return ki18nc( "Ethiopian month 3 - ShortName",  "Hed" ).toString( locale() );
        case 4:
            return ki18nc( "Ethiopian month 4 - ShortName",  "Tah" ).toString( locale() );
        case 5:
            return ki18nc( "Ethiopian month 5 - ShortName",  "Ter" ).toString( locale() );
        case 6:
            return ki18nc( "Ethiopian month 6 - ShortName",  "Yak" ).toString( locale() );
        case 7:
            return ki18nc( "Ethiopian month 7 - ShortName",  "Mag" ).toString( locale() );
        case 8:
            return ki18nc( "Ethiopian month 8 - ShortName",  "Miy" ).toString( locale() );
        case 9:
            return ki18nc( "Ethiopian month 9 - ShortName",  "Gen" ).toString( locale() );
        case 10:
            return ki18nc( "Ethiopian month 10 - ShortName", "Sen" ).toString( locale() );
        case 11:
            return ki18nc( "Ethiopian month 11 - ShortName", "Ham" ).toString( locale() );
        case 12:
            return ki18nc( "Ethiopian month 12 - ShortName", "Neh" ).toString( locale() );
        case 13:
            return ki18nc( "Ethiopian month 13 - ShortName", "Pag" ).toString( locale() );
        default:
            return QString();
        }
    }

    // Default to LongName
    switch ( month ) {
    case 1:
        return ki18nc( "Ethiopian month 1 - LongName",  "Meskerem" ).toString( locale() );
    case 2:
        return ki18nc( "Ethiopian month 2 - LongName",  "Tequemt" ).toString( locale() );
    case 3:
        return ki18nc( "Ethiopian month 3 - LongName",  "Hedar" ).toString( locale() );
    case 4:
        return ki18nc( "Ethiopian month 4 - LongName",  "Tahsas" ).toString( locale() );
    case 5:
        return ki18nc( "Ethiopian month 5 - LongName",  "Ter" ).toString( locale() );
    case 6:
        return ki18nc( "Ethiopian month 6 - LongName",  "Yakatit" ).toString( locale() );
    case 7:
        return ki18nc( "Ethiopian month 7 - LongName",  "Magabit" ).toString( locale() );
    case 8:
        return ki18nc( "Ethiopian month 8 - LongName",  "Miyazya" ).toString( locale() );
    case 9:
        return ki18nc( "Ethiopian month 9 - LongName",  "Genbot" ).toString( locale() );
    case 10:
        return ki18nc( "Ethiopian month 10 - LongName", "Sene" ).toString( locale() );
    case 11:
        return ki18nc( "Ethiopian month 11 - LongName", "Hamle" ).toString( locale() );
    case 12:
        return ki18nc( "Ethiopian month 12 - LongName", "Nehase" ).toString( locale() );
    case 13:
        return ki18nc( "Ethiopian month 13 - LongName", "Pagumen" ).toString( locale() );
    default:
        return QString();
    }
}

QString KCalendarSystemEthiopian::monthName( const QDate &date, MonthNameFormat format ) const
{
    return KCalendarSystemCoptic::monthName( date, format );
}

// Names taken from http://www.ethiopianembassy.at/dates_cycles.htm, alternative transliterations exist
QString KCalendarSystemEthiopian::weekDayName( int weekDay, WeekDayNameFormat format ) const
{
    if ( format == ShortDayName ) {
        switch ( weekDay ) {
        case 1:  return ki18nc( "Ethiopian weekday 1 - ShortDayName", "Seg" ).toString( locale() );
        case 2:  return ki18nc( "Ethiopian weekday 2 - ShortDayName", "Mak" ).toString( locale() );
        case 3:  return ki18nc( "Ethiopian weekday 3 - ShortDayName", "Rob" ).toString( locale() );
        case 4:  return ki18nc( "Ethiopian weekday 4 - ShortDayName", "Ham" ).toString( locale() );
        case 5:  return ki18nc( "Ethiopian weekday 5 - ShortDayName", "Arb" ).toString( locale() );
        case 6:  return ki18nc( "Ethiopian weekday 6 - ShortDayName", "Qed" ).toString( locale() );
        case 7:  return ki18nc( "Ethiopian weekday 7 - ShortDayName", "Ehu" ).toString( locale() );
        default: return QString();
        }
    }

    switch ( weekDay ) {
    case 1:  return ki18nc( "Ethiopian weekday 1 - LongDayName", "Segno" ).toString( locale() );
    case 2:  return ki18nc( "Ethiopian weekday 2 - LongDayName", "Maksegno" ).toString( locale() );
    case 3:  return ki18nc( "Ethiopian weekday 3 - LongDayName", "Rob" ).toString( locale() );
    case 4:  return ki18nc( "Ethiopian weekday 4 - LongDayName", "Hamus" ).toString( locale() );
    case 5:  return ki18nc( "Ethiopian weekday 5 - LongDayName", "Arb" ).toString( locale() );
    case 6:  return ki18nc( "Ethiopian weekday 6 - LongDayName", "Qedame" ).toString( locale() );
    case 7:  return ki18nc( "Ethiopian weekday 7 - LongDayName", "Ehud" ).toString( locale() );
    default: return QString();
    }
}

QString KCalendarSystemEthiopian::weekDayName( const QDate &date, WeekDayNameFormat format ) const
{
    return KCalendarSystemCoptic::weekDayName( date, format );
}

QString KCalendarSystemEthiopian::yearString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystemCoptic::yearString( pDate, format );
}

QString KCalendarSystemEthiopian::monthString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystemCoptic::monthString( pDate, format );
}

QString KCalendarSystemEthiopian::dayString( const QDate &pDate, StringFormat format ) const
{
    return KCalendarSystemCoptic::dayString( pDate, format );
}

int KCalendarSystemEthiopian::yearStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystemCoptic::yearStringToInteger( sNum, iLength );
}

int KCalendarSystemEthiopian::monthStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystemCoptic::monthStringToInteger( sNum, iLength );
}

int KCalendarSystemEthiopian::dayStringToInteger( const QString &sNum, int &iLength ) const
{
    return KCalendarSystemCoptic::dayStringToInteger( sNum, iLength );
}

QString KCalendarSystemEthiopian::formatDate( const QDate &date, KLocale::DateFormat format ) const
{
    return KCalendarSystemCoptic::formatDate( date, format );
}

QDate KCalendarSystemEthiopian::readDate( const QString &str, bool *ok ) const
{
    return KCalendarSystemCoptic::readDate( str, ok );
}

QDate KCalendarSystemEthiopian::readDate( const QString &intstr, const QString &fmt, bool *ok ) const
{
    return KCalendarSystemCoptic::readDate( intstr, fmt, ok );
}

QDate KCalendarSystemEthiopian::readDate( const QString &str, KLocale::ReadDateFlags flags, bool *ok ) const
{
    return KCalendarSystemCoptic::readDate( str, flags, ok );
}

int KCalendarSystemEthiopian::weekStartDay() const
{
    return KCalendarSystemCoptic::weekStartDay();
}

int KCalendarSystemEthiopian::weekDayOfPray() const
{
    return 7;
}

bool KCalendarSystemEthiopian::isLunar() const
{
    return KCalendarSystemCoptic::isLunar();
}

bool KCalendarSystemEthiopian::isLunisolar() const
{
    return KCalendarSystemCoptic::isLunisolar();
}

bool KCalendarSystemEthiopian::isSolar() const
{
    return KCalendarSystemCoptic::isSolar();
}

bool KCalendarSystemEthiopian::isProleptic() const
{
    return false;
}

bool KCalendarSystemEthiopian::julianDayToDate( int jd, int &year, int &month, int &day ) const
{
    return KCalendarSystemCoptic::julianDayToDate( jd, year, month, day );
}

bool KCalendarSystemEthiopian::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    return KCalendarSystemCoptic::dateToJulianDay( year, month, day, jd );
}
