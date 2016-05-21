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

#include "kdatetimeparser_p.h"

#include "kcalendarsystemprivate_p.h"
#include "kcalendarsystem.h"
#include "kcalendarera_p.h"

#include "kdebug.h"

KDateTimeParser::KDateTimeParser()
{
}

KDateTimeParser::~KDateTimeParser()
{
}

// Parse a DateTime input string and return just the Date component
QDate KDateTimeParser::parseDate(const QString &inputString,
                                 const QString &formatString,
                                 const KCalendarSystem *calendar,
                                 const KLocale *locale,
                                 KLocale::DigitSet digitSet,
                                 KLocale::DateTimeFormatStandard formatStandard) const
{
    DateTimeComponents result;
    if (formatStandard == KLocale::UnicodeFormat) {
        result = parseDateUnicode(inputString, formatString, calendar, locale, digitSet);
    } else {
        result = parseDatePosix(inputString, formatString, calendar, locale, digitSet, formatStandard);
    }

    QDate resultDate;

    if (!result.error &&
            formatString.simplified().length() <= result.formatPosition &&
            inputString.simplified().length() <= result.inputPosition) {

        // If there were no parsing errors, and we have reached the end of both the input and
        // format strings, then see if we have a valid date based on the components parsed

        // If we haven't parsed a year component, then assume this year
        if (!result.parsedYear) {
            result.year = calendar->year(QDate::currentDate());
        }

        if ((!result.eraName.isEmpty() || result.yearInEra > -1) && result.month > 0 && result.day > 0) {
            // Have parsed Era components as well as month and day components
            calendar->setDate(resultDate, result.eraName, result.yearInEra, result.month, result.day);
        } else if (result.month > 0 && result.day > 0) {
            // Have parsed month and day components
            calendar->setDate(resultDate, result.year, result.month, result.day);
        } else if (result.dayInYear > 0) {
            // Have parsed Day In Year component
            calendar->setDate(resultDate, result.year, result.dayInYear);
        } else if (result.isoWeekNumber > 0 && result.dayOfIsoWeek > 0) {
            // Have parsed ISO Week components
            calendar->setDateIsoWeek(resultDate, result.year, result.isoWeekNumber, result.dayOfIsoWeek);
        }

    }

    return resultDate;
}

DateTimeComponents KDateTimeParser::parseDatePosix(const QString &inputString,
                                                   const QString &formatString,
                                                   const KCalendarSystem *calendar,
                                                   const KLocale *locale,
                                                   KLocale::DigitSet digitSet,
                                                   KLocale::DateTimeFormatStandard standard) const
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

    while (fmt.length() > fmtpos && str.length() > strpos && !error) {

        QChar fmtChar = fmt.at(fmtpos++);

        if (fmtChar != QLatin1Char('%')) {

            if (fmtChar.isSpace() && str.at(strpos).isSpace()) {
                strpos++;
            } else if (fmtChar.toLower() == str.at(strpos)) {
                strpos++;
            } else {
                error = true;
            }

        } else {
            int j;
            QString shortName, longName;
            QChar modifierChar;
            // remove space at the beginning
            if (str.length() > strpos && str.at(strpos).isSpace()) {
                strpos++;
            }

            fmtChar = fmt.at(fmtpos++);
            if (fmtChar == QLatin1Char('E')) {
                modifierChar = fmtChar;
                fmtChar = fmt.at(fmtpos++);
            }

            switch (fmtChar.unicode()) {
            case 'a':  // Weekday Name Short
            case 'A':  // Weekday Name Long
                error = true;
                j = 1;
                while (error && j <= calendar->d_ptr->maxDaysInWeek()) {
                    shortName = calendar->weekDayName(j, KCalendarSystem::ShortDayName).toLower();
                    longName = calendar->weekDayName(j, KCalendarSystem::LongDayName).toLower();
                    if (str.mid(strpos, longName.length()) == longName) {
                        strpos += longName.length();
                        error = false;
                    } else if (str.mid(strpos, shortName.length()) == shortName) {
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
                while (error && j <= calendar->d_ptr->maxMonthsInYear()) {
                    // This may be a problem in calendar systems with variable number of months
                    // in the year and/or names of months that change depending on the year, e.g
                    // Hebrew.  We really need to know the correct year first, but we may not have
                    // read it yet and will be using the current year instead
                    int monthYear;
                    if (parsedYear) {
                        monthYear = yy;
                    } else {
                        monthYear = calendar->year(QDate::currentDate());
                    }
                    if (calendar->locale()->dateMonthNamePossessive()) {
                        shortName = calendar->monthName(j, monthYear, KCalendarSystem::ShortNamePossessive).toLower();
                        longName = calendar->monthName(j, monthYear, KCalendarSystem::LongNamePossessive).toLower();
                    } else {
                        shortName = calendar->monthName(j, monthYear, KCalendarSystem::ShortName).toLower();
                        longName = calendar->monthName(j, monthYear, KCalendarSystem::LongName).toLower();
                    }
                    if (str.mid(strpos, longName.length()) == longName) {
                        mm = j;
                        strpos += longName.length();
                        error = false;
                    } else if (str.mid(strpos, shortName.length()) == shortName) {
                        mm = j;
                        strpos += shortName.length();
                        error = false;
                    }
                    ++j;
                }
                break;
            case 'd': // Day Number Long
            case 'e': // Day Number Short
                dd = calendar->dayStringToInteger(str.mid(strpos), readLength);
                strpos += readLength;
                error = readLength <= 0;
                break;
            case 'n':
                // PosixFormat %n is Newline
                // KdeFormat %n is Month Number Short
                if (standard == KLocale::KdeFormat) {
                    mm = calendar->monthStringToInteger(str.mid(strpos), readLength);
                    strpos += readLength;
                    error = readLength <= 0;
                }
                // standard == KLocale::PosixFormat
                // all whitespace already 'eaten', no action required
                break;
            case 'm': // Month Number Long
                mm = calendar->monthStringToInteger(str.mid(strpos), readLength);
                strpos += readLength;
                error = readLength <= 0;
                break;
            case 'Y': // Year Number Long
            case 'y': // Year Number Short
                if (modifierChar == QLatin1Char('E')) {    // Year In Era
                    if (fmtChar == QLatin1Char('y')) {
                        ey = calendar->yearStringToInteger(str.mid(strpos), readLength);
                        strpos += readLength;
                        error = readLength <= 0;
                    } else {
                        error = true;
                        j = calendar->eraList()->count() - 1; // Start with the most recent
                        while (error && j >= 0) {
                            QString subFormat = calendar->eraList()->at(j).format();
                            QString subInput = str.mid(strpos);
                            DateTimeComponents subResult = parseDatePosix(subInput, subFormat, calendar, locale, digitSet, standard);
                            if (!subResult.error) {
                                if (subResult.parsedYear) {
                                    yy = subResult.year;
                                    parsedYear = true;
                                    error = false;
                                    strpos += subResult.inputPosition;
                                } else if (!subResult.eraName.isEmpty() && subResult.yearInEra >= 0) {
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
                    yy = calendar->yearStringToInteger(str.mid(strpos), readLength);
                    strpos += readLength;
                    if (fmtChar == QLatin1Char('y')) {
                        yy = calendar->applyShortYearWindow(yy);
                    }
                    error = readLength <= 0;
                    if (!error) {
                        parsedYear = true;
                    }
                }
                break;
            case 'C': // Era
                error = true;
                if (modifierChar == QLatin1Char('E')) {
                    j = calendar->eraList()->count() - 1; // Start with the most recent
                    while (error && j >= 0) {
                        shortName = calendar->d_ptr->m_eraList->at(j).name(KLocale::ShortName).toLower();
                        longName = calendar->eraList()->at(j).name(KLocale::LongName).toLower();
                        if (str.mid(strpos, longName.length()) == longName) {
                            strpos += longName.length();
                            ee = longName;
                            error = false;
                        } else if (str.mid(strpos, shortName.length()) == shortName) {
                            strpos += shortName.length();
                            ee = shortName;
                            error = false;
                        }
                        --j;
                    }
                }
                break;
            case 'j': // Day Of Year Number
                dayInYear = integerFromString(str.mid(strpos), 3, readLength);
                strpos += readLength;
                error = readLength <= 0;
                break;
            case 'V': // ISO Week Number
                isoWeekNumber = integerFromString(str.mid(strpos), 2, readLength);
                strpos += readLength;
                error = readLength <= 0;
                break;
            case 'u': // ISO Day Of Week
                dayOfIsoWeek = integerFromString(str.mid(strpos), 1, readLength);
                strpos += readLength;
                error = readLength <= 0;
                break;
            }
        }
    }

    DateTimeComponents result;
    result.error = error;
    result.inputPosition = strpos;
    result.formatPosition = fmtpos;
    if (error) {
        result.day = -1;
        result.month = -1;
        result.year = 0;
        result.parsedYear = false;
        result.eraName.clear();
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

// Parse an input string to match a UNICODE DateTime format string and return any components found
DateTimeComponents KDateTimeParser::parseDateUnicode(const QString &inputString,
                                                     const QString &formatString,
                                                     const KCalendarSystem *calendar,
                                                     const KLocale *locale,
                                                     KLocale::DigitSet digitSet) const
{
    Q_UNUSED(calendar);
    Q_UNUSED(locale);
    Q_UNUSED(digitSet);
    Q_UNUSED(inputString);
    Q_UNUSED(formatString);

    kWarning() << "KDateTimeParser::parseDateUnicode is not implemented";

    DateTimeComponents result;
    result.error = true;
    result.inputPosition = 0;
    result.formatPosition = 0;
    result.day = -1;
    result.month = -1;
    result.year = 0;
    result.parsedYear = false;
    result.eraName.clear();
    result.yearInEra = -1;
    result.dayInYear = -1;
    result.isoWeekNumber = -1;
    result.dayOfIsoWeek = -1;
    return result;
}

// Peel a number off the front of a string which may have other trailing chars after the number
// Stop either at either maxLength, eos, or first non-digit char
int KDateTimeParser::integerFromString(const QString &string, int maxLength, int &readLength) const
{
    int value = -1;
    int position = 0;
    readLength = 0;
    bool ok = false;

    if (maxLength < 0) {
        maxLength = string.length();
    }

    while (position < string.length() &&
            position < maxLength &&
            string.at(position).isDigit()) {
        position++;
    }

    if (position > 0) {
        value = string.left(position).toInt(&ok);
        if (ok) {
            readLength = position;
        } else {
            value = -1;
        }
    }

    return value;
}
