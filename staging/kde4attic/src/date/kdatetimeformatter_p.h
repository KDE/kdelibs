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

#ifndef KDATETIMEFORMATTER_H
#define KDATETIMEFORMATTER_H

#include "klocale.h"

class QChar;
class QString;
class QDate;
class QTime;

class KDateTime;
class KCalendarSystem;

class KDateTimeFormatter
{
public:
    explicit KDateTimeFormatter();

    virtual ~KDateTimeFormatter();

    virtual QString formatDate(const QDate &fromDate,
                               const QString &toFormat,
                               const KCalendarSystem *calendar  = KLocale::global()->calendar(),
                               const KLocale *locale = KLocale::global(),
                               KLocale::DigitSet digitSet = KLocale::global()->dateTimeDigitSet(),
                               KLocale::DateTimeFormatStandard standard = KLocale::KdeFormat) const;

    virtual QString formatTime(const QTime &fromTime,
                               const QString &toFormat,
                               KLocale::TimeFormatOptions timeOptions = 0,
                               const KCalendarSystem *calendar  = KLocale::global()->calendar(),
                               const KLocale *locale = KLocale::global(),
                               KLocale::DigitSet digitSet = KLocale::global()->dateTimeDigitSet(),
                               KLocale::DateTimeFormatStandard standard = KLocale::KdeFormat) const;

    virtual QString formatDateTime(const KDateTime &fromDateTime,
                                   const QString &toFormat,
                                   KLocale::TimeFormatOptions timeOptions = 0,
                                   const KCalendarSystem *calendar  = KLocale::global()->calendar(),
                                   const KLocale *locale = KLocale::global(),
                                   KLocale::DigitSet digitSet = KLocale::global()->dateTimeDigitSet(),
                                   KLocale::DateTimeFormatStandard standard = KLocale::KdeFormat) const;

private:
    virtual QString formatDateTimePosix(const KDateTime &fromDateTime,
                                        const QString &toFormat,
                                        KLocale::TimeFormatOptions timeOptions,
                                        const KCalendarSystem *calendar,
                                        const KLocale *locale,
                                        KLocale::DigitSet digitSet,
                                        KLocale::DateTimeFormatStandard standard) const;

    virtual void initEnglish(const KCalendarSystem *calendar, const KLocale *locale) const;

    virtual QString formatDateTimeUnicode(const KDateTime &fromDateTime,
                                          const QString &toFormat,
                                          KLocale::TimeFormatOptions timeOptions,
                                          const KCalendarSystem *calendar,
                                          const KLocale *locale,
                                          KLocale::DigitSet digitSet) const;

    virtual QString getUnicodeString(const KDateTime &fromDateTime,
                                     const QString &toFormat,
                                     KLocale::TimeFormatOptions timeOptions,
                                     const KCalendarSystem *calendar,
                                     const KLocale *locale,
                                     KLocale::DigitSet digitSet) const;

    virtual QString stringFromInteger(int number, int padWidth, QChar padChar, QChar signChar,
                                      KLocale::DigitSet digitSet, const KLocale *locale) const;

    // Is private class, but if ever made public need to move these into a d->
    // Some format modifiers force English names to be returned
    mutable KLocale *m_englishLocale;
    mutable KCalendarSystem *m_englishCalendar;
};

#endif // KDATETIMEFORMATTER_H
