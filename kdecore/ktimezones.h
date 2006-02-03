/*
   This file is part of the KDE libraries
   Copyright (c) 2005 David Jarvie <software@astrojar.org.uk>
   Copyright (c) 2005 S.R.Haque <srhaque@iee.org>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

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
 * Time zone functions
 * @author David Jarvie <software@astrojar.org.uk>.
 * @author S.R.Haque <srhaque@iee.org>.
 */

#ifndef _KTIMEZONES_H
#define _KTIMEZONES_H

#include <ctime>
#include <QDateTime>
#include <QMap>
#include <QList>
#include <QString>
#include <QByteArray>
#include "kdelibs_export.h"

class KTimezone;
class KTimezoneData;
class KTimezoneSource;
class KTimezonesPrivate;
class KTimezonePrivate;
class KTimezoneDataPrivate;
class KSystemTimezoneSource;
class KSystemTimezonePrivate;
class KSystemTimezonesPrivate;
class KSystemTimezoneSourcePrivate;
class KSystemTimezoneDataPrivate;

/** @defgroup timezones Time zone classes
 *
 * The time zone classes provide a framework for accessing time zone data, and
 * converting times and dates between different time zones. They provide access
 * to the system time zone database, and also allow developers to derive classes
 * to access custom sources of time zone information such as calendar files.
 *
 * A time zone is represented by the KTimezone class. This provides access to
 * the time zone's detailed definition and contains methods to convert times to
 * and from that zone. In order to save processing, KTimezone obtains its time
 * zone details only when they are actually required.
 *
 * A collection of time zones is represented by the KTimezones class, which acts
 * as a container of KTimezone objects. Within any KTimezones object, each
 * KTimezone instance is uniquely identified by its name. Typically, each
 * individual source of time zone information would be represented by a different
 * KTimezones object. This scheme allows conflicting time zone definitions
 * between the different sources to be handled, since KTimezone names need only
 * be unique within a single KTimezones object. Note that KTimezone instances do
 * not have to belong to any KTimezones container.
 *
 * Time zone source data can come in all sorts of different forms: TZFILE format
 * for a UNIX system time zone database, definitions within calendar files, calls
 * to libraries (e.g. libc), etc. The KTimezoneSource class provides reading and
 * parsing functions to access such data, handing off the parsed data for a
 * specific time zone in a KTimezoneData object. Both of these are base classes
 * from which should be derived other classes which know about the particular
 * access method and data format (KTimezoneSource) and which details are actually
 * provided (KTimezoneData). When a KTimezone instance needs its time zone's
 * definition, it calls KTimezoneSource::parse() and receives the data back in a
 * KTimezoneData object which it keeps for reference.
 *
 * The mapping of the different classes to external data is as follows:
 *
 * - Each different source data format or access method is represented by a
 *   different KTimezoneSource class.
 *
 * - Each different set of data provided from source data is represented by a
 *   different KTimezoneData class. For example, some time zone sources provide
 *   only the absolute basic information about time zones, i.e. name, transition
 *   times and offsets from UTC. Others provide information on leap second
 *   adjustments, while still others might contain information on which countries
 *   use the time zone. To allow for this variation, KTimezoneData is made
 *   available for inheritance.
 *
 * - Each KTimezoneData class will have a corresponding KTimezone class which
 *   can interpret its data.
 *
 * - Each different source database will typically be represented by a different
 *   KTimezones instance, to avoid possible conflicts between time zone definitions.
 *   If it is known that two source databases are definitely compatible, they can
 *   be grouped together into the same KTimezones instance.
 *
 *
 * \section sys System time zones
 *
 * Access to system time zones is provided by the KSystemTimezones class, which
 * reads the zone.tab file to obtain the list of system time zones, and creates a
 * KSystemTimezone instance for each one. KSystemTimezone uses the KSystemTimezoneSource
 * and KSystemTimezoneData classes to obtain time zone data via libc library
 * functions.
 *
 * Normally, KSystemTimezoneSource and KSystemTimezoneData operate in the
 * background and you will not need to use them directly.
 *
 *
 * \section tzfile Tzfile access
 *
 * The KTzfileTimezone class provides access to tzfile(5) time zone definition
 * files, which are used to form the time zone database on UNIX systems. Usually,
 * it is easier to use the KSystemTimezones class to access system tzfile data;
 * you can use the KTzfileTimezone class to obtain more detailed information or
 * to read non-system tzfile files.
 *
 * KTzfileTimezone uses the KTzfileTimezoneSource and KTzfileTimezoneData classes
 * to obtain time zone data from tzfile files.
 *
 *
 * \section deriving Handling time zone data from other sources
 *
 * To implement time zone classes to access a new time zone data source, you need
 * as a minimum to derive a new class from KTimezoneSource, and implement one or
 * more parse() methods. If can know in advance what KTimezone instances to create
 * without having to parse the source data, you should reimplement the virtual method
 * KTimezoneSource::parse(const KTimezone*). Otherwise, you need to define your
 * own parse() methods with appropriate signatures, to both read and parse the new
 * data, and create new KTimezone instances.
 *
 * If the data for each time zone which is available from the new source happens
 * to be the same as for another source for which KTimezone classes already exist,
 * you could simply use the existing KTimezone and KTimezoneData derived classes
 * to receive the parsed data from your new KTimezoneSource class:
 *
 * \code
 * class NewTimezoneSource : public KTimezoneSource
 * {
 *     public:
 *         NewTimezoneSource(...);  // parameters might include location of data source ...
 *         ~NewTimezoneSource();
 *
 *         // Option 1: reimplement KTimezoneSource::parse() if you can
 *         // pre-create the KTimezone instances.
 *         KTimezoneData *parse(const KTimezone *zone) const;
 *
 *         // Option 2: implement new parse() methods if you don't know
 *         // in advance what KTimezone instances to create.
 *         void parse(..., KTimezones *zones) const;
 *         NewTimezone *parse(...) const;
 * };
 *
 * // Option 1:
 * KTimezoneData *NewTimezoneSource::parse(const KTimezone *zone) const
 * {
 *     QString zoneName = zone->name();
 *     ExistingTimezoneData* data = new ExistingTimezoneData();
 *
 *     // Read the data for 'zoneName' from the new data source
 *
 *     // Parse what we have read, and write it into 'data'
 *
 *     return data;
 * }
 * \endcode
 *
 * If the data from the new source is different from what any existing
 * KTimezoneData class contains, you will need to implement new KTimezone and
 * KTimezoneData classes in addition to the KTimezoneSource class illustrated
 * above:
 *
 * \code
 * class NewTimezone : public KTimezone
 * {
 *     public:
 *         NewTimezone(NewTimezoneSource *source, const QString &name, ...);
 *         ~NewTimezone();
 *
 *         // Virtual methods which need to be reimplemented
 *         int offsetAtZoneTime(const QDateTime &zoneDateTime, int *secondOffset = 0) const;
 *         int offsetAtUTC(const QDateTime &utcDateTime) const;
 *         int offset(time_t t) const;
 *         int isDstAtUTC(const QDateTime &utcDateTime) const;
 *         bool isDst(time_t t) const;
 *
 *         // Anything else which you need
 * };
 *
 * class NewTimezoneData : public KTimezoneData
 * {
 *         friend class NewTimezoneSource;
 *
 *     public:
 *         NewTimezoneData();
 *         ~NewTimezoneData();
 *
 *         // Virtual methods which need to be reimplemented
 *         KTimezoneData *clone();
 *         QList<QByteArray> abbreviations() const;
 *         QByteArray abbreviation(const QDateTime &utcDateTime) const;
 *
 *         // Data members containing whatever is read by NewTimezoneSource
 * };
 * \endcode
 *
 * Here is a guide to implementing the offset() and offsetAtUTC() methods, in
 * the case where the source data does not use time_t for its time measurement:
 *
 * \code
 * int NewTimezone::offsetAtUTC(const QDateTime &utcDateTime) const
 * {
 *     // Access this time zone's data. If we haven't already read it,
 *     // force a read from source now.
 *     NewTimezoneData *zdata = data(true);
 *
 *     // Use 'zdata' contents to work out the UTC offset
 *
 *     return offset;
 * }
 *
 * int NewTimezone::offset(time_t t) const
 * {
 *     return offsetAtUTC(fromTime_t(t));
 * }
 * \endcode
 *
 * The other NewTimezone methods would work in an analogous way to
 * NewTimezone::offsetAtUTC() and NewTimezone::offset().
 */

/**
 * The KTimezones class represents a time zone database which consists of a
 * collection of individual time zone definitions.
 *
 * Each individual time zone is defined in a KTimezone instance, which provides
 * generic support for private or system time zones. The time zones in the
 * collection are indexed by name, which must be unique within the collection.
 * KTimezone instances in the collection are owned by the KTimezones instance,
 * and are deleted when the KTimezones instance is destructed.
 *
 * Different time zone sources can define the same time zone differently. (For
 * example, a calendar file originating from another system might hold its own
 * time zone definitions, which may not necessarily be identical to your own
 * system's definitions.) In order to keep conflicting definitions separate,
 * it will often be necessary when dealing with multiple time zone sources to
 * create a separate KTimezones instance for each source collection.
 *
 * If you want to access system time zones, use the KSystemTimezones class.
 *
 * @short Represents a time zone database or collection
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @author S.R.Haque <srhaque@iee.org>.
 * @since 3.5
 */
class KDECORE_EXPORT KTimezones
{
public:
    KTimezones();
    ~KTimezones();

    /**
     * Returns the time zone with the given name.
     * Note that the KTimezone returned remains a member of the KTimezones
     * collection, and should not be deleted without calling detach() first.
     *
     * @param name name of time zone
     * @return time zone, or 0 if not found
     */
    const KTimezone *zone(const QString &name) const;

    typedef QMap<QString, const KTimezone*> ZoneMap;

    /**
     * Returns all the time zones defined in this collection.
     *
     * @return time zone collection
     */
    const ZoneMap zones() const;

    /**
     * Adds a time zone to the collection.
     * KTimezones takes ownership of the KTimezone instance, which will be deleted
     * when the KTimezones instance is destructed.
     * The time zone's name must be unique within the collection.
     *
     * @param zone time zone to add
     * @return @c true if successful, @c false if zone's name duplicates one already in the collection
     */
    bool add(KTimezone *zone);

    /**
     * Adds a time zone to the collection.
     * KTimezones does not take ownership of the KTimezone instance.
     * The time zone's name must be unique within the collection.
     *
     * @param zone time zone to add
     * @return @c true if successful, @c false if zone's name duplicates one already in the collection
     */
    bool addConst(const KTimezone *zone);

    /**
     * Removes a time zone from the collection.
     * The caller assumes responsibility for deleting the removed KTimezone. If
     * the removed KTimezone was created by the caller, the constness of the return
     * value may safely be cast away.
     *
     * @param zone time zone to remove
     * @return the time zone which was removed, or 0 if not found or not a deletable object
     */
    const KTimezone *detach(const KTimezone *zone);

    /**
     * Removes a time zone from the collection.
     * The caller assumes responsibility for deleting the removed KTimezone.
     *
     * @param name name of time zone to remove
     * @return the time zone which was removed, or 0 if not found or not a deletable object
     */
    const KTimezone *detach(const QString &name);

    /**
     * Returns a standard UTC time zone, with name "UTC".
     *
     * @note The KTimezone returned by this method does not belong to any
     * KTimezones collection, and is statically allocated and therefore cannot
     * be deleted and cannot be added to a KTimezones collection. Any KTimezones
     * instance may contain its own UTC KTimezone, but that will be a different
     * instance than this KTimezone.
     *
     * @return UTC time zone
     */
    static const KTimezone *utc();

private:
    KTimezones(const KTimezones &);              // prohibit copying
    KTimezones &operator=(const KTimezones &);   // prohibit copying

    KTimezonesPrivate *d;
};


/**
 * Base class representing a time zone.
 *
 * The KTimezone base class contains general descriptive data about the time zone, and
 * provides an interface for methods to read and parse time zone definitions, and to
 * translate between UTC and local time. Derived classes must implement these methods,
 * and may also hold the actual details of the dates and times of daylight savings
 * changes, offsets from UTC, etc. They should be tailored to deal with the type and
 * format of data held by a particular type of time zone database.
 *
 * If this class is instantiated, it represents the UTC time zone.
 *
 * KTimezone is designed to work in partnership with KTimezoneSource. KTimezone
 * provides access to individual time zones, while classes derived from
 * KTimezoneSource read and parse a particular format of time zone definition.
 * Because time zone sources can differ in what information they provide about time zones,
 * the parsed data retured by KTimezoneSource can vary between different sources,
 * resulting in the need to create different KTimezone classes to handle the data.
 *
 * KTimezone instances are often grouped into KTimezones collections. If a KTimezone is
 * part of such a collection, it is owned by the KTimezones instance and should not be
 * deleted.
 *
 * @short Base class representing a time zone
 * @see KTimezoneSource, KTimezoneData
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @author S.R.Haque <srhaque@iee.org>.
 * @since 3.5
 */
class KDECORE_EXPORT KTimezone
{
public:
    /**
     * Construct a UTC time zone.
     */
    explicit KTimezone(const QString &name = QLatin1String("UTC"));

    KTimezone(const KTimezone &);
    virtual ~KTimezone();

    KTimezone &operator=(const KTimezone &);

    /**
     * Returns the name of the time zone.
     * If it is held in a KTimezones container, the name is the time zone's unique
     * identifier within that KTimezones instance.
     *
     * @return name in system-dependent format
     */
    QString name() const;

    /**
     * Returns the two-letter country code of the time zone.
     *
     * @return ISO 3166 2-character country code, empty if unknown
     */
    QString countryCode() const;

    /**
     * Returns the latitude of the time zone.
     *
     * @return latitude in degrees, UNKNOWN if not known
     */
    float latitude() const;

    /**
     * Returns the latitude of the time zone.
     *
     * @return latitude in degrees, UNKNOWN if not known
     */
    float longitude() const;

    /**
     * Returns any comment for the time zone.
     *
     * @return comment, may be empty
     */
    QString comment() const;

    /**
     * Returns the list of time zone abbreviations used by the time zone.
     * This may include historical ones which are no longer in use or have
     * been superseded.
     *
     * @return list of abbreviations
     */
    QList<QByteArray> abbreviations() const;

    /**
     * Returns the time zone abbreviation current at a specified time.
     *
     * @param utcDateTime UTC date/time. An error occurs if
     *                    @p utcDateTime.timeSpec() is not Qt::UTC.
     * @return time zone abbreviation, or empty string if error
     */
    QByteArray abbreviation(const QDateTime &utcDateTime) const;

    /**
     * Returns the complete list of UTC offsets used by the time zone. This may
     * include historical ones which are no longer in use or have been
     * superseded.
     *
     * A UTC offset is the number of seconds which you must add to UTC to get
     * local time in this time zone.
     *
     * If due to the nature of the source data for the time zone, compiling a
     * complete list would require significant processing, an empty list is
     * returned instead.
     *
     * @return sorted list of UTC offsets, or empty list if not readily available.
     */
    QList<int> UTCOffsets() const;

    /**
     * Converts a date/time, which is interpreted as being local time in this
     * time zone, into local time in another time zone.
     *
     * @param newZone other time zone which the time is to be converted into
     * @param zoneDateTime local date/time. An error occurs if
     *                     @p zoneDateTime.timeSpec() is not Qt::LocalTime.
     * @return converted date/time, or invalid date/time if error
     */
    QDateTime convert(const KTimezone *newZone, const QDateTime &zoneDateTime) const;

    /**
     * Converts a date/time, which is interpreted as local time in this time
     * zone, into UTC.
     *
     * Because of daylight savings time shifts, the date/time may occur twice. In
     * such cases, this method returns the UTC time for the first occurrence.
     * If you need the UTC time of the second occurrence, use offsetAtZoneTime().
     *
     * @param zoneDateTime local date/time. An error occurs if
     *                     @p zoneDateTime.timeSpec() is not Qt::LocalTime.
     * @return UTC date/time, or invalid date/time if error
     */
    QDateTime toUTC(const QDateTime &zoneDateTime) const;

    /**
     * Converts a UTC date/time into local time in this time zone.
     *
     * @param utcDateTime UTC date/time. An error occurs if
     *                    @p utcDateTime.timeSpec() is not Qt::UTC.
     * @return local date/time, or invalid date/time if error
     */
    QDateTime toZoneTime(const QDateTime &utcDateTime) const;

    /**
     * Returns the current offset of this time zone to UTC or the local
     * system time zone. The offset is the number of seconds which you must
     * add to UTC or the local system time to get local time in this time zone.
     *
     * Take care if you cache the results of this routine; that would
     * break if the result were stored across a daylight savings change.
     *
     * @param basis Qt::UTC to return the offset to UTC, Qt::LocalTime
     *                  to return the offset to local system time
     * @return offset in seconds
     */
    int currentOffset(Qt::TimeSpec basis = Qt::UTC) const;

    /**
     * Returns the offset of this time zone to UTC at the given local date/time.
     * Because of daylight savings time shifts, the date/time may occur twice. Optionally,
     * the offsets at both occurrences of @p dateTime are calculated.
     *
     * The offset is the number of seconds which you must add to UTC to get
     * local time in this time zone.
     *
     * The base class implementation always returns 0.
     *
     * @param zoneDateTime the date/time at which the offset is to be calculated. This
     *                     is interpreted as a local time in this time zone. An error
     *                     occurs if @p zoneDateTime.timeSpec() is not Qt::LocalTime.
     * @param secondOffset if non-null, and the @p zoneDateTime occurs twice, receives the
     *                     UTC offset for the second occurrence. Otherwise, it is set
     *                     the same as the return value.
     * @return offset in seconds. If @p zoneDateTime occurs twice, it is the offset at the
     *         first occurrence which is returned.
     */
    virtual int offsetAtZoneTime(const QDateTime &zoneDateTime, int *secondOffset = 0) const;

    /**
     * Returns the offset of this time zone to UTC at the given UTC date/time.
     *
     * The offset is the number of seconds which you must add to UTC to get
     * local time in this time zone.
     *
     * The base class implementation always returns 0. Derived classes should always
     * reimplement both this method and offset(). If the derived class needs to work
     * in terms of time_t (as when accessing the system time functions, for example),
     * it should implement its offset calculations in offset() and reimplement this
     * method simply as
     * \code
     *     offset(toTime_t(utcDateTime));
     * \endcode
     *
     * @param utcDateTime the UTC date/time at which the offset is to be calculated.
     *                    An error occurs if @p utcDateTime.timeSpec() is not Qt::UTC.
     * @return offset in seconds, or 0 if error
     */
    virtual int offsetAtUTC(const QDateTime &utcDateTime) const;

    /**
     * Returns the offset of this time zone to UTC at a specified UTC time.
     *
     * The offset is the number of seconds which you must add to UTC to get
     * local time in this time zone.
     *
     * Note that time_t has a more limited range than QDateTime, so consider using
     * offsetAtUTC() instead.
     *
     * The base class implementation always returns 0. Derived classes should always
     * reimplement both this method and offsetAtUTC(). If the derived class can work
     * in terms of QDateTime rather than time_t, it should implement its offset
     * calculations in offsetAtUTC() and reimplement this
     * method simply as
     * \code
     *     offsetAtUTC(fromTime_t(t));
     * \endcode
     *
     * @param t the UTC time at which the offset is to be calculated, measured in seconds
     *          since 00:00:00 UTC 1st January 1970 (as returned by time(2))
     * @return offset in seconds, or 0 if error
     */
    virtual int offset(time_t t) const;

    /**
     * Returns whether daylight savings time is in operation at the given UTC date/time.
     *
     * The base class implementation always returns false. Derived classes should always
     * reimplement both this method and isDst(). If the derived class needs to work
     * in terms of time_t (as when accessing the system time functions, for example),
     * it should implement its offset calculations in isDst() and reimplement this
     * method simply as
     * \code
     *     isDst(toTime_t(utcDateTime));
     * \endcode
     *
     * @param utcDateTime the UTC date/time. An error occurs if
     *                    @p utcDateTime.timeSpec() is not Qt::UTC.
     * @return @c true if daylight savings time is in operation, @c false otherwise
     */
    virtual bool isDstAtUTC(const QDateTime &utcDateTime) const;

    /**
     * Returns whether daylight savings time is in operation at a specified UTC time.
     *
     * Note that time_t has a more limited range than QDateTime, so consider using
     * isDstAtUTC() instead.
     *
     * The base class implementation always returns false. Derived classes should always
     * reimplement both this method and isDstAtUTC(). If the derived class can work
     * in terms of QDateTime rather than time_t, it should implement its offset
     * calculations in isDstAtUTC() and reimplement this
     * method simply as
     * \code
     *     isDstAtUTC(fromTime_t(t));
     * \endcode
     *
     * @param t the UTC time, measured in seconds since 00:00:00 UTC 1st January 1970
     *          (as returned by time(2))
     * @return @c true if daylight savings time is in operation, @c false otherwise
     */
    virtual bool isDst(time_t t) const;

    /**
     * Returns the source reader/parser for the time zone's source database.
     *
     * @return reader/parser
     */
    KTimezoneSource *source() const;

    /**
     * Extracts time zone detail information for this time zone from the source database.
     *
     * @return @c false if the parse encountered errors, @c true otherwise
     */
    bool parse() const;

    /**
     * Converts a UTC time, measured in seconds since 00:00:00 UTC 1st January 1970
     * (as returned by time(2)), to a UTC QDateTime value.
     *
     * @return converted time, or QDateTime() if @p t < 0
     */
    static QDateTime fromTime_t(time_t t);

    /**
     * Converts a UTC QDateTime to a UTC time, measured in seconds since 00:00:00 UTC
     * 1st January 1970 (as returned by time(2)).
     *
     * @return converted time, or -1 if the date is out of range for time_t or
     *         @p utcDateTime.timeSpec() is not Qt::UTC 
     */
    static time_t toTime_t(const QDateTime &utcDateTime);

    /**
     * A representation for unknown locations; this is a float
     * that does not represent a real latitude or longitude.
     */
    static const float UNKNOWN;

protected:
    /**
     * Constructs a time zone.
     *
     * @param source      reader/parser for the database containing this time zone. This will
     *                    be an instance of a class derived from KTimezoneSource.
     * @param name        in system-dependent format. The name must be unique within any
     *                    KTimezones instance which contains this KTimezone.
     * @param countryCode ISO 3166 2-character country code, empty if unknown
     * @param latitude    in degrees (between -90 and +90), UNKNOWN if not known
     * @param longitude   in degrees (between -180 and +180), UNKNOWN if not known
     * @param comment     description of the time zone, if any
     */
    KTimezone(
        KTimezoneSource *source, const QString &name,
        const QString &countryCode = QString(), float latitude = UNKNOWN, float longitude = UNKNOWN,
        const QString &comment = QString());

    /**
     * Returns the detailed parsed data for the time zone.
     * This will return null unless either parse() has been called beforehand, or
     * @p create is true.
     *
     * @param create true to parse the zone's data first if not already parsed
     * @return pointer to data, or null if data has not been parsed
     */
    const KTimezoneData *data(bool create = false) const;

    /**
     * Sets the detailed parsed data for the time zone.
     *
     * @param data parsed data
     */
    void setData(KTimezoneData *data);

private:
    KTimezonePrivate *d;
};


/**
 * Base class representing a source of time zone information.
 *
 * Derive subclasses from KTimezoneSource to read and parse time zone details
 * from a time zone database or other source of time zone information. If can know
 * in advance what KTimezone instances to create without having to parse the source
 * data, you should reimplement the virtual method parse(const KTimezone*). Otherwise,
 * you need to define your own parse() methods with appropriate signatures, to both
 * read and parse the new data, and create new KTimezone instances.
 *
 * KTimezoneSource itself may be used as a dummy source which returns empty
 * time zone details.
 *
 * @short Base class representing a source of time zone information
 * @see KTimezone, KTimezoneData
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @author S.R.Haque <srhaque@iee.org>.
 * @since 3.5
 */
class KDECORE_EXPORT KTimezoneSource
{
public:
    KTimezoneSource()  {}
    virtual ~KTimezoneSource()  {}

    /**
     * Extracts detail information for one time zone from the source database.
     * In this base class, the method always succeeds and returns an empty data
     * instance. Derived classes should reimplement this method to return an
     * appropriate data class derived from KTimezoneData.
     *
     * @param zone the time zone for which data is to be extracted.
     * @return an instance of a class derived from KTimezoneData containing
     *         the parsed data. The caller is responsible for deleting the
     *         KTimezoneData instance.
     *         Null is returned on error.
     */
    virtual KTimezoneData *parse(const KTimezone *zone) const;
};


/**
 * Base class for the parsed data returned by a KTimezoneSource class.
 *
 * This base class can be instantiated, but contains no data.
 *
 * @short Base class for parsed time zone data
 * @see KTimezone, KTimezoneSource
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @since 4.0
 */
class KDE_EXPORT KTimezoneData
{
public:
    KTimezoneData();
    virtual ~KTimezoneData();

    /**
     * Creates a new copy of this object.
     * The caller is responsible for deleting the copy.
     * Derived classes must reimplement this method to return a copy of the
     * calling instance.
     *
     * @return copy of this instance
     */
    virtual KTimezoneData *clone();

    /**
     * Returns the complete list of time zone abbreviations.
     *
     * @return the list of abbreviations.
     *         In this base class, it consists of the single string "UTC".
     */
    virtual QList<QByteArray> abbreviations() const;

    /**
     * Returns the time zone abbreviation current at a specified time.
     *
     * @param utcDateTime UTC date/time. An error occurs if
     *                    @p utcDateTime.timeSpec() is not Qt::UTC.
     * @return time zone abbreviation, or empty string if error
     */
    virtual QByteArray abbreviation(const QDateTime &utcDateTime) const;

    /**
     * Returns the complete list of UTC offsets for the time zone, if the time
     * zone's source makes such information readily available. If compiling a
     * complete list would require significant processing, an empty list is
     * returned instead.
     *
     * @return sorted list of UTC offsets, or empty list if not readily available.
     *         In this base class, it consists of the single value 0.
     */
    virtual QList<int> UTCOffsets() const;

private:
    KTimezoneDataPrivate *d;
};


/**
 * The KSystemTimezones class represents the system time zone database, consisting
 * of a collection of individual system time zone definitions, indexed by name.
 * Each individual time zone is defined in a KSystemTimezone instance. Additional
 * time zones (of any class derived from KTimezone) may be added if desired.
 *
 * At initialisation, KSystemTimezones reads the zone.tab file to obtain the list
 * of system time zones, and creates a KSystemTimezone instance for each one.
 *
 * Note that KSystemTimezones is not derived from KTimezones, but instead contains
 * a KTimezones instance which holds the system time zone database. Convenience
 * static methods are defined to access its data, or alternatively you can access
 * the KTimezones instance directly via the timezones() method.
 *
 * As an example, find the local time in Oman corresponding to the local system
 * time of 12:15:00 on 13th November 1999:
 * \code
 * QDateTime sampleTime(QDate(1999,11,13), QTime(12,15,0), Qt::LocalTime);
 * const KTimezone *local = KSystemTimezones::local();
 * const KTimezone *oman  = KSystemTimezones::zone("Asia/Muscat");
 * QDateTime omaniTime = local->convert(oman, sampleTime);
 * \endcode
 *
 * @short System time zone access
 * @see KTimezones, KSystemTimezone, KSystemTimezoneSource
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @since 4.0
 */
class KDECORE_EXPORT KSystemTimezones
{
public:
    /**
     * Returns all the time zones defined in this collection.
     *
     * @return time zone collection
     */
    static const KTimezones::ZoneMap zones()   { return timezones()->zones(); }

    /**
     * Returns the time zone with the given name.
     *
     * @param name name of time zone
     * @return time zone (usually a KSystemTimezone instance), or 0 if not found
     */
    static const KTimezone *zone(const QString &name)   { return timezones()->zone(name); }

    /**
     * Returns the current local system time zone.
     *
     * The idea of this routine is to provide a robust lookup of the local time
     * zone. The problem is that on Unix systems, there are a variety of mechanisms
     * for setting this information, and no well defined way of getting it. For
     * example, if you set your time zone to "Europe/London", then the tzname[]
     * maintained by tzset() typically returns { "GMT", "BST" }. The point of
     * this routine is to actually return "Europe/London" (or rather, the
     * corresponding KTimezone).
     *
     * @return local system time zone. If necessary, we will use a series of
     *         heuristics which end by returning UTC. We will never return NULL.
     *         Note that if UTC is returned as a default, it may not belong to the
     *         the collection returned by KSystemTimezones::zones().
     */
    static const KTimezone *local();

    /**
     * Returns the location of the system time zone zoneinfo database.
     *
     * @return path of directory containing the zoneinfo database
     */
    static QString zoneinfoDir();

    /**
     * Returns the unique KTimezones instance containing the system time zones
     * collection. It is first created if it does not already exist.
     *
     * @return time zones.
     */
    static KTimezones *timezones();

private:
    KSystemTimezonesPrivate *d;
};


/**
 * The KSystemTimezone class represents a time zone in the system database.
 *
 * It works in partnership with the KSystemTimezoneSource class which reads and parses the
 * time zone definition files.
 *
 * Typically, instances are created and accessed via the KSystemTimezones class.
 *
 * @short System time zone
 * @see KSystemTimezones, KSystemTimezoneSource, KSystemTimezoneData
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @since 4.0
 */
class KDECORE_EXPORT KSystemTimezone : public KTimezone
{
public:

    /**
     * Creates a time zone.
     *
     * @param source      tzfile reader and parser
     * @param name        time zone's unique name
     * @param countryCode ISO 3166 2-character country code, empty if unknown
     * @param latitude    in degrees (between -90 and +90), UNKNOWN if not known
     * @param longitude   in degrees (between -180 and +180), UNKNOWN if not known
     * @param comment     description of the time zone, if any
     */
    KSystemTimezone(KSystemTimezoneSource *source, const QString &name,
        const QString &countryCode = QString(), float latitude = UNKNOWN, float longitude = UNKNOWN,
        const QString &comment = QString());

    ~KSystemTimezone();

    /**
     * Returns the offset of this time zone to UTC at the given local date/time.
     * Because of daylight savings time shifts, the date/time may occur twice. Optionally,
     * the offsets at both occurrences of @p dateTime are calculated.
     *
     * The offset is the number of seconds which you must add to UTC to get
     * local time in this time zone.
     *
     * @param zoneDateTime the date/time at which the offset is to be calculated. This
     *                     is interpreted as a local time in this time zone. An error
     *                     occurs if @p zoneDateTime.timeSpec() is not Qt::LocalTime.
     * @param secondOffset if non-null, and the @p zoneDateTime occurs twice, receives the
     *                     UTC offset for the second occurrence. Otherwise, it is set
     *                     the same as the return value.
     * @return offset in seconds. If @p zoneDateTime occurs twice, it is the offset at the
     *         first occurrence which is returned.
     */
    virtual int offsetAtZoneTime(const QDateTime &zoneDateTime, int *secondOffset = 0) const;

    /**
     * Returns the offset of this time zone to UTC at the given UTC date/time.
     *
     * The offset is the number of seconds which you must add to UTC to get
     * local time in this time zone.
     *
     * Note that system times are represented using time_t. An error occurs if the date
     * falls outside the range supported by time_t.
     *
     * @param utcDateTime the UTC date/time at which the offset is to be calculated.
     *                    An error occurs if @p utcDateTime.timeSpec() is not Qt::UTC.
     * @return offset in seconds, or 0 if error
     */
    virtual int offsetAtUTC(const QDateTime &utcDateTime) const;

    /**
     * Returns the offset of this time zone to UTC at a specified UTC time.
     *
     * The offset is the number of seconds which you must add to UTC to get
     * local time in this time zone.
     *
     * @param t the UTC time at which the offset is to be calculated, measured in seconds
     *          since 00:00:00 UTC 1st January 1970 (as returned by time(2))
     * @return offset in seconds, or 0 if error
     */
    virtual int offset(time_t t) const;

    /**
     * Returns whether daylight savings time is in operation at the given UTC date/time.
     *
     * Note that system times are represented using time_t. An error occurs if the date
     * falls outside the range supported by time_t.
     *
     * @param utcDateTime the UTC date/time. An error occurs if
     *                    @p utcDateTime.timeSpec() is not Qt::UTC.
     * @return @c true if daylight savings time is in operation, @c false otherwise
     */
    virtual bool isDstAtUTC(const QDateTime &utcDateTime) const;

    /**
     * Returns whether daylight savings time is in operation at a specified UTC time.
     *
     * @param t the UTC time, measured in seconds since 00:00:00 UTC 1st January 1970
     *          (as returned by time(2))
     * @return @c true if daylight savings time is in operation, @c false otherwise
     */
    virtual bool isDst(time_t t) const;

private:
    KSystemTimezonePrivate *d;
};


/**
 * A class to read and parse system time zone data.
 *
 * Access is performed via the system time zone library functions.
 *
 * @short Reads and parses system time zone data
 * @see KSystemTimezones, KSystemTimezone, KSystemTimezoneData
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @since 4.0
 */
class KDECORE_EXPORT KSystemTimezoneSource : public KTimezoneSource
{
public:
    /**
     * Constructs a system time zone source.
     */
    KSystemTimezoneSource();
    virtual ~KSystemTimezoneSource();

    /**
     * Extract detailed information for one time zone, via the system time zone
     * library functions.
     *
     * @param zone the time zone for which data is to be extracted
     * @return a KSystemTimezoneData instance containing the parsed data.
     *         The caller is responsible for deleting the KTimezoneData instance.
     *         Null is returned on error.
     */
    virtual KTimezoneData *parse(const KTimezone *zone) const;

    /**
     * Use in conjunction with endParseBlock() to improve efficiency when calling parse()
     * for a group of KSystemTimezone instances in succession.
     * Call startParseBlock() before the first parse(), and call endParseBlock() after the last.
     *
     * The effect of calling these methods is to save and restore the TZ environment variable
     * only once before and after the group of parse() calls, rather than before and
     * after every call. So, between calls to startParseBlock() and endParseBlock(), do not
     * call any functions which rely directly or indirectly on the local time zone setting.
     */
    static void startParseBlock();

    /**
     * @see startParseBlock()
     */
    static void endParseBlock();

private:
    KSystemTimezoneSourcePrivate *d;
};


/**
 * The parsed system time zone data returned by KSystemTimezoneSource.
 *
 * @short Parsed system time zone data
 * @see KSystemTimezoneSource, KSystemTimezone
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @since 4.0
 */
class KSystemTimezoneData : public KTimezoneData
{
    friend class KSystemTimezoneSource;

public:
    KSystemTimezoneData();
    /** Copy constructor; no special ownership assumed. */
    KSystemTimezoneData(const KSystemTimezoneData &);
    virtual ~KSystemTimezoneData();

    /** Assignment; no special ownership assumed. Everything is value based. */
    KSystemTimezoneData &operator=(const KSystemTimezoneData &);

    /**
     * Creates a new copy of this object.
     * The caller is responsible for deleting the copy.
     * Derived classes must reimplement this method to return a copy of the
     * calling instance 
     *
     * @return copy of this instance. This is a KSystemTimezoneData pointer.
     */
    virtual KTimezoneData *clone();

    /**
     * Returns the complete list of time zone abbreviations.
     *
     * @return the list of abbreviations
     */
    virtual QList<QByteArray> abbreviations() const;
    virtual QByteArray abbreviation(const QDateTime &utcDateTime) const;

    /**
     * Returns the complete list of UTC offsets for the time zone. For system
     * time zones, significant processing would be required to obtain such a
     * list, so instead an empty list is returned.
     *
     * @return empty list
     */
    virtual QList<int> UTCOffsets() const;

private:
    KSystemTimezoneDataPrivate *d;
};

#endif
