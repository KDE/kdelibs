/*
    Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
    Copyright (c) 2002 Hans Petter Bieker <bieker@kde.org>
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

#include "kcalendarsystem.h"
#include "kcalendarsystemprivate_p.h"

#include "kglobal.h"
#include "kdebug.h"
#include "kconfiggroup.h"

#include <QtCore/QDateTime>

#include "kdatetime.h"
#include "kcalendarera_p.h"
#include "kcalendarsystemcoptic_p.h"
#include "kcalendarsystemethiopian_p.h"
#include "kcalendarsystemgregorian_p.h"
#include "kcalendarsystemgregorianproleptic_p.h"
#include "kcalendarsystemhebrew_p.h"
#include "kcalendarsystemhijri_p.h"
#include "kcalendarsystemindiannational_p.h"
#include "kcalendarsystemjalali_p.h"
#include "kcalendarsystemjapanese_p.h"
#include "kcalendarsystemjulian_p.h"
#include "kcalendarsystemminguo_p.h"
#include "kcalendarsystemthai_p.h"

KCalendarSystem *KCalendarSystem::create( const QString &calendarType, const KLocale *locale )
{
    return create( calendarType, KSharedConfig::Ptr(), locale );
}

KCalendarSystem *KCalendarSystem::create( const QString &calendarType, KSharedConfig::Ptr config,
                                          const KLocale *locale )
{
    if ( calendarType == "coptic" ) {
        return new KCalendarSystemCoptic( config, locale );
    }

    if ( calendarType == "ethiopian" ) {
        return new KCalendarSystemEthiopian( config, locale );
    }

    if ( calendarType == "gregorian" ) {
        return new KCalendarSystemGregorian( config, locale );
    }

    if ( calendarType == "gregorian-proleptic" ) {
        return new KCalendarSystemGregorianProleptic( config, locale );
    }

    if ( calendarType == "hebrew" ) {
        return new KCalendarSystemHebrew( config, locale );
    }

    if ( calendarType == "hijri" ) {
        return new KCalendarSystemHijri( config, locale );
    }

    if ( calendarType == "indian-national" ) {
        return new KCalendarSystemIndianNational( config, locale );
    }

    if ( calendarType == "jalali" ) {
        return new KCalendarSystemJalali( config, locale );
    }

    if ( calendarType == "japanese" ) {
        return new KCalendarSystemJapanese( config, locale );
    }

    if ( calendarType == "julian" ) {
        return new KCalendarSystemJulian( config, locale );
    }

    if ( calendarType == "minguo" ) {
        return new KCalendarSystemMinguo( config, locale );
    }

    if ( calendarType == "thai" ) {
        return new KCalendarSystemThai( config, locale );
    }

    // ### HPB: Should it really be a default here?
    return new KCalendarSystemGregorian( config, locale );
}

QStringList KCalendarSystem::calendarSystems()
{
    QStringList lst;

    lst.append( "coptic" );
    lst.append( "ethiopian" );
    lst.append( "gregorian" );
    //Do not return in list as we don't want used unless the client absolutely knows what they are doing
    //This is to prevent interop issues with the "gregorian" being a hybrid Julian/Gregorian, and to prevent
    //double listing of Gregorian confusing users about which to use.
    //lst.append( "gregorian-proleptic" );
    lst.append( "hebrew" );
    lst.append( "hijri" );
    lst.append( "indian-national" );
    lst.append( "jalali" );
    lst.append( "japanese" );
    lst.append( "julian" );
    lst.append( "minguo" );
    lst.append( "thai" );

    return lst;
}

QString KCalendarSystem::calendarLabel( const QString &calendarType )
{
    if ( calendarType == "coptic" ) {
        return ki18nc( "@item Calendar system", "Coptic" ).toString( KGlobal::locale() );
    }

    if ( calendarType == "ethiopian" ) {
        return ki18nc( "@item Calendar system", "Ethiopian" ).toString( KGlobal::locale() );
    }

    if ( calendarType == "gregorian" ) {
        return ki18nc( "@item Calendar system", "Gregorian" ).toString( KGlobal::locale() );
    }

    if ( calendarType == "gregorian-proleptic" ) {
        return ki18nc( "@item Calendar system", "Gregorian (Proleptic)" ).toString( KGlobal::locale() );
    }

    if ( calendarType == "hebrew" ) {
        return ki18nc( "@item Calendar system", "Hebrew" ).toString( KGlobal::locale() );
    }

    if ( calendarType == "hijri" ) {
        return ki18nc("@item Calendar system", "Hijri").toString( KGlobal::locale());
    }

    if ( calendarType == "indian-national" ) {
        return ki18nc("@item Calendar system", "Indian National").toString( KGlobal::locale());
    }

    if ( calendarType == "jalali" ) {
        return ki18nc( "@item Calendar system", "Jalali" ).toString( KGlobal::locale() );
    }

    if ( calendarType == "japanese" ) {
        return ki18nc( "@item Calendar system", "Japanese" ).toString( KGlobal::locale() );
    }

    if ( calendarType == "julian" ) {
        return ki18nc( "@item Calendar system", "Julian" ).toString( KGlobal::locale() );
    }

    if ( calendarType == "minguo" ) {
        return ki18nc( "@item Calendar system", "Taiwanese" ).toString( KGlobal::locale() );
    }

    if ( calendarType == "thai" ) {
        return ki18nc( "@item Calendar system", "Thai" ).toString( KGlobal::locale() );
    }

    return ki18nc( "@item Calendar system", "Invalid Calendar Type" ).toString( KGlobal::locale() );
}


// Shared d pointer base class definitions

KCalendarSystemPrivate::KCalendarSystemPrivate( KCalendarSystem *q_ptr ): q( q_ptr ),
                                                                          m_eraList( 0 )
{
}

KCalendarSystemPrivate::~KCalendarSystemPrivate()
{
    delete m_eraList;
}

// Dummy version as an example, remember to translate (see Gregorian for example)
// Add the Era's in chronilogical order, from earliest to latest
// This method MUST be re-implemented in any new Calendar System
void KCalendarSystemPrivate::initDefaultEraList()
{
    addEra( '-', 1, q->epoch().addDays( -1 ), -1, q->earliestValidDate(), "Before KDE", "BK", "%Ey %EC" );
    addEra( '+', 1, q->epoch(), 1, q->latestValidDate(), "Anno KDE", "AK", "%Ey %EC" );
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
int KCalendarSystemPrivate::monthsInYear( int year ) const
{
    Q_UNUSED( year )
    return 12;
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
int KCalendarSystemPrivate::daysInMonth( int year, int month ) const
{
    if ( month == 2 ) {
        if ( isLeapYear( year ) ) {
            return 29;
        } else {
            return 28;
        }
    }

    if ( month == 4 || month == 6 || month == 9 || month == 11 ) {
        return 30;
    }

    return 31;
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
int KCalendarSystemPrivate::daysInYear( int year ) const
{
    if ( isLeapYear( year ) ) {
        return 366;
    } else {
        return 365;
    }
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
int KCalendarSystemPrivate::daysInWeek() const
{
    return 7;
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
bool KCalendarSystemPrivate::isLeapYear( int year ) const
{
    if ( year < 1 ) {
        year = year + 1;
    }

    if ( year % 4 == 0 ) {
        if ( year % 100 != 0 ) {
            return true;
        } else if ( year % 400 == 0 ) {
            return true;
        }
    }

    return false;
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
bool KCalendarSystemPrivate::hasLeapMonths() const
{
    return false;
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
bool KCalendarSystemPrivate::hasYearZero() const
{
    return false;
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
int KCalendarSystemPrivate::maxDaysInWeek() const
{
    return 7;
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
int KCalendarSystemPrivate::maxMonthsInYear() const
{
    return 12;
}

// Convenince, faster than calling year( ealiestValidDate() ),
// needed in fake-virtual functions so don't remove
// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
int KCalendarSystemPrivate::earliestValidYear() const
{
    return -4712;
}

// Convenince, faster than calling year( latestValidDate() ),
// needed in fake-virtual functions so don't remove
// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
int KCalendarSystemPrivate::latestValidYear() const
{
    return 9999;
}

// Reimplement if special maths handling required, e.g. Hebrew.
// Works for calendars with constant number of months, or where leap month is last month of year
// Will not work for Hebrew or others where leap month is inserted in middle of year
void KCalendarSystemPrivate::dateDifference( const QDate &fromDate, const QDate &toDate,
                                             int *yearsDiff, int *monthsDiff, int *daysDiff, int *direction ) const
{
    // This could be optimised a little but is left in full as it's easier to understand
    int dy = 0;
    int dm = 0;
    int dd = 0;
    int dir = 1;

    if ( toDate < fromDate ) {
        dateDifference( toDate, fromDate, &dy, &dm, &dd, 0 );
        dir = -1;
    } else if ( toDate > fromDate ) {

        int fromYear = q->year( fromDate );
        int toYear = q->year( toDate );
        int fromMonth = q->month( fromDate );
        int toMonth = q->month( toDate );
        int fromDay = q->day( fromDate );
        int toDay = q->day( toDate );

        int monthsInPrevYear = monthsInYear( addYears( toYear, -1 ) );
        int daysInPrevMonth = q->daysInMonth( q->addMonths( toDate, -1 ) );
        int daysInFromMonth = daysInMonth( fromYear, fromMonth );
        int daysInToMonth = daysInMonth( toYear, toMonth );

        // Calculate years difference
        if ( toYear == fromYear ) {
            dy = 0;
        } else if ( toMonth > fromMonth ) {
            dy = differenceYearNumbers( fromYear, toYear );
        } else if ( toMonth < fromMonth ) {
            dy = differenceYearNumbers( fromYear, toYear ) - 1;
        } else { // toMonth == fromMonth
            // Allow for last day of month to last day of month and leap days
            // e.g. 2000-02-29 to 2001-02-28 is 1 year not 0 years
            if ( ( toDay >= fromDay ) || ( fromDay == daysInFromMonth && toDay == daysInToMonth ) ) {
                dy = differenceYearNumbers( fromYear, toYear );
            } else {
                dy = differenceYearNumbers( fromYear, toYear ) - 1;
            }
        }

        // Calculate months and days difference
        if ( toDay >= fromDay ) {
            dm = ( monthsInPrevYear + toMonth - fromMonth ) % monthsInPrevYear;
            dd = toDay - fromDay;
        } else { // toDay < fromDay
            // Allow for last day of month to last day of month and leap days
            // e.g. 2010-03-31 to 2010-04-30 is 1 month
            //      2000-02-29 to 2001-02-28 is 1 year
            //      2000-02-29 to 2001-03-01 is 1 year 1 day
            int prevMonth = q->month( q->addMonths( toDate, -1 ) );
            if ( fromDay == daysInFromMonth && toDay == daysInToMonth ) {
                dm = ( monthsInPrevYear + toMonth - fromMonth ) % monthsInPrevYear;
                dd = 0;
            } else if ( prevMonth == fromMonth && daysInPrevMonth < daysInFromMonth ) {
                // Special case where fromDate = leap day and toDate in month following but non-leap year
                // e.g. 2000-02-29 to 2001-03-01 needs to use 29 to calculate day number not 28
                dm = ( monthsInPrevYear + toMonth - fromMonth - 1 ) % monthsInPrevYear;
                dd = ( daysInFromMonth + toDay - fromDay ) % daysInFromMonth;
            } else {
                dm = ( monthsInPrevYear + toMonth - fromMonth - 1 ) % monthsInPrevYear;
                dd = ( daysInPrevMonth + toDay - fromDay ) % daysInPrevMonth;
            }
        }

    }

    // Only return values if we have a valid pointer
    if ( yearsDiff ) {
        *yearsDiff = dy;
    }
    if ( monthsDiff ) {
        *monthsDiff = dm;
    }
    if ( daysDiff ) {
        *daysDiff = dd;
    }
    if ( direction ) {
        *direction = dir;
    }
}

// Reimplement if special maths handling required, e.g. Hebrew
// Allows for calendars with leap months at end of year but not during year
int KCalendarSystemPrivate::yearsDifference( const QDate &fromDate, const QDate &toDate ) const
{
    // This could be optimised a little but is left in full as it's easier to understand
    // Alternatively could just call dateDifference(), but this is slightly more efficient

    if ( toDate < fromDate ) {
        return 0 - yearsDifference( toDate, fromDate );
    }

    if ( toDate == fromDate ) {
        return 0;
    }

    int fromYear = q->year( fromDate );
    int toYear = q->year( toDate );

    if ( toYear == fromYear ) {
        return 0;
    }

    int fromMonth = q->month( fromDate );
    int toMonth = q->month( toDate );

    if ( toMonth > fromMonth ) {
        return differenceYearNumbers( fromYear, toYear );
    }

    if ( toMonth < fromMonth ) {
        return differenceYearNumbers( fromYear, toYear ) - 1;
    }

    // toMonth == fromMonth
    int fromDay = q->day( fromDate );
    int toDay = q->day( toDate );

    // Adjust for month numbers in from and to year
    // Allow for last day of month to last day of month and leap days
    // e.g. 2000-02-29 to 2001-02-28 is 1 year not 0 years
    if ( ( toDay >= fromDay ) ||
         ( fromDay == daysInMonth( fromYear, fromMonth ) &&
           toDay == daysInMonth( toYear, toMonth ) ) ) {
        return differenceYearNumbers( fromYear, toYear );
    } else {
        return differenceYearNumbers( fromYear, toYear ) - 1;
    }

}

// Reimplement if special maths handling required, e.g. maybe Hebrew?
// Allows for calendars with leap months
int KCalendarSystemPrivate::monthsDifference( const QDate &fromDate, const QDate &toDate ) const
{
    if ( toDate < fromDate ) {
        return 0 - monthsDifference( toDate, fromDate );
    }

    if ( toDate == fromDate ) {
        return 0;
    }

    int fromYear = q->year( fromDate );
    int toYear = q->year( toDate );
    int fromMonth = q->month( fromDate );
    int toMonth = q->month( toDate );
    int fromDay = q->day( fromDate );
    int toDay = q->day( toDate );

    int monthsInPreceedingYears;

    // Calculate number of months in full years preceeding toYear
    if ( toYear == fromYear ) {
        monthsInPreceedingYears = 0;
    } else if ( hasLeapMonths() ) {
        monthsInPreceedingYears = 0;
        for ( int y = fromYear; y < toYear; y = addYears( y, 1 ) ) {
            monthsInPreceedingYears = monthsInPreceedingYears + monthsInYear( y );
        }
    } else {
        monthsInPreceedingYears = differenceYearNumbers( fromYear, toYear ) * monthsInYear( toYear );
    }

    // Adjust for months in from and to year
    // Allow for last day of month to last day of month and leap days
    // e.g. 2010-03-31 to 2010-04-30 is 1 month not 0 months
    // also 2000-02-29 to 2001-02-28 is 12 months not 11 months
    if ( ( toDay >= fromDay ) ||
         ( fromDay == daysInMonth( fromYear, fromMonth ) &&
           toDay == daysInMonth( toYear, toMonth ) ) ) {
        return monthsInPreceedingYears + toMonth - fromMonth;
    } else {
        return monthsInPreceedingYears + toMonth - fromMonth - 1;
    }
}

// Reimplement if special string handling required
// Format an input date to match a POSIX date format string
QString KCalendarSystemPrivate::formatDatePosix( const QDate &fromDate,
                                                 const QString &toFormat, KLocale::DigitSet digitSet,
                                                 KLocale::DateTimeFormatStandard standard ) const
{
    QChar thisChar;
    QString buffer;

    bool escape = false;

    bool escapeWidth = false;
    int padWidth = 0;

    bool escapePad = false;
    QChar padChar = '0';

    bool escapeMod = false;
    QChar modifierChar = QChar();

    QChar caseChar = QChar();

    QChar signChar = QChar();

    int yy, mm, dd;
    q->getDate( fromDate, &yy, &mm, &dd );

    for ( int format_index = 0; format_index < toFormat.length(); ++format_index ) {

        thisChar = toFormat.at( format_index ).unicode();

        if ( !escape ) {

            if ( thisChar == '%' ) {
                escape = true;
            } else {
                buffer.append( toFormat.at( format_index ) );
            }

        } else if ( !escapeMod && !escapeWidth && thisChar == '-' ) { // no padding

            padChar = '\0';
            escapePad = true;

        } else if ( !escapeMod && !escapeWidth && thisChar == '_' ) { // space padding

            padChar = ' ';
            escapePad = true;

        } else if ( !escapeMod && !escapeWidth && thisChar == '0' ) { // 0 padding

            padChar = '0';
            escapePad = true;

        } else if ( !escapeMod && !escapeWidth && ( thisChar == '^' || thisChar == '#' ) ) { // Change case

            caseChar = thisChar;

        } else if ( !escapeMod &&
                    ( ( !escapeWidth && thisChar >= '1' && thisChar <= '9' ) ||
                      ( escapeWidth && thisChar >= '0' && thisChar <= '9' ) ) ) { // Change width

            if ( escapeWidth ) {
                padWidth = padWidth * 10;
            }
            padWidth = padWidth + QString( thisChar ).toInt();
            escapeWidth = true;

        } else if ( !escapeMod && ( thisChar == 'E' || thisChar == 'O' ) ) { // Set modifier

            escapeMod = true;
            modifierChar = thisChar;

        } else {

            QString componentString;
            int componentInteger = 0;
            int minWidth = 0;
            int isoWeekYear = yy;
            QDate yearDate;

            //Default settings unless overridden by pad and case flags and width: are 0 pad to 0 width no sign
            //Names will override 0 pad with no pad unless flagged
            //Numbers will override with correct width unless flagged
            switch ( toFormat.at( format_index ).unicode() ) {
                case '%':  //Literal %
                    componentString = QLatin1Char('%');
                    if ( !escapePad ) {
                        padChar = QChar();
                    }
                    break;
                case 't':  //Tab
                    componentString = "\t";
                    if ( !escapePad ) {
                        padChar = QChar();
                    }
                    break;
                case 'Y':
                    if ( modifierChar == 'E' ) {  //Era Year, default no pad to 0 places no sign
                        if ( !escapePad ) {
                            padChar = ' ';
                        }
                        componentString = q->eraYear( fromDate );
                    } else {  //Long year numeric, default 0 pad to 4 places with sign
                        componentInteger = qAbs( yy );
                        minWidth = 4;
                        if ( yy < 0 ) {
                            signChar = '-';
                        }
                    }
                    break;
                case 'C':
                    if ( modifierChar == 'E' ) {  //Era name, default no pad to 0 places no sign
                        if ( !escapePad ) {
                            padChar = ' ';
                        }
                        componentString = q->eraName( fromDate );
                    } else {  //Century numeric, default 0 pad to 2 places with sign
                        componentInteger =  qAbs( yy ) / 100 ;
                        minWidth = 2;
                        if ( yy < 0 ) {
                            signChar = '-';
                        }
                    }
                    break;
                case 'y':
                    if ( modifierChar == 'E' ) {  //Year in Era number, default 0 pad to 1 places no sign
                        componentInteger =  q->yearInEra( fromDate );
                        minWidth = 1;
                    } else {  //Short year numeric, default 0 pad to 2 places with sign
                        componentInteger =  qAbs( yy ) % 100;
                        minWidth = 2;
                        if ( yy < 0 ) {
                            signChar = '-';
                        }
                    }
                    break;
                case 'm':  //Long month numeric, default 0 pad to 2 places no sign
                    componentInteger =  mm;
                    minWidth = 2;
                    break;
                case 'n':
                    //PosixFormat %n is newline
                    //KdeFormat %n is short month
                    if ( standard == KLocale::KdeFormat ) {
                        //Copy what %e does, no padding by default
                        //Short month numeric, default no pad to 1 places no sign
                        componentInteger =  mm;
                        minWidth = 1;
                        if ( !escapePad ) {
                            padChar = QChar();
                        }
                    } else {  // standard == KLocale::PosixFormat
                        componentString = '\n';
                    }
                    break;
                case 'd':  //Long day numeric, default 0 pad to 2 places no sign
                    componentInteger =  dd;
                    minWidth = 2;
                    break;
                case 'e':  //Short day numeric, default no sign
                    //PosixFormat %e is space pad to 2 places
                    //KdeFormat %n is no pad to 1 place
                    componentInteger =  dd;
                    if ( standard == KLocale::KdeFormat ) {
                        minWidth = 1;
                        if ( !escapePad ) {
                            padChar = QChar();
                        }
                    } else {  // standard == KLocale::PosixFormat
                        minWidth = 2;
                        if ( !escapePad ) {
                            padChar = ' ';
                        }
                    }
                    break;
                case 'B':  //Long month name, default space pad to 0 places no sign
                    if ( q->locale()->dateMonthNamePossessive() ) {
                        componentString = q->monthName( mm, yy, KCalendarSystem::LongNamePossessive );
                    } else {
                        componentString = q->monthName( mm, yy, KCalendarSystem::LongName );
                    }
                    if ( !escapePad ) {
                        padChar = ' ';
                    }
                    break;
                case 'h':  //Short month name, default space pad to 0 places no sign
                case 'b':  //Short month name, default space pad to 0 places no sign
                    if ( q->locale()->dateMonthNamePossessive() ) {
                        componentString = q->monthName( mm, yy, KCalendarSystem::ShortNamePossessive );
                    } else {
                        componentString = q->monthName( mm, yy, KCalendarSystem::ShortName );
                    }
                    if ( !escapePad ) {
                        padChar = ' ';
                    }
                    break;
                case 'A':  //Long weekday name, default space pad to 0 places no sign
                    if ( !escapePad ) {
                        padChar = ' ';
                    }
                    componentString = q->weekDayName( fromDate, KCalendarSystem::LongDayName );
                    break;
                case 'a':  //Short weekday name, default space pad to 0 places no sign
                    componentString = q->weekDayName( fromDate, KCalendarSystem::ShortDayName );
                    if ( !escapePad ) {
                        padChar = ' ';
                    }
                    break;
                case 'j':  //Long day of year numeric, default 0 pad to 3 places no sign
                    componentInteger = q->dayOfYear( fromDate );
                    minWidth = 3;
                    break;
                case 'V':  //Long ISO week of year numeric, default 0 pad to 2 places no sign
                    componentInteger = q->weekNumber( fromDate );
                    minWidth = 2;
                    break;
                case 'G':  //Long year of ISO week of year numeric, default 0 pad to 4 places with sign
                    q->weekNumber( fromDate, &isoWeekYear );
                    q->setDate( yearDate, isoWeekYear, 1, 1 );
                    componentInteger = qAbs( isoWeekYear );
                    minWidth = 4;
                    if ( isoWeekYear < 0 ) {
                        signChar = '-';
                    }
                    break;
                case 'g':  //Short year of ISO week of year numeric, default 0 pad to 2 places with sign
                    q->weekNumber( fromDate, &isoWeekYear );
                    q->setDate( yearDate, isoWeekYear, 1, 1 );
                    componentInteger = qAbs( isoWeekYear ) % 100;
                    minWidth = 2;
                    if ( isoWeekYear < 0 ) {
                        signChar = '-';
                    }
                    break;
                case 'u':  //Short day of week numeric
                    componentInteger = q->dayOfWeek( fromDate );
                    minWidth = 1;
                    break;
                case 'D':  //US short date format, ignore any overrides
                    componentString = formatDatePosix( fromDate, "%m/%d/%y", digitSet, standard );
                    padWidth = 0;
                    padChar = QChar();
                    caseChar = QChar();
                    break;
                case 'F':  //Full or ISO short date format, ignore any overrides
                    componentString = formatDatePosix( fromDate, "%Y-%m-%d", digitSet, standard );
                    padWidth = 0;
                    padChar = QChar();
                    caseChar = QChar();
                    break;
                case 'x':  //Locale short date format, ignore any overrides
                    componentString = formatDatePosix( fromDate, q->locale()->dateFormatShort(), digitSet, standard );
                    padWidth = 0;
                    padChar = QChar();
                    caseChar = QChar();
                    break;
                default:  //No valid format code, treat as literal but apply any overrides
                    //GNU date returns all chars since and including % and applies the overrides which seems wrong.
                    componentString = toFormat.at( format_index );
                    break;
            }

            if ( componentString.isEmpty() ) {
                padWidth = qMax( minWidth, padWidth );
                componentString = stringFromInteger( componentInteger, padWidth, padChar, digitSet );
                if ( !signChar.isNull() ) {
                    componentString.prepend( signChar );
                }
            } else {
                if ( padChar != '\0' && padWidth != 0 ) {
                    componentString = componentString.rightJustified( padWidth, padChar );
                }

                if ( caseChar == '^' ) {
                    componentString = componentString.toUpper();
                } else if ( caseChar == '#' ) {
                    componentString = componentString.toUpper(); // JPL ???
                }
            }

            buffer.append( componentString );

            escape = false;
            escapePad = false;
            padChar = '0';
            escapeMod = false;
            modifierChar = QChar();
            caseChar = QChar();
            escapeWidth = false;
            padWidth = 0;
            signChar = QChar();
        }
    }

    return buffer;
}

// Original QDate::getFmtString() code taken from Qt 4.7 under LGPL, now heavily modifed
// Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
// Replaces tokens by their value. See QDateTime::toString() for a list of valid tokens
QString KCalendarSystemPrivate::getUnicodeString( const QDate &fromDate, const QString &toFormat ) const
{
    if ( toFormat.isEmpty() ) {
        return QString();
    }

    QString buffer = toFormat;
    int removed = 0;

    if ( toFormat.startsWith( QLatin1String( "dddd" ) ) ) {
        buffer = q->weekDayName( fromDate, KCalendarSystem::LongDayName );
        removed = 4;
    } else if ( toFormat.startsWith(QLatin1String( "ddd" ) ) ) {
        buffer = q->weekDayName( fromDate, KCalendarSystem::ShortDayName );
        removed = 3;
    } else if ( toFormat.startsWith( QLatin1String( "dd" ) ) ) {
        buffer = QString::number( q->day( fromDate ) ).rightJustified( 2, QLatin1Char('0'), true );
        removed = 2;
    } else if ( toFormat.at(0) == QLatin1Char('d') ) {
        buffer = QString::number( q->day( fromDate ) );
        removed = 1;
    } else if (toFormat.startsWith(QLatin1String("MMMM"))) {
        buffer = q->monthName( q->month( fromDate ), q->year( fromDate ), KCalendarSystem::LongName );
        removed = 4;
    } else if (toFormat.startsWith(QLatin1String("MMM"))) {
        buffer = q->monthName( q->month( fromDate ), q->year( fromDate ), KCalendarSystem::ShortName );
        removed = 3;
    } else if (toFormat.startsWith(QLatin1String("MM"))) {
        buffer = QString::number( q->month( fromDate ) ).rightJustified( 2, QLatin1Char('0'), true );
        removed = 2;
    } else if (toFormat.at(0) == QLatin1Char('M')) {
        buffer = QString::number( q->month( fromDate ) );
        removed = 1;
    } else if (toFormat.startsWith(QLatin1String("yyyy"))) {
        const int year = q->year( fromDate );
        buffer = QString::number( qAbs( year ) ).rightJustified( 4, QLatin1Char('0') );
        if( year > 0 )
            removed = 4;
        else
        {
            buffer.prepend( QLatin1Char('-') );
            removed = 5;
        }

    } else if ( toFormat.startsWith( QLatin1String("yy") ) ) {
        buffer = QString::number( q->year(fromDate) ).right( 2 ).rightJustified( 2, QLatin1Char('0') );
        removed = 2;
    }

    if ( removed == 0 || removed >= toFormat.size() ) {
        return buffer;
    }

    return buffer + getUnicodeString( fromDate, toFormat.mid( removed ) );
}

// Reimplement if special string handling required
// Format an input date to match a UNICODE date format string
// Original QDate::fmtDateTime() code taken from Qt 4.7 under LGPL, now heavily modifed
// Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
QString KCalendarSystemPrivate::formatDateUnicode( const QDate &fromDate, const QString &toFormat,
                                                   KLocale::DigitSet digitSet ) const
{
    const QLatin1Char quote('\'');

    QString buffer;
    QString format;
    QChar status(QLatin1Char('0'));

    for (int i = 0; i < (int)toFormat.length(); ++i) {
        if (toFormat.at(i) == quote) {
            if (status == quote) {
                if (i > 0 && toFormat.at(i - 1) == quote)
                    buffer += QLatin1Char('\'');
                status = QLatin1Char('0');
            } else {
                if (!format.isEmpty()) {
                    buffer += getUnicodeString( fromDate, format );
                    format.clear();
                }
                status = quote;
            }
        } else if (status == quote) {
            buffer += toFormat.at(i);
        } else if (toFormat.at(i) == status) {
            if ( toFormat.at(i) == QLatin1Char('P') ||
                 toFormat.at(i) == QLatin1Char('p') ) {
                status = QLatin1Char('0');
            }
            format += toFormat.at( i );
        } else {
            buffer += getUnicodeString( fromDate, format );
            format.clear();
            if ( ( toFormat.at(i) == QLatin1Char('d') ) ||
                 ( toFormat.at(i) == QLatin1Char('M') ) ||
                 ( toFormat.at(i) == QLatin1Char('y') ) ) {
                status = toFormat.at( i );
                format += toFormat.at( i );
            } else {
                buffer += toFormat.at( i );
                status = QLatin1Char('0');
            }
        }
    }

    buffer += getUnicodeString( fromDate, format );

    return buffer;
}

// Reimplement if special string handling required
// Parse an input string to match a POSIX date format string and return any components found
DateComponents KCalendarSystemPrivate::parseDatePosix( const QString &inputString, const QString &formatString,
                                                       KLocale::DateTimeFormatStandard standard  ) const
{
    QString str = inputString.simplified().toLower();
    QString fmt = formatString.simplified();
    int dd = -1;
    int mm = -1;
    int yy = 0;
    bool parsedYear = false;
    int ey = -1;
    QString ee;
    int dayInYear = -1;
    int isoWeekNumber = -1;
    int dayOfIsoWeek = -1;
    int strpos = 0;
    int fmtpos = 0;
    int readLength; // Temporary variable used when reading input
    bool error = false;

    while ( fmt.length() > fmtpos && str.length() > strpos && !error ) {

        QChar fmtChar = fmt.at( fmtpos++ );

        if ( fmtChar != '%' ) {

            if ( fmtChar.isSpace() && str.at(strpos).isSpace() ) {
                strpos++;
            } else if ( fmtChar.toLower() == str.at(strpos) ) {
                strpos++;
            } else {
                error = true;
            }

        } else {
            int j;
            QString shortName, longName;
            QChar modifierChar;
            // remove space at the beginning
            if ( str.length() > strpos && str.at( strpos ).isSpace() ) {
                strpos++;
            }

            fmtChar = fmt.at( fmtpos++ );
            if ( fmtChar == 'E' ) {
                modifierChar = fmtChar;
                fmtChar = fmt.at( fmtpos++ );
            }

            switch ( fmtChar.unicode() )
            {
                case 'a':  // Weekday Name Short
                case 'A':  // Weekday Name Long
                    error = true;
                    j = 1;
                    while ( error && j <= maxDaysInWeek() ) {
                        shortName = q->weekDayName( j, KCalendarSystem::ShortDayName ).toLower();
                        longName = q->weekDayName( j, KCalendarSystem::LongDayName ).toLower();
                        if ( str.mid( strpos, longName.length() ) == longName ) {
                            strpos += longName.length();
                            error = false;
                        } else if ( str.mid( strpos, shortName.length() ) == shortName ) {
                            strpos += shortName.length();
                            error = false;
                        }
                        ++j;
                    }
                    break;
                case 'b':  // Month Name Short
                case 'h':  // Month Name Short
                case 'B':  // Month Name Long
                    error = true;
                    j = 1;
                    while ( error && j <= maxMonthsInYear() ) {
                        // This may be a problem in calendar systems with variable number of months
                        // in the year and/or names of months that change depending on the year, e.g
                        // Hebrew.  We really need to know the correct year first, but we may not have
                        // read it yet and will be using the current year instead
                        int monthYear;
                        if ( parsedYear ) {
                            monthYear = yy;
                        } else {
                            monthYear = q->year( QDate::currentDate() );
                        }
                        if ( q->locale()->dateMonthNamePossessive() ) {
                            shortName = q->monthName( j, yy, KCalendarSystem::ShortNamePossessive ).toLower();
                            longName = q->monthName( j, yy, KCalendarSystem::LongNamePossessive ).toLower();
                        } else {
                            shortName = q->monthName( j, yy, KCalendarSystem::ShortName ).toLower();
                            longName = q->monthName( j, yy, KCalendarSystem::LongName ).toLower();
                        }
                        if ( str.mid( strpos, longName.length() ) == longName ) {
                            mm = j;
                            strpos += longName.length();
                            error = false;
                        } else if ( str.mid( strpos, shortName.length() ) == shortName ) {
                            mm = j;
                            strpos += shortName.length();
                            error = false;
                        }
                        ++j;
                    }
                    break;
                case 'd': // Day Number Long
                case 'e': // Day Number Short
                    dd = q->dayStringToInteger( str.mid( strpos ), readLength );
                    strpos += readLength;
                    error = readLength <= 0;
                    break;
                case 'n':
                    // PosixFormat %n is Newline
                    // KdeFormat %n is Month Number Short
                    if ( standard == KLocale::KdeFormat ) {
                        mm = q->monthStringToInteger( str.mid( strpos ), readLength );
                        strpos += readLength;
                        error = readLength <= 0;
                    }
                    // standard == KLocale::PosixFormat
                    // all whitespace already 'eaten', no action required
                    break;
                case 'm': // Month Number Long
                    mm = q->monthStringToInteger( str.mid( strpos ), readLength );
                    strpos += readLength;
                    error = readLength <= 0;
                    break;
                case 'Y': // Year Number Long
                case 'y': // Year Number Short
                    if ( modifierChar == 'E' ) {  // Year In Era
                        if ( fmtChar == 'y' ) {
                            ey = q->yearStringToInteger( str.mid( strpos ), readLength );
                            strpos += readLength;
                            error = readLength <= 0;
                        } else {
                            error = true;
                            j = m_eraList->count() -1; // Start with the most recent
                            while ( error && j >= 0  ) {
                                QString subFormat = m_eraList->at( j ).format();
                                QString subInput = str.mid( strpos );
                                DateComponents subResult = parseDatePosix( subInput, subFormat, standard );
                                if ( !subResult.error ) {
                                    if ( subResult.parsedYear ) {
                                        yy = subResult.year;
                                        parsedYear = true;
                                        error = false;
                                        strpos += subResult.inputPosition;
                                    } else if ( !subResult.eraName.isEmpty() && subResult.yearInEra >= 0 ) {
                                        ee = subResult.eraName;
                                        ey = subResult.yearInEra;
                                        error = false;
                                        strpos += subResult.inputPosition;
                                    }
                                }
                                --j;
                            }
                        }
                    } else {
                        yy = q->yearStringToInteger( str.mid( strpos ), readLength );
                        strpos += readLength;
                        // JPL are we sure about this? Do users really want 99 = 2099 or 1999? Should we use a Y2K style range?
                        // Using 2000 only valid for Gregorian, Hebrew should be 5000, etc.
                        if ( fmtChar == 'y' && yy >= 0 && yy < 100 ) {
                            yy += 2000; // QDate assumes 19xx by default, but this isn't what users want...
                        }
                        error = readLength <= 0;
                        if ( !error ) {
                            parsedYear = true;
                        }
                    }
                    break;
                case 'C': // Era
                    error = true;
                    if ( modifierChar == 'E' ) {
                        j = m_eraList->count() -1; // Start with the most recent
                        while ( error && j >= 0  ) {
                            shortName = m_eraList->at( j ).shortName().toLower();
                            longName = m_eraList->at( j ).name().toLower();
                            if ( str.mid( strpos, longName.length() ) == longName ) {
                                strpos += longName.length();
                                ee = longName;
                                error = false;
                            } else if ( str.mid( strpos, shortName.length() ) == shortName ) {
                                strpos += shortName.length();
                                ee = shortName;
                                error = false;
                            }
                            --j;
                        }
                    }
                    break;
                case 'j': // Day Of Year Number
                    dayInYear = integerFromString( str.mid( strpos ), 3, readLength );
                    strpos += readLength;
                    error = readLength <= 0;
                    break;
                case 'V': // ISO Week Number
                    isoWeekNumber = integerFromString( str.mid( strpos ), 2, readLength );
                    strpos += readLength;
                    error = readLength <= 0;
                    break;
                case 'u': // ISO Day Of Week
                    dayOfIsoWeek = integerFromString( str.mid( strpos ), 1, readLength );
                    strpos += readLength;
                    error = readLength <= 0;
                    break;
            }
        }
    }

    DateComponents result;
    result.error = error;
    result.inputPosition = strpos;
    result.formatPosition = fmtpos;
    if ( error ) {
        result.day = -1;
        result.month = -1;
        result.year = 0;
        result.parsedYear = false;
        result.eraName = QString();
        result.yearInEra = -1;
        result.dayInYear = -1;
        result.isoWeekNumber = -1;
        result.dayOfIsoWeek = -1;
    } else {
        result.day = dd;
        result.month = mm;
        result.year = yy;
        result.parsedYear = parsedYear;
        result.eraName = ee;
        result.yearInEra = ey;
        result.dayInYear = dayInYear;
        result.isoWeekNumber = isoWeekNumber;
        result.dayOfIsoWeek = dayOfIsoWeek;
    }
    return result;
}

// Reimplement if special string handling required
// Parse an input string to match a UNICODE date format string and return any components found
DateComponents KCalendarSystemPrivate::parseDateUnicode( const QString &inputString, const QString &formatString ) const
{
    QString str = inputString.simplified().toLower();
    QString fmt = formatString.simplified();
    int dd = -1;
    int mm = -1;
    int yy = 0;
    bool parsedYear = false;
    int ey = -1;
    QString ee;
    int dayInYear = -1;
    int isoWeekNumber = -1;
    int dayOfIsoWeek = -1;
    int strpos = 0;
    int fmtpos = 0;
    int readLength; // Temporary variable used when reading input
    bool error = false;

    DateComponents result;
    result.error = error;
    result.inputPosition = strpos;
    result.formatPosition = fmtpos;
    if ( error ) {
        result.day = -1;
        result.month = -1;
        result.year = 0;
        result.parsedYear = false;
        result.eraName = QString();
        result.yearInEra = -1;
        result.dayInYear = -1;
        result.isoWeekNumber = -1;
        result.dayOfIsoWeek = -1;
    } else {
        result.day = dd;
        result.month = mm;
        result.year = yy;
        result.parsedYear = parsedYear;
        result.eraName = ee;
        result.yearInEra = ey;
        result.dayInYear = dayInYear;
        result.isoWeekNumber = isoWeekNumber;
        result.dayOfIsoWeek = dayOfIsoWeek;
    }
    return result;
}

// Reimplement if special string to integer handling required, e.g. Hebrew.
// Peel a number off the front of a string which may have other trailing chars after the number
// Stop either at either maxLength, eos, or first non-digit char
int KCalendarSystemPrivate::integerFromString( const QString &string, int maxLength, int &readLength ) const
{
    int value = -1;
    int position = 0;
    readLength = 0;
    bool ok = false;

    if ( maxLength < 0 ) {
        maxLength = string.length();
    }

    while ( position < string.length() &&
            position < maxLength &&
            string.at( position ).isDigit() ) {
        position++;
    }

    if ( position > 0 ) {
        value = string.left( position ).toInt( &ok );
        if ( ok ) {
            readLength = position;
        } else {
            value = -1;
        }
    }

    return value;
}

// Reimplement if special integer to string handling required, e.g. Hebrew.
// Utility to convert an integer into the correct display string form
QString KCalendarSystemPrivate::stringFromInteger( int number, int padWidth, QChar padChar ) const
{
    return stringFromInteger( number, padWidth, padChar, q->locale()->dateTimeDigitSet() );
}

// Reimplement if special integer to string handling required, e.g. Hebrew.
// Utility to convert an integer into the correct display string form
QString KCalendarSystemPrivate::stringFromInteger( int number, int padWidth, QChar padChar, KLocale::DigitSet digitSet ) const
{
    if ( padChar == '\0' || padWidth == 0 ) {
        return q->locale()->convertDigits( QString::number( number ), digitSet );
    } else {
        return q->locale()->convertDigits( QString::number( number ).rightJustified( padWidth, padChar ), digitSet );
    }
}

// Allows us to set dates outside publically valid range, USE WITH CARE!!!!
bool KCalendarSystemPrivate::setAnyDate( QDate &date, int year, int month, int day ) const
{
    int jd;
    q->dateToJulianDay( year, month, day, jd );
    date = QDate::fromJulianDay( jd );
    return true;
}

// Utility to correctly add years to a year number because some systems such as
// Julian and Gregorian calendars don't have a year 0
int KCalendarSystemPrivate::addYears( int originalYear, int addYears ) const
{
    int newYear = originalYear + addYears;

    if ( !hasYearZero() ) {
        if ( originalYear > 0 && newYear <= 0 ) {
            newYear = newYear - 1;
        } else if ( originalYear < 0 && newYear >= 0 ) {
            newYear = newYear + 1;
        }
    }

    return newYear;
}

// Utility to correctly return number of years between two year numbers because some systems such as
// Julian and Gregorian calendars don't have a year 0
int KCalendarSystemPrivate::differenceYearNumbers( int fromYear, int toYear ) const
{
    int dy = toYear - fromYear;

    if ( !hasYearZero() ) {
        if ( toYear > 0 && fromYear < 0 ) {
            dy = dy - 1;
        } else if ( toYear < 0 && fromYear > 0 ) {
            dy = dy + 1;
        }
    }

    return dy;
}

QDate KCalendarSystemPrivate::invalidDate() const
{
    //Is QDate's way of saying is invalid
    return QDate();
}

QString KCalendarSystemPrivate::simpleDateString( const QString &str ) const
{
    QString newStr;
    for ( int i = 0; i < str.length(); i++ ) {
        if ( str.at(i).isLetterOrNumber() ) {
            newStr.append( str.at(i) );
        } else {
            newStr.append( ' ' );
        }
    }
    newStr.simplified();
    return newStr;
}

QDate KCalendarSystemPrivate::firstDayOfYear( int year ) const
{
    int jd;
    q->dateToJulianDay( year, 1, 1, jd );
    return QDate::fromJulianDay( jd );
}

QDate KCalendarSystemPrivate::lastDayOfYear( int year ) const
{
    int jd;
    q->dateToJulianDay( year, 1, 1, jd );
    jd = jd + daysInYear( year ) - 1;
    return QDate::fromJulianDay( jd );
}

const KLocale * KCalendarSystemPrivate::locale() const
{
    if ( m_locale ) {
        return m_locale;
    } else {
        return KGlobal::locale();
    }
}

KCalendarEra KCalendarSystemPrivate::era( const QDate &eraDate ) const
{
    for ( int i = m_eraList->count() -1; i >= 0; --i ) {
        if ( m_eraList->at( i ).isInEra( eraDate ) ) {
            return m_eraList->at( i );
        }
    }
    return KCalendarEra();
}

KCalendarEra KCalendarSystemPrivate::era( const QString &eraName, int yearInEra ) const
{
    Q_UNUSED( yearInEra )

    for ( int i = m_eraList->count() - 1; i >= 0; --i ) {
        KCalendarEra era = m_eraList->at( i );
        if ( era.name().toLower() == eraName.toLower() ||
             era.shortName().toLower() == eraName.toLower() ) {
            return era;
        }
    }
    return KCalendarEra();
}

void KCalendarSystemPrivate::initialiseEraList( const QString & calendarType )
{
    delete m_eraList;
    m_eraList = new QList<KCalendarEra>;
    loadGlobalEraList( calendarType );
    if ( m_eraList->isEmpty() ) {
        initDefaultEraList();
    }
}

void KCalendarSystemPrivate::loadGlobalEraList( const QString & calendarType )
{
    m_eraList->clear();
    KConfigGroup cg( config(), QString( "KCalendarSystem %1" ).arg( calendarType ) );
    if ( cg.exists() ) {
        int eraCount = cg.readEntry( "EraCount", 0 );
        for ( int i = 1; i <= eraCount; ++i ) {
            QString eraEntry = cg.readEntry( QString( "Era%1" ).arg( i ), QString() );
            if ( !eraEntry.isEmpty() ) {
                // Based on LC_TIME, but different!
                // Includes long and short names, uses ISO fomat dates
                // e.g. +:1:0001-01-01:9999-12-31:Anno Domini:AD:%EC %Ey
                QChar direction = eraEntry.section( ':', 0, 0 ).at( 0 );
                QDate startDate, endDate;
                int startYear;
                QString buffer = eraEntry.section( ':', 2, 2 );
                if ( buffer.isEmpty() ) {
                    if ( direction == '-' ) {
                        startDate = q->latestValidDate();
                    } else {
                        startDate = q->earliestValidDate();
                    }
                } else {
                    startDate = q->readDate( buffer, KLocale::IsoFormat );
                    if ( q->isValid( startDate ) ) {
                        startYear = q->year( startDate );
                    } else {
                        startYear = eraEntry.section( ':', 1, 1 ).toInt(); //Use offset
                    }
                }
                buffer = eraEntry.section( ':', 3, 3 );
                if ( buffer.isEmpty() ) {
                    if ( direction == '-' ) {
                        endDate = q->earliestValidDate();
                    } else {
                        endDate = q->latestValidDate();
                    }
                } else {
                    endDate = q->readDate( buffer, KLocale::IsoFormat );
                }
                addEra( direction, eraEntry.section( ':', 1, 1 ).toInt(),
                        startDate, startYear, endDate, eraEntry.section( ':', 4, 4 ),
                        eraEntry.section( ':', 5, 5 ), eraEntry.section( ':', 6 ) );
            }
        }
    }
}

void KCalendarSystemPrivate::addEra( const QChar &direction, int offset,
                                     const QDate &startDate, int startYear, const QDate &endDate,
                                     const QString &name, const QString &shortName,
                                     const QString &format )
{
    KCalendarEra newEra;

    newEra.m_sequence = m_eraList->count() + 1;
    if ( direction == QChar('-') ) {
        newEra.m_direction = -1;
    } else {
        newEra.m_direction = 1;
    }
    newEra.m_offset = offset;
    newEra.m_startDate = startDate;
    newEra.m_startYear = startYear;
    newEra.m_endDate = endDate;
    newEra.m_name = name;
    newEra.m_shortName = shortName;
    newEra.m_format = format;

    m_eraList->append( newEra );
}

KSharedConfig::Ptr KCalendarSystemPrivate::config()
{
    if ( m_config == KSharedConfig::Ptr() ) {
        return KGlobal::config();
    } else {
        return m_config;
    }
}


KCalendarSystem::KCalendarSystem( const KLocale *locale )
               : d_ptr( new KCalendarSystemPrivate( this ) )
{
    d_ptr->m_config = KSharedConfig::Ptr();
    d_ptr->m_locale = locale;
}

KCalendarSystem::KCalendarSystem( const KSharedConfig::Ptr config, const KLocale *locale )
               : d_ptr( new KCalendarSystemPrivate( this ) )
{
    d_ptr->m_config = config;
    d_ptr->m_locale = locale;
}

KCalendarSystem::KCalendarSystem( KCalendarSystemPrivate &dd, const KSharedConfig::Ptr config, const KLocale *locale )
               : d_ptr( &dd )
{
    d_ptr->m_config = config;
    d_ptr->m_locale = locale;
}

KCalendarSystem::~KCalendarSystem()
{
    delete d_ptr;
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
QDate KCalendarSystem::epoch() const
{
    return QDate::fromJulianDay( 38 );
}

QDate KCalendarSystem::earliestValidDate() const
{
    return epoch();
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
QDate KCalendarSystem::latestValidDate() const
{
    // Default to Gregorian 9999-12-31
    return QDate::fromJulianDay( 5373484 );
}

bool KCalendarSystem::isValid( int year, int month, int day ) const
{
    Q_D( const KCalendarSystem );

    if ( year < d->earliestValidYear() || year > d->latestValidYear() ||
         ( !d->hasYearZero() && year == 0 ) ) {
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

// NOT VIRTUAL - If override needed use shared-d
bool KCalendarSystem::isValid( int year, int dayOfYear ) const
{
    Q_D( const KCalendarSystem );

    return ( isValid( year, 1, 1 ) && dayOfYear > 0 && dayOfYear <= d->daysInYear( year ) );
}

// NOT VIRTUAL - If override needed use shared-d
bool KCalendarSystem::isValid( const QString &eraName, int yearInEra, int month, int day ) const
{
    Q_D( const KCalendarSystem );

    KCalendarEra era = d->era( eraName, yearInEra );
    return ( era.isValid() && isValid( era.year( yearInEra ), month, day ) );
}

// NOT VIRTUAL - If override needed use shared-d
bool KCalendarSystem::isValidIsoWeekDate( int year, int isoWeekNumber, int dayOfIsoWeek ) const
{
    Q_D( const KCalendarSystem );

    //Tests Year value in standard YMD isValid()
    if ( !isValid( year, 1, 1 ) ) {
        return false;
    }

    //Test Week Number falls in valid range for this year
    int weeksInThisYear = weeksInYear( year );
    if ( isoWeekNumber < 1 || isoWeekNumber  > weeksInThisYear ) {
        return false;
    }

    //Test Day of Week Number falls in valid range
    if ( dayOfIsoWeek < 1 || dayOfIsoWeek > d->daysInWeek() ) {
        return false;
    }

    //If not in earliest or latest years then all OK
    //Otherwise need to check don't fall into previous or next year that would be invalid
    if ( year == d->earliestValidYear() && isoWeekNumber == 1 ) {
        //If firstDayOfYear falls on or before Thursday then firstDayOfYear falls in week 1 this
        //year and if wanted dayOfIsoWeek falls before firstDayOfYear then falls in previous year
        //and so in invalid year
        int dowFirstDay = dayOfWeek( d->firstDayOfYear( year ) );
        if ( dowFirstDay <= 4 && dayOfIsoWeek < dowFirstDay ) {
            return false;
        }
    } else if ( year == d->latestValidYear() && isoWeekNumber == weeksInThisYear ) {
        //If lastDayOfYear falls on or after Thursday then lastDayOfYear falls in last week this
        //year and if wanted dayOfIsoWeek falls after lastDayOfYear then falls in next year
        //and so in invalid year
        int dowLastDay = dayOfWeek( d->lastDayOfYear( year ) );
        if ( dowLastDay >= 4 && dayOfIsoWeek > dowLastDay ) {
            return false;
        }
    }

    return true;
}

bool KCalendarSystem::isValid( const QDate &date ) const
{
    if ( date.isNull() || date < earliestValidDate() || date > latestValidDate() ) {
        return false;
    }
    return true;
}

bool KCalendarSystem::setDate( QDate &date, int year, int month, int day ) const
{
    Q_D( const KCalendarSystem );

    date = d->invalidDate();

    if ( isValid( year, month, day ) ) {
        int jd;
        dateToJulianDay( year, month, day, jd );
        QDate calcDate = QDate::fromJulianDay( jd );

        if ( isValid( calcDate ) ) {
            date = calcDate;
            return true;
        }
    }

    return false;
}

// NOT VIRTUAL - If override needed use shared-d
bool KCalendarSystem::setDate( QDate &date, int year, int dayOfYear ) const
{
    Q_D( const KCalendarSystem );

    date = d->invalidDate();

    if ( isValid( year, dayOfYear ) ) {
        int jd;
        dateToJulianDay( year, 1, 1, jd );
        QDate calcDate = QDate::fromJulianDay( jd + dayOfYear - 1 );
        if ( isValid( calcDate ) ) {
            date = calcDate;
            return true;
        }
    }

    return false;
}

// NOT VIRTUAL - If override needed use shared-d
bool KCalendarSystem::setDate( QDate &date, QString eraName, int yearInEra, int month, int day ) const
{
    Q_D( const KCalendarSystem );

    KCalendarEra era = d->era( eraName, yearInEra );
    return ( era.isValid() && setDate( date, era.year( yearInEra ), month, day ) );
}

// NOT VIRTUAL - If override needed use shared-d
bool KCalendarSystem::setDateIsoWeek( QDate &date, int year, int isoWeekNumber, int dayOfIsoWeek ) const
{
    Q_D( const KCalendarSystem );

    date = d->invalidDate();

    if ( isValidIsoWeekDate( year, isoWeekNumber, dayOfIsoWeek ) ) {

        QDate calcDate = d->firstDayOfYear( year );
        int dowFirstDayOfYear = dayOfWeek( calcDate );

        int daysToAdd = ( d->daysInWeek() * ( isoWeekNumber - 1 ) ) + dayOfIsoWeek;

        if ( dowFirstDayOfYear <= 4 ) {
            calcDate = calcDate.addDays( daysToAdd - dowFirstDayOfYear );
        } else {
            calcDate = calcDate.addDays( daysInWeek( calcDate ) + daysToAdd - dowFirstDayOfYear );
        }

        if ( isValid( calcDate ) ) {
            date = calcDate;
            return true;
        }
    }

    return false;
}

// Deprecated
bool KCalendarSystem::setYMD( QDate &date, int year, int month, int day ) const
{
    return setDate( date, year, month, day );
}

// NOT VIRTUAL - If override needed use shared-d
void KCalendarSystem::getDate( const QDate date, int *year, int *month, int *day ) const
{
    int y, m, d;

    if ( isValid( date ) ) {
        julianDayToDate( date.toJulianDay(), y, m, d );
    } else {
        y = 0;  // How do you denote invalid year when we support -ve years?
        m = 0;
        d = 0;
    }

    if ( year ) {
        *year = y;
    }
    if ( month ) {
        *month = m;
    }
    if ( day ) {
        *day = d;
    }

}

int KCalendarSystem::year( const QDate &date ) const
{
    if ( isValid( date ) ) {
        int year, month, day;

        julianDayToDate( date.toJulianDay(), year, month, day );

        return year;
    }

    return 0;  // How do you denote invalid year when we support -ve years?
}

int KCalendarSystem::month( const QDate &date ) const
{
    if ( isValid( date ) ) {
        int year, month, day;

        julianDayToDate( date.toJulianDay(), year, month, day );

        return month;
    }

    return 0;
}

int KCalendarSystem::day( const QDate &date ) const
{
    if ( isValid( date ) ) {
        int year, month, day;

        julianDayToDate( date.toJulianDay(), year, month, day );

        return day;
    }

    return 0;
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::eraName( const QDate &date, StringFormat format ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( date ) ) {
        if ( format == LongFormat ) {
            return d->era( date ).name();
        } else {
            return d->era( date ).shortName();
        }
    }

    return QString();
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::eraYear( const QDate &date, StringFormat format ) const
{
    Q_UNUSED( format )
    Q_D( const KCalendarSystem );

    if ( isValid( date ) ) {
        return formatDate( date, d->era( date ).format() );
    }

    return QString();
}

// NOT VIRTUAL - If override needed use shared-d
int KCalendarSystem::yearInEra( const QDate &date ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( date ) ) {
        return d->era( date ).yearInEra( year( date ) );
    }

    return -1;
}

QDate KCalendarSystem::addYears( const QDate &date, int numYears ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( date ) ) {

        int originalYear, originalMonth, originalDay;
        julianDayToDate( date.toJulianDay(), originalYear, originalMonth, originalDay );

        int newYear = d->addYears( originalYear, numYears );
        int newMonth = originalMonth;
        int newDay = originalDay;

        //Adjust day number if new month has fewer days than old month
        int daysInNewMonth = d->daysInMonth( newYear, newMonth );
        if ( daysInNewMonth < originalDay ) {
            newDay = daysInNewMonth;
        }

        QDate newDate;
        setDate( newDate, newYear, newMonth, newDay );
        return newDate;

    }

    return d->invalidDate();
}

QDate KCalendarSystem::addMonths( const QDate &date, int numMonths ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( date ) ) {

        int originalYear, originalMonth, originalDay;
        julianDayToDate( date.toJulianDay(), originalYear, originalMonth, originalDay );

        int monthsInOriginalYear = d->monthsInYear( originalYear );

        int newYear = d->addYears( originalYear, ( originalMonth + numMonths ) / monthsInOriginalYear );
        int newMonth = ( originalMonth + numMonths ) % monthsInOriginalYear;
        int newDay = originalDay;

        if ( newMonth == 0 ) {
            newYear = d->addYears( newYear, - 1 );
            newMonth = monthsInOriginalYear;
        }
        if ( newMonth < 0 ) {
            newYear = d->addYears( newYear, - 1 );
            newMonth = newMonth + monthsInOriginalYear;
        }

        //Adjust day number if new month has fewer days than old month
        int daysInNewMonth = d->daysInMonth( newYear, newMonth );
        if ( daysInNewMonth < originalDay ) {
            newDay = daysInNewMonth;
        }

        QDate newDate;
        setDate( newDate, newYear, newMonth, newDay );
        return newDate;

    }

    return d->invalidDate();
}

QDate KCalendarSystem::addDays( const QDate &date, int numDays ) const
{
    Q_D( const KCalendarSystem );

    // QDate only holds a uint and has no boundary checking in addDays(), so we need to check
    if ( isValid( date ) && (long) date.toJulianDay() + (long) numDays > 0 ) {
        // QDate adds straight to jd
        QDate temp = date.addDays( numDays );
        if ( isValid( temp ) ) {
            return temp;
        }
    }

    return d->invalidDate();
}

// NOT VIRTUAL - Uses shared-d instead
void KCalendarSystem::dateDifference( const QDate &fromDate, const QDate &toDate,
                                             int *yearsDiff, int *monthsDiff, int *daysDiff, int *direction ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( fromDate ) && isValid( toDate ) ) {
        d->dateDifference( fromDate, toDate, yearsDiff, monthsDiff, daysDiff, direction );
    }
}

// NOT VIRTUAL - Uses shared-d instead
int KCalendarSystem::yearsDifference( const QDate &fromDate, const QDate &toDate ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( fromDate ) && isValid( toDate ) ) {
        return d->yearsDifference( fromDate, toDate );
    }

    return 0;
}

// NOT VIRTUAL - Uses shared-d instead
int KCalendarSystem::monthsDifference( const QDate &fromDate, const QDate &toDate ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( fromDate ) && isValid( toDate ) ) {
        return d->monthsDifference( fromDate, toDate );
    }

    return 0;
}

// NOT VIRTUAL - Uses shared-d instead
int KCalendarSystem::daysDifference( const QDate &fromDate, const QDate &toDate ) const
{
    if ( isValid( fromDate ) && isValid( toDate ) ) {
        return toDate.toJulianDay() - fromDate.toJulianDay();
    }

    return 0;
}

int KCalendarSystem::monthsInYear( const QDate &date ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( date ) ) {
        return d->monthsInYear( year( date ) );
    }

    return -1;
}

// NOT VIRTUAL - Uses shared-d instead
int KCalendarSystem::monthsInYear( int year ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( year, 1, 1 ) ) {
        return d->monthsInYear( year );
    }

    return -1;
}

int KCalendarSystem::weeksInYear( const QDate &date ) const
{
    if ( isValid( date ) ) {
        return weeksInYear( year( date ) );
    }

    return -1;
}

// ISO compliant week numbering, not traditional number, rename in KDE5 to isoWeeksInYear()
int KCalendarSystem::weeksInYear( int year ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( year, 1, 1 ) ) {
        QDate firstDayOfThisYear = d->firstDayOfYear( year );
        QDate lastDayOfThisYear = d->lastDayOfYear( year );

        int weekYear = year;
        int lastWeekInThisYear = weekNumber( lastDayOfThisYear, &weekYear );

        // If error, or the last day of the year is in the first week of next year use the week before
        if ( lastWeekInThisYear < 1 || weekYear != year ) {
            lastDayOfThisYear = addDays( lastDayOfThisYear, -7 );
            lastWeekInThisYear = weekNumber( lastDayOfThisYear );
        }

        return lastWeekInThisYear;
    }

    return -1;
}

int KCalendarSystem::daysInYear( const QDate &date ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( date ) ) {
        return d->daysInYear( year( date ) );
    }

    return -1;
}

// NOT VIRTUAL - Uses shared-d instead
int KCalendarSystem::daysInYear( int year ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( year, 1, 1 ) ) {
        return d->daysInYear( year );
    }

    return -1;
}

int KCalendarSystem::daysInMonth( const QDate &date ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( date ) ) {
        return d->daysInMonth( year( date ), month( date ) );
    }

    return -1;
}

// NOT VIRTUAL - Uses shared-d instead
int KCalendarSystem::daysInMonth( int year, int month ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( year, 1, 1 ) ) {
        return d->daysInMonth( year, month );
    }

    return -1;
}

int KCalendarSystem::daysInWeek( const QDate &date ) const
{
    Q_UNUSED( date )
    Q_D( const KCalendarSystem );
    return d->daysInWeek();
}

int KCalendarSystem::dayOfYear( const QDate &date ) const
{
    //Take the jd of the given date, and subtract the jd of the first day of that year
    Q_D( const KCalendarSystem );

    if ( isValid( date ) ) {
        return ( date.toJulianDay() - d->firstDayOfYear( year( date ) ).toJulianDay() + 1 );
    }

    return -1;
}

int KCalendarSystem::dayOfWeek( const QDate &date ) const
{
    // Makes assumption that Julian Day 0 was day 1 of week
    // This is true for Julian/Gregorian calendar with jd 0 being Monday
    // We add 1 for ISO compliant numbering for 7 day week
    // Assumes we've never skipped weekdays
    Q_D( const KCalendarSystem );

    if ( isValid( date ) ) {
        return ( ( date.toJulianDay() % d->daysInWeek() ) + 1 );
    }

    return -1;
}

// ISO compliant week numbering, not traditional number, rename in KDE5 to isoWeekNumber()
// JPL still need to fully clean up here
int KCalendarSystem::weekNumber( const QDate &date, int *yearNum ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( date ) ) {
        QDate firstDayWeek1, lastDayOfYear;
        int y = year( date );
        int week;
        int weekDay1, dayOfWeek1InYear;

        // let's guess 1st day of 1st week
        firstDayWeek1 = d->firstDayOfYear( y );
        weekDay1 = dayOfWeek( firstDayWeek1 );

        // iso 8601: week 1  is the first containing thursday and week starts on monday
        if ( weekDay1 > 4 /*Thursday*/ ) {
            firstDayWeek1 = addDays( firstDayWeek1 , d->daysInWeek() - weekDay1 + 1 ); // next monday
        }

        dayOfWeek1InYear = dayOfYear( firstDayWeek1 );

        // our date in prev year's week
        if ( dayOfYear( date ) < dayOfWeek1InYear ) { 
            if ( yearNum ) {
                *yearNum = d->addYears( y, - 1 );
            }
            return weeksInYear( d->addYears( y, - 1 ) );
        }

        // let's check if its last week belongs to next year
        lastDayOfYear = d->lastDayOfYear( y );

        // if our date is in last week && 1st week in next year has thursday
        if ( ( dayOfYear( date ) >= d->daysInYear( y ) - dayOfWeek( lastDayOfYear ) + 1 )
             && dayOfWeek( lastDayOfYear ) < 4 ) {
            if ( yearNum ) {
                * yearNum = d->addYears( y, 1 );
            }
             week = 1;
        } else {
            // To calculate properly the number of weeks from day a to x let's make a day 1 of week
            if( weekDay1 < 5 ) {
                firstDayWeek1 = addDays( firstDayWeek1, -( weekDay1 - 1 ) );
            }

            week = firstDayWeek1.daysTo( date ) / d->daysInWeek() + 1;
        }

        return week;
    }

    return -1;
}

bool KCalendarSystem::isLeapYear( int year ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( year, 1, 1 ) ) {
        return d->isLeapYear( year );
    }

    return false;
}

bool KCalendarSystem::isLeapYear( const QDate &date ) const
{
    Q_D( const KCalendarSystem );

    if ( isValid( date ) ) {
        return d->isLeapYear( year( date ) );
    }

    return false;
}

QString KCalendarSystem::monthName( const QDate &date, MonthNameFormat format ) const
{
    if ( isValid( date ) ) {
        int year, month;
        getDate( date, &year, &month, 0 );
        return monthName( month, year, format );
    }

    return QString();
}

QString KCalendarSystem::weekDayName( const QDate &date, WeekDayNameFormat format ) const
{
    if ( isValid( date ) ) {
        return weekDayName( dayOfWeek( date ), format );
    }

    return QString();
}

QString KCalendarSystem::yearString( const QDate &date, StringFormat format ) const
{
    if ( format == ShortFormat ) {
        return formatDate( date, "%y" );
    } else {
        return formatDate( date, "%Y" );
    }
}

QString KCalendarSystem::monthString( const QDate &date, StringFormat format ) const
{
    if ( format == ShortFormat ) {
        return formatDate( date, "%n" );
    } else {
        return formatDate( date, "%m" );
    }
}

QString KCalendarSystem::dayString( const QDate &date, StringFormat format ) const
{
    if ( format == ShortFormat ) {
        return formatDate( date, "%e" );
    } else {
        return formatDate( date, "%d" );
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::yearInEraString( const QDate &date, StringFormat format ) const
{
    if ( format == ShortFormat ) {
        return formatDate( date, "%Ey" );
    } else {
        return formatDate( date, "%4Ey" );
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::dayOfYearString( const QDate &date, StringFormat format ) const
{
    if ( format == ShortFormat ) {
        return formatDate( date, "%-j" );
    } else {
        return formatDate( date, "%j" );
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::dayOfWeekString( const QDate &date ) const
{
    return formatDate( date, "%-u" );
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::weekNumberString( const QDate &date, StringFormat format ) const
{
    if ( format == ShortFormat ) {
        return formatDate( date, "%-V" );
    } else {
        return formatDate( date, "%V" );
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::monthsInYearString( const QDate &date, StringFormat format ) const
{
    Q_D( const KCalendarSystem );

    if ( format == ShortFormat ) {
        return d->stringFromInteger( monthsInYear( date ), 0, '0' );
    } else {
        return d->stringFromInteger( monthsInYear( date ), 2, '0' );
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::weeksInYearString( const QDate &date, StringFormat format ) const
{
    Q_D( const KCalendarSystem );

    if ( format == ShortFormat ) {
        return d->stringFromInteger( weeksInYear( date ), 0, '0' );
    } else {
        return d->stringFromInteger( weeksInYear( date ), 2, '0' );
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::daysInYearString( const QDate &date, StringFormat format ) const
{
    Q_D( const KCalendarSystem );

    if ( format == ShortFormat ) {
        return d->stringFromInteger( daysInYear( date ), 0, '0' );
    } else {
        return d->stringFromInteger( daysInYear( date ), 3, '0' );
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::daysInMonthString( const QDate &date, StringFormat format ) const
{
    Q_D( const KCalendarSystem );

    if ( format == ShortFormat ) {
        return d->stringFromInteger( daysInMonth( date ), 0, '0' );
    } else {
        return d->stringFromInteger( daysInMonth( date ), 2, '0' );
    }
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::daysInWeekString( const QDate &date) const
{
    Q_D( const KCalendarSystem );
    Q_UNUSED( date );
    return d->stringFromInteger( d->daysInWeek(), 0 );
}

int KCalendarSystem::yearStringToInteger( const QString &yearString, int &readLength ) const
{
    Q_D( const KCalendarSystem );

    QString minus = i18nc( "Negative symbol as used for year numbers, e.g. -5 = 5 BC", "-" );
    if ( yearString.startsWith( minus ) ) {
        int value = d->integerFromString( yearString.mid( minus.length() ), 4, readLength );
        if ( readLength > 0 && value >= 0 ) {
            readLength = readLength + minus.length();
            return value * -1;
        } else {
            return value;
        }
    }

    return d->integerFromString( yearString, 4, readLength );
}

int KCalendarSystem::monthStringToInteger( const QString &monthString, int &readLength ) const
{
    Q_D( const KCalendarSystem );
    return d->integerFromString( monthString, 2, readLength );
}

int KCalendarSystem::dayStringToInteger( const QString &dayString, int &readLength ) const
{
    Q_D( const KCalendarSystem );
    return d->integerFromString( dayString, 2, readLength );
}

QString KCalendarSystem::formatDate( const QDate &fromDate, KLocale::DateFormat toFormat ) const
{
    if ( !fromDate.isValid() ) {
        return QString();
    }

    if ( toFormat == KLocale::FancyShortDate || toFormat == KLocale::FancyLongDate ) {
        QDate now = KDateTime::currentLocalDate();
        int daysToNow = fromDate.daysTo( now );
        switch ( daysToNow ) {
        case 0:
            return i18n("Today");
        case 1:
            return i18n("Yesterday");
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            return weekDayName( fromDate );
        default:
            break;
        }
    }

    switch ( toFormat ) {
    case KLocale::LongDate:
    case KLocale::FancyLongDate:
        return formatDate( fromDate, locale()->dateFormat() );
    case KLocale::IsoDate:
        return formatDate( fromDate, "%Y-%m-%d" );
    case KLocale::IsoWeekDate:
        return formatDate( fromDate, "%Y-W%V-%u" );
    case KLocale::IsoOrdinalDate:
        return formatDate( fromDate, "%Y-%j" );
    case KLocale::ShortDate:
    case KLocale::FancyShortDate:
    default:
        return formatDate( fromDate, locale()->dateFormatShort() );
    }

}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::formatDate( const QDate &fromDate, const QString &toFormat,
                                     KLocale::DateTimeFormatStandard standard ) const
{
    return formatDate( fromDate, toFormat, locale()->dateTimeDigitSet(), standard );
}

// NOT VIRTUAL - If override needed use shared-d
QString KCalendarSystem::formatDate( const QDate &fromDate, const QString &toFormat, KLocale::DigitSet digitSet,
                                     KLocale::DateTimeFormatStandard formatStandard ) const
{
    Q_D( const KCalendarSystem );

    if ( !isValid( fromDate ) ) {
        return QString();
    }

    if ( formatStandard == KLocale::UnicodeFormat ) {
        return d->formatDateUnicode( fromDate, toFormat, digitSet );
    } else {
        return d->formatDatePosix( fromDate, toFormat, digitSet, formatStandard );
    }
}

QDate KCalendarSystem::readDate( const QString &str, bool *ok ) const
{
    //Try each standard format in turn, start with the locale ones,
    //then the well defined standards
    QDate date = readDate( str, KLocale::ShortFormat, ok);
    if ( !isValid( date ) ) {
        date = readDate( str, KLocale::NormalFormat, ok);
        if ( !isValid( date )) {
            date = readDate( str, KLocale::IsoFormat, ok);
            if ( !isValid( date ) ) {
                date = readDate( str, KLocale::IsoWeekFormat, ok);
                if ( !isValid( date ) ) {
                    date = readDate( str, KLocale::IsoOrdinalFormat, ok);
                }
            }
        }
    }

    return date;
}

QDate KCalendarSystem::readDate( const QString &str, KLocale::ReadDateFlags flags, bool *ok ) const
{
    Q_D( const KCalendarSystem );

    if ( flags & KLocale::ShortFormat ) {
        return readDate( str, locale()->dateFormatShort(), ok );
    } else if ( flags & KLocale::NormalFormat ) {
        return readDate( str, locale()->dateFormat(), ok );
    } else if ( flags & KLocale::IsoFormat ) {
        return readDate( str, "%Y-%m-%d", ok );
    } else if ( flags & KLocale::IsoWeekFormat ) {
        return readDate( str, "%Y-W%V-%u", ok );
    } else if ( flags & KLocale::IsoOrdinalFormat ) {
        return readDate( str, "%Y-%j", ok );
    }
    return d->invalidDate();
}

QDate KCalendarSystem::readDate( const QString &inputString, const QString &formatString, bool *ok ) const
{
    return readDate( inputString, formatString, ok, KLocale::KdeFormat );
}

// NOT VIRTUAL - If override needed use shared-d
QDate KCalendarSystem::readDate( const QString &inputString, const QString &formatString, bool *ok,
                                 KLocale::DateTimeFormatStandard formatStandard ) const
{
    Q_D( const KCalendarSystem );

    DateComponents result;
    if ( formatStandard == KLocale::UnicodeFormat ) {
        result = d->parseDateUnicode( inputString, formatString );
    } else {
        result = d->parseDatePosix( inputString, formatString, formatStandard );
    }

    QDate resultDate = d->invalidDate();
    bool resultStatus = false;

    if ( !result.error &&
         formatString.simplified().length() <= result.formatPosition &&
         inputString.simplified().length() <= result.inputPosition ) {

        // If there were no parsing errors, and we have reached the end of both the input and
        // format strings, then see if we have a valid date based on the components parsed

        // If we haven't parsed a year component, then assume this year
        if ( !result.parsedYear ) {
            result.year = year( QDate::currentDate() );
        }

        if ( ( !result.eraName.isEmpty() || result.yearInEra > -1 ) && result.month > 0 && result.day > 0 ) {
            // Have parsed Era components as well as month and day components
            resultStatus = setDate( resultDate, result.eraName, result.yearInEra, result.month, result.day );
        } else if ( result.month > 0 && result.day > 0 ) {
            // Have parsed month and day components
            resultStatus = setDate( resultDate, result.year, result.month, result.day );
        } else if ( result.dayInYear > 0 ) {
            // Have parsed Day In Year component
            resultStatus = setDate( resultDate, result.year, result.dayInYear );
        } else if ( result.isoWeekNumber > 0 && result.dayOfIsoWeek > 0 ) {
            // Have parsed ISO Week components
            resultStatus = setDateIsoWeek( resultDate, result.year, result.isoWeekNumber, result.dayOfIsoWeek );
        }

    }

    if (ok) {
        *ok = resultStatus;
    }

    return resultDate;
}

int KCalendarSystem::weekStartDay() const
{
    return locale()->weekStartDay();
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
// The implementation MUST NOT do validity checking on date ranges, all calls to this function MUST
// instead be wrapped in validity checks, as sometimes we want this to work outside the public valid
// range, i.e. to allow us to internally set dates of 1/1/10000 which are not publically valid but
// are required for internal maths
bool KCalendarSystem::julianDayToDate( int jd, int &year, int &month, int &day ) const
{
    // Formula from The Calendar FAQ by Claus Tondering
    // http://www.tondering.dk/claus/cal/node3.html#SECTION003161000000000000000
    // NOTE: Coded from scratch from mathematical formulas, not copied from
    // the Boost licensed source code

    int a = jd + 32044;
    int b = ( ( 4 * a ) + 3 ) / 146097;
    int c = a - ( ( 146097 * b ) / 4 );
    int d = ( ( 4 * c ) + 3 ) / 1461;
    int e = c - ( ( 1461 * d ) / 4 );
    int m = ( ( 5 * e ) + 2 ) / 153;
    day = e - ( ( (153 * m ) + 2 ) / 5 ) + 1;
    month = m + 3 - ( 12 * ( m / 10 ) );
    year = ( 100 * b ) + d - 4800 + ( m / 10 );

    // If year is -ve then is BC.  In Gregorian there is no year 0, but the maths
    // is easier if we pretend there is, so internally year of 0 = 1BC = -1 outside
    if ( year < 1 ) {
        year = year - 1;
    }

    return true;
}

// Dummy version using Gregorian as an example
// This method MUST be re-implemented in any new Calendar System
// The implementation MUST NOT do validity checking on date ranges, all calls to this function MUST
// instead be wrapped in validity checks, as sometimes we want this to work outside the public valid
// range, i.e. to allow us to internally set dates of 1/1/10000 which are not publically valid but
// are required for internal maths
bool KCalendarSystem::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    // Formula from The Calendar FAQ by Claus Tondering
    // http://www.tondering.dk/claus/cal/node3.html#SECTION003161000000000000000
    // NOTE: Coded from scratch from mathematical formulas, not copied from
    // the Boost licensed source code

    // If year is -ve then is BC.  In Gregorian there is no year 0, but the maths
    // is easier if we pretend there is, so internally year of -1 = 1BC = 0 internally
    int y;
    if ( year < 1 ) {
        y = year + 1;
    } else {
        y = year;
    }

    int a = ( 14 - month ) / 12;
    y = y + 4800 - a;
    int m = month + ( 12 * a ) - 3;

    jd = day
         + ( ( ( 153 * m ) + 2 ) / 5 )
         + ( 365 * y )
         + ( y / 4 )
         - ( y / 100 )
         + ( y / 400 )
         - 32045;

    return true;
}

const KLocale * KCalendarSystem::locale() const
{
    Q_D( const KCalendarSystem );

    return d->locale();
}

// Deprecated
void KCalendarSystem::setMaxMonthsInYear( int maxMonths )
{
    Q_UNUSED( maxMonths )
}

// Deprecated
void KCalendarSystem::setMaxDaysInWeek( int maxDays )
{
    Q_UNUSED( maxDays )
}

// Deprecated
void KCalendarSystem::setHasYear0( bool hasYear0 )
{
    Q_UNUSED( hasYear0 )
}
