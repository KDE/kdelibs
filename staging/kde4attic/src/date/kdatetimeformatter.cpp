/*
    Copyright 2009-2010 John Layt <john@layt.net>
    Copyright 2005-2010 David Jarvie <djarvie@kde.org>

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

#include "kdatetimeformatter_p.h"

#include <QDebug>
#include <QtCore/QDate>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QChar>

#include "kdatetime.h"
#include "ktimezone.h"
#include "kcalendarsystem.h"
#include "kdayperiod_p.h"
#include "klocale_p.h"

KDateTimeFormatter::KDateTimeFormatter()
    : m_englishLocale(0),
      m_englishCalendar(0)
{
}

KDateTimeFormatter::~KDateTimeFormatter()
{
    delete m_englishCalendar;
    delete m_englishLocale;
}

QString KDateTimeFormatter::formatDate(const QDate &fromDate,
                                       const QString &toFormat,
                                       const KCalendarSystem *calendar,
                                       const KLocale *locale,
                                       KLocale::DigitSet digitSet,
                                       KLocale::DateTimeFormatStandard formatStandard) const
{
    // If not valid input, don't waste our time
    if (!calendar->isValid(fromDate) || toFormat.isEmpty()) {
        return QString();
    }

    return formatDateTime(KDateTime(fromDate), toFormat, 0, calendar, locale, digitSet, formatStandard);
}

QString KDateTimeFormatter::formatTime(const QTime &fromTime,
                                       const QString &toFormat,
                                       KLocale::TimeFormatOptions timeOptions,
                                       const KCalendarSystem *calendar,
                                       const KLocale *locale,
                                       KLocale::DigitSet digitSet,
                                       KLocale::DateTimeFormatStandard formatStandard) const
{
    // If not valid input, don't waste our time
    if (fromTime.isValid() || toFormat.isEmpty()) {
        return QString();
    }

    return formatDateTime(KDateTime(QDate::currentDate(), fromTime), toFormat, timeOptions, calendar, locale, digitSet, formatStandard);
}

// Format an input date to match a POSIX date format string
QString KDateTimeFormatter::formatDateTime(const KDateTime &fromDateTime,
                                           const QString &toFormat,
                                           KLocale::TimeFormatOptions timeOptions,
                                           const KCalendarSystem *calendar,
                                           const KLocale *locale,
                                           KLocale::DigitSet digitSet,
                                           KLocale::DateTimeFormatStandard formatStandard) const
{
    // If not valid input, don't waste our time
    if (!fromDateTime.isValid() || !calendar->isValid(fromDateTime.date()) || toFormat.isEmpty()) {
        return QString();
    }

    if (formatStandard == KLocale::UnicodeFormat) {
        return formatDateTimeUnicode(fromDateTime, toFormat, timeOptions, calendar, locale, digitSet);
    } else {
        return formatDateTimePosix(fromDateTime, toFormat, timeOptions, calendar, locale, digitSet, formatStandard);
    }
}

// Format an input date to match a POSIX date format string
QString KDateTimeFormatter::formatDateTimePosix(const KDateTime &fromDateTime,
                                                const QString &toFormat,
                                                KLocale::TimeFormatOptions timeOptions,
                                                const KCalendarSystem *calendar,
                                                const KLocale *locale,
                                                KLocale::DigitSet digitSet,
                                                KLocale::DateTimeFormatStandard formatStandard) const
{
//qDebug() << "formatDateTimePosix(" << fromDateTime << toFormat << ")";
    // If not valid input, don't waste our time
    if (!fromDateTime.isValid() || toFormat.isEmpty()) {
        return QString();
    }

    QChar thisChar;  // Current toFormat char being processed
    QString result;  // Output string

    int padWidth = 0;     // The width to pad numbers to
    QChar padChar = QLatin1Char('0');  // The char to use when padding numbers
    QChar signChar;       // The sign to use when formatting numbers
    QChar caseChar;       // The case modifier to use

    bool escape = false;       // Are we processing an escape char (%)
    bool escapeWidth = false;  // Are we processing an escape width
    bool escapePad = false;    // Are we processing an escape pad char
    bool escapeMod = false;    // Are we processing an escape modifier
    int escapeIndex = 0;         // Position in string of current escape char (%)

    QChar modifierChar = QChar();
    bool invalidModifier = false;

    // Pre-fetch the core date components as they get used a lot
    // and it is 1/3rd more efficient than 3 separatre calls
    int year, month, day;
    calendar->getDate(fromDateTime.date(), &year, &month, &day);

    for (int formatIndex = 0; formatIndex < toFormat.length(); ++formatIndex) {

        thisChar = toFormat.at(formatIndex);

        if (!escape) {

            if (thisChar == QLatin1Char('%')) {
                escape = true;
                escapeIndex = formatIndex;
            } else {
                result.append(toFormat.at(formatIndex));
            }

        } else if (!escapeMod && !escapeWidth && thisChar == QLatin1Char('-')) {   // no padding

            padChar = QChar();
            escapePad = true;

        } else if (!escapeMod && !escapeWidth && thisChar == QLatin1Char('_')) {   // space padding

            padChar = QLatin1Char(' ');
            escapePad = true;

        } else if (!escapeMod && !escapeWidth && thisChar == QLatin1Char('0')) {   // 0 padding

            padChar = QLatin1Char('0');
            escapePad = true;

        } else if (!escapeMod && !escapeWidth && (thisChar == QLatin1Char('^') || thisChar == QLatin1Char('#'))) {     // Change case

            caseChar = thisChar;

        } else if (!escapeMod &&
                   ((!escapeWidth && thisChar >= QLatin1Char('1') && thisChar <= QLatin1Char('9')) ||
                    (escapeWidth && thisChar >= QLatin1Char('0') && thisChar <= QLatin1Char('9')))) {     // Change width

            if (escapeWidth) {
                padWidth = padWidth * 10;
            }
            padWidth = padWidth + QString(thisChar).toInt();
            escapeWidth = true;

        } else if (!escapeMod && (thisChar == QLatin1Char('E') || thisChar == QLatin1Char('O') || thisChar == QLatin1Char(':'))) {     // Set modifier

            escapeMod = true;
            modifierChar = thisChar;
            if (thisChar == QLatin1Char(':')) {
                invalidModifier = true;
            }

        } else {

            bool invalidComponent = false;
            QString componentString;
            int componentInteger = 0;
            int minWidth = 0;
            int isoWeekYear = year;
            QDate yearDate;
            KDateTime::SpecType timeSpecType;

            //Default settings unless overridden by pad and case flags and width: are 0 pad to 0 width no sign
            //Names will override 0 pad with no pad unless flagged
            //Numbers will override with correct width unless flagged
            QChar thisChar = toFormat.at(formatIndex).unicode();
            switch (thisChar.unicode()) {
            case '%':  //Literal %
                if (modifierChar != QLatin1Char(':')) {    // E and O mods are ignored if not used, but : is treated as literal
                    componentString = QLatin1Char('%');
                    if (!escapePad) {
                        padChar = QChar();
                    }
                }
                break;
            case 't':  //Tab
                if (modifierChar != QLatin1Char(':')) {
                    componentString = QString::fromLatin1("\t");
                    if (!escapePad) {
                        padChar = QChar();
                    }
                }
                break;
            case 'Y':
                if (modifierChar == QLatin1Char('E')) {    //Era Year, default no pad to 0 places no sign
                    if (!escapePad) {
                        padChar = QLatin1Char(' ');
                    }
                    componentString = calendar->eraYear(fromDateTime.date());
                } else if (modifierChar != QLatin1Char(':')) {    //Long year numeric, default 0 pad to 4 places with sign
                    componentInteger = qAbs(year);
                    minWidth = 4;
                    if (year < 0) {
                        signChar = QLatin1Char('-');
                    }
                }
                break;
            case 'C':
                if (modifierChar == QLatin1Char('E')) {    //Era name, default no pad to 0 places no sign
                    if (!escapePad) {
                        padChar = QLatin1Char(' ');
                    }
                    componentString = calendar->eraName(fromDateTime.date());
                } else if (modifierChar != QLatin1Char(':')) {    //Century numeric, default 0 pad to 2 places with sign
                    componentInteger =  qAbs(year) / 100 ;
                    minWidth = 2;
                    if (year < 0) {
                        signChar = QLatin1Char('-');
                    }
                }
                break;
            case 'y':
                if (modifierChar == QLatin1Char('E')) {    //Year in Era number, default 0 pad to 1 places no sign
                    componentInteger =  calendar->yearInEra(fromDateTime.date());
                    minWidth = 1;
                } else if (modifierChar != QLatin1Char(':')) {    //Short year numeric, default 0 pad to 2 places with sign
                    componentInteger =  qAbs(year) % 100;
                    minWidth = 2;
                    if (year < 0) {
                        signChar = QLatin1Char('-');
                    }
                }
                break;
            case 'm':  // Month numeric
                componentInteger =  month;
                if (modifierChar == QLatin1Char(':')) {    //Short month numeric, default no pad to 1 places no sign
                    minWidth = 1;
                    if (!escapePad) {
                        padChar = QChar();
                    }
                    invalidModifier = false;
                } else {  //Long month numeric, default 0 pad to 2 places no sign
                    componentInteger =  month;
                    minWidth = 2;
                }
                break;
            case 'n':
                //PosixFormat %n is newline
                //KdeFormat %n is short month numeric
                if (modifierChar != QLatin1Char(':')) {
                    if (formatStandard == KLocale::KdeFormat) {
                        //Copy what %e does, no padding by default
                        //Short month numeric, default no pad to 1 places no sign
                        componentInteger =  month;
                        minWidth = 1;
                        if (!escapePad) {
                            padChar = QChar();
                        }
                    } else {  // formatStandard == KLocale::PosixFormat
                        componentString = QLatin1Char('\n');
                    }
                }
                break;
            case 'd':  //Long day numeric, default 0 pad to 2 places no sign
                if (modifierChar != QLatin1Char(':')) {
                    componentInteger =  day;
                    minWidth = 2;
                }
                break;
            case 'e':  //Short day numeric, default no sign
                //PosixFormat %e is space pad to 2 places
                //KdeFormat %e is no pad to 1 place
                if (modifierChar != QLatin1Char(':')) {
                    componentInteger =  day;
                    if (formatStandard == KLocale::KdeFormat) {
                        minWidth = 1;
                        if (!escapePad) {
                            padChar = QChar();
                        }
                    } else {  // formatStandard == KLocale::PosixFormat
                        minWidth = 2;
                        if (!escapePad) {
                            padChar = QLatin1Char(' ');
                        }
                    }
                }
                break;
            case 'B':  //Long month name, default space pad to 0 places no sign
                if (locale->dateMonthNamePossessive()) {
                    if (modifierChar == QLatin1Char(':')) {
                        invalidModifier = false;
                        initEnglish(calendar, locale);
                        componentString = m_englishCalendar->monthName(month, year, KCalendarSystem::LongNamePossessive);
                    } else {
                        componentString = calendar->monthName(month, year, KCalendarSystem::LongNamePossessive);
                    }
                } else {
                    if (modifierChar == QLatin1Char(':')) {
                        invalidModifier = false;
                        initEnglish(calendar, locale);
                        componentString = m_englishCalendar->monthName(month, year, KCalendarSystem::LongName);
                    } else {
                        componentString = calendar->monthName(month, year, KCalendarSystem::LongName);
                    }
                }
                if (!escapePad) {
                    padChar = QLatin1Char(' ');
                }
                break;
            case 'h':  //Short month name, default space pad to 0 places no sign
            case 'b':  //Short month name, default space pad to 0 places no sign
                if (locale->dateMonthNamePossessive()) {
                    if (modifierChar == QLatin1Char(':')) {
                        invalidModifier = false;
                        initEnglish(calendar, locale);
                        componentString = m_englishCalendar->monthName(month, year, KCalendarSystem::ShortNamePossessive);
                    } else {
                        componentString = calendar->monthName(month, year, KCalendarSystem::ShortNamePossessive);
                    }
                } else {
                    if (modifierChar == QLatin1Char(':')) {
                        invalidModifier = false;
                        initEnglish(calendar, locale);
                        componentString = m_englishCalendar->monthName(month, year, KCalendarSystem::ShortName);
                    } else {
                        componentString = calendar->monthName(month, year, KCalendarSystem::ShortName);
                    }
                }
                if (!escapePad) {
                    padChar = QLatin1Char(' ');
                }
                break;
            case 'A':  //Long weekday name, default space pad to 0 places no sign
                if (modifierChar == QLatin1Char(':')) {
                    invalidModifier = false;
                    initEnglish(calendar, locale);
                    componentString = m_englishCalendar->weekDayName(fromDateTime.date(), KCalendarSystem::LongDayName);
                } else {
                    componentString = calendar->weekDayName(fromDateTime.date(), KCalendarSystem::LongDayName);
                }
                if (!escapePad) {
                    padChar = QLatin1Char(' ');
                }
                break;
            case 'a':  //Short weekday name, default space pad to 0 places no sign
                if (modifierChar == QLatin1Char(':')) {
                    invalidModifier = false;
                    initEnglish(calendar, locale);
                    componentString = m_englishCalendar->weekDayName(fromDateTime.date(), KCalendarSystem::ShortDayName);
                } else {
                    componentString = calendar->weekDayName(fromDateTime.date(), KCalendarSystem::ShortDayName);
                }
                if (!escapePad) {
                    padChar = QLatin1Char(' ');
                }
                break;
            case 'j':  //Long day of year numeric, default 0 pad to 3 places no sign
                if (modifierChar != QLatin1Char(':')) {
                    componentInteger = calendar->dayOfYear(fromDateTime.date());
                    minWidth = 3;
                }
                break;
            case 'V':  //Long ISO week of year numeric, default 0 pad to 2 places no sign
                if (modifierChar != QLatin1Char(':')) {
                    componentInteger = calendar->week(fromDateTime.date(), KLocale::IsoWeekNumber);
                    minWidth = 2;
                }
                break;
            case 'G':  //Long year of ISO week of year numeric, default 0 pad to 4 places with sign
                if (modifierChar != QLatin1Char(':')) {
                    calendar->week(fromDateTime.date(), KLocale::IsoWeekNumber, &isoWeekYear);
                    calendar->setDate(yearDate, isoWeekYear, 1, 1);
                    componentInteger = qAbs(isoWeekYear);
                    minWidth = 4;
                    if (isoWeekYear < 0) {
                        signChar = QLatin1Char('-');
                    }
                }
                break;
            case 'g':  //Short year of ISO week of year numeric, default 0 pad to 2 places with sign
                if (modifierChar != QLatin1Char(':')) {
                    calendar->week(fromDateTime.date(), KLocale::IsoWeekNumber, &isoWeekYear);
                    calendar->setDate(yearDate, isoWeekYear, 1, 1);
                    componentInteger = qAbs(isoWeekYear) % 100;
                    minWidth = 2;
                    if (isoWeekYear < 0) {
                        signChar = QLatin1Char('-');
                    }
                }
                break;
            case 'u':
                if (modifierChar == QLatin1Char(':')) {    // TZ UTC offset hours
                    invalidModifier = false;
                    KDateTime::SpecType timeSpecType = fromDateTime.timeType();
                    if (timeSpecType == KDateTime::UTC || timeSpecType == KDateTime::TimeZone ||
                            timeSpecType == KDateTime::OffsetFromUTC) {
                        componentInteger = fromDateTime.utcOffset() / 3600;
                        if (componentInteger >= 0) {
                            signChar = QLatin1Char('+');
                        } else {
                            componentInteger = -componentInteger;
                            signChar = QLatin1Char('-');
                        }
                        minWidth = 2;
                    }
                } else {  // Short day of week numeric
                    componentInteger = calendar->dayOfWeek(fromDateTime.date());
                    minWidth = 1;
                }
                break;
            case 'D':  // US short date format, ignore any overrides
                if (modifierChar != QLatin1Char(':')) {
                    componentString = formatDateTimePosix(fromDateTime, QString::fromLatin1("%m/%d/%y"), timeOptions, calendar, locale, digitSet, formatStandard);
                    padWidth = 0;
                    padChar = QChar();
                    caseChar = QChar();
                }
                break;
            case 'F':  // Full or ISO short date format, ignore any overrides
                if (modifierChar != QLatin1Char(':')) {
                    componentString = formatDateTimePosix(fromDateTime, QString::fromLatin1("%Y-%m-%d"), timeOptions, calendar, locale, digitSet, formatStandard);
                    padWidth = 0;
                    padChar = QChar();
                    caseChar = QChar();
                }
                break;
            case 'x':  // Locale short date format, ignore any overrides
                if (modifierChar != QLatin1Char(':')) {
                    componentString = formatDateTimePosix(fromDateTime, locale->dateFormatShort(), timeOptions, calendar, locale, digitSet, formatStandard);
                    padWidth = 0;
                    padChar = QChar();
                    caseChar = QChar();
                }
                break;
            case 'H':  // Long 24 hour
            case 'k':  // Short 24 hour
                if (modifierChar != QLatin1Char(':')) {
                    componentInteger =  fromDateTime.time().hour();
                    minWidth = 1;
                    if (!escapePad) {
                        padChar = QChar();
                    }
                }
                break;
            case 'I':  // Long 12 hour
            case 'l':  // Short 12 hour
                if (modifierChar != QLatin1Char(':')) {
                    if ((timeOptions & KLocale::TimeDuration) == KLocale::TimeDuration) {
                        componentInteger =  fromDateTime.time().hour();
                    } else {
                        componentInteger = locale->d->dayPeriodForTime(fromDateTime.time()).hourInPeriod(fromDateTime.time());
                    }
                    if (thisChar == QLatin1Char('I')) {
                        minWidth = 2;
                    } else {
                        minWidth = 1;
                        if (!escapePad) {
                            padChar = QChar();
                        }
                    }
                }
                break;
            case 'M':   // Long minutes
                if (modifierChar != QLatin1Char(':')) {
                    componentInteger = fromDateTime.time().minute();
                    minWidth = 2;
                }
                break;
            case 'S':   // Long seconds
                invalidModifier = false;
                if ((timeOptions & KLocale::TimeWithoutSeconds) == KLocale::TimeWithoutSeconds) {
                    //TODO strip the preceding/following punctuation
                } else {
                    componentInteger = fromDateTime.time().second();
                    if (modifierChar == QLatin1Char(':')) {    // Only if not 00 seconds
                        if (componentInteger > 0 || fromDateTime.time().msec() > 0) {
                            result.append(QLatin1Char(':'));
                            minWidth = 2;
                        }
                    } else {
                        minWidth = 2;
                    }
                }
                break;
            case 's':
                if (modifierChar == QLatin1Char(':')) {    // Milliseconds
                    invalidModifier = false;
                    componentInteger = fromDateTime.time().msec();
                    minWidth = 3;
                } else {  // Whole seconds since Unix Epoch
                    KDateTime unixEpoch;
                    unixEpoch.setTime_t(0);
                    componentInteger = unixEpoch.secsTo(fromDateTime);
                }
                break;
            case 'p':   // AM/PM symbol
            case 'P':   // AM/PM symbol in lowercase
                if ((timeOptions & KLocale::TimeWithoutAmPm) == KLocale::TimeWithoutAmPm) {
                    //TODO strip the preceding/following punctuation
                } else {
                    if (modifierChar == QLatin1Char(':')) {
                        invalidModifier = false;
                        initEnglish(calendar, locale);
                        componentString = m_englishLocale->d->dayPeriodForTime(fromDateTime.time()).periodName(KLocale::ShortName);
                    } else {
                        componentString = locale->d->dayPeriodForTime(fromDateTime.time()).periodName(KLocale::ShortName);
                    }
                    if (thisChar == QLatin1Char('P')) {
                        componentString = componentString.toLower();
                    }
                }
                break;
            case 'z':  // TZ UTC Offset
                invalidModifier = false;
                timeSpecType = fromDateTime.timeType();
                if (timeSpecType == KDateTime::UTC || timeSpecType == KDateTime::TimeZone ||
                        timeSpecType == KDateTime::OffsetFromUTC) {
                    if (modifierChar == QLatin1Char(':')) {    // TZ UTC offset hours & minutes with colon
                        int offsetInSeconds = fromDateTime.utcOffset();
                        if (offsetInSeconds >= 0) {
                            signChar = QLatin1Char('+');
                        } else {
                            offsetInSeconds = -offsetInSeconds;
                            signChar = QLatin1Char('-');
                        }
                        int offsetHours = offsetInSeconds / 3600;
                        int offsetMinutes = (offsetInSeconds / 60) % 60;
                        //int offsetSeconds = offsetInSeconds % 60;
                        QString hourComponent = stringFromInteger(offsetHours, 2, QLatin1Char('0'), signChar, digitSet, locale);
                        QString minuteComponent = stringFromInteger(offsetMinutes, 2, QLatin1Char('0'), QChar(), digitSet, locale);
                        componentString = hourComponent + QLatin1Char(':') + minuteComponent;
                        minWidth = 0;
                        padChar = QChar();
                        padWidth = 0;
                    } else {  // TZ UTC offset hours & minutes
                        componentInteger = fromDateTime.utcOffset() / 60;
                        if (componentInteger >= 0) {
                            signChar = QLatin1Char('+');
                        } else {
                            componentInteger = -componentInteger;
                            signChar = QLatin1Char('-');
                        }
                        minWidth = 4;
                    }
                }
                break;
            case 'Z':  // TZ Name
                invalidModifier = false;
                timeSpecType = fromDateTime.timeType();
                if (timeSpecType == KDateTime::UTC || timeSpecType == KDateTime::TimeZone) {
                    KTimeZone tz = fromDateTime.timeZone();
                    if (tz.isValid()) {
                        if (modifierChar == QLatin1Char(':')) {    // TZ full name
                            componentString = QString::fromLatin1(tz.abbreviation(fromDateTime.toUtc().dateTime()));
                        } else {  // TZ abbreviated name
                            componentString = tz.name();
                        }
                    }
                }
                break;
            default:  //No valid format code, treat as literal
                invalidComponent = true;
                break;
            }

            if (invalidComponent || invalidModifier) {    // If escape sequence invalid treat as literal
                componentString = toFormat.mid(escapeIndex, formatIndex);
            } else if (componentString.isEmpty()) {    //i.e. is a number component
                padWidth = qMax(minWidth, padWidth);
                componentString = stringFromInteger(componentInteger, padWidth, padChar, signChar, digitSet, locale);
            } else { //i.e. is a string component
                if (padChar != QChar() && padWidth != 0) {
                    componentString = componentString.rightJustified(padWidth, padChar);
                }

                if (caseChar == QLatin1Char('^')) {
                    componentString = componentString.toUpper();
                } else if (caseChar == QLatin1Char('#')) {
                    componentString = componentString.toUpper(); // JPL ???
                }
            }

            result.append(componentString);

            escape = false;
            escapePad = false;
            padChar = QLatin1Char('0');
            escapeMod = false;
            invalidModifier = false;
            invalidComponent = false;
            modifierChar = QChar();
            caseChar = QChar();
            escapeWidth = false;
            padWidth = 0;
            signChar = QChar();
        }
    }
//qDebug() << " return = " << result;
//qDebug() << "";
    return result;
}

void KDateTimeFormatter::initEnglish(const KCalendarSystem *calendar, const KLocale *locale) const
{
    if (!m_englishCalendar || m_englishCalendar->calendarSystem() != calendar->calendarSystem()) {
        // Set up an English locale and calendar for use with ':' modifier which forces English names
        if (!m_englishLocale) {
            m_englishLocale = new KLocale(*locale);
            m_englishLocale->setLanguage(QStringList() << QString::fromLatin1("en_US"));
        }
        delete m_englishCalendar;
        m_englishCalendar = KCalendarSystem::create(calendar->calendarSystem(), m_englishLocale);
    }
}

// Reimplement if special string handling required
// Format an input date to match a UNICODE date format string
// Original QDate::fmtDateTime() code taken from Qt 4.7 under LGPL, now heavily modifed
// Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
QString KDateTimeFormatter::formatDateTimeUnicode(const KDateTime &fromDateTime,
                                                  const QString &toFormat,
                                                  KLocale::TimeFormatOptions timeOptions,
                                                  const KCalendarSystem *calendar,
                                                  const KLocale *locale,
                                                  KLocale::DigitSet digitSet) const
{
    const QLatin1Char quote('\'');

    QString result;
    QString format;
    QChar status(QLatin1Char('0'));

    for (int i = 0; i < toFormat.length(); ++i) {
        if (toFormat.at(i) == quote) {
            if (status == quote) {
                if (i > 0 && toFormat.at(i - 1) == quote)
                    result += QLatin1Char('\'');
                status = QLatin1Char('0');
            } else {
                if (!format.isEmpty()) {
                    result += getUnicodeString(fromDateTime, format, timeOptions, calendar, locale, digitSet);
                    format.clear();
                }
                status = quote;
            }
        } else if (status == quote) {
            result += toFormat.at(i);
        } else if (toFormat.at(i) == status) {
            if (toFormat.at(i) == QLatin1Char('P') ||
                    toFormat.at(i) == QLatin1Char('p')) {
                status = QLatin1Char('0');
            }
            format += toFormat.at(i);
        } else {
            result += getUnicodeString(fromDateTime, format, timeOptions, calendar, locale, digitSet);
            format.clear();
            if ((toFormat.at(i) == QLatin1Char('d')) ||
                    (toFormat.at(i) == QLatin1Char('M')) ||
                    (toFormat.at(i) == QLatin1Char('y'))) {
                status = toFormat.at(i);
                format += toFormat.at(i);
            } else {
                result += toFormat.at(i);
                status = QLatin1Char('0');
            }
        }
    }

    result += getUnicodeString(fromDateTime, format, timeOptions, calendar, locale, digitSet);

    return result;
}

// Original QDate::getFmtString() code taken from Qt 4.7 under LGPL, now heavily modifed
// Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
// Replaces tokens by their value. See QDateTime::toString() for a list of valid tokens
QString KDateTimeFormatter::getUnicodeString(const KDateTime &fromDateTime,
                                             const QString &toFormat,
                                             KLocale::TimeFormatOptions timeOptions,
                                             const KCalendarSystem *calendar,
                                             const KLocale *locale,
                                             KLocale::DigitSet digitSet) const
{
    if (toFormat.isEmpty()) {
        return QString();
    }

    QString result = toFormat;
    int removed = 0;

    if (toFormat.startsWith(QLatin1String("dddd"))) {
        result = calendar->weekDayName(fromDateTime.date(), KCalendarSystem::LongDayName);
        removed = 4;
    } else if (toFormat.startsWith(QLatin1String("ddd"))) {
        result = calendar->weekDayName(fromDateTime.date(), KCalendarSystem::ShortDayName);
        removed = 3;
    } else if (toFormat.startsWith(QLatin1String("dd"))) {
        result = QString::number(calendar->day(fromDateTime.date())).rightJustified(2, QLatin1Char('0'), true);
        removed = 2;
    } else if (toFormat.at(0) == QLatin1Char('d')) {
        result = QString::number(calendar->day(fromDateTime.date()));
        removed = 1;
    } else if (toFormat.startsWith(QLatin1String("MMMM"))) {
        result = calendar->monthName(calendar->month(fromDateTime.date()), calendar->year(fromDateTime.date()), KCalendarSystem::LongName);
        removed = 4;
    } else if (toFormat.startsWith(QLatin1String("MMM"))) {
        result = calendar->monthName(calendar->month(fromDateTime.date()), calendar->year(fromDateTime.date()), KCalendarSystem::ShortName);
        removed = 3;
    } else if (toFormat.startsWith(QLatin1String("MM"))) {
        result = QString::number(calendar->month(fromDateTime.date())).rightJustified(2, QLatin1Char('0'), true);
        removed = 2;
    } else if (toFormat.at(0) == QLatin1Char('M')) {
        result = QString::number(calendar->month(fromDateTime.date()));
        removed = 1;
    } else if (toFormat.startsWith(QLatin1String("yyyy"))) {
        const int year = calendar->year(fromDateTime.date());
        result = QString::number(qAbs(year)).rightJustified(4, QLatin1Char('0'));
        if (year > 0)
            removed = 4;
        else {
            result.prepend(QLatin1Char('-'));
            removed = 5;
        }
    } else if (toFormat.startsWith(QLatin1String("yy"))) {
        const int year = calendar->year(fromDateTime.date());
        result = QString::number(year).right(2).rightJustified(2, QLatin1Char('0'));
        if (year > 0)
            removed = 2;
        else {
            if (result.startsWith('0')) {
                result = result.right(1);
            }
            result.prepend(QLatin1Char('-'));
            removed = 3;
        }
    }

    if (removed == 0 || removed >= toFormat.size()) {
        return result;
    }

    return result + getUnicodeString(fromDateTime, toFormat.mid(removed), timeOptions, calendar, locale, digitSet);
}

// Reimplement if special integer to string handling required, e.g. Hebrew.
// Utility to convert an integer into the correct display string form
QString KDateTimeFormatter::stringFromInteger(int number, int padWidth, QChar padChar, QChar signChar,
                                              KLocale::DigitSet digitSet, const KLocale *locale) const
{
    if (padChar == QChar() && signChar == QChar()) {
//qDebug() << "  stringFromInteger(" << number << padWidth << "null" << "null" << ")";
    } else if (padChar == QChar()) {
//qDebug() << "  stringFromInteger(" << number << padWidth << "null" << signChar << ")";
    } else if (signChar == QChar()) {
//qDebug() << "  stringFromInteger(" << number << padWidth << padChar << "null" << ")";
    } else if (signChar == QChar()) {
//qDebug() << "  stringFromInteger(" << number << padWidth << padChar << signChar << ")";
    }
    QString result;
    if (padChar == QChar() || padWidth == 0) {   // If null pad char or 0 width don't bother padding
//qDebug() << "    no pad";
        if (signChar == QChar()) {
            result = locale->convertDigits(QString::number(number), digitSet);
        } else {
            result = locale->convertDigits(QString::number(number).prepend(signChar), digitSet);
        }
    } else if (signChar != QChar()) {    // If sign required
        if (padChar == QLatin1Char('0')) {   // If zero-padded, zero considered part of the number, so pad the number then prepend the sign
//qDebug() << "    zero pad with sign";
            result = locale->convertDigits(QString::number(number).rightJustified(padWidth, padChar).prepend(signChar), digitSet);
        } else { // If space-padded space not considered part of the number, so prepend the sign and then pad the number
//qDebug() << "    space pad with sign";
            result = locale->convertDigits(QString::number(number).prepend(signChar).rightJustified(padWidth, padChar), digitSet);
        }
    } else {  // No sign required so just pad
//qDebug() << "    pad no sign";
        result = locale->convertDigits(QString::number(number).rightJustified(padWidth, padChar), digitSet);
    }
//qDebug() << "    result = " << result;
    return result;
}
