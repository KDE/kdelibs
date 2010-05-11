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

#ifndef KCALENDARSYSTEMPRIVATE_H
#define KCALENDARSYSTEMPRIVATE_H

class QChar;
class QString;
class QDate;

class KCalendarSystem;
class KCalendarEra;
class KLocale;

struct DateComponents
{
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

class KCalendarSystemPrivate
{
public:
    explicit KCalendarSystemPrivate( KCalendarSystem *q );

    virtual ~KCalendarSystemPrivate();

    // Virtual methods each calendar system must re-implement
    virtual void initDefaultEraList();
    virtual int monthsInYear( int year ) const;
    virtual int daysInMonth( int year, int month ) const;
    virtual int daysInYear( int year ) const;
    virtual int daysInWeek() const;
    virtual bool isLeapYear( int year ) const;
    virtual bool hasLeapMonths() const;
    virtual bool hasYearZero() const;
    virtual int maxDaysInWeek() const;
    virtual int maxMonthsInYear() const;
    virtual int earliestValidYear() const;
    virtual int latestValidYear() const;

    // Virtual methods to re-implement if special maths needed
    // Currently only Hebrew may need special conversion, rest should be OK
    virtual int yearsDifference( const QDate &fromDate, const QDate &toDate ) const;
    virtual int monthsDifference( const QDate &fromDate, const QDate &toDate ) const;
    virtual void dateDifference( const QDate &fromDate, const QDate &toDate,
                                 int *yearsDiff, int *monthsDiff, int *daysDiff, int *direction ) const;

    // Virtual methods to re-implement if special number/string conversion needed
    // Currently only Hebrew needs special conversion, rest use KLocale DigitSet
    virtual QString formatDatePosix( const QDate &fromDate,
                                     const QString &toFormat, KLocale::DigitSet digitSet,
                                     KLocale::DateTimeFormatStandard standard ) const;
    virtual QString formatDateUnicode( const QDate &fromDate, const QString &toFormat, KLocale::DigitSet digitSet ) const;
    virtual QString getUnicodeString( const QDate &fromDate, const QString &toFormat ) const;
    virtual DateComponents parseDatePosix( const QString &inputString, const QString &formatString,
                                           KLocale::DateTimeFormatStandard standard ) const;
    virtual DateComponents parseDateUnicode( const QString &inputString, const QString &formatString ) const;
    virtual int integerFromString( const QString &string, int maxLength, int &readLength ) const;
    virtual QString stringFromInteger( int number, int padWidth = 0, QChar padChar = '0' ) const;
    virtual QString stringFromInteger( int number, int padWidth, QChar padChar, KLocale::DigitSet digitSet ) const;

    // Utility functions
    bool setAnyDate( QDate &date, int year, int month, int day ) const;
    int addYears( int startYear, int yearsToAdd ) const;
    int differenceYearNumbers( int fromYear, int toYear ) const;
    QDate invalidDate() const;
    QString simpleDateString( const QString &str ) const;
    QDate firstDayOfYear( int year ) const;
    QDate lastDayOfYear( int year ) const;
    const KLocale *locale() const;
    KCalendarEra era( const QDate &eraDate ) const;
    KCalendarEra era( const QString &eraName, int yearInEra ) const;
    void initialiseEraList( const QString & calendarType );
    void loadGlobalEraList( const QString & calendarType );
    void addEra( const QChar &direction, int offset, const QDate &startDate, int startYear, const QDate &endDate,
                 const QString &name, const QString &shortName, const QString &format );
    KSharedConfig::Ptr config();


    // Global variables each calendar system must initialise
    const KCalendarSystem *q;
    const KLocale *m_locale;
    KSharedConfig::Ptr m_config;
    QList<KCalendarEra> *m_eraList;
};

#endif // KCALENDARSYSTEMPRIVATE_H
