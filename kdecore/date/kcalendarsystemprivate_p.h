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

class KCalendarSystemPrivate
{
public:
    explicit KCalendarSystemPrivate(KCalendarSystem *q);

    virtual ~KCalendarSystemPrivate();

    // Virtual methods each calendar system must re-implement
    virtual void loadDefaultEraList() = 0;
    virtual int monthsInYear(int year) const = 0;
    virtual int daysInMonth(int year, int month) const = 0;
    virtual int daysInYear(int year) const = 0;
    virtual bool isLeapYear(int year) const = 0;
    virtual bool hasLeapMonths() const = 0;
    virtual bool hasYearZero() const = 0;
    virtual int maxMonthsInYear() const = 0;
    virtual int earliestValidYear() const = 0;
    virtual int latestValidYear() const = 0;
    virtual QString monthName(int month, int year, KLocale::DateTimeComponentFormat format, bool possessive = false) const = 0;
    virtual QString weekDayName(int weekDay, KLocale::DateTimeComponentFormat format) const = 0;

    // Virtual methods to re-implement if special maths needed
    // Currently only Hebrew may need special conversion, rest should be OK
    virtual int yearsDifference(const QDate &fromDate, const QDate &toDate) const;
    virtual int monthsDifference(const QDate &fromDate, const QDate &toDate) const;
    virtual void dateDifference(const QDate &fromDate, const QDate &toDate,
                                int *yearsDiff, int *monthsDiff, int *daysDiff, int *direction) const;

    // Virtual methods to re-implement if special number/string conversion needed
    // Currently only Hebrew needs special conversion, rest use KLocale DigitSet
    virtual int integerFromString(const QString &string, int maxLength, int &readLength) const;
    virtual QString stringFromInteger(int number, int padWidth = 0, QChar padChar = QLatin1Char('0')) const;
    virtual QString stringFromInteger(int number, int padWidth, QChar padChar, KLocale::DigitSet digitSet) const;

    // Utility functions
    int addYears(int startYear, int yearsToAdd) const;
    int differenceYearNumbers(int fromYear, int toYear) const;
    QString simpleDateString(const QString &str) const;
    int daysInWeek() const;
    int dayOfYear(const QDate &date) const;
    int dayOfWeek(const QDate &date) const;
    QDate firstDayOfYear(int year) const;
    QDate lastDayOfYear(int year) const;
    QDate firstDayOfMonth(int year, int month) const;
    QDate lastDayOfMonth(int year, int month) const;
    const KLocale *locale() const;
    void loadEraList(const KConfigGroup & cg);
    void addEra(char direction, int offset, const QDate &startDate, int startYear, const QDate &endDate,
                const QString &name, const QString &shortName, const QString &format);
    QList<KCalendarEra> *eraList() const;
    KCalendarEra era(const QDate &eraDate) const;
    KCalendarEra era(const QString &eraName, int yearInEra) const;
    int shortYearWindowStartYear() const;
    int applyShortYearWindow(int inputYear) const;
    void loadShortYearWindowStartYear(const KConfigGroup & cg);
    KSharedConfig::Ptr config();
    void loadConfig(const QString & calendarType);

    // Week utility functions
    int isoWeekNumber(const QDate &date, int *yearNum) const;
    int regularWeekNumber(const QDate &date, int weekStartDay, int firstWeekNumber, int *weekYear) const;
    int simpleWeekNumber(const QDate &date, int *yearNum) const;
    int isoWeeksInYear(int year) const;
    int regularWeeksInYear(int year, int weekStartDay, int firstWeekNumber) const;
    int simpleWeeksInYear(int year) const;

    // Global variables each calendar system must initialise
    const KCalendarSystem *q;
    const KLocale *m_locale;
    KSharedConfig::Ptr m_config;
    QList<KCalendarEra> *m_eraList;
    int m_shortYearWindowStartYear;
};

#endif // KCALENDARSYSTEMPRIVATE_H
