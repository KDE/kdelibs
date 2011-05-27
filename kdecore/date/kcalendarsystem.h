/*
    Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
    Copyright (c) 2002-2003 Hans Petter Bieker <bieker@kde.org>
    Copyright 2007, 2009, 2010 John Layt <john@layt.net>

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

#ifndef KCALENDARSYSTEM_H
#define KCALENDARSYSTEM_H

#include <kdecore_export.h>
#include "klocale.h"  // needed for enums
#include "kglobal.h"

#include <QtCore/QStringList>
#include <QtCore/QDate>

class KCalendarSystemPrivate;
class KCalendarEra;

/**
 * KCalendarSystem abstract base class, provides support for local Calendar Systems in KDE
 *
 * Derived classes must be created through the create() static method
 */
class KDECORE_EXPORT KCalendarSystem
{
public:

    /**
     * Format for returned year number / month number / day number as string.
     */
    enum StringFormat {
        ShortFormat,      /**< Short string format, e.g. 2000 = "00" or 6 = "6" */
        LongFormat        /**< Long string format, e.g. 2000 = "2000" or 6 = "06" */
    };

    /**
     * Format for returned month / day name.
     */
    enum MonthNameFormat {
        ShortName,                /**< Short name format, e.g. "Dec" */
        LongName,                 /**< Long name format, e.g. "December" */
        ShortNamePossessive,      /**< Short name possessive format, e.g. "of Dec" */
        LongNamePossessive,       /**< Long name possessive format, e.g. "of December" */
        NarrowName                /**< Narrow name format, e.g. "D". @since 4.7 */
    };

    /**
     * Format for returned month / day name.
     */
    enum WeekDayNameFormat {
        ShortDayName,                /**< Short name format, e.g. "Fri" */
        LongDayName,                 /**< Long name format, e.g. "Friday" */
        NarrowDayName                /**< Narrow name format, e.g. "F". @since 4.7 */
    };

    //KDE5 remove
    /**
     * @deprecated use create(KLocale::CalendarSystem, KLocale) instead
     *
     * Creates specific calendar type
     *
     * @param calType string identification of the specific calendar type
     * to be constructed
     * @param locale locale to use for translations. The global locale is used if null.
     * @return a KCalendarSystem object
     */
    KDE_DEPRECATED static KCalendarSystem *create(const QString & calType = QLatin1String("gregorian"),
                                                  const KLocale * locale = 0);

    //KDE5 remove
    /**
     * @deprecated use create(KLocale::CalendarSystem, KSharedConfig, KLocale) instead
     *
     * @since 4.5
     *
     * Creates specific calendar type
     *
     * @param calType string identification of the specific calendar type to be constructed
     * @param config a configuration file with a 'KCalendarSystem %calendarType' group detailing
     *               locale-related preferences (such as era options).  The global config is used
                     if null.
     * @param locale locale to use for translations. The global locale is used if null.
     * @return a KCalendarSystem object
     */
    KDE_DEPRECATED static KCalendarSystem *create(const QString & calType, KSharedConfig::Ptr config,
                                                  const KLocale * locale = 0);

    //KDE5 add default value to calendarSystem
    /**
     * @since 4.6
     *
     * Creates a KCalendarSystem object for the required Calendar System
     *
     * @param calendarSystem the Calendar System to create, defaults to QDate compatible
     * @param locale locale to use for translations. The global locale is used if null.
     * @return a KCalendarSystem object
     */
    static KCalendarSystem *create(KLocale::CalendarSystem calendarSystem,
                                   const KLocale *locale = 0);

    /**
     * @since 4.6
     *
     * Creates a KCalendarSystem object for the required Calendar System
     *
     * @param calendarSystem the Calendar System to create
     * @param config a configuration file with a 'KCalendarSystem %calendarType' group detailing
     *               locale-related preferences (such as era options).  The global config is used
                     if null.
     * @param locale locale to use for translations. The global locale is used if null.
     * @return a KCalendarSystem object
     */
    static KCalendarSystem *create(KLocale::CalendarSystem calendarSystem, KSharedConfig::Ptr config,
                                   const KLocale *locale = 0);

    //KDE5 remove
    /**
     * @deprecated use calendarSystemsList() instead
     *
     * Gets a list of names of supported calendar systems.
     *
     * @return list of names
     */
    KDE_DEPRECATED static QStringList calendarSystems();

    /**
     * @since 4.6
     *
     * Returns the list of currently supported Calendar Systems
     *
     * @return list of Calendar Systems
     */
    static QList<KLocale::CalendarSystem> calendarSystemsList();

    //KDE5 remove
    /**
     * @deprecated use calendarLabel( KLocale::CalendarSystem ) instead
     *
     * Returns a typographically correct and translated label to display for
     * the calendar system type.  Use with calendarSystems() to neatly
     * format labels to display on combo widget of available calendar systems.
     *
     * @param calendarType the specific calendar type to return the label for
     *
     * @return label for calendar
     */
    KDE_DEPRECATED static QString calendarLabel(const QString &calendarType);

    /**
     * @since 4.6
     *
     * Returns a localized label to display for the required Calendar System type.
     *
     * Use with calendarSystemsList() to populate selection lists of available
     * calendar systems.
     *
     * @param calendarSystem the specific calendar type to return the label for
     * @param locale the locale to use for the label, defaults to global
     * @return label for calendar
     */
    static QString calendarLabel(KLocale::CalendarSystem calendarSystem, const KLocale *locale = KGlobal::locale());

    //KDE5 Remove
    /**
     * @deprecated use calendarSystem(const QString &calendarType) instead
     * @since 4.6
     *
     * Returns the Calendar System enum value for a given Calendar Type,
     * e.g. KLocale::QDateCalendar for "gregorian"
     *
     * @param calendarType the calendar type to convert
     * @return calendar system for calendar type
     */
    KDE_DEPRECATED static KLocale::CalendarSystem calendarSystemForCalendarType(const QString &calendarType);

    //KDE5 Remove
    /**
     * @since 4.7
     *
     * Returns the Calendar System enum value for a given Calendar Type,
     * e.g. KLocale::QDateCalendar for "gregorian"
     *
     * @param calendarType the calendar type to convert
     * @return calendar system for calendar type
     */
    static KLocale::CalendarSystem calendarSystem(const QString &calendarType);

    //KDE5 remove
    /**
     * @since 4.7
     *
     * Returns the deprecated Calendar Type for a given Calendar System enum value,
     * e.g. "gregorian" for KLocale::QDateCalendar
     *
     * @param calendarSystem the calendar system to convert
     * @return calendar type for calendar system
     */
    static QString calendarType(KLocale::CalendarSystem calendarSystem);

    /**
     * Constructor of abstract calendar class. This will be called by derived classes.
     *
     * @param locale locale to use for translations. The global locale is used if null.
     */
    explicit KCalendarSystem(const KLocale *locale = 0);

    /**
     * Constructor of abstract calendar class. This will be called by derived classes.
     *
     * @param config a configuration file with a 'KCalendarSystem %calendarName' group detailing
     *               locale-related preferences (such as era options).  The global config is used
                     if null.
     * @param locale locale to use for translations. The global locale is used if null.
     */
    explicit KCalendarSystem(const KSharedConfig::Ptr config, const KLocale *locale = 0);

    /**
     * Destructor.
     */
    virtual ~KCalendarSystem();

    /**
     * @deprecated use calendarSystem() instead
     *
     * Returns the calendar system type.
     *
     * @return type of calendar system
     */
    KDE_DEPRECATED virtual QString calendarType() const = 0;

    //KDE5 make virtual?
    /**
     * @since 4.6
     *
     * Returns the Calendar System type of the KCalendarSystem object
     *
     * @return type of calendar system
     */
    KLocale::CalendarSystem calendarSystem() const;

    //KDE5 make virtual?
    /**
     * @since 4.6
     *
     * Returns a localized label to display for the current Calendar System type.
     *
     * @return localized label for this Calendar System
     */
    QString calendarLabel() const;

    /**
     * Returns a QDate holding the epoch of the calendar system.  Usually YMD
     * of 1/1/1, access the returned QDates method toJulianDay() if you
     * require the actual Julian day number.  Note: a particular calendar
     * system implementation may not include the epoch in its supported range,
     * or the calendar system may be proleptic in which case it supports dates
     * before the epoch.
     *
     * @see KCalendarSystem::earliestValidDate
     * @see KCalendarSystem::latestValidDate
     * @see KCalendarSystem::isProleptic
     * @see KCalendarSystem::isValid
     *
     * @return epoch of calendar system
     */
    virtual QDate epoch() const;

    /**
     * Returns the earliest date valid in this calendar system implementation.
     *
     * If the calendar system is proleptic then this may be before epoch.
     *
     * @see KCalendarSystem::epoch
     * @see KCalendarSystem::latestValidDate
     *
     * @return date the earliest valid date
     */
    virtual QDate earliestValidDate() const;

    /**
     * Returns the latest date valid in this calendar system implementation.
     *
     * @see KCalendarSystem::epoch
     * @see KCalendarSystem::earliestValidDate
     *
     * @return date the latest valid date
     */
    virtual QDate latestValidDate() const;

    /**
     * Returns whether a given date is valid in this calendar system.
     *
     * @param year the year portion of the date to check
     * @param month the month portion of the date to check
     * @param day the day portion of the date to check
     * @return @c true if the date is valid, @c false otherwise
     */
    virtual bool isValid(int year, int month, int day) const = 0;

    //KDE5 make virtual?
    /**
     * @since 4.4
     *
     * Returns whether a given date is valid in this calendar system.
     *
     * @param year the year portion of the date to check
     * @param dayOfYear the day of year portion of the date to check
     * @return @c true if the date is valid, @c false otherwise
     */
    bool isValid(int year, int dayOfYear) const;

    //KDE5 make virtual?
    /**
     * @since 4.5
     *
     * Returns whether a given date is valid in this calendar system.
     *
     * @param eraName the Era Name portion of the date to check
     * @param yearInEra the Year In Era portion of the date to check
     * @param month the Month portion of the date to check
     * @param day the Day portion of the date to check
     * @return @c true if the date is valid, @c false otherwise
     */
    bool isValid(const QString &eraName, int yearInEra, int month, int day) const;

    //KDE5 make virtual?
    /**
     * @since 4.4
     *
     * Returns whether a given date is valid in this calendar system.
     *
     * @param year the year portion of the date to check
     * @param isoWeekNumber the ISO week portion of the date to check
     * @param dayOfIsoWeek the day of week portion of the date to check
     * @return @c true if the date is valid, @c false otherwise
     */
    bool isValidIsoWeekDate(int year, int isoWeekNumber, int dayOfIsoWeek) const;

    /**
     * Returns whether a given date is valid in this calendar system.
     *
     * @param date the date to check
     * @return @c true if the date is valid, @c false otherwise
     */
    virtual bool isValid(const QDate &date) const;

    /**
     * Changes the date's year, month and day. The range of the year, month
     * and day depends on which calendar is being used.  All years entered
     * are treated literally, i.e. no Y2K translation is applied to years
     * entered in the range 00 to 99.  Replaces setYMD.
     *
     * @param date date to change
     * @param year year
     * @param month month number
     * @param day day of month
     * @return @c true if the date is valid, @c false otherwise
     */
    virtual bool setDate(QDate &date, int year, int month, int day) const;

    //KDE5 make virtual?
    /**
     * @since 4.4
     *
     * Set a date using the year number and day of year number only.
     *
     * @param date date to change
     * @param year year
     * @param dayOfYear day of year
     * @return @c true if the date is valid, @c false otherwise
     */
    bool setDate(QDate &date, int year, int dayOfYear) const;

    //KDE5 make virtual?
    /**
     * @since 4.5
     *
     * Set a date using the era, year in era number, month and day
     *
     * @param date date to change
     * @param eraName Era string
     * @param yearInEra Year In Era number
     * @param month Month number
     * @param day Day Of Month number
     * @return @c true if the date is valid, @c false otherwise
     */
    bool setDate(QDate &date, QString eraName, int yearInEra, int month, int day) const;

    //KDE5 make virtual?
    /**
     * @since 4.4
     *
     * Set a date using the year number, ISO week number and day of week number.
     *
     * @param date date to change
     * @param year year
     * @param isoWeekNumber ISO week of year
     * @param dayOfIsoWeek day of week Mon..Sun (1..7)
     * @return @c true if the date is valid, @c false otherwise
     */
    bool setDateIsoWeek(QDate &date, int year, int isoWeekNumber, int dayOfIsoWeek) const;

    /**
     * @deprecated Use setDate() instead
     *
     * Some implementations reject year range 00 to 99, but extended date
     * ranges now require these to be accepted.  Equivalent in QDate is
     * obsoleted.
     *
     * Changes the date's year, month and day. The range of the year, month
     * and day depends on which calendar is being used.
     *
     * @param date Date to change
     * @param y Year
     * @param m Month number
     * @param d Day of month
     * @return true if the date is valid; otherwise returns false.
     */
    KDE_DEPRECATED virtual bool setYMD(QDate &date, int y, int m, int d) const;

    //KDE5 make virtual?
    /**
     * @since 4.5
     *
     * Returns the year, month and day portion of a given date in the current calendar system
     *
     * @param date date to get year, month and day for
     * @param year year number returned in this variable
     * @param month month number returned in this variable
     * @param day day of month returned in this variable
     */
    void getDate(const QDate date, int *year, int *month, int *day) const;

    /**
     * Returns the year portion of a given date in the current calendar system
     *
     * @param date date to return year for
     * @return year, 0 if input date is invalid
     */
    virtual int year(const QDate &date) const;

    /**
     * Returns the month portion of a given date in the current calendar system
     *
     * @param date date to return month for
     * @return month of year, 0 if input date is invalid
     */
    virtual int month(const QDate &date) const;

    /**
     * Returns the day portion of a given date in the current calendar system
     *
     * @param date date to return day for
     * @return day of the month, 0 if input date is invalid
     */
    virtual int day(const QDate &date) const;

    //KDE5 make virtual?
    /**
     * @since 4.5
     *
     * Returns the Era Name portion of a given date in the current calendar system,
     * for example "AD" or "Anno Domini" for the Gregorian calendar and Christian Era.
     *
     * @param date date to return Era Name for
     * @param format format to return, either short or long
     * @return era name, empty string if input date is invalid
     */
    QString eraName(const QDate &date, StringFormat format = ShortFormat) const;

    //KDE5 make virtual?
    /**
     * @since 4.5
     *
     * Returns the Era Year portion of a given date in the current
     * calendar system, for example "2000 AD" or "Heisei 22".
     *
     * @param date date to return Era Year for
     * @param format format to return, either short or long
     * @return era name, empty string if input date is invalid
     */
    QString eraYear(const QDate &date, StringFormat format = ShortFormat) const;

    //KDE5 make virtual?
    /**
     * @since 4.5
     *
     * Returns the Year In Era portion of a given date in the current calendar
     * system, for example 1 for "1 BC".
     *
     * @param date date to return Year In Era for
     * @return Year In Era, -1 if input date is invalid
     */
    int yearInEra(const QDate &date) const;

    /**
     * Returns a QDate containing a date @p nyears years later.
     *
     * @param date The old date
     * @param nyears The number of years to add
     * @return The new date, null date if any errors
     */
    virtual QDate addYears(const QDate &date, int nyears) const;

    /**
     * Returns a QDate containing a date @p nmonths months later.
     *
     * @param date The old date
     * @param nmonths number of months to add
     * @return The new date, null date if any errors
     */
    virtual QDate addMonths(const QDate &date, int nmonths) const;

    /**
     * Returns a QDate containing a date @p ndays days later.
     *
     * @param date The old date
     * @param ndays number of days to add
     * @return The new date, null date if any errors
     */
    virtual QDate addDays(const QDate &date, int ndays) const;

    //KDE5 make virtual?
    /**
     * Returns the difference between two dates in years, months and days.
     * The difference is always caculated from the earlier date to the later
     * date in year, month and day order, with the @p direction parameter
     * indicating which direction the difference is applied from the @p toDate.
     *
     * For example, the difference between 2010-06-10 and 2012-09-5 is 2 years,
     * 2 months and 26 days.  Note that the difference between two last days of
     * the month is always 1 month, e.g. 2010-01-31 to 2010-02-28 is 1 month
     * not 28 days.
     *
     * @param fromDate The date to start from
     * @param toDate The date to end at
     * @param yearsDiff Returns number of years difference
     * @param monthsDiff Returns number of months difference
     * @param daysDiff Returns number of days difference
     * @param direction Returns direction of difference, 1 if fromDate <= toDate, -1 otherwise
     */
    void dateDifference(const QDate &fromDate, const QDate &toDate,
                        int *yearsDiff, int *monthsDiff, int *daysDiff, int *direction) const;

    //KDE5 make virtual?
    /**
    * Returns the difference between two dates in completed calendar years.
    * The returned value will be negative if @p fromDate > @p toDate.
    *
    * For example, the difference between 2010-06-10 and 2012-09-5 is 2 years.
    *
    * @param fromDate The date to start from
    * @param toDate The date to end at
    * @return The number of years difference
    */
    int yearsDifference(const QDate &fromDate, const QDate &toDate) const;

    //KDE5 make virtual?
    /**
     * Returns the difference between two dates in completed calendar months
     * The returned value will be negative if @p fromDate > @p toDate.
     *
     * For example, the difference between 2010-06-10 and 2012-09-5 is 26 months.
     * Note that the difference between two last days of the month is always 1
     * month, e.g. 2010-01-31 to 2010-02-28 is 1 month not 28 days.
     *
     * @param fromDate The date to start from
     * @param toDate The date to end at
     * @return The number of months difference
     */
    int monthsDifference(const QDate &fromDate, const QDate &toDate) const;

    //KDE5 make virtual?
    /**
     * Returns the difference between two dates in days
     * The returned value will be negative if @p fromDate > @p toDate.
     *
     * @param fromDate The date to start from
     * @param toDate The date to end at
     * @return The number of days difference
     */
    int daysDifference(const QDate &fromDate, const QDate &toDate) const;

    /**
     * Returns number of months in the given year
     *
     * @param date the date to obtain year from
     * @return number of months in the year, -1 if input date invalid
     */
    virtual int monthsInYear(const QDate &date) const;

    //KDE5 make virtual?
    /**
     * @since 4.5
     *
     * Returns number of months in the given year
     *
     * @param year the required year
     * @return number of months in the year, -1 if input date invalid
     */
    int monthsInYear(int year) const;

    /**
     * Returns the number of localized weeks in the given year.
     *
     * @param date the date to obtain year from
     * @return number of weeks in the year, -1 if input date invalid
     */
    virtual int weeksInYear(const QDate &date) const;

    //KDE5 Merge with virtual weeksInYear with default
    /**
     * @since 4.7
     *
     * Returns the number of Weeks in a year using the required Week Number System.
     *
     * Unless you specifically want a particular Week Number System (e.g. ISO Weeks)
     * you should use the localized number of weeks provided by weeksInYear().
     *
     * @see week()
     * @see formatDate()
     * @param date the date to obtain year from
     * @param weekNumberSystem the week number system to use
     * @return number of weeks in the year, -1 if  date invalid
     */
    int weeksInYear(const QDate &date, KLocale::WeekNumberSystem weekNumberSystem) const;

    /**
     * Returns the number of localized weeks in the given year.
     *
     * @param year the year
     * @return number of weeks in the year, -1 if input date invalid
     */
    virtual int weeksInYear(int year) const;

    //KDE5 Merge with virtual weeksInYear with default
    /**
     * @since 4.7
     *
     * Returns the number of Weeks in a year using the required Week Number System.
     *
     * Unless you specifically want a particular Week Number System (e.g. ISO Weeks)
     * you should use the localized number of weeks provided by weeksInYear().
     *
     * @see week()
     * @see formatDate()
     * @param year the year
     * @param weekNumberSystem the week number system to use
     * @return number of weeks in the year, -1 if  date invalid
     */
    int weeksInYear(int year, KLocale::WeekNumberSystem weekNumberSystem) const;

    /**
     * Returns the number of days in the given year.
     *
     * @param date the date to obtain year from
     * @return number of days in year, -1 if input date invalid
     */
    virtual int daysInYear(const QDate &date) const;

    //KDE5 make virtual?
    /**
     * @since 4.5
     *
     * Returns the number of days in the given year.
     *
     * @param year the year
     * @return number of days in year, -1 if input date invalid
     */
    int daysInYear(int year) const;

    /**
     * Returns the number of days in the given month.
     *
     * @param date the date to obtain month from
     * @return number of days in month, -1 if input date invalid
     */
    virtual int daysInMonth(const QDate &date) const;

    //KDE5 make virtual?
    /**
     * @since 4.5
     *
     * Returns the number of days in the given month.
     *
     * @param year the year the month is in
     * @param month the month
     * @return number of days in month, -1 if input date invalid
     */
    int daysInMonth(int year, int month) const;

    /**
     * Returns the number of days in the given week.
     *
     * @param date the date to obtain week from
     * @return number of days in week, -1 if input date invalid
     */
    virtual int daysInWeek(const QDate &date) const;

    /**
     * Returns the day number of year for the given date
     *
     * The days are numbered 1..daysInYear()
     *
     * @param date the date to obtain day from
     * @return day of year number, -1 if input date not valid
     */
    virtual int dayOfYear(const QDate &date) const;

    /**
     * Returns the weekday number for the given date
     *
     * The weekdays are numbered 1..7 for Monday..Sunday.
     *
     * This value is @em not affected by the value of weekStartDay()
     *
     * @param date the date to obtain day from
     * @return day of week number, -1 if input date not valid
     */
    virtual int dayOfWeek(const QDate &date) const;

    /**
     * @deprecated use week() instead
     *
     * Returns the ISO week number for the given date.
     *
     * ISO 8601 defines the first week of the year as the week containing the first Thursday.
     * See http://en.wikipedia.org/wiki/ISO_8601 and http://en.wikipedia.org/wiki/ISO_week_date
     *
     * If the date falls in the last week of the previous year or the first week of the following
     * year, then the yearNum returned will be set to the appropriate year.
     *
     * @param date the date to obtain week from
     * @param yearNum returns the year the date belongs to
     * @return ISO week number, -1 if input date invalid
     */
    KDE_DEPRECATED virtual int weekNumber(const QDate &date, int *yearNum = 0) const;

    //KDE5 Make virtual?
    /**
     * Returns the localized Week Number for the date.
     *
     * This may be ISO, US, or any other supported week numbering scheme.  If
     * you specifically require the ISO Week or any other scheme, you should use
     * the week(KLocale::WeekNumberSystem) form.
     *
     * If the date falls in the last week of the previous year or the first
     * week of the following year, then the yearNum returned will be set to the
     * appropriate year.
     *
     * @see weeksInYear()
     * @see formatDate()
     * @param date the date to obtain week from
     * @param yearNum returns the year the date belongs to
     * @return localized week number, -1 if input date invalid
     */
    int week(const QDate &date, int *yearNum = 0) const;

    //KDE5 Make virtual?
    /**
     * Returns the Week Number for the date in the required Week Number System.
     *
     * Unless you want a specific Week Number System (e.g. ISO Week), you should
     * use the localized Week Number form of week().
     *
     * If the date falls in the last week of the previous year or the first
     * week of the following year, then the yearNum returned will be set to the
     * appropriate year.
     *
     * Technically, the ISO Week Number only applies to the ISO/Gregorian Calendar
     * System, but the same rules will be applied to the current Calendar System.
     *
     * @see weeksInYear()
     * @see formatDate()
     * @param date the date to obtain week from
     * @param weekNumberSystem the Week Number System to use
     * @param yearNum returns the year the date belongs to
     * @return week number, -1 if input date invalid
     */
    int week(const QDate &date, KLocale::WeekNumberSystem weekNumberSystem, int *yearNum = 0) const;

    /**
     * Returns whether a given year is a leap year.
     *
     * Input year must be checked for validity in current Calendar System prior to calling, no
     * validity checking performed in this routine, behaviour is undefined in invalid case.
     *
     * @param year the year to check
     * @return @c true if the year is a leap year, @c false otherwise
     */
    virtual bool isLeapYear(int year) const = 0;

    /**
     * Returns whether a given date falls in a leap year.
     *
     * Input date must be checked for validity in current Calendar System prior to calling, no
     * validity checking performed in this routine, behaviour is undefined in invalid case.
     *
     * @param date the date to check
     * @return @c true if the date falls in a leap year, @c false otherwise
     */
    virtual bool isLeapYear(const QDate &date) const;

    //KDE5 Make virtual?
    /**
     * @since 4.6
     *
     * Returns a QDate containing the first day of the year
     *
     * @param year The year to return the date for
     * @return The first day of the year
     */
    QDate firstDayOfYear(int year) const;

    //KDE5 Make virtual?
    /**
     * @since 4.6
     *
     * Returns a QDate containing the last day of the year
     *
     * @param year The year to return the date for
     * @return The last day of the year
     */
    QDate lastDayOfYear(int year) const;

    //KDE5 Make virtual?
    /**
     * @since 4.6
     *
     * Returns a QDate containing the first day of the year
     *
     * @param date The year to return the date for, defaults to today
     * @return The first day of the year
     */
    QDate firstDayOfYear(const QDate &date = QDate::currentDate()) const;

    //KDE5 Make virtual?
    /**
     * @since 4.6
     *
     * Returns a QDate containing the last day of the year
     *
     * @param date The year to return the date for, defaults to today
     * @return The last day of the year
     */
    QDate lastDayOfYear(const QDate &date = QDate::currentDate()) const;

    //KDE5 Make virtual?
    /**
     * @since 4.6
     *
     * Returns a QDate containing the first day of the month
     *
     * @param year The year to return the date for
     * @param month The month to return the date for
     * @return The first day of the month
     */
    QDate firstDayOfMonth(int year, int month) const;

    //KDE5 Make virtual?
    /**
     * @since 4.6
     *
     * Returns a QDate containing the last day of the month
     *
     * @param year The year to return the date for
     * @param month The month to return the date for
     * @return The last day of the month
     */
    QDate lastDayOfMonth(int year, int month) const;

    //KDE5 Make virtual?
    /**
     * @since 4.6
     *
     * Returns a QDate containing the first day of the month
     *
     * @param date The month to return the date for, defaults to today
     * @return The first day of the month
     */
    QDate firstDayOfMonth(const QDate &date = QDate::currentDate()) const;

    //KDE5 Make virtual?
    /**
     * @since 4.6
     *
     * Returns a QDate containing the last day of the month
     *
     * @param date The month to return the date for, defaults to today
     * @return The last day of the month
     */
    QDate lastDayOfMonth(const QDate &date = QDate::currentDate()) const;

    /**
     * Gets specific calendar type month name for a given month number
     * If an invalid month is specified, QString() is returned.
     *
     * @param month the month number
     * @param year the year the month belongs to
     * @param format specifies whether the short month name or long month name should be used
     * @return name of the month, empty string if any error
     */
    virtual QString monthName(int month, int year, MonthNameFormat format = LongName) const = 0;

    /**
     * Gets specific calendar type month name for a given date
     *
     * @param date date to obtain month from
     * @param format specifies whether the short month name or long month name should be used
     * @return name of the month, empty string if any error
     */
    virtual QString monthName(const QDate &date, MonthNameFormat format = LongName) const;

    /**
     * Gets specific calendar type week day name.
     * If an invalid week day is specified, QString() is returned.
     *
     * @param weekDay number of day in week (Monday = 1, ..., Sunday = 7)
     * @param format specifies whether the short month name or long month name should be used
     * @return day name, empty string if any error
     */
    virtual QString weekDayName(int weekDay, WeekDayNameFormat format = LongDayName) const = 0;

    /**
     * Gets specific calendar type week day name.
     *
     * @param date the date
     * @param format specifies whether the short month name or long month name should be used
     * @return day name, empty string if any error
     */
    virtual QString weekDayName(const QDate &date, WeekDayNameFormat format = LongDayName) const;

    /**
     * @deprecated use formatDate(QDate, KLocale::DateTimeComponant, KLocale::DateTimeComponentFormat)
     *
     * Converts a date into a year literal
     *
     * @param date date to convert
     * @param format format to return, either short or long
     * @return year literal of the date, empty string if any error
     * @see year()
     */
    KDE_DEPRECATED virtual QString yearString(const QDate &date, StringFormat format = LongFormat) const;

    /**
     * @deprecated use formatDate(QDate, KLocale::DateTimeComponant, KLocale::DateTimeComponentFormat)
     *
     * Converts a date into a month literal
     *
     * @param pDate The date to convert
     * @param format The format to return, either short or long
     * @return The month literal of the date, empty string if any error
     * @see month()
     */
    KDE_DEPRECATED virtual QString monthString(const QDate &pDate, StringFormat format = LongFormat) const;

    /**
     * @deprecated use formatDate(QDate, KLocale::DateTimeComponant, KLocale::DateTimeComponentFormat)
     *
     * Converts a date into a day literal
     *
     * @param pDate The date to convert
     * @param format The format to return, either short or long
     * @return The day literal of the date, empty string if any error
     * @see day()
     */
    KDE_DEPRECATED virtual QString dayString(const QDate &pDate, StringFormat format = LongFormat) const;

    //KDE5 make virtual?
    /**
     * @deprecated use formatDate(QDate, KLocale::DateTimeComponant, KLocale::DateTimeComponentFormat)
     *
     * @since 4.5
     *
     * Converts a date into a Year In Era literal
     *
     * @param date date to return Year In Era for
     * @param format format to return, either short or long
     * @return Year In Era literal of the date, empty string if any error
     */
    KDE_DEPRECATED QString yearInEraString(const QDate &date, StringFormat format = ShortFormat) const;

    //KDE5 make virtual?
    /**
     * @deprecated use formatDate(QDate, KLocale::DateTimeComponant, KLocale::DateTimeComponentFormat)
     *
     * @since 4.4
     *
     * Converts a date into a day of year literal
     *
     * @param pDate The date to convert
     * @param format The format to return, either short or long
     * @return The day of year literal of the date, empty string if any error
     * @see dayOfYear()
     */
    KDE_DEPRECATED QString dayOfYearString(const QDate &pDate, StringFormat format = LongFormat) const;

    //KDE5 make virtual?
    /**
     * @deprecated use formatDate(QDate, KLocale::DateTimeComponant, KLocale::DateTimeComponentFormat)
     *
     * @since 4.4
     *
     * Converts a date into a day of week literal
     *
     * @param pDate The date to convert
     * @return The day of week literal of the date, empty string if any error
     * @see dayOfWeek()
     */
    KDE_DEPRECATED QString dayOfWeekString(const QDate &pDate) const;

    //KDE5 make virtual?
    /**
     * @deprecated use formatDate(QDate, KLocale::DateTimeComponant, KLocale::DateTimeComponentFormat)
     *
     * @since 4.4
     *
     * Converts a date into a week number literal
     *
     * @param pDate The date to convert
     * @param format The format to return, either short or long
     * @return The day literal of the date, empty string if any error
     * @see weekNumber()
     */
    KDE_DEPRECATED QString weekNumberString(const QDate &pDate, StringFormat format = LongFormat) const;

    //KDE5 make virtual?
    /**
     * @deprecated use formatDate(QDate, KLocale::DateTimeComponant, KLocale::DateTimeComponentFormat)
     *
     * @since 4.4
     *
     * Returns the months in year for a date as a numeric string
     *
     * @param pDate The date to convert
     * @param format The format to return, either short or long
     * @return The months in year literal of the date, empty string if any error
     * @see monthsInYear()
     */
    KDE_DEPRECATED QString monthsInYearString(const QDate &pDate, StringFormat format = LongFormat) const;

    //KDE5 make virtual?
    /**
     * @deprecated use formatDate(QDate, KLocale::DateTimeComponant, KLocale::DateTimeComponentFormat)
     *
     * @since 4.4
     *
     * Returns the weeks in year for a date as a numeric string
     *
     * @param pDate The date to convert
     * @param format The format to return, either short or long
     * @return The weeks in year literal of the date, empty string if any error
     * @see weeksInYear()
     */
    KDE_DEPRECATED QString weeksInYearString(const QDate &pDate, StringFormat format = LongFormat) const;

    //KDE5 make virtual?
    /**
     * @deprecated use formatDate(QDate, KLocale::DateTimeComponant, KLocale::DateTimeComponentFormat)
     *
     * @since 4.4
     *
     * Returns the days in year for a date as a numeric string
     *
     * @param pDate The date to convert
     * @param format The format to return, either short or long
     * @return The days in year literal of the date, empty string if any error
     * @see daysInYear()
     */
    KDE_DEPRECATED QString daysInYearString(const QDate &pDate, StringFormat format = LongFormat) const;

    //KDE5 make virtual?
    /**
     * @deprecated use formatDate(QDate, KLocale::DateTimeComponant, KLocale::DateTimeComponentFormat)
     *
     * @since 4.4
     *
     * Returns the days in month for a date as a numeric string
     *
     * @param pDate The date to convert
     * @param format The format to return, either short or long
     * @return The days in month literal of the date, empty string if any error
     * @see daysInMonth()
     */
    KDE_DEPRECATED QString daysInMonthString(const QDate &pDate, StringFormat format = LongFormat) const;

    //KDE5 make virtual?
    /**
     * @deprecated use formatDate(QDate, KLocale::DateTimeComponant, KLocale::DateTimeComponentFormat)
     *
     * @since 4.4
     *
     * Returns the days in week for a date as a numeric string
     *
     * @param date The date to convert
     * @return The days in week literal of the date, empty string if any error
     * @see daysInWeek()
     */
    KDE_DEPRECATED QString daysInWeekString(const QDate &date) const;

    //KDE5 make protected or remove?
    /**
     * @deprecated for internal use only
     *
     * Converts a year literal of a part of a string into a integer starting at the beginning of the string
     *
     * @param sNum The string to parse
     * @param iLength The number of QChars used, and 0 if no valid symbols was found in the string
     * @return An integer corresponding to the year
     */
    virtual int yearStringToInteger(const QString &sNum, int &iLength) const;

    //KDE5 make protected or remove?
    /**
     * @deprecated for internal use only
     *
     * Converts a month literal of a part of a string into a integer starting at the beginning of the string
     *
     * @param sNum The string to parse
     * @param iLength The number of QChars used, and 0 if no valid symbols was found in the string
     * @return An integer corresponding to the month
     */
    virtual int monthStringToInteger(const QString &sNum, int &iLength) const;

    //KDE5 make protected or remove?
    /**
     * @deprecated for internal use only
     *
     * Converts a day literal of a part of a string into a integer starting at the beginning of the string
     *
     * @param sNum The string to parse
     * @param iLength The number of QChars used, and 0 if no valid symbols was found in the string
     * @return An integer corresponding to the day
     */
    virtual int dayStringToInteger(const QString &sNum, int &iLength) const;

    /**
     * Returns a string formatted to the current locale's conventions
     * regarding dates.
     *
     * Uses the calendar system's internal locale set when the instance was
     * created, which ensures that the correct calendar system and locale
     * settings are respected, which would not occur in some cases if using
     * the global locale.  Defaults to global locale.
     *
     * @see KLocale::formatDate
     *
     * @param fromDate the date to be formatted
     * @param toFormat category of date format to use
     *
     * @return The date as a string
     */
    virtual QString formatDate(const QDate &fromDate, KLocale::DateFormat toFormat = KLocale::LongDate) const;

    //KDE5 Make virtual
    /**
     * @since 4.4
     *
     * Returns a string formatted to the given format and localised to the
     * correct language and digit set using the requested format standard.
     *
     *        *** WITH GREAT POWER COMES GREAT RESPONSIBILITY ***
     * Please use with care and only in situations where the DateFormat enum
     * or locale formats or individual string methods do not provide what you
     * need.  You should almost always translate your format string as
     * documented.  Using the standard DateFormat options instead would take
     * care of the translation for you.
     *
     * Warning: The %n element differs from the GNU/POSIX standard where it is
     * defined as a newline.  KDE currently uses this for short day number.  It
     * is recommended for compatibility purposes to use %-m instead.
     *
     * The toFormat parameter is a good candidate to be made translatable,
     * so that translators can adapt it to their language's convention.
     * There should also be a context using the "kdedt-format" keyword (for
     * automatic validation of translations) and stating the format's purpose:
     * \code
     * QDate reportDate;
     * KGlobal::locale()->calendar()->setDate(reportDate, reportYear, reportMonth, 1);
     * dateFormat = i18nc("(kdedt-format) Report month and year in report header", "%B %Y"));
     * dateString = KGlobal::locale()->calendar()->formatDate(reportDate, dateFormat);
     * \endcode
     *
     * The date format string can be defined using either the KDE or POSIX standards.
     * The KDE standard closely follows the POSIX standard but with some exceptions.
     * Always use the KDE standard within KDE, but where interaction is required with
     * external POSIX compliant systems (e.g. Gnome, glibc, etc) the POSIX standard
     * should be used.
     *
     * Date format strings are made up of date componants and string literals.
     * Date componants are prefixed by a % escape character and are made up of
     * optional padding and case modifier flags, an optional width value, and a
     * compulsary code for the actual date componant:
     *   %[Flags][Width][Componant]
     * e.g. %_^5Y
     * No spaces are allowed.
     *
     * The Flags can modify the padding character and/or case of the Date Componant.
     * The Flags are optional and may be combined and/or repeated in any order,
     * in which case the last Padding Flag and last Case Flag will be the
     * ones used.  The Flags must be immediately after the % and before any Width.
     *
     * The Width can modify how wide the date Componant is padded to.  The Width
     * is an optional interger value and must be after any Flags but before the
     * Componant.  If the Width is less than the minimum defined for a Componant
     * then the default minimum will be used instead.
     *
     * By default most numeric Date Componants are right-aligned with leading 0's.
     *
     * By default all string name fields are capital case and unpadded.
     *
     * The following Flags may be specified:
     * @li - (hyphen) no padding (e.g. 1 Jan and "%-j" = "1")
     * @li _ (underscore) pad with spaces (e.g. 1 Jan and "%-j" = "  1")
     * @li 0 (zero) pad with 0's  (e.g. 1 Jan and "%0j" = "001")
     * @li ^ (caret) make uppercase (e.g. 1 Jan and "%^B" = "JANUARY")
     * @li # (hash) invert case (e.g. 1 Jan and "%#B" = "???")
     *
     * The following Date Componants can be specified:
     * @li %Y the year to 4 digits (e.g. "1984" for 1984, "0584" for 584, "0084" for 84)
     * @li %C the 'century' portion of the year to 2 digits (e.g. "19" for 1984, "05" for 584, "00" for 84)
     * @li %y the lower 2 digits of the year to 2 digits (e.g. "84" for 1984, "05" for 2005)
     * @li %EY the full local era year (e.g. "2000 AD")
     * @li %EC the era name short form (e.g. "AD")
     * @li %Ey the year in era to 1 digit (e.g. 1 or 2000)
     * @li %m the month number to 2 digits (January="01", December="12")
     * @li %n the month number to 1 digit (January="1", December="12"), see notes!
     * @li %d the day number of the month to 2 digits (e.g. "01" on the first of March)
     * @li %e the day number of the month to 1 digit (e.g. "1" on the first of March)
     * @li %B the month name long form (e.g. "January")
     * @li %b the month name short form (e.g. "Jan" for January)
     * @li %h the month name short form (e.g. "Jan" for January)
     * @li %A the weekday name long form (e.g. "Wednesday" for Wednesday)
     * @li %a the weekday name short form (e.g. "Wed" for Wednesday)
     * @li %j the day of the year number to 3 digits (e.g. "001"  for 1 Jan)
     * @li %V the ISO week of the year number to 2 digits (e.g. "01"  for ISO Week 1)
     * @li %G the year number in long form of the ISO week of the year to 4 digits (e.g. "2004"  for 1 Jan 2005)
     * @li %g the year number in short form of the ISO week of the year to 2 digits (e.g. "04"  for 1 Jan 2005)
     * @li %u the day of the week number to 1 digit (e.g. "1"  for Monday)
     * @li %D the US short date format (e.g. "%m/%d/%y")
     * @li %F the ISO short date format (e.g. "%Y-%m-%d")
     * @li %x the KDE locale short date format
     * @li %% the literal "%"
     * @li %t a tab character
     *
     * Everything else in the format string will be taken as literal text.
     *
     * Examples:
     * "%Y-%m-%d" = "2009-01-01"
     * "%Y-%-m-%_4d" = "2009-1-   1"
     *
     * The following format codes behave differently in the KDE and POSIX standards
     * @li %e in GNU/POSIX is space padded to 2 digits, in KDE is not padded
     * @li %n in GNU/POSIX is newline, in KDE is short month number
     *
     * The following POSIX format codes are currently not supported:
     * @li %U US week number
     * @li %w US day of week
     * @li %W US week number
     * @li %O locale's alternative numeric symbols, in KDE is not supported
     *
     * %0 is not supported as the returned result is always in the locale's chosen numeric symbol digit set.
     *
     * @see KLocale::formatDate
     *
     * @param fromDate the date to be formatted
     * @param toFormat the date format to use
     * @param formatStandard the standard the date format uses, defaults to KDE Standard
     *
     * @return The date as a string
     */
    QString formatDate(const QDate &fromDate, const QString &toFormat,
                       KLocale::DateTimeFormatStandard formatStandard = KLocale::KdeFormat) const;

    //KDE5 Make virtual
    /**
     * @since 4.4
     *
     * Returns a string formatted to the given format string and Digit Set.
     * Only use this version if you need control over the Digit Set and do
     * not want to use the locale Digit Set.
     *
     * @see formatDate
     *
     * @param fromDate the date to be formatted
     * @param toFormat the date format to use
     * @param digitSet the Digit Set to format the date in
     * @param formatStandard the standard the date format uses, defaults to KDE Standard
     *
     * @return The date as a string
     */
    QString formatDate(const QDate &fromDate, const QString &toFormat, KLocale::DigitSet digitSet,
                       KLocale::DateTimeFormatStandard formatStandard = KLocale::KdeFormat) const;

    //KDE5 Make virtual
    /**
     * @since 4.6
     *
     * Returns a Date Component as a localized string in the requested format.
     *
     * For example for 2010-01-01 the KLocale::Month with en_US Locale and Gregorian calendar may return:
     *   KLocale::ShortNumber = "1"
     *   KLocale::LongNumber  = "01"
     *   KLocale::NarrowName  = "J"
     *   KLocale::ShortName   = "Jan"
     *   KLocale::LongName    = "January"
     *
     * @param date The date to format
     * @param component The date component to return
     * @param format The format to return the @p component in
     * @param weekNumberSystem To override the default Week Number System to use
     * @return The localized string form of the date component
     */
    QString formatDate(const QDate &date, KLocale::DateTimeComponent component,
                       KLocale::DateTimeComponentFormat format = KLocale::DefaultComponentFormat,
                       KLocale::WeekNumberSystem weekNumberSystem = KLocale::DefaultWeekNumber) const;

    /**
     * Converts a localized date string to a QDate.
     * The bool pointed by @p ok will be @c false if the date entered was invalid.
     *
     * Uses the calendar system's internal locale set when the instance was
     * created, which ensures that the correct calendar system and locale
     * settings are respected, which would not occur in some cases if using
     * the global locale.  Defaults to global locale.
     *
     * @see KLocale::readDate
     *
     * @param str the string to convert
     * @param ok if non-null, will be set to @c true if the date is valid, @c false if invalid
     *
     * @return the string converted to a QDate
     */
    virtual QDate readDate(const QString &str, bool *ok = 0) const;

    /**
     * Converts a localized date string to a QDate.
     * This method is stricter than readDate(str,&ok): it will either accept
     * a date in full format or a date in short format, depending on @p flags.
     *
     * Uses the calendar system's internal locale set when the instance was
     * created, which ensures that the correct calendar system and locale
     * settings are respected, which would not occur in some cases if using
     * the global locale.  Defaults to global locale.
     *
     * @see KLocale::readDate
     *
     * @param str the string to convert
     * @param flags whether the date string is to be in full format or in short format
     * @param ok if non-null, will be set to @c true if the date is valid, @c false if invalid
     *
     * @return the string converted to a QDate
     */
    virtual QDate readDate(const QString &str, KLocale::ReadDateFlags flags, bool *ok = 0) const;

    /**
     * Converts a localized date string to a QDate, using the specified @p format.
     * You will usually not want to use this method.  Uses teh KDE format standard.
     *
     * @param dateString the string to convert
     * @param dateFormat the date format to use, in KDE format standard
     * @param ok if non-null, will be set to @c true if the date is valid, @c false if invalid
     *
     * @return the string converted to a QDate
     *
     * @see formatDate
     * @see KLocale::readDate
     */
    virtual QDate readDate(const QString &dateString, const QString &dateFormat, bool *ok = 0) const;

    //KDE5 Make virtual
    /**
     * Converts a localized date string to a QDate, using the specified @p format.
     * You will usually not want to use this method.
     *
     * You must supply a format and string containing at least one of the following combinations to
     * create a valid date:
     * @li a month and day of month
     * @li a day of year
     * @li a ISO week number and day of week
     *
     * If a year number is not supplied then the current year will be assumed.
     *
     * All date componants must be separated by a non-numeric character.
     *
     * The format is not applied strictly to the input string:
     * @li extra whitespace is ignored
     * @li leading 0's on numbers are ignored
     * @li capitalisation of literals is ignored
     *
     * The allowed format componants are almost the same as the formatDate() function.
     * The following date componants will be read:
     * @li %Y the whole year (e.g. "1984" for 1984)
     * @li %y the lower 2 digits of the year (e.g. "84" for 1984)
     * @li %EY the full local era year (e.g. "2000 AD")
     * @li %EC the era name short form (e.g. "AD")
     * @li %Ey the year in era to 1 digit (e.g. 1 or 2000)
     * @li %m the month number to two digits (January="01", December="12")
     * @li %n the month number (January="1", December="12")
     * @li %d the day number of the month to two digits (e.g. "01" on the first of March)
     * @li %e the day number of the month (e.g. "1" on the first of March)
     * @li %B the month name long form (e.g. "January")
     * @li %b the month name short form (e.g. "Jan" for January)
     * @li %h the month name short form (e.g. "Jan" for January)
     * @li %A the weekday name long form (e.g. "Wednesday" for Wednesday)
     * @li %a the weekday name short form (e.g. "Wed" for Wednesday)
     * @li %j the day of the year number to three digits (e.g. "001"  for 1 Jan)
     * @li %V the ISO week of the year number to two digits (e.g. "01"  for ISO Week 1)
     * @li %u the day of the week number (e.g. "1"  for Monday)
     *
     * The following date componants are NOT supported:
     * @li %C the 'century' portion of the year (e.g. "19" for 1984, "5" for 584, "" for 84)
     * @li %G the year number in long form of the ISO week of the year (e.g. "2004"  for 1 Jan 2005)
     * @li %g the year number in short form of the ISO week of the year (e.g. "04"  for 1 Jan 2005)
     * @li %D the US short date format (e.g. "%m/%d/%y")
     * @li %F the ISO short date format (e.g. "%Y-%m-%d")
     * @li %x the KDE locale short date format
     * @li %% the literal "%"
     * @li %t a tab character
     *
     * @param dateString the string to convert
     * @param dateFormat the date format to use
     * @param ok if non-null, will be set to @c true if the date is valid, @c false if invalid
     * @param formatStandard the standard the date format uses
     *
     * @return the string converted to a QDate
     *
     * @see formatDate
     * @see KLocale::readDate
     */
    QDate readDate(const QString &dateString, const QString &dateFormat, bool *ok,
                   KLocale::DateTimeFormatStandard formatStandard) const;

    //KDE5 Make virtual
    /**
     * @since 4.6
     *
     * Returns the Short Year Window Start Year for the current Calendar System.
     *
     * Use this function to get the Start Year for the Short Year Window to be
     * applied when 2 digit years are entered for a Short Year input format,
     * e.g. if the Short Year Window Start Year is 1930, then the input Short
     * Year value of 40 is interpreted as 1940 and the input Short Year value
     * of 10 is interpreted as 2010.
     *
     * The Short Year Window is only ever applied when reading the Short Year
     * format and not the Long Year format, i.e. KLocale::ShortFormat or '%y'
     * only and not KLocale::LongFormat or '%Y'.
     *
     * The Start Year 0 effectively means not to use a Short Year Window
     *
     * Each Calendar System requires a different Short Year Window as they have
     * different epochs. The Gregorian Short Year Window usually pivots around
     * the year 2000, whereas the Hebrew Short Year Window usually pivots around
     * the year 5000.
     *
     * This value must always be used when evaluating user input Short Year
     * strings.
     *
     * @see KLocale::shortYearWindowStartYear
     * @see KLocale::applyShortYearWindow
     * @return the short year window start year
     */
    int shortYearWindowStartYear() const;

    //KDE5 Make virtual
    /**
     * @since 4.6
     *
     * Returns the Year Number after applying the Year Window.
     *
     * If the @p inputYear is between 0 and 99, then apply the Year Window and
     * return the calculated Year Number.
     *
     * If the @p inputYear is not between 0 and 99, then the original Year Number
     * is returned.
     *
     * @see KLocale::setYearWindowOffset
     * @see KLocale::yearWindowOffset
     * @param inputYear the year number to apply the year window to
     * @return the year number after applying the year window
     */
    int applyShortYearWindow(int inputYear) const;

    /**
     * Use this to determine which day is the first day of the week.
     *
     * Uses the calendar system's internal locale set when the instance was
     * created, which ensures that the correct calendar system and locale
     * settings are respected, which would not occur in some cases if using
     * the global locale.  Defaults to global locale.
     *
     * @see KLocale::weekStartDay
     *
     * @return an integer (Monday = 1, ..., Sunday = 7)
     */
    virtual int weekStartDay() const;

    /**
     * @deprecated use KLocale::weekDayOfPray() instead
     *
     * Returns the day of the week traditionally associated with religious
     * observance for this calendar system.  Note this may not be accurate
     * for the users locale, e.g. Gregorian calendar used in non-Christian
     * countries, in use cases where this could be an issue it is recommended
     * to use KLocale::weekDayOfPray() instead.
     *
     * @return day number (None = 0, Monday = 1, ..., Sunday = 7)
     */
    KDE_DEPRECATED virtual int weekDayOfPray() const = 0;

    /**
     * Returns whether the calendar is lunar based.
     *
     * @return @c true if the calendar is lunar based, @c false if not
     */
    virtual bool isLunar() const = 0;

    /**
     * Returns whether the calendar is lunisolar based.
     *
     * @return @c true if the calendar is lunisolar based, @c false if not
     */
    virtual bool isLunisolar() const = 0;

    /**
     * Returns whether the calendar is solar based.
     *
     * @return @c true if the calendar is solar based, @c false if not
     */
    virtual bool isSolar() const = 0;

    /**
     * Returns whether the calendar system is proleptic, i.e. whether dates
     * before the epoch are supported.
     *
     * @see KCalendarSystem::epoch
     *
     * @return @c true if the calendar system is proleptic, @c false if not
     */
    virtual bool isProleptic() const = 0;

protected:

    /**
     * Internal method to convert a Julian Day number into the YMD values for
     * this calendar system.
     *
     * All calendar system implementations MUST implement julianDayToDate and
     * dateToJulianDay methods as all other methods can be expressed as
     * functions of these.  Does no internal validity checking.
     *
     * @see KCalendarSystem::dateToJulianDay
     *
     * @param jd Julian day number to convert to date
     * @param year year number returned in this variable
     * @param month month number returned in this variable
     * @param day day of month returned in this variable
     * @return @c true if the date is valid, @c false otherwise
     */
    virtual bool julianDayToDate(int jd, int &year, int &month, int &day) const = 0;

    /**
     * Internal method to convert YMD values for this calendar system into a
     * Julian Day number.
     *
     * All calendar system implementations MUST implement julianDayToDate and
     * dateToJulianDay methods as all other methods can be expressed as
     * functions of these.  Does no internal validity checking.
     *
     * @see KCalendarSystem::julianDayToDate
     *
     * @param year year number
     * @param month month number
     * @param day day of month
     * @param jd Julian day number returned in this variable
     * @return @c true if the date is valid, @c false otherwise
     */
    virtual bool dateToJulianDay(int year, int month, int day, int &jd) const = 0;

    /**
     * Returns the locale used for translations and formats for this
     * calendar system instance.  This allows a calendar system instance to be
     * independent of the global translations and formats if required.  All
     * implementations must refer to this locale.
     *
     * Only for internal calendar system use; if public access is required then
     * provide public methods only for those methods actually required.  Any
     * app that creates an instance with its own locale overriding global will
     * have the original handle to the locale and can manipulate it that way if
     * required, e.g. to change default date format.  Only expose those methods
     * that library widgets require access to internally.
     *
     * @see KCalendarSystem::formatDate
     * @see KLocale::formatDate
     * @see KCalendarSystem::weekStartDay
     * @see KLocale::weekStartDay
     * @see KCalendarSystem::readDate
     * @see KLocale::readDate
     *
     * @return locale to use
     */
    const KLocale *locale() const;

    /**
     * @deprecated for internal use only
     *
     * Sets the maximum number of months in a year
     *
     * Only for internal calendar system use
     */
    KDE_DEPRECATED void setMaxMonthsInYear(int maxMonths);

    /**
     * @deprecated for internal use only
     *
     * Sets the maximum number of days in a week
     *
     * Only for internal calendar system use
     */
    KDE_DEPRECATED void setMaxDaysInWeek(int maxDays);

    /**
     * @deprecated for internal use only
     *
     * @since 4.4
     *
     * Sets if Calendar System has Year 0 or not
     *
     * Only for internal calendar system use
     */
    KDE_DEPRECATED void setHasYear0(bool hasYear0);

    /**
     * Constructor of abstract calendar class. This will be called by derived classes.
     *
     * @param dd derived private d-pointer.
     * @param config a configuration file with a 'KCalendarSystem %calendarName' group detailing
     *               locale-related preferences (such as era options).  The global config is used
                     if null.
     * @param locale locale to use for translations. The global locale is used if null.
     */
    KCalendarSystem(KCalendarSystemPrivate &dd,
                    const KSharedConfig::Ptr config = KSharedConfig::Ptr(),
                    const KLocale *locale = 0);

private:
    //Required for shared d-pointer as already private, remove in KDE5
    friend class KCalendarSystemCoptic;
    friend class KCalendarSystemEthiopian;
    friend class KCalendarSystemGregorian;
    friend class KCalendarSystemHebrew;
    friend class KCalendarSystemIndianNational;
    friend class KCalendarSystemIslamicCivil;
    friend class KCalendarSystemJalali;
    friend class KCalendarSystemJapanese;
    friend class KCalendarSystemJulian;
    friend class KCalendarSystemMinguo;
    friend class KCalendarSystemQDate;
    friend class KCalendarSystemThai;
    //Other friends that need access to protected/private functions
    friend class KLocalizedDate;
    friend class KLocalizedDatePrivate;
    friend class KDateTimeParser;
    friend class KDateTable;

    // Era functions needed by friends, may be made public later if needed in KCM
    QList<KCalendarEra> *eraList() const;
    KCalendarEra era(const QDate &eraDate) const;
    KCalendarEra era(const QString &eraName, int yearInEra) const;

    Q_DISABLE_COPY(KCalendarSystem)
    KCalendarSystemPrivate * const d_ptr; // KDE5 make protected
    Q_DECLARE_PRIVATE(KCalendarSystem)
};

#endif
