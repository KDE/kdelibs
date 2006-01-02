/*
    This file is part of the KDE libraries
    Copyright (c) 2005 David Jarvie <software@astrojar.org.uk>

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

/** @file
 * Date/times with associated time zone
 * @author David Jarvie <software@astrojar.org.uk>.
 */

#ifndef _KDATETIME_H_
#define _KDATETIME_H_

#include <ktimezones.h>
#include "kdelibs_export.h"

class QDataStream;
class KDateTimePrivate;

/**
 * @short A class representing a date and time with an associated time zone
 *
 * Topics:
 *  - @ref intro
 *  - @ref manipulation
 *  - @ref compatibility
 *
 * @section intro Introduction
 *
 * The class KDateTime combines a date and time with support for an
 * associated time zone or UTC offset. When manipulating KDateTime objects,
 * their time zones or UTC offsets are automatically taken into account. KDateTime
 * can also be set to represent a date-only value with no associated time.
 *
 * The class uses QDateTime internally to represent date/time values, and
 * therefore can only be used for dates in the Gregorian calendar, in the range
 * 1752 - 7999 AD. The Gregorian calendar started in 1582, but adoption was
 * slow. 1752 was when it was adopted by the United Kingdom; the last European
 * country to adopt it, Greece, did so only in 1923. See QDateTime
 * Considerations section below for further discussion of the date range
 * limitations.
 *
 * The time specification types which KDateTime supports are:
 * - the UTC time zone
 * - a local time with a specified offset from UTC
 * - a local time in a specified time zone
 * - a local time using the current system time zone (a special case of the
 *   previous item)
 * - local clock time, using whatever the local system clock says on whichever
 *   computer it happens to be on. In this case, the equivalent UTC time will
 *   vary depending on system. As a result, calculations involving local clock
 *   times do not necessarily produce reliable results.
 * These characteristics are more fully described in the description of the
 * TimeSpec enumeration. Also see
 * <a href="http://www.w3.org/TR/timezone/">W3C: Working with Time Zones</a>
 * for a good overview of the different ways of representing times.
 *
 * To set the time specification, use one of the setTimeSpec() methods, to get
 * the time specification, call timeSpec(), isUTC(), isLocalZone(),
 * isOffsetFromUTC() or isClockTime(). To determine whether two KDateTime
 * instances have the same time specification, use compareTimeSpec().
 *
 * @section manipulation Date and Time Manipulation
 *
 * A KDateTime object can be created by passing a date and time in its
 * constructor, together with a time specification.
 *
 * If both the date and time are null, isNull() returns true. If the date, time
 * and time specification are all valid, isValid() returns true.
 *
 * A KDateTime object can be converted to a different time specification by
 * using toUTC(), toLocalZone() or toClockTime(). It can be converted to a
 * specific time zone by toZone(). To return the time as an elapsed time since
 * 1 January 1970 (as used by time(2)), use toTime_t().
 *
 * The date and time can be set either in the constructor, or afterwards by
 * calling setDate(), setTime() or setDateTime(). To return the date and/or
 * time components of the KDateTime, use date(), time() and dateTime(). You
 * can determine whether the KDateTime represents a date and time, or a date
 * only, by isDateOnly(). You can change between a date and time or a date only
 * value using setDateOnly().
 *
 * You can increment or decrement the date/time using addSecs(), addDays(),
 * addMonths() and addYears(). The interval between two date/time values can
 * be found using secsTo() or daysTo().
 *
 * The comparison operators (operator==(), operator<(), etc.) all take the time
 * zone properly into account; if the two KDateTime objects have different time
 * zones, they are first converted to UTC before the comparison is performed.
 *
 * KDateTime values may be converted to and from a string representation using
 * the toString() and fromString() methods. These handle a variety of text
 * formats including ISO 8601 and RFC 2822.
 *
 * @section compatibility QDateTime Considerations
 *
 * KDateTime's interface is designed to be as compatible as possible with that
 * of QDateTime, but with adjustments to cater for time zone handling. Because
 * QDateTime lacks virtual methods, KDateTime is not inherited from QDateTime,
 * but instead is implemented using a private QDateTime object.
 *
 * The date range restriction due to the use of QDateTime internally may at
 * first sight seem a design limitation. However, two factors should be
 * considered:
 *
 * - there are significant problems in the representation of dates before the
 *   Gregorian calendar was adopted. The numbering of dates - even to some
 *   extent year numbering - varied from place to place. So any date/time
 *   system which attempted to represent dates as actually used in history
 *   would be too specialised to belong to the core KDE libraries. Date/time
 *   systems for scientific applications can be much simpler, but may differ
 *   from historical records.
 *
 * - time zones were not invented until the middle of the 19th century.
 *
 * Because of these issues, together with the fact that KDateTime's aim is to
 * provide automatic time zone handling for date/time values, QDateTime was
 * chosen as the basis for KDateTime. For those who need an extended date
 * range, other classes exist.
 *
 * @see KTimezone, KSystemTimezones, QDateTime, QDate, QTime
 * @see <a href="http://www.w3.org/TR/timezone/">W3C: Working with Time Zones</a>
 * @author David Jarvie \<software@astrojar.org.uk\>.
 * @since 4.0
 */
class KDECORE_EXPORT KDateTime
{
  public:
    /**
     * The time specification of a KDateTime instance.
     * This specifies how the date/time component of the KDateTime instance
     * should be interpreted, i.e. what time zone (if any) the date/time is
     * expressed in.
     */
    enum TimeSpec
    {
        UTC,        /**< a UTC time */
        OffsetFromUTC, /**< a local time which has a fixed offset from UTC. */
        TimeZone,   /**< a time in a specified time zone. If the time zone is
                     *   the current system time zone (i.e. that returned by
                     *   KSystemTimezones::local()), LocalZone may be used
                     *   instead.
                     */
        LocalZone,  /**< a time in the current system time zone.
                     *   When used to initialise a KDateTime instance, this is
                     *   simply a shorthand for calling the setting method with
                     *   a time zone parameter KSystemTimezones::local(). Note
                     *   that if the system is changed to a different time zone
                     *   afterwards, the KDateTime instance will still use the
                     *   original system time zone rather than adopting the new
                     *   zone.
                     *   When returned by a method, it indicates that the time
                     *   zone stored in the instance is that currently returned
                     *   by KSystemTimezones::local().
                     */
        ClockTime   /**< a clock time which ignores time zones and simply uses
                     *   whatever the local system clock says the time is. You
                     *   could, for example, set a wake-up time of 07:30 on
                     *   some date, and then no matter where you were in the
                     *   world, you would be in time for breakfast as long as
                     *   your computer was aligned with the local time.
                     *
                     *   Note that any calculations which involve clock times
                     *   cannot be guaranteed to be accurate, since by
                     *   definition they contain no information about time
                     *   zones or daylight savings changes.
                     */
    };

    /** Format for strings representing date/time values. */
    enum TimeFormat
    {
        ISODate,    /**< ISO 8601 format, i.e. YYYY-MM-DDThh[:mm[:ss[.sss]]]TZ,
                     *   where TZ is the time zone offset (blank for local
                     *   time, Z for UTC, or ±hhmm for an offset from UTC).
                     *   When parsing a string, the ISO 8601 basic format,
                     *   YYYYMMDDThh[mm[ss[.sss]]]TZ, is also accepted. For
                     *   date-only values, the formats YYYY-MM-DD and
                     *   YYYYMMDD (without time zone specifier) are used. All
                     *   formats may contain a day of the year instead of day
                     *   and month.
                     */
        RFCDate,    /**< RFC 2822 format,
                     *   i.e. "[Wdy,] DD Mon YYYY hh:mm[:ss] ±hhmm". This format
                     *   also covers RFCs 822, 850, 1036 and 1123.
                     *   When parsing a string, it also accepts the format
                     *   "Wdy Mon DD HH:MM:SS YYYY" specified by RFCs 850 and
                     *   1036. There is no valid date-only format.
                     */
        RFCDateDay, /**< RFC 2822 format including day of the week,
                     *   i.e. "Wdy, DD Mon YYYY hh:mm:ss ±hhmm"
                     */
        QtTextDate, /**< Same format as Qt::TextDate (i.e. Day Mon DD hh:mm:ss YYYY)
                     *   with, if not local time, the UTC offset appended. The
                     *   time may be omitted to indicate a date-only value.
                     */
        LocalDate   /**< Same format as Qt::LocalDate (i.e. locale dependent)
                     *   with, if not local time, the UTC offset appended. The
                     *   time may be omitted to indicate a date-only value.
                     */
    };

    /**
     * How this KDateTime compares with another.
     * If any date-only value is involved, comparison of KDateTime values
     * requires them to be considered as representing time periods. A date-only
     * instance represents a time period from 00:00:00 to 23:59:59.999 on a given
     * date, while a date/time instance can be considered to represent a time
     * period whose start and end times are the same. They may therefore be
     * earlier or later, or may overlap or be contained one within the other.
     *
     * In the descriptions of the values below,
     * - s1 = start time of this instance
     * - e1 = end time of this instance
     * - s2 = start time of other instance
     * - e2 = end time of other instance.
     */
    enum Comparison
    {
        Before,        /**< This KDateTime is strictly earlier than the other,
                        *   i.e. e1 < s2.
                        */
        BeforeOverlap, /**< This KDateTime starts earlier than the other, but
                        *   the time periods partly overlap, i.e. s1 < s2 &&
                        *   e1 >= s2 && e1 < e2.
                        */
        Equal,         /**< Simultaneous, i.e. s1 = s2 && e1 = e2.
                        */
        Contains,      /**< The other KDateTime is contained within this
                        *   date-only value, i.e. (s1 <= s2 && e1 > e2) ||
                        *   (s1 < s2 && e1 >= e2).
                        */
        ContainedBy,   /**< This KDateTime is contained within the other
                        *   date-only value, i.e. (s1 >= s2 && e1 < e2) ||
                        *   (s1 > s2 && e1 <= e2).
                        */
        AfterOverlap,  /**< This KDateTime ends later than the other, but the
                        *   time periods partly overlap, i.e. s1 > s2 &&
                        *   s1 < e2 && e1 > e2.
                        */
        After          /**< This KDateTime is strictly later than the other,
                        *   i.e. s1 > e2.
                        */
    };

  
    /**
     * Constructs an invalid date/time.
     */
    KDateTime();

    /**
     * Constructs a date-only value with associated time zone. The time is
     * set to 00:00:00.
     *
     * @param date date in the time zone @p tz
     * @param tz   time zone
     */
    KDateTime(const QDate &date, const KTimezone *tz);

    /**
     * Constructs a date/time with associated time zone.
     *
     * @param date date in the time zone @p tz
     * @param time time in the time zone @p tz
     * @param tz   time zone
     */
    KDateTime(const QDate &date, const QTime &time, const KTimezone *tz);

    /**
     * Constructs a date/time with associated time zone.
     * If @p dt is specified as a UTC time (i.e. @c dt.timeSpec() is @c Qt::UTC),
     * it is first converted to local time in time zone @p tz before being stored.
     *
     * @param dt date and time
     * @param tz time zone
     */
    KDateTime(const QDateTime &dt, const KTimezone *tz);

    /**
     * Constructs a date-only value expressed as specified by @p spec. The time
     * component is set to 00:00:00.
     *
     * The instance is initialised according to the value of @p spec as follows:
     * - @c UTC       : date is stored as UTC.
     * - @c OffsetFromUTC : date is a local time at the specified offset
     *                      from UTC.
     * - @c LocalZone : date is a local date in the current system time
     *                  zone.
     * - @c ClockTime : time zones are ignored.
     * - Note that @c TimeZone is invalid here, and will construct an invalid
     *   date.
     *
     * @param date      date in the time zone indicated by @p spec
     * @param spec      how the date is stored
     * @param utcOffset number of seconds to add to UTC to get the local
     *                  time. Ignored if @p spec is not @c OffsetFromUTC.
     */
    explicit KDateTime(const QDate &date, TimeSpec spec = LocalZone, int utcOffset = 0);

    /**
     * Constructs a date/time expressed as specified by @p spec.
     *
     * @p date and @p time are interpreted and stored according to the value of
     * @p spec as follows:
     * - @c UTC       : @p date and @p time are in UTC.
     * - @c OffsetFromUTC : date/time is a local time at the specified offset
     *                      from UTC.
     * - @c LocalZone : @p date and @p time are local times in the current system
     *                  time zone.
     * - @c ClockTime : time zones are ignored.
     * - Note that @c TimeZone is invalid here, and will construct an invalid
     *   date/time.
     *
     * @param date      date in the time zone indicated by @p spec
     * @param time      time in the time zone indicated by @p spec
     * @param spec      how the date and time are stored
     * @param utcOffset number of seconds to add to UTC to get the local
     *                  time. Ignored if @p spec is not @c OffsetFromUTC.
     */
    KDateTime(const QDate &date, const QTime &time, TimeSpec spec = LocalZone, int utcOffset = 0);

    /**
     * Constructs a date/time expressed as specified by @p spec.
     *
     * @p dt is interpreted and stored according to the value of @p spec as
     * follows:
     * - @c UTC       : @p dt is stored as a UTC value. If
     *                  @c dt.timeSpec() is @c Qt::LocalTime, @p dt is first
     *                  converted from the current system time zone to UTC
     *                  before storage.
     * - @c OffsetFromUTC : date/time is stored as a local time at the specified
     *                  offset from UTC. If @c dt.timeSpec() is @c Qt::UTC,
     *                  the time is adjusted by the UTC offset before
     *                  storage. If @c dt.timeSpec() is @c Qt::LocalTime,
     *                  it is assumed to be a local time at the specified
     *                  offset from UTC, and is stored without adjustment.
     * - @c LocalZone : @p dt is stored as a local time in the current system
     *                  time zone. If @c dt.timeSpec() is @c Qt::UTC, @p dt is
     *                  first converted to local time before storage.
     * - @c ClockTime : If @c dt.timeSpec() is @c Qt::UTC, @p dt is first
     *                  converted to local time in the current system time zone
     *                  before storage. After storage, the time is treated as a
     *                  simple clock time, ignoring time zones.
     * - Note that @c TimeZone is invalid here, and will construct an invalid
     *   date/time.
     *
     * @param dt        date and time
     * @param spec      how the date and time are stored
     * @param utcOffset number of seconds to add to UTC to get the local
     *                  time. Ignored if @p spec is not @c OffsetFromUTC.
     */
    KDateTime(const QDateTime &dt, TimeSpec spec, int utcOffset = 0);

    /**
     * Constructs a date/time from a QDateTime.
     * The KDateTime is expressed in either UTC or the local system time zone,
     * according to @p dt.timeSpec().
     *
     * @param dt date and time
     */
    KDateTime(const QDateTime &dt);

    KDateTime(const KDateTime &other);
    ~KDateTime();

    KDateTime &operator=(const KDateTime &other);

    /**
     * Returns whether the date/time is null.
     *
     * @return @c true if both date and time are null, else @c false
     * @see isValid(), QDateTime::isNull()
     */
    bool isNull() const;

    /**
     * Returns whether the date/time is valid.
     *
     * @return @c true if both date and time are valid, else @c false
     * @see isNull(), QDateTime::isValid()
     */
    bool isValid() const;

    /**
     * Returns whether the instance represents a date/time or a date-only value.
     *
     * @return @c true if date-only, @c false if date and time
     */
    bool isDateOnly() const;

    /**
     * Returns the date part of the date/time. The value returned should be
     * interpreted in terms of the instance's time zone or UTC offset.
     *
     * @return date value
     * @see time(), dateTime()
     */
    QDate date() const;

    /**
     * Returns the time part of the date/time. The value returned should be
     * interpreted in terms of the instance's time zone or UTC offset. If
     * the instance is date-only, the time returned is 00:00:00.
     *
     * @return time value
     * @see date(), dateTime(), isDateOnly()
     */
    QTime time() const;

    /**
     * Returns the date/time component of the instance, ignoring the time
     * zone. The value returned should be interpreted in terms of the
     * instance's time zone or UTC offset. The returned value's @c timeSpec()
     * value will be @c Qt::UTC if the instance is a UTC time, else
     * @c Qt::LocalTime. If the instance is date-only, the time value is set to
     * 00:00:00.
     *
     * @return date/time
     * @see date(), time()
     */
    QDateTime dateTime() const;

    /**
     * Returns the time zone for the date/time. If the date/time is specified
     * as a UTC time, a UTC time zone is always returned.
     *
     * @return time zone, or null if a local time at a fixed UTC offset or a
     *         local clock time
     * @see isUTC(), isLocal()
     */
    const KTimezone *timeZone() const;

    /**
     * Returns the time specification of the date/time, i.e. whether it is
     * UTC, has a time zone, or is a local clock time.
     *
     * @return specification type
     * @see isLocalZone(), isClockTime(), isUTC(), timeZone()
     */
    TimeSpec timeSpec() const;

    /**
     * Returns whether the time zone for the date/time is the current local
     * system time zone.
     *
     * @return @c true if local system time zone
     * @see isUTC(), isOffsetFromUTC(), timeZone()
     */
    bool isLocalZone() const;

    /**
     * Returns whether the date/time is a local clock time.
     *
     * @return @c true if local clock time
     * @see isUTC(), timeZone()
     */
    bool isClockTime() const;

    /**
     * Returns whether the date/time is a UTC time.
     * It is considered to be a UTC time if it either has a UTC time
     * specification (TimeSpec == UTC) or has the time zone KTimezones::utc().
     *
     * @return @c true if UTC
     * @see isLocal(), isOffsetFromUTC(), timeZone()
     */
    bool isUTC() const;

    /**
     * Returns whether the date/time is a local time at a fixed offset from
     * UTC.
     *
     * @return @c true if local time at fixed offset from UTC
     * @see isLocal(), isUTC(), UTCOffset()
     */
    bool isOffsetFromUTC() const;

    /**
     * Returns the UTC offset associated with the date/time. The UTC offset is
     * the number of seconds to add to UTC to get the local time.
     *
     * @return UTC offset in seconds, or 0 if local clock time
     * @see isClockTime()
     */
    int UTCOffset() const;

    /**
     * Compares the time specifications of this instance and another. The time
     * specifications are considered to be the same if they are either both in
     * the same time zone (UTC, local time zone or another time zone), both
     * type OffsetFromUTC with the same UTC offset, or both local clock times.
     *
     * @param other other KDateTime
     * @return @c true if the time specifications are the same
     * @see toTimeSpec()
     */
    bool compareTimeSpec(const KDateTime &other) const;

    /**
     * Returns the time converted to UTC. The converted time has a UTC offset
     * of zero.
     * If the instance is a local clock time, it is first set to the local time
     * zone, and then converted to UTC.
     * If the instance is a date-only value, a date-only UTC value is returned,
     * with the date unchanged.
     *
     * @return converted time
     * @see toLocal(), toZone(), toTimeSpec(), toTime_t(), KTimezone::convert()
     */
    KDateTime toUTC() const;

    /**
     * Returns the time converted to the current local system time zone.
     * If the instance is a date-only value, a date-only local time zone value
     * is returned, with the date unchanged.
     *
     * @return converted time
     * @see toUTC(), toZone(), toTimeSpec(), KTimezone::convert()
     */
    KDateTime toLocalZone() const;

    /**
     * Returns the time converted to the local clock time. The time is first
     * converted to the local system time zone before setting its type to
     * ClockTime, i.e. no associated time zone.
     * If the instance is a date-only value, a date-only clock time value is
     * returned, with the date unchanged.
     *
     * @return converted time
     * @see toLocalZone(), toTimeSpec()
     */
    KDateTime toClockTime() const;

    /**
     * Returns the time converted to a specified time zone.
     * If the instance is a local clock time, it is first set to the local time
     * zone, and then converted to @p zone.
     * If the instance is a date-only value, a date-only value in @p zone is
     * returned, with the date unchanged.
     *
     * @param zone time zone to convert to
     * @return converted time
     * @see toUTC(), toLocal(), toTimeSpec(), KTimezone::convert()
     */
    KDateTime toZone(const KTimezone *zone) const;

    /**
     * Returns the time converted to the same time specification as @p other.
     * If the instance is a local clock time, it is first set to the local time
     * zone, and then converted to the @p other time specification.
     * If the instance is a date-only value, a date-only value is returned,
     * with the date unchanged.
     *
     * @return converted time
     * @see toLocal(), toUTC(), toZone(), compareTimeSpec(), KTimezone::convert()
     */
    KDateTime toTimeSpec(const KDateTime &other) const;

    /**
     * Converts the time to a UTC time, measured in seconds since 00:00:00 UTC
     * 1st January 1970 (as returned by time(2)).
     *
     * @return converted time, or -1 if the date is out of range
     * @see setTime_t()
     */
    uint toTime_t() const;

    /**
     * Sets the instance either to being a date and time value, or a date-only
     * value. If its status is changed to date-only, its time is set to
     * 00:00:00.
     *
     * @param dateOnly @c true to set to date-only, @c false to set to date
     *                 and time.
     * @see isDateOnly(), setTime()
     */
    void setDateOnly(bool dateOnly);

    /**
     * Sets the date part of the date/time.
     *
     * @param date new date value
     * @see date(), setTime(), setTimeSpec(), setTime_t(), setDateOnly()
     */
    void setDate(const QDate &date);

    /**
     * Sets the time part of the date/time. If the instance was date-only, it
     * is changed to being a date and time value.
     *
     * @param time new time value
     * @see time(), setDate(), setTimeSpec(), setTime_t()
     */
    void setTime(const QTime &time);

    /**
     * Sets the date/time part of the instance, leaving the time specification
     * unaffected.
     *
     * If @p dt is a local time (\code dt.timeSpec() == Qt::LocalTime \endcode)
     * and the instance is UTC, @p dt is first converted from the current
     * system time zone to UTC before being stored. If @p dt is UTC and the
     * instance is not UTC, @p dt is first converted to the current system time
     * zone before being stored.
     *
     * If the instance was date-only, it is changed to being a date and time
     * value.
     *
     * @param dt date and time
     * @see dateTime(), setDate(), setTime(), setTimeSpec()
     */
    void setDateTime(const QDateTime &dt);

    /**
     * Changes the time specification of the instance to UTC, the local time
     * zone or to local clock time.
     *
     * Any previous time zone is forgotten. The stored date/time component of
     * the instance is left unchanged. Usually this method will change the
     * absolute time which this instance represents.
     *
     * @param spec @c UTC to use UTC time zone, @c LocalZone to use the current
     *             local system time zone, or @c ClockTime to use local clock
     *             time. Note that @c TimeZone cannot be used here.
     * @param utcOffset number of seconds to add to UTC to get the local
     *                  time. Ignored if @p spec is not @c OffsetFromUTC.
     * @see timeSpec(), timeZone(), setTimeSpec(const KTimezone*)
     */
    void setTimeSpec(TimeSpec spec, int utcOffset = 0);

    /**
     * Sets the time zone in which the date/time is expressed.
     *
     * The stored date/time component of the instance is left unchanged. Usually
     * this method will change the absolute time which this instance represents.
     *
     * To set the time zone to UTC or to local clock time, use
     * setTimeSpec(TimeSpec). To set the time zone to the current local system
     * time zone, setTimeSpec(TimeSpec) may optionally be used.
     *
     * @param tz new time zone
     * @see timeZone(), setTimeSpec()
     */
    void setTimeSpec(const KTimezone *tz);

    /**
     * Changes the time specification of the instance to be the same as that
     * of another KDateTime instance.
     *
     * Any previous time zone is forgotten. The stored date/time component of
     * the instance is left unchanged. Usually this method will change the
     * absolute time which this instance represents.
     *
     * @param other KDateTime instance whose time specification is to be copied
     * @see timeSpec(), timeZone(), setTimeSpec()
     */
    void setTimeSpec(const KDateTime &other);

    /**
     * Returns a date/time @p secs seconds later than the stored date/time.
     *
     * Except when the instance is a local clock time (type @c ClockTime), the
     * calculation is done in UTC to ensure that the result takes proper account
     * of clock changes (e.g. daylight savings) in the time zone. The result is
     * expressed using the same time specification as the original instance.
     *
     * Note that if the instance is a local clock time (type @c ClockTime), any
     * daylight savings changes or time zone changes during the period will
     * render the result inaccurate.
     *
     * If the instance is date-only, @p secs is rounded down to a whole number
     * of days and that value is added to the date to find the result.
     *
     * @return resultant date/time
     * @see addDays(), addMonths(), addYears(), secsTo()
     */
    KDateTime addSecs(int secs) const;

    /**
     * Returns a date/time @p days days later than the stored date/time.
     * The result is expressed using the same time specification as the
     * original instance.
     *
     * Note that if the instance is a local clock time (type @c ClockTime), any
     * daylight savings changes or time zone changes during the period may
     * render the result inaccurate.
     *
     * @return resultant date/time
     * @see addSecs(), addMonths(), addYears(), daysTo()
     */
    KDateTime addDays(int days) const;

    /**
     * Returns a date/time @p months months later than the stored date/time.
     * The result is expressed using the same time specification as the
     * original instance.
     *
     * Note that if the instance is a local clock time (type @c ClockTime), any
     * daylight savings changes or time zone changes during the period may
     * render the result inaccurate.
     *
     * @return resultant date/time
     * @see addSecs(), addDays(), addYears(), daysTo()
     */
    KDateTime addMonths(int months) const;

    /**
     * Returns a date/time @p years years later than the stored date/time.
     * The result is expressed using the same time specification as the
     * original instance.
     *
     * Note that if the instance is a local clock time (type @c ClockTime), any
     * daylight savings changes or time zone changes during the period may
     * render the result inaccurate.
     *
     * @return resultant date/time
     * @see addSecs(), addDays(), addMonths(), daysTo()
     */
    KDateTime addYears(int years) const;

    /**
     * Returns the number of seconds from this date/time to the @p other date/time.
     *
     * Before performing the comparison, the two date/times are converted to UTC
     * to ensure that the result is correct if one of the two date/times has
     * daylight saving time (DST) and the other doesn't. The exception is when
     * both instances are local clock time, in which case no conversion to UTC
     * is done.
     *
     * Note that if either instance is a local clock time (type @c ClockTime),
     * the result cannot be guaranteed to be accurate, since by definition they
     * contain no information about time zones or daylight savings changes.
     *
     * If one instance is date-only and the other is date-time, the date-time
     * value is first converted to the same time specification as the date-only
     * value, and the result is the difference in days between the resultant
     * date and the date-only date.
     *
     * If both instances are date-only, the result is the difference in days
     * between the two dates, ignoring time zones.
     *
     * @param other other date/time
     * @return number of seconds difference
     * @see addSecs(), daysTo()
     */
    int secsTo(const KDateTime &other) const;

    /**
     * Calculates the number of days from this date/time to the @p other date/time.
     * In calculating the result, @p other is first converted to this instance's
     * time zone. The number of days difference is then calculated ignoring
     * the time parts of the two date/times. For example, if this date/time
     * was 13:00 on 1 January 2000, and @p other was 02:00 on 2 January 2000,
     * the result would be 1.
     *
     * Note that if either instance is a local clock time (type @c ClockTime),
     * the result cannot be guaranteed to be accurate, since by definition they
     * contain no information about time zones or daylight savings changes.
     *
     * If one instance is date-only and the other is date-time, the date-time
     * value is first converted to the same time specification as the date-only
     * value, and the result is the difference in days between the resultant
     * date and the date-only date.
     *
     * If both instances are date-only, the calculation ignores time zones.
     *
     * @param other other date/time
     * @return number of days difference
     * @see secsTo(), addDays()
     */
    int daysTo(const KDateTime &other) const;

    /**
     * Returns the current date and time, as reported by the system clock,
     * expressed in the local system time zone.
     *
     * @return current date/time
     */
    static KDateTime currentDateTime();

    /**
     * Returns the date/time as a string. The @p format parameter determines the
     * format of the result string. The @p format codes used for the date and time
     * components follow those used elsewhere in KDE, and are similar but not
     * identical to those used by strftime(3). Conversion specifiers are
     * introduced by a '%' character, and are replaced in @p format as follows:
     *
     * Date
     * ====
     * %y   2-digit year excluding century (00 - 99)
     * %Y   full year number
     * %:m  month number, without leading zero (1 - 12)
     * %m   month number, 2 digits (01 - 12)
     * %b   abbreviated month name in current locale
     * %B   full month name in current locale
     * %:b  abbreviated month name in English (Jan, Feb, ...)
     * %:B  full month name in English
     * %e   day of the month (1 - 31)
     * %d   day of the month, 2 digits (01 - 31)
     * %a   abbreviated weekday name in current locale
     * %A   full weekday name in current locale
     * %:a  abbreviated weekday name in English (Mon, Tue, ...)
     * %:A  full weekday name in English
     *
     * Time
     * ====
     * %H   hour in the 24 hour clock, 2 digits (00 - 23)
     * %k   hour in the 24 hour clock, without leading zero (0 - 23)
     * %I   hour in the 12 hour clock, 2 digits (01 - 12)
     * %l   hour in the 12 hour clock, without leading zero (1 - 12)
     * %M   minute, 2 digits (00 - 59)
     * %S   seconds (00 - 59)
     * %:S  seconds preceded with ':', but omitted if seconds value is zero
     * %:s  milliseconds, 3 digits (000 - 999)
     * %P   "am" or "pm" in the current locale, or if undefined there, in English
     * %p   "AM" or "PM" in the current locale, or if undefined there, in English
     * %:P  "am" or "pm"
     * %:p  "AM" or "PM"
     *
     * Time zone
     * =========
     * %:u  UTC offset of the time zone in hours, e.g. -02. If the offset
     *      is not a whole number of hours, the output is the same as for '%U'.
     * %z   UTC offset of the time zone in hours and minutes, e.g. -0200.
     * %:z  UTC offset of the time zone in hours and minutes, e.g. +02:00.
     * %Z   time zone abbreviation, e.g. UTC, EDT, GMT. This is not guaranteed
     *      to be unique among different time zones. If not applicable (i.e. if
     *      the instance is type OffsetFromUTC), the UTC offset is substituted.
     * %:Z  time zone name, e.g. Europe/London. This is system dependent. If
     *      not applicable (i.e. if the instance is type OffsetFromUTC), the
     *      UTC offset is substituted.
     *
     * %%   literal '%' character
     *
     * Note that if the instance has a time specification of ClockTime, the
     * time zone or UTC offset in the result will be blank.
     *
     * If you want to use the current locale's date format, you should call
     * KLocale::formatDate() to format the date part of the KDateTime.
     *
     * @param format format for the string
     * @return formatted string
     * @see fromString(), KLocale::formatDate()
     */
    QString toString(const QString &format) const;

    /**
     * Returns the date/time as a string, formatted according to the @p format
     * parameter, with the UTC offset appended.
     *
     * Note that if the instance has a time specification of ClockTime, the UTC
     * offset in the result will be blank, except for RFC 2822 format in which
     * it will be the offset for the local system time zone.
     *
     * If the instance is date-only, the time will when @p format permits be
     * omitted from the output string. This applies to @p format = QtTextDate
     * or LocalDate. It also applies to @p format = ISODate when the instance
     * has a time specification of ClockTime. For all other cases, a time of
     * 00:00:00 will be output.
     *
     * For RFC 2822 format, set @p format to RFCDateDay to include the day
     * of the week, or to RFCDate to omit it.
     *
     * @param format format for output string
     * @return formatted string
     * @see fromString(), QDateTime::toString()
     */
    QString toString(TimeFormat format = ISODate) const;

    /**
     * Returns the KDateTime represented by @p string, using the @p format given.
     *
     * This method is the inverse of toString(TimeFormat), except that it can
     * only return a time specification of UTC, OffsetFromUTC or ClockTime. An
     * actual named time zone cannot be returned since an offset from UTC only
     * partially specifies a time zone.
     *
     * The time specification of the result is determined by the UTC offset
     * present in the string:
     * - if the UTC offset is zero the result is type @c UTC.
     * - if the UTC offset is non-zero, the result is type @c OffsetFromUTC.
     * - if there is no UTC offset, the result is by default type
     *   @c ClockTime. You can use setFromStringDefault() to change this default.
     *
     * If no time is found in @p string, a date-only value is returned, except
     * when the specified @p format does not permit the time to be omitted, in
     * which case an error is returned. An error is therefore returned for
     * ISODate when @p string includes a time zone specification, and for
     * RFCDate in all cases.
     *
     * For RFC format strings, you should normally set @p format to
     * RFCDate. Only set it to RFCDateDay if you want to return an error
     * when the day of the week is omitted.
     *
     * For @p format = ISODate or RFCDate[Day], if an invalid KDateTime is
     * returned, you can check why @p format was considered invalid by use of
     * isTooEarly() or isTooLate(). If either of these methods returns true, it
     * indicates that @p format was in fact valid, but the date lies outside
     * the range which can be represented by QDate.
     *
     * @param string string to convert
     * @param format format code. LocalDate cannot be used here.
     * @param negZero if non-null, the value is set to true if a UTC offset of
     *                '-0000' is found or, for RFC 2822 format, an unrecognised
     *                or invalid time zone abbreviation is found, else false.
     * @return KDateTime value, or an invalid KDateTime if either parameter is invalid
     * @see setFromStringDefault(), toString(), isTooEarly(), isTooLate(), QString::fromString()
     */
    static KDateTime fromString(const QString &string, TimeFormat format = ISODate, bool *negZero = 0);

    /**
     * Returns the KDateTime represented by @p string, using the @p format
     * given, optionally using a time zone collection @p zones as the source of
     * time zone definitions. The @p format codes are basically the same as
     * those for toString(), and are similar but not identical to those used by
     * strftime(3).
     *
     * The @p format string consists of the same codes as that for
     * toString(). However, some codes which are distinct in toString() have
     * the same function as each other here.
     *
     * All numeric values permit, but do not require, leading zeroes. The
     * maximum number of digits consumed by a numeric code is the minimum needed
     * to cover the possible range of the number (e.g. for minutes, the range is
     * 0 - 59, so the maximum number of digits consumed is 2). All non-numeric
     * values are case insensitive.
     *
     * Date
     * ====
     * %y   year excluding century (0 - 99). Years 0 - 50 return 2000 - 2050,
     *      while years 51 - 99 return 1951 - 1999.
     * %Y   full year number
     * %:m
     * %m   month number (1 - 12)
     * %b
     * %B   month name in the current locale or, if no match, in English,
     *      abbreviated or in full
     * %:b
     * %:B  month name in English, abbreviated or in full
     * %e
     * %d   day of the month (1 - 31)
     * %a
     * %A   weekday name in the current locale or, if no match, in English,
     *      abbreviated or in full
     * %:a
     * %:A  weekday name in English, abbreviated or in full
     *
     * Time
     * ====
     * %H
     * %k   hour in the 24 hour clock (0 - 23)
     * %I
     * %l   hour in the 12 hour clock (1 - 12)
     * %M   minute (0 - 59)
     * %S   seconds (0 - 59)
     * %:S  optional seconds value (0 - 59) preceded with ':'. If no colon is
     *      found in @p string, no input is consumed and the seconds value is
     *      set to zero.
     * %:s  fractional seconds value, preceded with a decimal point (either '.'
     *      or the locale's decimal point symbol)
     * %P
     * %p   "am" or "pm", in the current locale or, if no match, in
     *      English. This format is only useful when used with %I or %l.
     * %:P
     * %:p  "am" or "pm" in English. This format is only useful when used with
     *      %I or %l.
     *
     * Time zone
     * =========
     * %:u
     * %z   UTC offset of the time zone in hours and optionally minutes,
     *      e.g. -02, -0200.
     * %:z  UTC offset of the time zone in hours and minutes, colon separated,
     *      e.g. +02:00.
     * %Z   time zone abbreviation, consisting of alphanumeric characters,
     *      e.g. UTC, EDT, GMT.
     * %:Z  time zone name, e.g. Europe/London. The name may contain any
     *      characters and is delimited by the following character in the
     *      @p format string. It will not work if you follow %:Z with another
     *      escape sequence (except %% or %t).
     *
     * Other
     * =====
     * %t   matches one or more whitespace characters
     * %%   literal '%' character
     *
     * Any other character must have a matching character in @p string, except
     * that a space will match zero or more whitespace characters in the input
     * string.
     *
     * If any time zone information is present in the string, the function
     * attempts to find a matching time zone in the @p zones collection. A time
     * zone name (format code %:Z) will provide an unambiguous look up in
     * @p zones. Any other type of time zone information (an abbreviated time
     * zone code (%Z) or UTC offset (%z, %:z, %:u) is searched for in @p zones
     * and if only one time zone is found to match, the result is set to that
     * zone. Otherwise:
     * - If more than one match of a UTC offset is found, the action taken is
     *   determined by @p offsetIfAmbiguous: if @p offsetIfAmbiguous is true,
     *   a local time with an offset from UTC (type @c OffsetFromUTC) will be
     *   returned; if false an invalid KDateTime is returned.
     * - If more than one match of a time zone abbreviation is found, the UTC
     *   offset for each matching time zone is compared and, if the offsets are
     *   the same, a local time with an offset from UTC (type @c OffsetFromUTC)
     *   will be returned provided that @p offsetIfAmbiguous is true. Otherwise
     *   an invalid KDateTime is returned.
     * - If a time zone abbreviation does not match any time zone in @p zones,
     *   or the abbreviation does not apply at the parsed date/time, an
     *   invalid KDateTime is returned.
     * - If a time zone name does not match any time zone in @p zones, an
     *   invalid KDateTime is returned.
     * - If the time zone UTC offset does not match any time zone in @p zones,
     *   a local time with an offset from UTC (type @c OffsetFromUTC) is
     *   returned.
     * If @p format contains more than one time zone or UTC offset code, an
     * error is returned.
     *
     * If no time zone information is present in the string, by default a local
     * clock time (type @c ClockTime) is returned. You can use
     * setFromStringDefault() to change this default.
     *
     * If no time is found in @p string, a date-only value is returned.
     *
     * If any inconsistencies are found, i.e. the same item of information
     * appears more than once but with different values, the weekday name does
     * not tally with the date, an invalid KDateTime is returned.
     *
     * If an invalid KDateTime is returned, you can check why @p format was
     * considered invalid by use of isTooEarly() or isTooLate(). If either of
     * these methods returns true, it indicates that @p format was in fact
     * valid, but the date lies outside the range which can be represented by
     * QDate.
     *
     * @param string string to convert
     * @param format format string
     * @param zones time zone collection, or null for none
     * @param offsetIfAmbiguous specifies what to do if more than one zone
     *                          matches the UTC offset found in the
     *                          string. Ignored if @p zones is null.
     * @return KDateTime value, or an invalid KDateTime if an error occurs, if
     *         time zone information doesn't match any in @p zones, or if the
     *         time zone information is ambiguous and @p offsetIfAmbiguous is
     *         false
     * @see setFromStringDefault(), toString(), isTooEarly(), isTooLate()
     */
    static KDateTime fromString(const QString &string, const QString &format,
                                const KTimezones *zones = 0, bool offsetIfAmbiguous = true);

    /**
     * Sets the default time specification for use by fromString() when no time
     * zone or UTC offset is found in the string being parsed, or when "-0000"
     * is found in an RFC 2822 string.
     *
     * By default, fromString() returns a local clock time (type @c ClockTime)
     * when no definite zone or UTC offset is found. You can use this method
     * or setFromStringDefault(const KTimezone*) to make it return the local
     * time zone, UTC, or whatever you wish.
     *
     * @param spec the new default time specification. Note that @c TimeZone
     *             cannot be used here.
     * @param utcOffset number of seconds to add to UTC to get the local
     *                  time. Ignored if @p spec is not @c OffsetFromUTC.
     * @see setFromStringDefault(const KTimezone*), fromString()
     */
    static void setFromStringDefault(TimeSpec spec, int utcOffset = 0);

    /**
     * Sets the default time zone for use by fromString() when no time zone or
     * UTC offset is found in the string being parsed, or when "-0000" is found
     * in an RFC 2822 string.
     *
     * By default, fromString() returns a local clock time (type @c ClockTime)
     * when no definite zone or UTC offset is found. You can use this method
     * or setFromStringDefault(TimeSpec) to make it return the local time zone,
     * UTC, or whatever you wish.
     *
     * @param tz the new default time zone
     * @see setFromStringDefault(TimeSpec), fromString()
     */
    static void setFromStringDefault(const KTimezone *tz);

    /**
     * Returns whether the date/time is invalid because an otherwise valid date
     * is too early to be represented by QDate. This status only occurs when
     * fromString() has read a valid string containing a date earlier than
     * 1752. If isTooEarly() returns @c true, isValid() will return @c false.
     *
     * @return @c true if date was earlier than 1752, else @c false
     * @see isValid(), fromString()
     */
    bool isTooEarly() const;

    /**
     * Returns whether the date/time is invalid because an otherwise valid date
     * is too late to be represented by QDate. This status only occurs when
     * fromString() has read a valid string containing a date later than
     * 7999. If isTooLate() returns @c true, isValid() will return @c false.
     *
     * @return @c true if date was later than 7999, else @c false
     * @see isValid(), fromString()
     */
    bool isTooLate() const;

    /**
     * Compare this instance with another to determine whether they are
     * simultaneous, earlier or later, and in the case of date-only values,
     * whether they overlap (i.e. partly coincide but are not wholly
     * simultaneous).
     * The comparison takes time zones into account; if the two instances have
     * different time zones, they are first converted to UTC before comparing.
     *
     * If both instances are date/time values, this instance is considered to
     * be either simultaneous, earlier or later, and does not overlap.
     *
     * If one instance is date-only and the other is a date/time, this instance
     * is either strictly earlier, strictly later, or overlaps.
     *
     * If both instance are date-only, they are considered simultaneous if both
     * their start of day  and end of day times are simultaneous with each
     * other. (Both start and end of day times need to be considered in case a
     * daylight savings change occurs during that day.) Otherwise, this instance
     * can be strictly earlier, earlier but overlapping, later but overlapping,
     * or strictly later.
     *
     * Note that if either instance is a local clock time (type @c ClockTime),
     * the result cannot be guaranteed to be correct, since by definition they
     * contain no information about time zones or daylight savings changes.
     *
     * @return @c true if the two instances represent the same time, @c false otherwise
     */
    Comparison compare(const KDateTime &other) const;

    /**
     * Check whether this date/time is simultaneous with another.
     * The comparison takes time zones into account; if the two instances have
     * different time zones, they are first converted to UTC before comparing.
     *
     * Note that if either instance is a local clock time (type @c ClockTime),
     * the result cannot be guaranteed to be correct, since by definition they
     * contain no information about time zones or daylight savings changes.
     *
     * If one instance is date-only and the other date/time, the comparison
     * returns true if the date/time falls during the 24 hours of the day
     * represented by the date-only value. The comparison is performed by
     * converting the date/time value to the date-only value's time zone, and
     * comparing the resultant date with that of the date-only value's date.
     *
     * If both instances are date-only, the comparison is performed between the
     * two dates, ignoring time zones.
     *
     * @return @c true if the two instances represent the same time, @c false otherwise
     */
    bool operator==(const KDateTime &other) const;

    bool operator!=(const KDateTime &other) const { return !(*this == other); }

    /**
     * Check whether this date/time is earlier than another.
     * The comparison takes time zones into account; if the two instances have
     * different time zones, they are first converted to UTC before comparing.
     *
     * Note that if either instance is a local clock time (type @c ClockTime),
     * the result cannot be guaranteed to be correct, since by definition they
     * contain no information about time zones or daylight savings changes.
     *
     * If this instance is date-only and the other date/time, the comparison
     * returns true if the date/time falls after the end of this instance's
     * day. The comparison is performed by converting the date/time value to
     * this instance's time zone, and comparing the resultant date with this
     * instance's date.
     *
     * If this instance is a date/time value and the other is date-only, the
     * comparison returns true if this date/time falls before the start of the
     * date-only day. The comparison is performed by converting this date/time
     * value to the other instance's time zone, and comparing the resultant
     * date with the other instance's date.
     *
     * If both instances are date-only, the comparison is performed between the
     * two dates, ignoring time zones.
     *
     * @return @c true if this instance represents an earlier time than @p other,
     *         @c false otherwise
     */
    bool operator<(const KDateTime &other) const;

    bool operator<=(const KDateTime &other) const { return !(other < *this); }
    bool operator>(const KDateTime &other) const { return other < *this; }
    bool operator>=(const KDateTime &other) const { return !(*this < other); }

    friend QDataStream &operator<<(QDataStream &out, const KDateTime &dateTime);
    friend QDataStream &operator>>(QDataStream &in, KDateTime &dateTime);

  private:
    KDateTimePrivate *d;
};

QDataStream &operator<<(QDataStream &out, const KDateTime &dateTime);
QDataStream &operator>>(QDataStream &in, KDateTime &dateTime);

#endif
