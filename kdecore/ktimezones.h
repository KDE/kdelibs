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
class KTzfileTimezoneSource;
class KTzfileTimezonePrivate;
class KTzfileTimezoneDataPrivate;
class KTzfileTimezoneSourcePrivate;

/** @defgroup timezones Timezone classes
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
 * data format (KTimezoneSource) and which details are actually provided
 * (KTimezoneData). When a KTimezone instance needs its time zone's
 * definition, it calls KTimezoneSource::parse() and receives the data back in a
 * KTimezoneData object.
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
 * Access to system time zones is provided by the KSystemTimezones class, which reads
 * the zone.tab file to obtain the list of system time zones, and creates a
 * KSystemTimezone instance for each one. KSystemTimezone uses the KSystemTimezoneSource
 * and KSystemTimezoneData classes to obtain time zone data via libc library functions.
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
    const KTimezone *zone(const QString &name);

    typedef QMap<QString, KTimezone*> ZoneMap;

    /**
     * Return all the time zones defined in this collection.
     *
     * @return time zone collection
     */
    const ZoneMap zones() const;

    /**
     * Add a time zone to the collection.
     * KTimezones takes ownership of the KTimezone instance, which will be deleted
     * when the KTimezones instance is destructed.
     * The time zone's name must be unique within the collection.
     *
     * @param zone time zone to add
     * @return @c true if successful, @c false if zone's name duplicates one already in the collection
     */
    bool add(KTimezone *zone);

    /**
     * Remove a time zone from the collection.
     * The caller assumes responsibility for deleting the removed KTimezone.
     *
     * @param zone time zone to remove
     * @return the time zone which was removed, or 0 if either not found or not a deletable object
     */
    KTimezone *detach(KTimezone *zone);

    /**
     * Remove a time zone from the collection.
     * The caller assumes responsibility for deleting the removed KTimezone.
     *
     * @param name name of time zone to remove
     * @return the time zone which was removed, or 0 either if not found or if not a deletable object
     */
    KTimezone *detach(const QString &name);

    /**
     * Return a standard UTC time zone, with name "UTC".
     *
     * @note The KTimezone returned by this method does not belong to any
     * KTimezones collection, and is statically allocated and therefore cannot
     * be deleted and cannot be added to a KTimezones collection. Any KTimezones
     * instance may contain its own UTC KTimezone, but that will be a different
     * instance than this KTimezone.
     *
     * @return UTC time zone
     */
    static KTimezone *utc();

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
    KTimezone(const QString &name = "UTC");

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
     * Convert a date/time, which is interpreted as being local time in this
     * time zone, into local time in another time zone.
     *
     * @param newZone other time zone which the time is to be converted into
     * @param zoneDateTime local date/time. An error occurs if
     *                     @p zoneDateTime.timeSpec() is not Qt::LocalTime.
     * @return converted date/time, or invalid date/time if error
     */
    QDateTime convert(const KTimezone *newZone, const QDateTime &zoneDateTime) const;

    /**
     * Convert a date/time, which is interpreted as local time in this time
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
     * Convert a UTC date/time into local time in this time zone.
     *
     * @param utcDateTime UTC date/time. An error occurs if
     *                    @p utcDateTime.timeSpec() is not Qt::UTC.
     * @return local date/time, or invalid date/time if error
     */
    QDateTime toZoneTime(const QDateTime &utcDateTime) const;

    /**
     * Returns the current offset of this time zone to UTC or the local
     * system time zone in seconds.
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
     * The base class implemetation always returns 0.
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
     * @param utcDateTime the UTC date/time at which the offset is to be calculated.
     *                    An error occurs if @p utcDateTime.timeSpec() is not Qt::UTC.
     * @return offset in seconds, or 0 if error
     */
    int offsetAtUTC(const QDateTime &utcDateTime) const;

    /**
     * Returns the offset of this time zone to UTC at a specified UTC time measured
     * in time_t (as returned by time(2)).
     * The base class implemetation always returns 0.
     *
     * @param t the UTC time at which the offset is to be calculated
     * @return offset in seconds, or 0 if error
     */
    virtual int offset(time_t t) const;

    /**
     * Returns whether daylight savings time is in operation at the given UTC date/time.
     *
     * @param utcDateTime the UTC date/time. An error occurs if
     *                    @p utcDateTime.timeSpec() is not Qt::UTC.
     * @return @c true if daylight savings time is in operation, @c false otherwise
     */
    bool isDstAtUTC(const QDateTime &utcDateTime) const;

    /**
     * Returns whether daylight savings time is in operation at a specified UTC time.
     *
     * @param t the UTC time in time_t units
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
     * Extract time zone detail information for this time zone from the source database.
     *
     * @return @c false if the parse encountered errors, @c true otherwise     */
    virtual bool parse() const;

    /**
     * A representation for unknown locations; this is a float
     * that does not represent a real latitude or longitude.
     */
    static const float UNKNOWN;

protected:
    /**
     * Construct a time zone.
     *
     * @param source      reader/parser for the database containing this time zone. This will
     *                    be an instance of a class derived from KTimezoneSource.
     * @param name        in system-dependent format. The name must be unique within any
     *                    KTimezones instance which contains this KTimezone.
     * @param countryCode ISO 3166 2-character country code, empty if unknown
     * @param latitude    in degrees, UNKNOWN if not known
     * @param longitude   in degrees, UNKNOWN if not known
     * @param comment     description of the time zone, if any
     */
    KTimezone(
        KTimezoneSource *source, const QString &name,
        const QString &countryCode = QString(), float latitude = UNKNOWN, float longitude = UNKNOWN,
        const QString &comment = QString());

    /**
     * Return the detailed parsed data for the time zone.
     * This will return null unless either parse() has been called beforehand, or
     * @p create is true.
     *
     * @param create true to parse the zone's data first if not already parsed
     * @return pointer to data, or null if data has not been parsed
     */
    const KTimezoneData *data(bool create = false) const;

private:
    KTimezonePrivate *d;
};


/**
 * Base class representing a source of time zone information.
 *
 * Derive subclasses from KTimezoneSource to read and parse time zone details
 * from a time zone database or other source of time zone information.
 *
 * KTimezoneSource itself may be used as a dummy source which returns empty
 * time zone details.
 *
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
    virtual ~KTimezoneSource()  {};

    /**
     * Extract detail information for one time zone from the source database.
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
 * @see KTimezone, KTimezoneSource
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @since 4.0
 */
class KTimezoneData
{
public:
    KTimezoneData();
    virtual ~KTimezoneData();

    /**
     * Create a new copy of this object.
     * The caller is responsible for deleting the copy.
     * Derived classes must reimplement this method to return a copy of the
     * calling instance.
     *
     * @return copy of this instance
     */
    virtual KTimezoneData *clone();

    /** Returns the list of time zone abbreviations.
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
 * @see KTimezones, KSystemTimezone, KSystemTimezoneSource
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @since 4.0
 */
class KDECORE_EXPORT KSystemTimezones
{
public:
    /**
     * Return all the time zones defined in this collection.
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
     * Returns the current local system time zone. The idea of this routine is to provide
     * a robust lookup of the local time zone.
     *
     * The problem is that on Unix systems, there are a variety of mechanisms for
     * setting this information, and no well defined way of getting it. For example,
     * if you set your time zone to "Europe/London", then the tzname[]
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
 * @see KSystemTimezones, KSystemTimezoneSource, KSystemTimezoneData
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @since 4.0
 */
class KDECORE_EXPORT KSystemTimezone : public KTimezone
{
public:

    /**
     * Create a time zone.
     *
     * @param source      tzfile reader and parser
     * @param name        time zone's unique name
     * @param countryCode ISO 3166 2-character country code, empty if unknown
     * @param latitude    in degrees, UNKNOWN if not known
     * @param longitude   in degrees, UNKNOWN if not known
     * @param comment     description of the time zone, if any
     */
    KSystemTimezone(KSystemTimezoneSource *source, const QString &name,
        const QString &countryCode = QString(), float latitude = UNKNOWN, float longitude = UNKNOWN,
        const QString &comment = QString());

    ~KSystemTimezone();

    virtual int offsetAtZoneTime(const QDateTime &zoneDateTime, int *secondOffset = 0) const;
    virtual int offset(time_t t) const;
    virtual bool isDst(time_t t) const;

private:
    KSystemTimezonePrivate *d;
};


/**
 * A class to read and parse system time zone data.
 *
 * Access is performed via the system time zone library functions.
 *
 * @see KSystemTimezones, KSystemTimezone, KSystemTimezoneData
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @since 4.0
 */
class KDECORE_EXPORT KSystemTimezoneSource : public KTimezoneSource
{
public:
    /**
     * Construct a system time zone source.
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
 * The parsed data returned by KSystemTimezoneSource.
 *
 * @see KSystemTimezoneSource
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @since 4.0
 */
class KSystemTimezoneData : public KTimezoneData
{
    friend class KSystemTimezoneSource;

public:
    KSystemTimezoneData();
    KSystemTimezoneData(const KSystemTimezoneData &);
    virtual ~KSystemTimezoneData();

    KSystemTimezoneData &operator=(const KSystemTimezoneData &);

    /**
     * Create a new copy of this object.
     * The caller is responsible for deleting the copy.
     * Derived classes must reimplement this method to return a copy of the
     * calling instance 
     *
     * @return copy of this instance. This is a KSystemTimezoneData pointer.
     */
    virtual KTimezoneData *clone();

    virtual QList<QByteArray> abbreviations() const;
    virtual QByteArray abbreviation(const QDateTime &utcDateTime) const;

private:
    KSystemTimezoneDataPrivate *d;
};


/**
 * The KTzfileTimezone class represents a time zone defined in tzfile(5) format.
 *
 * It works in partnership with the KTzfileTimezoneSource class which reads and parses the
 * time zone definition files.
 *
 * @see KTzfileTimezoneSource
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @since 4.0
 */
class KDECORE_EXPORT KTzfileTimezone : public KTimezone
{
public:
    /** Time adjustment details */
    struct Adjustment
    {
        int        utcOffset;   /**< Number of seconds to be added to UTC. */
        bool       isDst;       /**< Whether tm_isdst should be set by localtime(3). */
        QByteArray abbrev;      /**< Time zone abbreviation. */
    };

    /**
     * Create a time zone.
     *
     * @param source      tzfile reader and parser
     * @param name        time zone's unique name
     * @param countryCode ISO 3166 2-character country code, empty if unknown
     * @param latitude    in degrees, UNKNOWN if not known
     * @param longitude   in degrees, UNKNOWN if not known
     * @param comment     description of the time zone, if any
     */
    KTzfileTimezone(KTzfileTimezoneSource *source, const QString &name,
        const QString &countryCode = QString(), float latitude = UNKNOWN, float longitude = UNKNOWN,
        const QString &comment = QString());

    ~KTzfileTimezone();

    virtual int offsetAtZoneTime(const QDateTime &dateTime, int *secondOffset) const;
    virtual int offset(time_t t) const;
    virtual bool isDst(time_t t) const;

    /**
     * Return time adjustment details for a given date and time.
     *
     * @param utcDateTime UTC date/time for which details are to be returned.
     *                    An error occurs if @p utcDateTime.timeSpec() is not Qt::UTC.
     * @param adjustment receives the time adjustment details
     * @return @c true if successful, @c false if error
     */
    bool transitionTime(const QDateTime &utcDateTime, Adjustment &adjustment);

private:
    KTzfileTimezonePrivate *d;
};


/**
 * A class to read and parse tzfile time zone definition files.
 *
 * tzfile is the format used by zoneinfo files in the system time zone database.
 * The format is documented in the tzfile(5) manpage.
 *
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @since 4.0
 */
class KDECORE_EXPORT KTzfileTimezoneSource : public KTimezoneSource
{
public:
    /**
     * Construct a time zone source.
     *
     * @param location the local directory containing the time zone definition files
     */
    KTzfileTimezoneSource(const QString &location);
    virtual ~KTzfileTimezoneSource();

    /**
     * Return the local directory containing the time zone definition files.
     *
     * @return path to time zone definition files
     */
    QString location();

    /**
     * Parse a tzfile file to extract detailed information for one time zone.
     *
     * @param zone the time zone for which data is to be extracted
     * @return a KTzfileTimezoneData instance containing the parsed data.
     *         The caller is responsible for deleting the KTimezoneData instance.
     *         Null is returned on error.
     */
    virtual KTimezoneData *parse(const KTimezone *zone) const;

private:
    KTzfileTimezoneSourcePrivate *d;
};


/**
 * The parsed data returned by KTzfileTimezoneSource.
 *
 * @see KTzfileTimezoneSource
 * @see KTzfileTimezone
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @since 4.0
 */
class KTzfileTimezoneData : public KTimezoneData
{
    friend class KTzfileTimezoneSource;

public:
    KTzfileTimezoneData();
    KTzfileTimezoneData(const KTzfileTimezoneData &);
    virtual ~KTzfileTimezoneData();

    KTzfileTimezoneData &operator=(const KTzfileTimezoneData &);

    /**
     * Create a new copy of this object.
     * The caller is responsible for deleting the copy.
     * Derived classes must reimplement this method to return a copy of the
     * calling instance 
     *
     * @return copy of this instance. This is a KTzfileTimezoneData pointer.
     */
    virtual KTimezoneData *clone();

    /** Details of a change in the rules for computing local time. */
    struct TransitionTime
    {
        Q_INT32 time;             /**< Time (as returned by time(2)) at which the rules for computing local time change. */
        Q_UINT8  localTimeIndex;  /**< Index into the LocalTimeType array. */
    };
    /** The characteristics of a local time type. */
    struct LocalTimeType
    {
        Q_INT32 gmtoff;     /**< Number of seconds to be added to UTC. */
        bool    isdst;      /**< Whether tm_isdst should be set by localtime(3). */
        Q_UINT8 abbrIndex;  /**< Index into the list of time zone abbreviations. */
    };
    /** Details of a leap second adjustment. */
    struct LeapSecondAdjust
    {
        Q_INT32 time;          /**< Time (as returned by time(2)) at which the leap second occurs. */
        Q_UINT32 leapSeconds;  /**< Total number of leap seconds to be applied after this time. */
    };

    /** The number of transition times in the array m_transitionTimes */
    Q_UINT32 nTransitionTimes() const   { return m_nTransitionTimes; }
    /** The number of local time types in the array m_localTimeTypes */
    Q_UINT32 nLocalTimeTypes() const   { return m_nLocalTimeTypes; }
    /** The number of leap seconds adjustments in the array m_leapSecondAdjusts */
    Q_UINT32 nLeapSecondAdjustments() const   { return m_nLeapSecondAdjusts; }
    /** The number of standard/wall indicators in the array m_isStandard */
    Q_UINT32 nIsStandard() const   { return m_nIsStandard; }
    /** The number of UTC/local indicators in the array m_isUTC */
    Q_UINT32 nIsUTC() const   { return m_nIsUTC; }

    const TransitionTime *transitionTime(int index) const;
    const LocalTimeType *localTimeType(int index) const;
    const LeapSecondAdjust *leapSecondAdjustment(int index) const;
    bool isStandard(int index) const;
    bool isUTC(int index) const;

    /**
     * Return the m_transitionTimes details for a given UTC time.
     *
     * @param t UTC time in time_t units
     * @return pointer to details, or 0 if the time is before the start of data
     */
    const TransitionTime *getTransitionTime(time_t t) const;

    /**
     * Return the m_localTimeTypes details for a given UTC time.
     *
     * @param t UTC time in time_t units
     * @return pointer to details, or 0 if the time is before the start of data
     */
    const LocalTimeType *getLocalTime(time_t t) const;

    /**
     * Return the number of leap seconds to be applied after a given UTC time.
     *
     * @param t UTC time in time_t units
     * @return adjustment, or 0 if the time is before the start of data
     */
    Q_UINT32 getLeapSeconds(time_t t) const;

    virtual QList<QByteArray> abbreviations() const;
    virtual QByteArray abbreviation(const QDateTime &utcDateTime) const;

    /**
     * Returns the time zone abbreviation at a given index.
     *
     * @param index abbreviation's index, as contained in LocalTimeType's abbrIndex element
     * @return time zone abbreviation, or empty string if invalid index
     */
    QByteArray abbreviation(int index) const;

private:
    Q_UINT32 m_nTransitionTimes;
    Q_UINT32 m_nLocalTimeTypes;
    Q_UINT32 m_nLeapSecondAdjusts;
    Q_UINT32 m_nIsStandard;
    Q_UINT32 m_nIsUTC;
    TransitionTime *m_transitionTimes;
    LocalTimeType  *m_localTimeTypes;
    QList<QByteArray> m_abbreviations;
    LeapSecondAdjust *m_leapSecondAdjusts;
    bool *m_isStandard;
    bool *m_isUTC;
    KTzfileTimezoneDataPrivate *d;
};

#endif
