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

#ifndef KDATETIMEPARSER_H
#define KDATETIMEPARSER_H

#include "klocale.h"

class QChar;
class QString;
class QDate;
class QTime;

class KDateTime;
class KCalendarSystem;

struct DateTimeComponents {
    int day;
    int month;
    int year;
    bool parsedYear;
    QString eraName;
    int yearInEra;
    int dayInYear;
    int isoWeekNumber;
    int dayOfIsoWeek;
    int inputPosition;
    int formatPosition;
    bool error;
};

class KDateTimeParser
{
public:
    explicit KDateTimeParser();

    virtual ~KDateTimeParser();

    virtual QDate parseDate(const QString &dateString,
                            const QString &format,
                            const KCalendarSystem *calendar  = KLocale::global()->calendar(),
                            const KLocale *locale = KLocale::global(),
                            KLocale::DigitSet digitSet = KLocale::ArabicDigits,
                            KLocale::DateTimeFormatStandard standard = KLocale::KdeFormat) const;

private:
    virtual DateTimeComponents parseDatePosix(const QString &dateString,
                                              const QString &format,
                                              const KCalendarSystem *calendar,
                                              const KLocale *locale,
                                              KLocale::DigitSet digitSet,
                                              KLocale::DateTimeFormatStandard standard) const;

    virtual DateTimeComponents parseDateUnicode(const QString &inputString,
                                                const QString &format,
                                                const KCalendarSystem *calendar,
                                                const KLocale *locale,
                                                KLocale::DigitSet digitSet) const;

    virtual int integerFromString(const QString &string, int maxLength, int &readLength) const;
};

#endif // KDATETIMEPARSER_H
