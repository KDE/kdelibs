/*
    Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
    Copyright (c) 2002 Hans Petter Bieker <bieker@kde.org>
    Copyright (c) 2007-2009 John Layt <john@layt.net>
 
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

#include "kglobal.h"
#include "kdebug.h"

#include <QtCore/QDateTime>

#include "kdatetime.h"
#include "kcalendarsystemcoptic_p.h"
#include "kcalendarsystemethiopian_p.h"
#include "kcalendarsystemgregorian_p.h"
#include "kcalendarsystemgregorianproleptic_p.h"
#include "kcalendarsystemhebrew_p.h"
#include "kcalendarsystemhijri_p.h"
#include "kcalendarsystemindiannational_p.h"
#include "kcalendarsystemjalali_p.h"
#include "kcalendarsystemjulian_p.h"

KCalendarSystem *KCalendarSystem::create( const QString &calendarType, const KLocale *locale )
{
    if ( calendarType == "coptic" ) {
        return new KCalendarSystemCoptic( locale );
    }

    if ( calendarType == "ethiopian" ) {
        return new KCalendarSystemEthiopian( locale );
    }

    if ( calendarType == "gregorian" ) {
        return new KCalendarSystemGregorian( locale );
    }

    if ( calendarType == "gregorian-proleptic" ) {
        return new KCalendarSystemGregorianProleptic( locale );
    }

    if ( calendarType == "hebrew" ) {
        return new KCalendarSystemHebrew( locale );
    }

    if ( calendarType == "hijri" ) {
        return new KCalendarSystemHijri( locale );
    }

    if ( calendarType == "indian-national" ) {
        return new KCalendarSystemIndianNational( locale );
    }

    if ( calendarType == "jalali" ) {
        return new KCalendarSystemJalali( locale );
    }

    if ( calendarType == "julian" ) {
        return new KCalendarSystemJulian( locale );
    }

    // ### HPB: Should it really be a default here?
    return new KCalendarSystemGregorian( locale );
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
    lst.append( "julian" );

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

    if ( calendarType == "julian" ) {
        return ki18nc( "@item Calendar system", "Julian" ).toString( KGlobal::locale() );
    }

    return ki18nc( "@item Calendar system", "Invalid Calendar Type" ).toString( KGlobal::locale() );
}


class KCalendarSystemPrivate
{
public:
    KCalendarSystemPrivate( KCalendarSystem *q ): q( q )
    {
    }

    ~KCalendarSystemPrivate()
    {
    }

    KCalendarSystem *q;

    bool setAnyDate( QDate &date, int year, int month, int day ) const;

    int addYearNumber( int originalYear, int addYears ) const;

    QDate invalidDate() const;

    int integerFromString( const QString &string, int maxLength, int &readLength ) const;
    QString stringFromInteger( int number, int padWidth = 0, QChar padChar = '0' ) const;
    QString stringFromInteger( int number, int padWidth, QChar padChar, KLocale::DigitSet digitSet ) const;

    QString simpleDateString( const QString &str ) const;

    int maxDaysInWeek;
    int maxMonthsInYear;

    bool hasYear0;

    const KLocale *locale;
};

// Allows us to set dates outside publically valid range, USE WITH CARE!!!!
bool KCalendarSystemPrivate::setAnyDate( QDate &date, int year, int month, int day ) const
{
    int jd;
    q->dateToJulianDay( year, month, day, jd );
    date = QDate::fromJulianDay( jd );
    return true;
}

// Utility to correctly add year numbers together because some systems such as
// Julian and Gregorian calendars don't have a year 0
int KCalendarSystemPrivate::addYearNumber( int originalYear, int addYears ) const
{
    int newYear = originalYear + addYears;

    if (!hasYear0) {
        if ( originalYear > 0 && newYear <= 0 ) {
            newYear = newYear - 1;
        } else if ( originalYear < 0 && newYear >= 0 ) {
            newYear = newYear + 1;
        }
    }

    return newYear;
}

QDate KCalendarSystemPrivate::invalidDate() const
{
    //Is QDate's way of saying is invalid
    return QDate();
}

int KCalendarSystemPrivate::integerFromString( const QString &inputString, int maxLength, int &readLength ) const
{
    // Temp fix in 4.4, in 4.5 is in shared d pointer implementation KCalendarSystemHebrewPrivate
    if ( q->calendarType() == "hebrew" && q->locale()->language() == "he" ) {

        // Hebrew numbers are composed of combinations of normal letters which have a numeric value.
        // This is a non-positional system, the numeric values are simply added together, however
        // convention is for a RTL highest to lowest value ordering. There is also a degree of
        // ambiguity due to the lack of a letter for 0, hence 5 and 5000 are written the same.
        // Hebrew numbers are only used in dates.
        // See http://www.i18nguy.com/unicode/hebrew-numbers.html for more explaination

        /*
        Ref table for numbers to Hebrew chars

        Value     1       2       3        4        5       6         7        8      9

        x 1    Alef א  Bet  ב  Gimel ג  Dalet ד  He   ה  Vav  ו    Zayen ז  Het  ח  Tet  ט
               0x05D0  0x05D1  0x05D2   0x05D3   0x05D4  0x05D5    0x05D6   0x05D7  0x05D8

        x 10   Yod  י  Kaf  כ  Lamed ל  Mem  מ   Nun  נ  Samekh ס  Ayin ע   Pe   פ  Tzadi צ
               0x05D9  0x05DB  0x05DC   0x05DE   0x05E0  0x05E1    0x05E2   0x05E4  0x05E6

        x 100  Qof  ק  Resh ר  Shin ש   Tav  ת
               0x05E7  0x05E8  0x05E9   0x05EA

        Note special cases 15 = 9 + 6 = 96 טו and 16 = 9 + 7 = 97 טז
        */

        int decadeValues[14] = {10, 20, 20, 30, 40, 40, 50, 50, 60, 70, 80, 80, 90, 90};

        QChar thisChar, nextChar;
        QString string = inputString;

        int stringLength = string.length();
        readLength = 0;
        int position = 0;
        int result = 0;
        int value = 0;

        for ( ; position < stringLength ; ++position ) {

            thisChar = string[position];

            if ( position + 1 < stringLength ) {
                nextChar = string[position + 1];
                // Ignore any geresh or gershayim chars, we don't bother checking they are in the right place
                if ( nextChar == '\'' ||  nextChar == QChar( 0x05F3 ) ||   // geresh
                     nextChar == '\"' ||  nextChar == QChar( 0x05F4 ) ) {  // gershayim
                    string.remove( position + 1, 1 );
                    stringLength = string.length();
                    if ( position + 1 < stringLength ) {
                        nextChar = string[position + 1];
                    } else {
                        nextChar = QChar();
                    }
                    readLength = readLength + 1;
                }
            } else {
                nextChar = QChar();
            }

            if ( thisChar >= QChar( 0x05D0 ) && thisChar <= QChar( 0x05D7 ) ) {

                // If this char Alef to Het, 1 to 8, א to ח

                // If next char is any valid digit char (Alef to Tav, 1 to 400, א to ת)
                // then this char is a thousands digit
                // else this char is a ones digit

                if ( nextChar >= QChar( 0x05D0 ) && nextChar <= QChar( 0x05EA ) ) {
                    value = ( thisChar.unicode() - 0x05D0 + 1 ) * 1000;
                } else {
                    value = thisChar.unicode() - 0x05D0 + 1;
                }

            } else if ( thisChar == QChar( 0x05D8 ) ) {

                // If this char is Tet, 9, ט

                // If next char is any valid digit char (Alef to Tav, 1 to 400, א to ת)
                // and next char not 6 (Special case for 96 = 15)
                // and next char not 7 (Special case for 97 = 16)
                // then is a thousands digit else is 9

                if ( nextChar >= QChar( 0x05D0 ) && nextChar <= QChar( 0x05EA ) &&
                     nextChar != QChar( 0x05D5 ) && nextChar != QChar( 0x05D6 ) ) {
                    value = 9000;
                } else {
                    value = 9;
                }

            } else if ( thisChar >= QChar( 0x05D9 ) && thisChar <= QChar( 0x05E6 ) ) {

                // If this char Yod to Tsadi, 10 to 90, י to צ

                // If next char is a tens or hundreds char then is an error
                // Else is a tens digit

                if ( nextChar >= QChar( 0x05D9 ) ) {
                    return -1;
                } else {
                    value = decadeValues[thisChar.unicode() - 0x05D9];
                }

            } else if ( thisChar >= QChar( 0x05E7 ) && thisChar <= QChar( 0x05EA ) ) {

                // If this char Qof to Tav, 100 to 400, ק to ת, then is hundreds digit

                value = ( thisChar.unicode() - 0x05E7 + 1 ) * 100;

            } else {

                // If this char any non-digit char including whitespace or punctuation, we're done
                break;

            }

            result = result + value;

            value = 0;
        }

        readLength += position;

        return result;

    } else {
        int position = 0;

        if ( maxLength < 0 ) {
            maxLength = inputString.length();
        }

        while ( position < inputString.length() &&
                position < maxLength &&
                inputString.at( position ).isDigit() ) {
            ++position;
        }

        readLength = position;
        return inputString.left( position ).toInt();
    }
}

QString KCalendarSystemPrivate::stringFromInteger( int number, int padWidth, QChar padChar ) const
{
    return stringFromInteger( number, padWidth, padChar, q->locale()->dateTimeDigitSet() );
}

QString KCalendarSystemPrivate::stringFromInteger( int number, int padWidth, QChar padChar, KLocale::DigitSet digitSet ) const
{
    // Temp fix in 4.4, in 4.5 is in shared d pointer implementation KCalendarSystemHebrewPrivate
    if ( q->calendarType() == "hebrew" && q->locale()->language() == "he" ) {

        // Hebrew numbers are composed of combinations of normal letters which have a numeric value.
        // This is a non-positional system, the numeric values are simply added together, however
        // convention is for a RTL highest to lowest value ordering. There is also a degree of
        // ambiguity due to the lack of a letter for 0, hence 5 and 5000 are written the same.
        // Hebrew numbers are only used in dates.
        // See http://www.i18nguy.com/unicode/hebrew-numbers.html for more explaination

        /*
        Ref table for numbers to Hebrew chars

        Value     1       2       3        4        5       6         7        8      9

        x 1    Alef א  Bet  ב  Gimel ג  Dalet ד  He   ה  Vav  ו    Zayen ז  Het  ח  Tet  ט
               0x05D0  0x05D1  0x05D2   0x05D3   0x05D4  0x05D5    0x05D6   0x05D7  0x05D8

        x 10   Yod  י  Kaf  כ  Lamed ל  Mem  מ   Nun  נ  Samekh ס  Ayin ע   Pe   פ  Tzadi צ
               0x05D9  0x05DB  0x05DC   0x05DE   0x05E0  0x05E1    0x05E2   0x05E4  0x05E6

        x 100  Qof  ק  Resh ר  Shin ש   Tav  ת
               0x05E7  0x05E8  0x05E9   0x05EA

        Note special cases 15 = 9 + 6 = 96 טו and 16 = 9 + 7 = 97 טז
        */

        const QChar decade[] = {
        //  Tet = ט,    Yod = י,    Kaf = כ,    Lamed = ל,  Mem = מ
        //  Nun = נ,    Samekh = ס, Ayin = ע,   Pe = פ,     Tsadi = צ
            0x05D8,     0x05D9,     0x05DB,     0x05DC,     0x05DE,
            0x05E0,     0x05E1,     0x05E2,     0x05E4,     0x05E6
        };

        QString result;

        // We have no rules for coping with numbers outside this range
        if ( number < 1 || number > 9999 ) {
            return KCalendarSystemPrivate::stringFromInteger( number, padWidth, padChar, digitSet );
        }

        // Translate the thousands digit, just uses letter for number 1..9 ( א to ט, Alef to Tet )
        // Years 5001-5999 do not have the thousands by convention
        if ( number >= 1000 ) {
            if ( number <= 5000 || number >= 6000 ) {
                result += QChar( 0x05D0 - 1 + number / 1000 );  // Alef א to Tet ט
            }
            number %= 1000;
        }

        // Translate the hundreds digit
        // Use traditional method where we only have letters assigned values for 100, 200, 300 and 400
        // so may need to repeat 400 twice to make up the required number
        if ( number >= 100 ) {
            while ( number >= 500 ) {
                result += QChar( 0x05EA );  // Tav = ת
                number -= 400;
            }
            result += QChar( 0x05E7 - 1 + number / 100 ); // Qof = ק to xxx
            number %= 100;
        }

        // Translate the tens digit
        // The numbers 15 and 16 translate to letters that spell out the name of God which is
        // forbidden, so require special treatment where 15 = 9 + 6 and 1 = 9 + 7.
        if ( number >= 10 ) {
            if ( number == 15 || number == 16 )
                number -= 9;
            result += decade[number / 10];
            number %= 10;
        }

        // Translate the ones digit, uses letter for number 1..9 ( א to ט, Alef to Tet )
        if ( number > 0 ) {
            result += QChar( 0x05D0 - 1 + number );  // Alef = א to xxx
        }

        // When used in a string with mixed names and numbers the numbers need special chars to
        // distinguish them from words composed of the same letters.
        // Single digit numbers are followed by a geresh symbol ׳ (Unicode = 0x05F3), but we use
        // single quote for convenience.
        // Multiple digit numbers have a gershayim symbol ״ (Unicode = 0x05F4) as second-to-last
        // char, but we use double quote for convenience.
        if ( result.length() == 1 ) {
            result += '\'';
        } else {
            result.insert( result.length() - 1, '\"' );
        }

        return result;

    } else {
        if ( padChar == '\0' || padWidth == 0 ) {
            return q->locale()->convertDigits( QString::number( number ), digitSet );
        } else {
            return q->locale()->convertDigits( QString::number( number ).rightJustified( padWidth, padChar ), digitSet );
        }
    }
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

KCalendarSystem::KCalendarSystem( const KLocale *locale ) : d( new KCalendarSystemPrivate( this ) )
{
    setMaxDaysInWeek(7);
    setMaxMonthsInYear(12);
    setHasYear0(false);
    d->locale = locale;
}

KCalendarSystem::~KCalendarSystem()
{
    delete d;
}

// This method MUST be re-implemented in any new Calendar System
QDate KCalendarSystem::epoch() const
{
    // Earliest valid QDate
    return QDate::fromJulianDay( 1 );
}

QDate KCalendarSystem::earliestValidDate() const
{
    return epoch();
}

// This method MUST be re-implemented in any new Calendar System
QDate KCalendarSystem::latestValidDate() const
{
    // Default to Gregorian 9999-12-31
    return QDate::fromJulianDay( 5373484 );
}

// This method MUST be re-implemented in any new Calendar System
bool KCalendarSystem::isValid( int y, int month, int day ) const
{
    // Default to true Gregorian

    if ( y == 0 || y < year( earliestValidDate() ) || y > year( latestValidDate() ) ) {
        return false;
    }

    if ( month < 1 || month > 12 ) {
        return false;
    }

    if ( month == 2 ) {
        if ( isLeapYear( y ) ) {
            return ( day >= 1 && day <= 29 );
        } else {
            return ( day >= 1 && day <= 28 );
        }
    }

    if ( month == 4 || month == 6 || month == 9 || month == 11  ) {
        return ( day >= 1 && day <= 30 );
    }

    return ( day >= 1 && day <= 31 );
}

bool KCalendarSystem::isValid( int y, int dayOfYear ) const
{
    if ( y < year( earliestValidDate() ) || y > year( latestValidDate() ) ) {
        return false;
    }

    if ( !d->hasYear0 && y == 0 ) {
        return false;
    }

    QDate testDate;
    setDate( testDate, y, 1, 1 );

    return ( dayOfYear > 0 && isValid( testDate ) && dayOfYear <= daysInYear( testDate ) );
}

bool KCalendarSystem::isValidIsoWeekDate( int y, int isoWeekNumber, int dayOfIsoWeek ) const
{
    //Tests Year value in standard YMD isValid()
    if ( !isValid( y, 1, 1 ) ) {
        return false;
    }

    QDate firstDayOfYear;
    setDate( firstDayOfYear, y, 1, 1 );

    //Test Week Number falls in valid range for this year
    int weeksInThisYear = weeksInYear( y );
    if ( isoWeekNumber < 1 || isoWeekNumber  > weeksInThisYear ) {
        return false;
    }

    //Test Day of Week Number falls in valid range
    if ( dayOfIsoWeek < 1 || dayOfIsoWeek > daysInWeek( firstDayOfYear ) ) {
        return false;
    }

    //If not in earliest or latest years then all OK
    //Otherwise need to check don't fall into previous or next year that would be invalid
    if ( y == year( earliestValidDate() ) && isoWeekNumber == 1 ) {
        //If firstDayOfYear falls on or before Thursday then firstDayOfYear falls in week 1 this
        //year and if wanted dayOfIsoWeek falls before firstDayOfYear then falls in previous year
        //and so in invalid year
        int dowFirstDay = dayOfWeek( firstDayOfYear );
        if ( dowFirstDay <= 4 && dayOfIsoWeek < dowFirstDay ) {
            return false;
        }
    } else if ( y == year( latestValidDate() ) && isoWeekNumber == weeksInThisYear ) {
        //If lastDayOfYear falls on or after Thursday then lastDayOfYear falls in last week this
        //year and if wanted dayOfIsoWeek falls after lastDayOfYear then falls in next year
        //and so in invalid year
        QDate lastDayOfYear = addDays( firstDayOfYear, daysInYear( firstDayOfYear ) - 1 );
        int dowLastDay = dayOfWeek( lastDayOfYear );
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

bool KCalendarSystem::setDate( QDate &date, int year, int dayOfYear ) const
{
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

bool KCalendarSystem::setDateIsoWeek( QDate &date, int year, int isoWeekNumber, int dayOfIsoWeek ) const
{
    date = d->invalidDate();

    if ( isValidIsoWeekDate( year, isoWeekNumber, dayOfIsoWeek ) ) {

        QDate calcDate;
        setDate( calcDate, year, 1, 1 );

        int daysToAdd = ( daysInWeek( calcDate ) * ( isoWeekNumber - 1 ) ) + dayOfIsoWeek;
        int dowFirstDayOfYear = dayOfWeek( calcDate );

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

QDate KCalendarSystem::addYears( const QDate &date, int numYears ) const
{
    if ( isValid( date ) ) {

        int originalYear, originalMonth, originalDay;
        int newYear, newMonth, newDay;
        QDate firstOfNewMonth, newDate;

        julianDayToDate( date.toJulianDay(), originalYear, originalMonth, originalDay );

        newYear = d->addYearNumber( originalYear, numYears );

        newMonth = originalMonth;

        //Adjust day number if new month has fewer days than old month
        if ( setDate( firstOfNewMonth, newYear, newMonth, 1 ) ) {
            int daysInNewMonth = daysInMonth( firstOfNewMonth );
            newDay = ( daysInNewMonth < originalDay ) ? daysInNewMonth : originalDay;

            if ( setDate( newDate, newYear, newMonth, newDay ) ) {
                return newDate;
            }
        }

    }

    return d->invalidDate();
}

QDate KCalendarSystem::addMonths( const QDate &date, int numMonths ) const
{
    if ( isValid( date ) ) {

        int originalYear, originalMonth, originalDay;
        int newYear, newMonth, newDay;
        int monthsInOriginalYear, daysInNewMonth;
        QDate firstOfNewMonth, newDate;

        julianDayToDate( date.toJulianDay(), originalYear, originalMonth, originalDay );

        monthsInOriginalYear = monthsInYear( date );

        newYear = d->addYearNumber( originalYear, ( originalMonth + numMonths ) / monthsInOriginalYear );

        newMonth = ( originalMonth + numMonths ) % monthsInOriginalYear;

        if ( newMonth == 0 ) {
            newYear = d->addYearNumber( newYear, - 1 );
            newMonth = monthsInOriginalYear;
        }
        if ( newMonth < 0 ) {
            newYear = d->addYearNumber( newYear, - 1 );
            newMonth = newMonth + monthsInOriginalYear;
        }

        //Adjust day number if new month has fewer days than old month
        if ( setDate( firstOfNewMonth, newYear, newMonth, 1 ) ) {
            daysInNewMonth = daysInMonth( firstOfNewMonth );
            newDay = ( daysInNewMonth < originalDay ) ? daysInNewMonth : originalDay;

            if ( setDate( newDate, newYear, newMonth, newDay ) ) {
                return newDate;
            }
        }

    }

    return d->invalidDate();
}

QDate KCalendarSystem::addDays( const QDate &date, int numDays ) const
{
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

int KCalendarSystem::monthsInYear( const QDate &date ) const
{
    // Last day of this year = first day of next year minus 1 day
    // Use setAnyDate() to allow correct calculation in last valid year

    if ( isValid( date ) ) {
        QDate firstDayOfNextYear;
        d->setAnyDate( firstDayOfNextYear, d->addYearNumber( year( date ), 1 ), 1, 1 );
        QDate lastDayOfThisYear = addDays( firstDayOfNextYear, -1 );
        return month( lastDayOfThisYear );
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
    // Last day of this year = first day of this year plus days in this year minus 1 day

    if ( isValid( year, 1, 1 ) ) {
        QDate firstDayOfThisYear;
        setDate( firstDayOfThisYear, year, 1, 1);

        QDate lastDayOfThisYear = addDays( firstDayOfThisYear, daysInYear( firstDayOfThisYear ) - 1 );

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
    // Days in year = jd of first day of next year minus jd of first day of this year
    // Use setAnyDate() to allow correct calculation in last valid year

    if ( isValid( date ) ) {
        QDate firstDayOfThisYear, firstDayOfNextYear;

        setDate( firstDayOfThisYear, year( date ), 1, 1 );
        d->setAnyDate( firstDayOfNextYear, d->addYearNumber( year( date ), 1 ), 1, 1 );

        return ( firstDayOfNextYear.toJulianDay() - firstDayOfThisYear.toJulianDay() );
    }

    return -1;
}

int KCalendarSystem::daysInMonth( const QDate &date ) const
{
    // Days In Month = jd of first day of next month minus jd of first day of this month
    // Use setAnyDate() to allow correct calculation in last valid year

    if ( isValid( date ) ) {
        QDate firstDayOfThisMonth, firstDayOfNextMonth;

        int thisYear = year( date );
        int thisMonth = month( date );

        setDate( firstDayOfThisMonth, thisYear, thisMonth, 1 );

        //check if next month falls in next year
        if ( thisMonth < monthsInYear( date ) ) {
            setDate( firstDayOfNextMonth, thisYear, thisMonth + 1, 1 );
        } else {
            d->setAnyDate( firstDayOfNextMonth, d->addYearNumber( thisYear, 1 ), 1, 1 );
        }

        return ( firstDayOfNextMonth.toJulianDay() - firstDayOfThisMonth.toJulianDay() );
    }

    return -1;
}

int KCalendarSystem::daysInWeek( const QDate &date ) const
{
    Q_UNUSED( date );
    return 7;
}

int KCalendarSystem::dayOfYear( const QDate &date ) const
{
    //Take the jd of the given date, and subtract the jd of the first day of that year

    if ( isValid( date ) ) {
        QDate firstDayOfYear;

        if ( setDate( firstDayOfYear, year( date ), 1, 1 ) ) {
            return ( date.toJulianDay() - firstDayOfYear.toJulianDay() + 1 );
        }
    }

    return -1;
}

int KCalendarSystem::dayOfWeek( const QDate &date ) const
{
    // Makes assumption that Julian Day 0 was day 1 of week
    // This is true for Julian/Gregorian calendar with jd 0 being Monday
    // We add 1 for ISO compliant numbering for 7 day week
    // Assumes we've never skipped weekdays
    if ( isValid( date ) ) {
        return ( ( date.toJulianDay() % daysInWeek( date ) ) + 1 );
    }

    return -1;
}

// ISO compliant week numbering, not traditional number, rename in KDE5 to isoWeekNumber()
// JPL still need to fully clean up here
int KCalendarSystem::weekNumber( const QDate &date, int *yearNum ) const
{
    if ( isValid( date ) ) {
        QDate firstDayWeek1, lastDayOfYear;
        int y = year( date );
        int week;
        int weekDay1, dayOfWeek1InYear;

        // let's guess 1st day of 1st week
        setDate( firstDayWeek1, y, 1, 1 );
        weekDay1 = dayOfWeek( firstDayWeek1 );

        // iso 8601: week 1  is the first containing thursday and week starts on monday
        if ( weekDay1 > 4 /*Thursday*/ ) {
            firstDayWeek1 = addDays( firstDayWeek1 , daysInWeek( date ) - weekDay1 + 1 ); // next monday
        }

        dayOfWeek1InYear = dayOfYear( firstDayWeek1 );

        // our date in prev year's week
        if ( dayOfYear( date ) < dayOfWeek1InYear ) { 
            if ( yearNum ) {
                *yearNum = d->addYearNumber( y, - 1 );
            }
            return weeksInYear( d->addYearNumber( y, - 1 ) );
        }

        // let's check if its last week belongs to next year
        d->setAnyDate( lastDayOfYear, d->addYearNumber( y, 1 ), 1, 1 );
        lastDayOfYear = addDays( lastDayOfYear, -1 );
        // if our date is in last week && 1st week in next year has thursday
        if ( ( dayOfYear( date ) >= daysInYear( date ) - dayOfWeek( lastDayOfYear ) + 1 )
             && dayOfWeek( lastDayOfYear ) < 4 ) {
            if ( yearNum ) {
                * yearNum = d->addYearNumber( y, 1 );
            }
             week = 1;
        } else {
            // To calculate properly the number of weeks from day a to x let's make a day 1 of week
            if( weekDay1 < 5 ) {
                firstDayWeek1 = addDays( firstDayWeek1, -( weekDay1 - 1 ) );
            }

            week = firstDayWeek1.daysTo( date ) / daysInWeek( date ) + 1;
        }

        return week;
    }

    return -1;
}

// This method MUST be reimplemented in any derived Calendar Systems
bool KCalendarSystem::isLeapYear( int year ) const
{
    // Default to pure Gregorian

    int y;

    if ( year < 1 ) {
        y = year + 1;
    } else {
        y = year;
    }

    if ( y % 4 == 0 ) {
        if ( y % 100 != 0 ) {
            return true;
        } else if ( y % 400 == 0 ) {
            return true;
        }
    }
    return false;
}

bool KCalendarSystem::isLeapYear( const QDate &date ) const
{
    return isLeapYear( year( date ) );
}

QString KCalendarSystem::monthName( const QDate &date, MonthNameFormat format ) const
{
    if ( isValid( date ) ) {
        return monthName( month( date ), year( date ), format );
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

QString KCalendarSystem::dayOfYearString( const QDate &date, StringFormat format ) const
{
    if ( format == ShortFormat ) {
        return formatDate( date, "%-j" );
    } else {
        return formatDate( date, "%j" );
    }
}

QString KCalendarSystem::dayOfWeekString( const QDate &date ) const
{
    return formatDate( date, "%-u" );
}

QString KCalendarSystem::weekNumberString( const QDate &date, StringFormat format ) const
{
    if ( format == ShortFormat ) {
        return formatDate( date, "%-V" );
    } else {
        return formatDate( date, "%V" );
    }
}

// NOT VIRTUAL
QString KCalendarSystem::monthsInYearString( const QDate &date, StringFormat format ) const
{
    if ( format == ShortFormat ) {
        return d->stringFromInteger( monthsInYear( date ), 0, '0' );
    } else {
        return d->stringFromInteger( monthsInYear( date ), 2, '0' );
    }
}

// NOT VIRTUAL
QString KCalendarSystem::weeksInYearString( const QDate &date, StringFormat format ) const
{
    if ( format == ShortFormat ) {
        return d->stringFromInteger( weeksInYear( date ), 0, '0' );
    } else {
        return d->stringFromInteger( weeksInYear( date ), 2, '0' );
    }
}

// NOT VIRTUAL
QString KCalendarSystem::daysInYearString( const QDate &date, StringFormat format ) const
{
    if ( format == ShortFormat ) {
        return d->stringFromInteger( daysInYear( date ), 0, '0' );
    } else {
        return d->stringFromInteger( daysInYear( date ), 3, '0' );
    }
}

// NOT VIRTUAL
QString KCalendarSystem::daysInMonthString( const QDate &date, StringFormat format ) const
{
    if ( format == ShortFormat ) {
        return d->stringFromInteger( daysInMonth( date ), 0, '0' );
    } else {
        return d->stringFromInteger( daysInMonth( date ), 2, '0' );
    }
}

// NOT VIRTUAL
QString KCalendarSystem::daysInWeekString( const QDate &date) const
{
    return d->stringFromInteger( daysInWeek( date ), 0 );
}

int KCalendarSystem::yearStringToInteger( const QString &yearString, int &readLength ) const
{
    return d->integerFromString( yearString, 4, readLength );
}

int KCalendarSystem::monthStringToInteger( const QString &monthString, int &readLength ) const
{
    return d->integerFromString( monthString, 2, readLength );
}

int KCalendarSystem::dayStringToInteger( const QString &dayString, int &readLength ) const
{
    return d->integerFromString( dayString, 2, readLength );
}

QString KCalendarSystem::formatDate( const QDate &fromDate,
                                     KLocale::DateFormat toFormat ) const
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

QString KCalendarSystem::formatDate( const QDate &fromDate, const QString &toFormat,
                                     KLocale::DateTimeFormatStandard standard ) const
{
    return formatDate( fromDate, toFormat, locale()->dateTimeDigitSet(), standard );
}

QString KCalendarSystem::formatDate( const QDate &fromDate, const QString &toFormat, KLocale::DigitSet digitSet,
                                     KLocale::DateTimeFormatStandard standard ) const
{
    // Currently defaults to KLocale::KdeFormat, KDE 4.5 to support other standards (POSIX, Unicode)
    Q_UNUSED( standard )

    if ( !fromDate.isValid() ) {
        return QString();
    }

    QChar thisChar;
    QString buffer;

    bool escape = false;

    bool escapeWidth = false;
    int padWidth = 0;

    bool escapePad = false;
    QChar padChar = '0';

    QChar caseChar = QChar();

    QChar signChar = QChar();

    int yy = year( fromDate );
    int mm = month( fromDate );
    int dd = day( fromDate );

    for ( int format_index = 0; format_index < toFormat.length(); ++format_index ) {

        thisChar = toFormat.at( format_index ).unicode();

        if ( !escape ) {

            if ( thisChar == '%' ) {
                escape = true;
            } else {
                buffer.append( toFormat.at( format_index ) );
            }

        } else if ( !escapeWidth && thisChar == '-' ) { // no padding

            padChar = '\0';
            escapePad = true;

        } else if ( !escapeWidth && thisChar == '_' ) { // space padding

            padChar = ' ';
            escapePad = true;

        } else if ( !escapeWidth && thisChar == '0' ) { // 0 padding

            padChar = '0';
            escapePad = true;

        } else if ( !escapeWidth && ( thisChar == '^' || thisChar == '#' ) ) { // Change case

            caseChar = thisChar;

        } else if ( ( !escapeWidth && thisChar >= '1' && thisChar <= '9' ) ||
                    ( escapeWidth && thisChar >= '0' && thisChar <= '9' ) ) { // Change width

            if ( escapeWidth ) {
                padWidth = padWidth * 10;
            }
            padWidth = padWidth + QString( thisChar ).toInt();
            escapeWidth = true;

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
                case 'Y':  //Long year numeric, default 0 pad to 4 places with sign
                    componentInteger = qAbs( yy );
                    minWidth = 4;
                    if ( yy < 0 ) {
                        signChar = '-';
                    }
                    break;
                case 'C':  //Century numeric, default 0 pad to 2 places with sign
                    componentInteger =  qAbs( yy ) / 100 ;
                    minWidth = 2;
                    if ( yy < 0 ) {
                        signChar = '-';
                    }
                    break;
                case 'y':  //Short year numeric, default 0 pad to 2 places with sign
                    componentInteger =  qAbs( yy ) % 100;
                    minWidth = 2;
                    if ( yy < 0 ) {
                        signChar = '-';
                    }
                    break;
                case 'm':  //Long month numeric, default 0 pad to 2 places no sign
                    componentInteger =  mm;
                    minWidth = 2;
                    break;
                case 'n':  //Short month numeric, default no pad to 1 places no sign
                    //Note C/POSIX/GNU %n is actually newline not short month
                    //Copy what %e does, no padding by default
                    componentInteger =  mm;
                    minWidth = 1;
                    if ( !escapePad ) {
                        padChar = QChar();
                    }
                    break;
                case 'd':  //Long day numeric, default 0 pad to 2 places no sign
                    componentInteger =  dd;
                    minWidth = 2;
                    break;
                case 'e':  //Short day numeric, default no pad to 1 places no sign
                    //KDE does non-standard no-padding, C/POSIX/GNU pads with spaces by default
                    componentInteger =  dd;
                    minWidth = 1;
                    if ( !escapePad ) {
                        padChar = QChar();
                    }
                    break;
                case 'B':  //Long month name, default space pad to 0 places no sign
                    if ( locale()->dateMonthNamePossessive() ) {
                        componentString = monthName( mm, yy, KCalendarSystem::LongNamePossessive );
                    } else {
                        componentString = monthName( mm, yy, KCalendarSystem::LongName );
                    }
                    if ( !escapePad ) {
                        padChar = ' ';
                    }
                    break;
                case 'h':  //Short month name, default space pad to 0 places no sign
                case 'b':  //Short month name, default space pad to 0 places no sign
                    if ( locale()->dateMonthNamePossessive() ) {
                        componentString = monthName( mm, yy, KCalendarSystem::ShortNamePossessive );
                    } else {
                        componentString = monthName( mm, yy, KCalendarSystem::ShortName );
                    }
                    if ( !escapePad ) {
                        padChar = ' ';
                    }
                    break;
                case 'A':  //Long weekday name, default space pad to 0 places no sign
                    if ( !escapePad ) {
                        padChar = ' ';
                    }
                    componentString = weekDayName( fromDate, KCalendarSystem::LongDayName );
                    break;
                case 'a':  //Short weekday name, default space pad to 0 places no sign
                    componentString = weekDayName( fromDate, KCalendarSystem::ShortDayName );
                    if ( !escapePad ) {
                        padChar = ' ';
                    }
                    break;
                case 'j':  //Long day of year numeric, default 0 pad to 3 places no sign
                    componentInteger = dayOfYear( fromDate );
                    minWidth = 3;
                    break;
                case 'V':  //Long ISO week of year numeric, default 0 pad to 2 places no sign
                    componentInteger = weekNumber( fromDate );
                    minWidth = 2;
                    break;
                case 'G':  //Long year of ISO week of year numeric, default 0 pad to 4 places with sign
                    weekNumber( fromDate, &isoWeekYear );
                    setDate( yearDate, isoWeekYear, 1, 1 );
                    componentInteger = qAbs( isoWeekYear );
                    minWidth = 4;
                    if ( isoWeekYear < 0 ) {
                        signChar = '-';
                    }
                    break;
                case 'g':  //Short year of ISO week of year numeric, default 0 pad to 2 places with sign
                    weekNumber( fromDate, &isoWeekYear );
                    setDate( yearDate, isoWeekYear, 1, 1 );
                    componentInteger = qAbs( isoWeekYear ) % 100;
                    minWidth = 2;
                    if ( isoWeekYear < 0 ) {
                        signChar = '-';
                    }
                    break;
                case 'u':  //Short day of week numeric
                    componentInteger = dayOfWeek( fromDate );
                    minWidth = 1;
                    break;
                case 'D':  //US short date format, ignore any overrides
                    componentString = formatDate( fromDate, "%m/%d/%y" );
                    padWidth = 0;
                    padChar = QChar();
                    caseChar = QChar();
                    break;
                case 'F':  //Full or ISO short date format, ignore any overrides
                    componentString = formatDate( fromDate, "%Y-%m-%d" );
                    padWidth = 0;
                    padChar = QChar();
                    caseChar = QChar();
                    break;
                case 'x':  //Locale short date format, ignore any overrides
                    componentString = formatDate( fromDate, locale()->dateFormatShort() );
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
                componentString = d->stringFromInteger( componentInteger, padWidth, padChar, digitSet );
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
            caseChar = QChar();
            escapeWidth = false;
            padWidth = 0;
            signChar = QChar();
        }
    }

    return buffer;
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

QDate KCalendarSystem::readDate( const QString &intstr, const QString &fmtstr, bool *ok ) const
{
    QString str = intstr.simplified().toLower();
    QString fmt = fmtstr.simplified();
    int dd = -1;
    int mm = -1;
    int dayInYear = -1;
    int isoWeekNumber = -1;
    int dayOfIsoWeek = -1;
    // allow the year to be omitted if not in the format
    int yy = year( QDate::currentDate() );
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
            // remove space at the beginning
            if ( str.length() > strpos && str.at( strpos ).isSpace() ) {
                strpos++;
            }

            fmtChar = fmt.at( fmtpos++ );
            switch ( fmtChar.unicode() )
            {
                case 'a':  // Weekday Name Short
                case 'A':  // Weekday Name Long
                    error = true;
                    j = 1;
                    while ( error && j <= d->maxDaysInWeek ) {
                        shortName = weekDayName( j, KCalendarSystem::ShortDayName ).toLower();
                        longName = weekDayName( j, KCalendarSystem::LongDayName ).toLower();
                        if ( str.mid( strpos, longName.length() ) == longName ) {
                            strpos += longName.length();
                            error = false;
                        } else if ( str.mid( strpos, shortName.length() ) == shortName ) {
                            strpos += shortName.length();
                            error = false;
                        }
                        j++;
                    }
                    break;
                case 'b':  // Month Name Short
                case 'h':  // Month Name Short
                case 'B':  // Month Name Long
                    error = true;
                    j = 1;
                    while ( error && j <= d->maxMonthsInYear ) {
                        // This may be a problem in calendar systems with variable number of months
                        // in the year and/or names of months that change depending on the year, e.g
                        // Hebrew.  We really need to know the correct year first, but we may not have
                        // read it yet and will be using the current year instead
                        if ( locale()->dateMonthNamePossessive() ) {
                            shortName = monthName( j, yy, KCalendarSystem::ShortNamePossessive ).toLower();
                            longName = monthName( j, yy, KCalendarSystem::LongNamePossessive ).toLower();
                        } else {
                            shortName = monthName( j, yy, KCalendarSystem::ShortName ).toLower();
                            longName = monthName( j, yy, KCalendarSystem::LongName ).toLower();
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
                        j++;
                    }
                    break;
                case 'd': // Day Number Long
                case 'e': // Day Number Short
                    dd = dayStringToInteger( str.mid( strpos ), readLength );
                    strpos += readLength;
                    error = readLength <= 0;
                    break;
                case 'n': // Month Number Short
                case 'm': // Month Number Long
                    mm = monthStringToInteger( str.mid( strpos ), readLength );
                    strpos += readLength;
                    error = readLength <= 0;
                    break;
                case 'Y': // Year Number Long
                case 'y': // Year Number Short
                    yy = yearStringToInteger( str.mid( strpos ), readLength );
                    strpos += readLength;
                    // JPL are we sure about this? Do users really want 99 = 2099 or 1999? Should we use a Y2K style range?
                    // Using 2000 only valid for Gregorian, Hebrew should be 5000, etc.
                    if ( fmtChar == 'y' && yy >= 0 && yy < 100 )
                        yy += 2000; // QDate assumes 19xx by default, but this isn't what users want...
                        error = readLength <= 0;
                    break;
                case 'j': // Day Of Year Number
                    dayInYear = d->integerFromString( str.mid( strpos ), 3, readLength );
                    strpos += readLength;
                    error = readLength <= 0;
                    break;
                case 'V': // ISO Week Number
                    isoWeekNumber = d->integerFromString( str.mid( strpos ), 2, readLength );
                    strpos += readLength;
                    error = readLength <= 0;
                    break;
                case 'u': // ISO Day Of Week
                    dayOfIsoWeek = d->integerFromString( str.mid( strpos ), 1, readLength );
                    strpos += readLength;
                    error = readLength <= 0;
                    break;
            }
        }
    }

    // For a match, we should reach the end of both strings, not just one of them
    if ( fmt.length() > fmtpos || str.length() > strpos ) {
        error = true;
    }

    QDate resultDate = d->invalidDate();
    bool resultStatus = false;

    if ( error ) {
        resultStatus = false;
    } else if ( mm > 0 && dd > 0 ) {
        resultStatus = setDate( resultDate, yy, mm, dd );
    } else if ( dayInYear > 0 ) {
        resultStatus = setDate( resultDate, yy, dayInYear );
    } else if ( isoWeekNumber > 0 && dayOfIsoWeek > 0 ) {
        resultStatus = setDateIsoWeek( resultDate, yy, isoWeekNumber, dayOfIsoWeek );
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

// Fake version using QDate, each Calendar System MUST implement the correct version for themselves
// The implementation MUST NOT do validity checking on date ranges, all calls to this function MUST
// instead be wrapped in validity checks, as sometimes we want this to work outside the public valid
// range, i.e. to allow us to internally set dates of 1/1/10000 which are not publically valid but
// are required for internal maths
bool KCalendarSystem::julianDayToDate( int jd, int &year, int &month, int &day ) const
{
    QDate date = QDate::fromJulianDay( jd );

    if ( date.isValid() ) {
        year = date.year();
        month = date.month();
        day = date.day();
    }

    return date.isValid();
}

// Fake version using QDate, each Calendar System MUST implement the correct version for themselves
// The implementation MUST NOT do validity checking on date ranges, all calls to this function MUST
// instead be wrapped in validity checks, as sometimes we want this to work outside the public valid
// range, i.e. to allow us to internally set dates of 1/1/10000 which are not publically valid but
// are required for internal maths
bool KCalendarSystem::dateToJulianDay( int year, int month, int day, int &jd ) const
{
    QDate date;

    if ( date.setDate( year, month, day ) ) {
        jd = date.toJulianDay();
        return true;
    }

    return false;
}

const KLocale * KCalendarSystem::locale() const
{
    if ( d->locale ) {
        return d->locale;
    }

    return KGlobal::locale();
}

void KCalendarSystem::setMaxMonthsInYear( int maxMonths )
{
    d->maxMonthsInYear = maxMonths;
}

void KCalendarSystem::setMaxDaysInWeek( int maxDays )
{
    d->maxDaysInWeek = maxDays;
}

void KCalendarSystem::setHasYear0( bool hasYear0 )
{
    d->hasYear0 = hasYear0;
}
