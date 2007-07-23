/*
   Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
   Copyright (c) 2002-2003 Hans Petter Bieker <bieker@kde.org>
   Copyright (c) 2007 John Layt <john@layt.net>

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

#include <QtCore/QStringList>

class KCalendarSystemPrivate;

class QDate;

/**
 * CalendarSystem abstract class, default derived kde gregorian class and
 * factory class. Provides support for different calendar types for kde
 * calendar widget and related stuff.
 *
 * Derived classes must be created through KCalendarFactory class
 *
 * @author Carlos Moro <cfmoro@correo.uniovi.es>
 */
class KDECORE_EXPORT KCalendarSystem
{
public:

    /**
     * Format for returned year number / month number / day number as string.
     */
    enum StringFormat {
      ShortFormat,      /**< Short string format, e.g. 2000 = "00" or 6 = "6" */
      LongFormat,       /**< Long string format, e.g. 2000 = "2000" or 6 = "06" */
    };

    /**
     * Format for returned month / day name.
     */
    enum MonthNameFormat {
      ShortName,                /**< Short name format, e.g. "Dec" */
      LongName,                 /**< Long name format, e.g. "December" */
      ShortNamePossessive,      /**< Short name possessive format, e.g. "of Dec" */
      LongNamePossessive,       /**< Long name possessive format, e.g. "of December" */
    };

    /**
     * Format for returned month / day name.
     */
    enum WeekDayNameFormat {
      ShortDayName,                /**< Short name format, e.g. "Fri" */
      LongDayName,                 /**< Long name format, e.g. "Friday" */
    };

  /**
   * Creates specific calendar type
   *
   * @param calType string identification of the specific calendar type
   * to be constructed
   * @param locale Locale used for translations. Use the global locale when
   * 0 is specified.
   * @return a KCalendarSystem object
   */
  static KCalendarSystem *create ( const QString & calType = QLatin1String( "gregorian" ),
                                   const KLocale * locale = 0 );

  /**
   * Gets list of names of supported calendar systems
   *
   * @return A QStringList object
   */
  static QStringList calendarSystems();

    /**
     * Returns a typographically correct and translated label to display for
     * the calendar system type.  Use with method calendarSystems to neatly
     * format labels to display on combo widget of available calendar systems
     *
     * @param calendarType the specific calendar type to return the label for
     *
     * @return label for calendar
     */
    static QString calendarLabel( const QString &calendarType );

  /**
   * Constructor of abstract calendar class. This will be called by the derived classes.
   *
   * @param locale It will use this locale for translations, 0 means global.
   */
  explicit KCalendarSystem(const KLocale * locale = 0);

  /**
   * Destructor.
   */
  virtual ~KCalendarSystem();

    /**
     * Returns the calendar system type
     *
     * This is the same as the deprecated calendarName, but now matches
     * KLocale terminology of type in place of name.
     *
     * @return type of calendar system
     */
    virtual QString calendarType() const = 0;

    /**
     * Returns a QDate holding the epoch of the calendar system.  Usually YMD
     * of 1/1/1, access the returned QDates method toJulianDay() if you
     * require the actual Julian Day number.  Note: a particular Calendar
     * System implementation may not include the epoch in its supported range,
     * or the Calendar System may be Proleptic in which case it supports dates
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
     * If calendar system proleptic then may be before epoch.
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
     * Returns if a given date is valid in this calendar system
     *
     * @param year the year portion of the date to check
     * @param month the month portion of the date to check
     * @param day the day portion of the date to check
     * @return @c true if the date is valid, @c false otherwise
     */
    virtual bool isValid(int year, int month, int day) const = 0;

    /**
     * Returns if a given date is valid in this calendar system
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
     * @param date Date to change
     * @param year Year
     * @param month Month number
     * @param day Day of month
     * @return @c true if the date is valid, @c false otherwise
     */
    virtual bool setDate(QDate &date, int year, int month, int day) const;

  /**
   * Gets specific calendar type year for a given gregorian date
   *
   * @param date gregorian date
   * @return year
   */
  virtual int year (const QDate & date) const;

    /**
     * Converts a date into a year literal
     *
     * @param pDate The date to convert
     * @param format The format to return, either short or long
     * @return The year literal of the date
     */
    virtual QString yearString( const QDate & pDate, StringFormat format = LongFormat ) const;

  /**
   * Converts a year literal of a part of a string into a integer starting at the beginning of the string
   *
   * @param sNum The string to parse
   * @param iLength The number of QChars used, and 0 if no valid symbols was found in the string
   * @return An integer corresponding to the year
   */
  virtual int yearStringToInteger(const QString & sNum, int & iLength) const;

  /**
   * Gets specific calendar type month for a given gregorian date
   *
   * @param date gregorian date
   * @return month number
   */
  virtual int month (const QDate & date) const;

    /**
     * Converts a date into a month literal
     *
     * @param pDate The date to convert
     * @param format The format to return, either short or long
     * @return The month literal of the date
     */
    virtual QString monthString( const QDate & pDate, StringFormat format = LongFormat ) const;

  /**
   * Converts a month literal of a part of a string into a integer starting at the beginning of the string
   *
   * @param sNum The string to parse
   * @param iLength The number of QChars used, and 0 if no valid symbols was found in the string
   * @return An integer corresponding to the month
   */
  virtual int monthStringToInteger(const QString & sNum, int & iLength) const;

  /**
   * Gets specific calendar type day number of month for a given date
   *
   * @param date gregorian date equivalent to the specific one
   * @return day of the month
   */
  virtual int day (const QDate & date) const;

    /**
     * Converts a date into a day literal
     *
     * @param pDate The date to convert
     * @param format The format to return, either short or long
     * @return The day literal of the date
     */
    virtual QString dayString( const QDate & pDate, StringFormat format = LongFormat ) const;

  /**
   * Converts a day literal of a part of a string into a integer starting at the beginning of the string
   *
   * @param sNum The string to parse
   * @param iLength The number of QChars used, and 0 if no valid symbols was found in the string
   * @return An integer corresponding to the day
   */
  virtual int dayStringToInteger(const QString & sNum, int & iLength) const;

  /**
   * Gets specific calendar type number of day of week number for a given
   * date
   *
   * @param date gregorian date
   * @return day of week
   */
  virtual int dayOfWeek (const QDate & date) const;

  /**
   * Gets specific calendar type day number of year for a given date
   *
   * @param date gregorian date equivalent to the specific one
   * @return day number
   */
  virtual int dayOfYear (const QDate & date) const;

  /**
   * @deprecated
   *
   * Use setDate instead
   *
   * @see KCalendarSystem::setDate
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
  virtual bool setYMD(QDate & date, int y, int m, int d) const;

  /**
   * Returns a QDate object containing a date nyears later.
   *
   * @param date The old date
   * @param nyears The number of years to add
   * @return The new date
   */
  virtual QDate addYears(const QDate & date, int nyears) const;

  /**
   * Returns a QDate object containing a date nmonths later.
   *
   * @param date The old date
   * @param nmonths The number of months to add
   * @return The new date
   */
  virtual QDate addMonths(const QDate & date, int nmonths) const;

  /**
   * Returns a QDate object containing a date ndays later.
   *
   * @param date The old date
   * @param ndays The number of days to add
   * @return The new date
   */
  virtual QDate addDays(const QDate & date, int ndays) const;

    /**
     * Returns if a given year is a leap year
     *
     * @param year the year to check
     * @return @c true if the year is a leap year, @c false otherwise
     */
    virtual bool isLeapYear(int year) const = 0;

    /**
     * Returns if a given date falls in a leap year
     *
     * @param date the date to check
     * @return @c true if the date falls in a leap year, @c false otherwise
     */
    virtual bool isLeapYear(const QDate &date) const;

  /**
   * Gets specific calendar type number of month for a given year
   *
   * @param date The date whose year to use
   * @return The number of months in that year
   */
  virtual int monthsInYear (const QDate & date) const;

  /**
   * Gets the number of days in date whose years specified.
   *
   * @param date Gregorian date equivalent to the specific one
   * @return The number of days in year
   */
  virtual int daysInYear (const QDate & date) const;

  /**
   * Gets specific calendar type number of days in month for a given date
   *
   * @param date gregorian date
   * @return number of days for month in date
   */
  virtual int daysInMonth (const QDate & date) const;

    /**
     * Returns the number of ISO weeks in a specified year
     *
     * @param date the date to obtain year from
     * @return number of weeks in year
     */
    virtual int weeksInYear(const QDate &date) const;

  /**
   * Gets the number of weeks in a specified year
   *
   * @param year the year
   * @return number of weeks in year
   */
  virtual int weeksInYear(int year) const;

    /**
     * Returns the number of days in a specified week
     *
     * @param date the date to obtain week from
     * @return number of days in week
     */
    virtual int daysInWeek (const QDate &date) const;

    /**
     * Use this to determine which day is the first day of the week.
     *
     * Uses the calendar systems internal locale as set when instantiated,
     * which ensures the correct calendar system and locale settings are
     * respected, which would not occur in some cases if using the global
     * locale.  Defaults to global locale.
     *
     * @see KLocale::weekStartDay
     *
     * @return an integer (Monday=1..Sunday=7)
     */
    virtual int weekStartDay() const;

  /**
   * Gets specific calendar type week number for a given date
   *
   * @param date gregorian date
   * @param yearNum The year the date belongs to
   * @return week number
   */
  virtual int weekNumber(const QDate& date, int * yearNum = 0) const;

    /**
     * Gets specific calendar type month name for a given month number
     * If an invalid month is specified, QString() is returned.
     *
     * @param month The month number
     * @param year The year the month belongs to
     * @param format Specifies if the short month name or long month name should be used
     * @return The name of the month
     */
    virtual QString monthName( int month, int year, MonthNameFormat format = LongName ) const = 0;

    /**
     * Gets specific calendar type month name for a given date
     *
     * @param date Date to obtain month from
     * @param format Specifies if the short month name or long month name should be used
     * @return The name of the month
     */
    virtual QString monthName( const QDate & date, MonthNameFormat format = LongName ) const;

    /**
     * Gets specific calendar type week day name
     * If an invalid week day is specified, QString() is returned.
     *
     * @param weekDay number of day in week (1 -> Monday)
     * @param format Specifies if the short month name or long month name should be used
     * @return day name
     */
    virtual QString weekDayName( int weekDay, WeekDayNameFormat format = LongDayName ) const = 0;

    /**
     * Gets specific calendar type week day name
     *
     * @param date the date
     * @param format Specifies if the short month name or long month name should be used
     * @return day name
     */
    virtual QString weekDayName( const QDate & date, WeekDayNameFormat format = LongDayName ) const;

  /**
   * Gets the day of the week traditionaly associated with pray
   *
   * @return day number
   */
  virtual int weekDayOfPray () const = 0;

  /**
   * Gets if the calendar is lunar based
   *
   * @return if the calendar is lunar based
   */
  virtual bool isLunar() const = 0;

  /**
   * Gets if the calendar is lunisolar based
   *
   * @return if the calendar is lunisolar based
   */
  virtual bool isLunisolar() const = 0;

  /**
   * Gets if the calendar is solar based
   *
   * @return if the calendar is solar based
   */
  virtual bool isSolar() const = 0;

    /**
     * Returns a string formatted to the current locale's conventions
     * regarding dates.
     *
     * Uses the calendar systems internal locale as set when instantiated,
     * which ensures the correct calendar system and locale settings are
     * respected, which would not occur in some cases if using the global
     * locale.  Defaults to global locale.
     *
     * @see KLocale::formatDate
     *
     * @param date the date to be formatted.
     * @param format category of date format to use
     *
     * @return The date as a string
     */
    virtual QString formatDate(const QDate &date, KLocale::DateFormat format = KLocale::LongDate) const;

    /**
     * Converts a localized date string to a QDate.
     * The bool pointed by ok will be invalid if the date entered was not valid.
     *
     * Uses the calendar systems internal locale as set when instantiated,
     * which ensures the correct calendar system and locale settings are
     * respected, which would not occur in some cases if using the global
     * locale.  Defaults to global locale.
     *
     * @see KLocale::readDate
     *
     * @param str the string we want to convert.
     * @param ok the boolean that is set to false if it's not a valid date.
     *           If @p ok is 0, it will be ignored
     *
     * @return The string converted to a QDate
     */
    virtual QDate readDate(const QString &str, bool* ok = 0) const;

    /**
     * Converts a localized date string to a QDate, using the specified format.
     * You will usually not want to use this method.
     *
     * @see KLocale::readDate
     */
    virtual QDate readDate( const QString &intstr, const QString &fmt, bool* ok = 0) const;

    /**
     * Converts a localized date string to a QDate.
     * This method is stricter than readDate(str,&ok): it will either accept
     * a date in full format or a date in short format, depending on @p flags.
     *
     * Uses the calendar systems internal locale as set when instantiated,
     * which ensures the correct calendar system and locale settings are
     * respected, which would not occur in some cases if using the global
     * locale.  Default case uses global locale.
     *
     * @see KLocale::readDate
     *
     * @param str the string we want to convert.
     * @param flags whether the date string is to be in full format or in short format.
     * @param ok the boolean that is set to false if it's not a valid date.
     *           If @p ok is 0, it will be ignored
     *
     * @return The string converted to a QDate
     */
    virtual QDate readDate(const QString &str, KLocale::ReadDateFlags flags, bool *ok = 0) const;

    /**
     * Returns if the calendar system is proleptic, i.e. dates before the epoch
     * are supported.
     *
     * @see KCalendarSystem::epoch
     *
     * @return if the calendar system is proleptic
     */
    virtual bool isProleptic() const = 0;

protected:

    /**
     * Internal method to convert a Julian Day number into the YMD values for
     * this calendar system.
     *
     * All calendar system implementations MUST implement julianDayToDate and
     * dateToJulianDay methods as all other methods can be expressed as
     * functions of these.
     *
     * @see KCalendarSystem::dateToJulianDay
     *
     * @param jd julian day number to convert to date
     * @param year Year number returned in this variable
     * @param month Month number returned in this variable
     * @param day Day of month returned in this variable
     * @return @c true if the date is valid, @c false otherwise
     */
    virtual bool julianDayToDate(int jd, int &year, int &month, int &day) const = 0;

    /**
     * Internal method to convert YMD values for this calendar system into a
     * Julian Day number.
     *
     * All calendar system implementations MUST implement julianDayToDate and
     * dateToJulianDay methods as all other methods can be expressed as
     * functions of these.
     *
     * @see KCalendarSystem::julianDayToDate
     *
     * @param year Year number returned in this variable
     * @param month Month number returned in this variable
     * @param day Day of month returned in this variable
     * @param jd julian day number returned in this variable
     * @return @c true if the date is valid, @c false otherwise
     */
    virtual bool dateToJulianDay(int year, int month, int day, int &jd) const = 0;

    /**
     * Returns the locale to use for translations and formats for this 
     * calendar system instance.  This allows a calendar system instance to be
     * independant of the global translations and formats if required.  All 
     * implementations must refer to this locale.  Defaults to global.
     *
     * Only for internal calendar system use, if public access is required then
     * provide public methods only for those methods actually required.  Any
     * app that creates an instance with its own locale overriding global will
     * have the original handle to the locale and can manipulate it that way if
     * required, e.g to change default date format.  Only expose those methods
     * that library widgets require access to internally.
     *
     * @see KCalendarSystem::formatDate
     * @see KLocale::formatDate
     * @see KCalendarSystem::weekStartDay
     * @see KLocale::weekStartDay
     * @see KCalendarSystem::readDate
     * @see KLoacle::readDate
     *
     * @return locale to use
     */
    const KLocale * locale() const;

private:
  KCalendarSystemPrivate* const d;
};

#endif
