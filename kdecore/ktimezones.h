/*
   This file is part of the KDE libraries
   Copyright (c) 2005,2006 David Jarvie <software@astrojar.org.uk>
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
#include <QtCore/QDateTime>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include "kdelibs_export.h"

class KTimeZone;
class KTimeZoneData;
class KTimeZoneSource;
class KTimeZonesPrivate;
class KTimeZonePrivate;
class KTimeZoneDataPrivate;
class KSystemTimeZoneSource;
class KSystemTimeZonePrivate;
class KSystemTimeZonesPrivate;
class KSystemTimeZoneSourcePrivate;
class KSystemTimeZoneDataPrivate;
class KTimeZonePhasePrivate;
class KTimeZoneLeapSecondsPrivate;

/** @defgroup timezones Time zone classes
 *
 * The time zone classes provide a framework for accessing time zone data, and
 * converting times and dates between different time zones. They provide access
 * to the system time zone database, and also allow developers to derive classes
 * to access custom sources of time zone information such as calendar files.
 *
 * A time zone is represented by the KTimeZone class. This provides access to
 * the time zone's detailed definition and contains methods to convert times to
 * and from that zone. In order to save processing, KTimeZone obtains its time
 * zone details only when they are actually required.
 *
 * A collection of time zones is represented by the KTimeZones class, which acts
 * as a container of KTimeZone objects. Within any KTimeZones object, each
 * KTimeZone instance is uniquely identified by its name. Typically, each
 * individual source of time zone information would be represented by a different
 * KTimeZones object. This scheme allows conflicting time zone definitions
 * between the different sources to be handled, since KTimeZone names need only
 * be unique within a single KTimeZones object. Note that KTimeZone instances do
 * not have to belong to any KTimeZones container.
 *
 * Time zone source data can come in all sorts of different forms: TZFILE format
 * for a UNIX system time zone database, definitions within calendar files, calls
 * to libraries (e.g. libc), etc. The KTimeZoneSource class provides reading and
 * parsing functions to access such data, handing off the parsed data for a
 * specific time zone in a KTimeZoneData object. Both of these are base classes
 * from which should be derived other classes which know about the particular
 * access method and data format (KTimeZoneSource) and which details are actually
 * provided (KTimeZoneData). When a KTimeZone instance needs its time zone's
 * definition, it calls KTimeZoneSource::parse() and receives the data back in a
 * KTimeZoneData object which it keeps for reference.
 *
 * KTimeZoneData holds the sequence of daylight savings time changes in
 * KTimeZonePhase objects, and leap seconds adjustments in KTimeZoneLeapSeconds
 * objects. You can access this data directly via KTimeZone methods if required.
 *
 * The mapping of the different classes to external data is as follows:
 *
 * - Each different source data format or access method is represented by a
 *   different KTimeZoneSource class.
 *
 * - Each different set of data provided from source data is represented by a
 *   different KTimeZoneData class. For example, some time zone sources provide
 *   only the absolute basic information about time zones, i.e. name, transition
 *   times and offsets from UTC. Others provide information on leap second
 *   adjustments, while still others might contain information on which countries
 *   use the time zone. To allow for this variation, KTimeZoneData is made
 *   available for inheritance. When the necessary information is not available,
 *   the KTimeZonePhase and KTimeZoneLeapSeconds data will be empty.
 *
 * - Each KTimeZoneData class will have a corresponding KTimeZone class which
 *   can interpret its data.
 *
 * - Each different source database will typically be represented by a different
 *   KTimeZones instance, to avoid possible conflicts between time zone definitions.
 *   If it is known that two source databases are definitely compatible, they can
 *   be grouped together into the same KTimeZones instance.
 *
 *
 * \section sys System time zones
 *
 * Access to system time zones is provided by the KSystemTimeZones class, which
 * reads the zone.tab file to obtain the list of system time zones, and creates a
 * KSystemTimeZone instance for each one. KSystemTimeZone uses the KSystemTimeZoneSource
 * and KSystemTimeZoneData classes to obtain time zone data via libc library
 * functions.
 *
 * Normally, KSystemTimeZoneSource and KSystemTimeZoneData operate in the
 * background and you will not need to use them directly.
 *
 * @warning The KSystemTimeZone class uses the standard system libraries to
 * access time zone data, and its functionality is limited to what these libraries
 * provide. On many systems, dates earlier than 1970 are not handled, and on
 * non-GNU systems there is no guarantee that the time zone abbreviation returned
 * for a given date will be correct if the abbreviations applicable then were
 * not those currently in use.
 *
 * \section tzfile Tzfile access
 *
 * The KTzfileTimeZone class provides access to tzfile(5) time zone definition
 * files, which are used to form the time zone database on UNIX systems. Usually,
 * for current information, it is easier to use the KSystemTimeZones class to
 * access system tzfile data. However, for dealing with past data the
 * KTzfileTimeZone class provides better guarantees of accurary, although it
 * cannot handle dates earlier than 1970. It also provides more detailed
 * information, and allows you to read non-system tzfile files.
 *
 * KTzfileTimeZone uses the KTzfileTimeZoneSource and KTzfileTimeZoneData classes
 * to obtain time zone data from tzfile files.
 *
 *
 * \section deriving Handling time zone data from other sources
 *
 * To implement time zone classes to access a new time zone data source, you need
 * as a minimum to derive a new class from KTimeZoneSource, and implement one or
 * more parse() methods. If you can know in advance what KTimeZone instances to create
 * without having to parse the source data, you should reimplement the virtual method
 * KTimeZoneSource::parse(const KTimeZone*). Otherwise, you need to define your
 * own parse() methods with appropriate signatures, to both read and parse the new
 * data, and create new KTimeZone instances.
 *
 * If the data for each time zone which is available from the new source happens
 * to be the same as for another source for which KTimeZone classes already exist,
 * you could simply use the existing KTimeZone and KTimeZoneData derived classes
 * to receive the parsed data from your new KTimeZoneSource class:
 *
 * \code
 * class NewTimeZoneSource : public KTimeZoneSource
 * {
 *     public:
 *         NewTimeZoneSource(...);  // parameters might include location of data source ...
 *         ~NewTimeZoneSource();
 *
 *         // Option 1: reimplement KTimeZoneSource::parse() if you can
 *         // pre-create the KTimeZone instances.
 *         KTimeZoneData *parse(const KTimeZone *zone) const;
 *
 *         // Option 2: implement new parse() methods if you don't know
 *         // in advance what KTimeZone instances to create.
 *         void parse(..., KTimeZones *zones) const;
 *         NewTimeZone *parse(...) const;
 * };
 *
 * // Option 1:
 * KTimeZoneData *NewTimeZoneSource::parse(const KTimeZone *zone) const
 * {
 *     QString zoneName = zone->name();
 *     ExistingTimeZoneData* data = new ExistingTimeZoneData();
 *
 *     // Read the data for 'zoneName' from the new data source.
 *
 *     // Parse what we have read, and write it into 'data'.
 *     // Compile the sequence of daylight savings changes and leap
 *     // seconds adjustments (if available) and write into 'data'.
 *
 *     return data;
 * }
 * \endcode
 *
 * If the data from the new source is different from what any existing
 * KTimeZoneData class contains, you will need to implement new KTimeZone and
 * KTimeZoneData classes in addition to the KTimeZoneSource class illustrated
 * above:
 *
 * \code
 * class NewTimeZone : public KTimeZone
 * {
 *     public:
 *         NewTimeZone(NewTimeZoneSource *source, const QString &name, ...);
 *         ~NewTimeZone();
 *
 *         // Virtual methods which need to be reimplemented
 *         int offsetAtZoneTime(const QDateTime &zoneDateTime, int *secondOffset = 0) const;
 *         int offsetAtUtc(const QDateTime &utcDateTime) const;
 *         int offset(time_t t) const;
 *         int isDstAtUtc(const QDateTime &utcDateTime) const;
 *         bool isDst(time_t t) const;
 *
 *         // Anything else which you need
 * };
 *
 * class NewTimeZoneData : public KTimeZoneData
 * {
 *         friend class NewTimeZoneSource;
 *
 *     public:
 *         NewTimeZoneData();
 *         ~NewTimeZoneData();
 *
 *         // Virtual methods which need to be reimplemented
 *         KTimeZoneData *clone();
 *         QList<QByteArray> abbreviations() const;
 *         QByteArray abbreviation(const QDateTime &utcDateTime) const;
 *
 *         // Data members containing whatever is read by NewTimeZoneSource
 * };
 * \endcode
 *
 * Here is a guide to implementing the offset() and offsetAtUtc() methods, in
 * the case where the source data does not use time_t for its time measurement:
 *
 * \code
 * int NewTimeZone::offsetAtUtc(const QDateTime &utcDateTime) const
 * {
 *     // Access this time zone's data. If we haven't already read it,
 *     // force a read from source now.
 *     NewTimeZoneData *zdata = data(true);
 *
 *     // Use 'zdata' contents to work out the UTC offset
 *
 *     return offset;
 * }
 *
 * int NewTimeZone::offset(time_t t) const
 * {
 *     return offsetAtUtc(fromTime_t(t));
 * }
 * \endcode
 *
 * The other NewTimeZone methods would work in an analogous way to
 * NewTimeZone::offsetAtUtc() and NewTimeZone::offset().
 */

/**
 * The KTimeZones class represents a time zone database which consists of a
 * collection of individual time zone definitions.
 *
 * Each individual time zone is defined in a KTimeZone instance, which provides
 * generic support for private or system time zones. The time zones in the
 * collection are indexed by name, which must be unique within the collection.
 * KTimeZone instances in the collection are owned by the KTimeZones instance,
 * and are deleted when the KTimeZones instance is destructed.
 *
 * Different time zone sources could define the same time zone differently. (For
 * example, a calendar file originating from another system might hold its own
 * time zone definitions, which may not necessarily be identical to your own
 * system's definitions.) In order to keep conflicting definitions separate,
 * it will often be necessary when dealing with multiple time zone sources to
 * create a separate KTimeZones instance for each source collection.
 *
 * If you want to access system time zones, use the KSystemTimeZones class.
 *
 * @short Represents a time zone database or collection
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @author S.R.Haque <srhaque@iee.org>.
 */
class KDECORE_EXPORT KTimeZones
{
public:
    KTimeZones();
    ~KTimeZones();

    /**
     * Returns the time zone with the given name.
     * Note that the KTimeZone returned remains a member of the KTimeZones
     * collection, and should not be deleted without calling detach() first.
     *
     * @param name name of time zone
     * @return time zone, or 0 if not found
     */
    const KTimeZone *zone(const QString &name) const;

    typedef QMap<QString, const KTimeZone*> ZoneMap;

    /**
     * Returns all the time zones defined in this collection.
     *
     * @return time zone collection
     */
    const ZoneMap zones() const;

    /**
     * Adds a time zone to the collection.
     * KTimeZones takes ownership of the KTimeZone instance, which will be deleted
     * when the KTimeZones instance is destructed.
     * The time zone's name must be unique within the collection.
     *
     * @param zone time zone to add
     * @return @c true if successful, @c false if zone's name duplicates one already in the collection
     */
    bool add(KTimeZone *zone);

    /**
     * Adds a time zone to the collection.
     * KTimeZones does not take ownership of the KTimeZone instance.
     * The time zone's name must be unique within the collection.
     *
     * @param zone time zone to add
     * @return @c true if successful, @c false if zone's name duplicates one already in the collection
     */
    bool addConst(const KTimeZone *zone);

    /**
     * Removes a time zone from the collection.
     * The caller assumes responsibility for deleting the removed KTimeZone. If
     * the removed KTimeZone was created by the caller, the constness of the return
     * value may safely be cast away.
     *
     * @param zone time zone to remove
     * @return the time zone which was removed, or 0 if not found or not a deletable object
     */
    const KTimeZone *detach(const KTimeZone *zone);

    /**
     * Removes a time zone from the collection.
     * The caller assumes responsibility for deleting the removed KTimeZone.
     *
     * @param name name of time zone to remove
     * @return the time zone which was removed, or 0 if not found or not a deletable object
     */
    const KTimeZone *detach(const QString &name);

    /**
     * Returns a standard UTC time zone, with name "UTC".
     *
     * @note The KTimeZone returned by this method does not belong to any
     * KTimeZones collection, and is statically allocated and therefore cannot
     * be deleted and cannot be added to a KTimeZones collection. Any KTimeZones
     * instance may contain its own UTC KTimeZone, but that will be a different
     * instance than this KTimeZone.
     *
     * @return UTC time zone
     */
    static const KTimeZone *utc();

private:
    KTimeZones(const KTimeZones &);              // prohibit copying
    KTimeZones &operator=(const KTimeZones &);   // prohibit copying

    KTimeZonesPrivate *d;
};


/*
 * Time zone phase.
 *
 * A phase can be daylight savings time or standard time. Each instance
 * contains a list of start date/time values for the phase, in time
 * order. Typically, this will be one date/time per year during the period
 * when the phase is in operation. It is permissible to have gaps in the
 * start date/time sequence corresponding to years when the phase was not
 * used.
 *
 * @short Time zone phase
 * @see KTimeZone, KTimeZoneData
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 */
class KDECORE_EXPORT KTimeZonePhase
{
public:
    KTimeZonePhase();

    /**
     * Constructor.
     *
     * @param utcStarts list of UTC start times for the phase
     * @param utcOffset number of seconds to add to UTC to get local time in this phase
     * @param abbreviations time zone abbreviation for this phase. If translations exist,
     *                      concatenate all abbreviations as null-terminated strings.
     * @param dst true if daylight savings time, false if standard time
     * @param comment optional comment
     */
    KTimeZonePhase(const QList<QDateTime> &utcStarts, int utcOffset, const QByteArray &abbreviations,
                   bool dst, const QString &comment = QString());
    KTimeZonePhase(const QList<QDateTime> &utcStarts, int utcOffset, const QList<QByteArray> &abbreviations,
                   bool dst, const QString &comment = QString());
    KTimeZonePhase(const KTimeZonePhase &c);
    ~KTimeZonePhase();
    KTimeZonePhase &operator=(const KTimeZonePhase &c);

    /**
     * Return whether this instance holds valid data.
     *
     * @return true if valid, false if invalid
     */
    bool isValid() const;

    /**
     * Return the UTC times when this daylight savings phase started.
     *
     * @return ordered list of times, earliest first
     */
    QList<QDateTime> starts() const;

    /**
     * Return one UTC date/time when this daylight savings phase started.
     *
     * @param index desired index into start times array
     * @return date/time
     */
    QDateTime start(int index) const;

    /**
     * Return the UTC offset in seconds during this phase.
     *
     * @return offset in seconds to add to UTC
     */
    int utcOffset() const;

    /**
     * Return the time zone abbreviations which apply to this phase.
     *
     * More than one abbreviation may be returned, to allow for possible translations.
     *
     * @return time zone abbreviations
     */
    QList<QByteArray> abbreviations() const;

    /**
     * Return whether daylight savings time applies during this phase.
     *
     * @return true if daylight savings are in operation, false otherwise
     */
    bool isDst() const;

    /**
     * Return the comment (if any) applying to this phase.
     *
     * @return comment
     */
    QString comment() const;

    /**
     * Find the first start time strictly after a given UTC or local time.
     *
     * @param dt date/time. If it is local time, this is interpreted as being
     *           in the time zone to which the instance belongs.
     * @return UTC date/time, or invalid if none
     */
    QDateTime nextStartTime(const QDateTime &dt) const;

    /**
     * Find the last start time, at or before a given UTC or local time.
     *
     * @param dt date/time. If it is a local time, this is interpreted as being
     *           in the time zone to which the instance belongs.
     * @return UTC date/time, or invalid if none
     */
    QDateTime previousStartTime(const QDateTime &dt) const;

private:
    KTimeZonePhasePrivate *d;
};


/*
 * Leap seconds adjustment for a time zone.
 *
 * This class defines a leap seconds adjustment for a time zone by its UTC time of
 * occurrence and the number of leap seconds to be added.
 *
 * @short Leap seconds adjustment for a time zone
 * @see KTimeZone, KTimeZoneData
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 */
class KDECORE_EXPORT KTimeZoneLeapSeconds
{
public:
    KTimeZoneLeapSeconds();
    KTimeZoneLeapSeconds(const QDateTime &utcTime, int leapSeconds, const QString &comment = QString());
    KTimeZoneLeapSeconds(const KTimeZoneLeapSeconds &c);
    ~KTimeZoneLeapSeconds();
    KTimeZoneLeapSeconds &operator=(const KTimeZoneLeapSeconds &c);
    bool operator<(const KTimeZoneLeapSeconds &c) const;    // needed by qSort()

    /**
     * Return whether this instance holds valid data.
     *
     * @return true if valid, false if invalid
     */
    bool isValid() const;

    /**
     * Return the UTC date/time when this change occurred.
     *
     * @return date/time
     */
    QDateTime dateTime() const;

    /**
     * Return the number of leap seconds to be added after this change occurs.
     *
     * @return number of leap seconds
     */
    int leapSeconds() const;

    /**
     * Return the comment (if any) applying to this change.
     *
     * @return comment
     */
    QString comment() const;

private:
    KTimeZoneLeapSecondsPrivate *d;
};


/**
 * Base class representing a time zone.
 *
 * The KTimeZone base class contains general descriptive data about the time zone, and
 * provides an interface for methods to read and parse time zone definitions, and to
 * translate between UTC and local time. Derived classes must implement these methods,
 * and may also hold the actual details of the dates and times of daylight savings
 * changes, offsets from UTC, etc. They should be tailored to deal with the type and
 * format of data held by a particular type of time zone database.
 *
 * If this class is instantiated, it represents the UTC time zone.
 *
 * KTimeZone is designed to work in partnership with KTimeZoneSource. KTimeZone
 * provides access to individual time zones, while classes derived from
 * KTimeZoneSource read and parse a particular format of time zone definition.
 * Because time zone sources can differ in what information they provide about time zones,
 * the parsed data retured by KTimeZoneSource can vary between different sources,
 * resulting in the need to create different KTimeZone classes to handle the data.
 *
 * KTimeZone instances are often grouped into KTimeZones collections. If a KTimeZone is
 * part of such a collection, it is owned by the KTimeZones instance and should not be
 * deleted.
 *
 * @short Base class representing a time zone
 * @see KTimeZoneSource, KTimeZoneData
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @author S.R.Haque <srhaque@iee.org>.
 */
class KDECORE_EXPORT KTimeZone
{
public:
    /**
     * Construct a UTC time zone.
     */
    explicit KTimeZone(const QString &name = QLatin1String("UTC"));

    KTimeZone(const KTimeZone &);
    virtual ~KTimeZone();

    KTimeZone &operator=(const KTimeZone &);

    /**
     * Returns the name of the time zone.
     * If it is held in a KTimeZones container, the name is the time zone's unique
     * identifier within that KTimeZones instance.
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
    QList<int> utcOffsets() const;

    /**
     * Converts a date/time, which is interpreted as being local time in this
     * time zone, into local time in another time zone.
     *
     * @param newZone other time zone which the time is to be converted into
     * @param zoneDateTime local date/time. An error occurs if
     *                     @p zoneDateTime.timeSpec() is not Qt::LocalTime.
     * @return converted date/time, or invalid date/time if error
     */
    QDateTime convert(const KTimeZone *newZone, const QDateTime &zoneDateTime) const;

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
    QDateTime toUtc(const QDateTime &zoneDateTime) const;

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
     * If a derived class needs to work in terms of time_t (as when accessing the
     * system time functions, for example), it should override both this method and
     * offset() so as to implement its offset calculations in offset(), and
     * reimplement this method simply as
     * \code
     *     offset(toTime_t(utcDateTime));
     * \endcode
     *
     * @param utcDateTime the UTC date/time at which the offset is to be calculated.
     *                    An error occurs if @p utcDateTime.timeSpec() is not Qt::UTC.
     * @return offset in seconds, or 0 if error
     */
    virtual int offsetAtUtc(const QDateTime &utcDateTime) const;

    /**
     * Returns the offset of this time zone to UTC at a specified UTC time.
     *
     * The offset is the number of seconds which you must add to UTC to get
     * local time in this time zone.
     *
     * Note that time_t has a more limited range than QDateTime, so consider using
     * offsetAtUtc() instead.
     *
     * @param t the UTC time at which the offset is to be calculated, measured in seconds
     *          since 00:00:00 UTC 1st January 1970 (as returned by time(2))
     * @return offset in seconds, or 0 if error
     */
    virtual int offset(time_t t) const;

    /**
     * Returns whether daylight savings time is in operation at the given UTC date/time.
     *
     * If a derived class needs to work in terms of time_t (as when accessing the
     * system time functions, for example), it should override both this method and
     * isDst() so as to implement its offset calculations in isDst(), and reimplement
     * this method simply as
     * \code
     *     isDst(toTime_t(utcDateTime));
     * \endcode
     *
     * @param utcDateTime the UTC date/time. An error occurs if
     *                    @p utcDateTime.timeSpec() is not Qt::UTC.
     * @return @c true if daylight savings time is in operation, @c false otherwise
     */
    virtual bool isDstAtUtc(const QDateTime &utcDateTime) const;

    /**
     * Returns whether daylight savings time is in operation at a specified UTC time.
     *
     * Note that time_t has a more limited range than QDateTime, so consider using
     * isDstAtUtc() instead.
     *
     * @param t the UTC time, measured in seconds since 00:00:00 UTC 1st January 1970
     *          (as returned by time(2))
     * @return @c true if daylight savings time is in operation, @c false otherwise
     */
    virtual bool isDst(time_t t) const;

    /**
     * Return all daylight savings time phases for the time zone.
     *
     * Note that some time zone data sources (such as system time zones accessed
     * via the system libraries) may not allow a list of daylight savings time
     * changes to be compiled easily. In such cases, this method will return an
     * empty list.
     *
     * @return list of phases
     */
    QList<KTimeZonePhase> phases() const;

    /**
     * Find the daylight savings time phase which is current at a given UTC or
     * local time.
     * Because of daylight savings time shifts, a local time may occur twice. Optionally,
     * the phases current at both occurrences of @p dt are calculated.
     *
     * @param dt date/time
     * @param secondPhase if non-null, and the @p dt occurs twice, receives the
     *                     phase for the second occurrence. Otherwise, it is set
     *                     the same as the return value.
     * @return daylight savings time phase with a single start date/time, or
     *         invalid if @p dt is outside the defined range of the phase data
     */
    KTimeZonePhase phase(const QDateTime &dt, KTimeZonePhase *secondPhase = 0) const;

    /**
     * Find the first daylight savings time phase strictly after a given UTC or
     * local time.
     *
     * @param dt date/time. If it is local time, this is interpreted as being
     *           in the time zone to which the KTimeZoneChanges instance belongs.
     * @return daylight savings time phase with a single start date/time, or
     *         invalid if none
     */
    KTimeZonePhase nextPhase(const QDateTime &dt) const;

    /**
     * Find the last daylight savings time phase, at or before a given UTC or
     * local time.
     *
     * @param dt date/time. If it is local time, this is interpreted as being
     *           in the time zone to which the KTimeZoneChanges instance belongs.
     * @return daylight savings time phase with a single start date/time, or
     *         invalid if none
     */
    KTimeZonePhase previousPhase(const QDateTime &dt) const;

    /**
     * Return all leap second adjustments, in time order.
     *
     * Note that some time zone data sources (such as system time zones accessed
     * via the system libraries) may not provide information on leap second
     * adjustments. In such cases, this method will return an empty list.
     *
     * @return list of adjustments
     */
    QList<KTimeZoneLeapSeconds> leapSecondChanges() const;

    /**
     * Returns the source reader/parser for the time zone's source database.
     *
     * @return reader/parser
     */
    KTimeZoneSource *source() const;

    /**
     * Extracts time zone detail information for this time zone from the source database.
     *
     * @return @c false if the parse encountered errors, @c true otherwise
     */
    bool parse() const;

    /**
     * Converts a UTC time, measured in seconds since 00:00:00 UTC 1st January 1970
     * (as returned by time(2)), to a UTC QDateTime value.
     * QDateTime::setTime_t() is limited to handling @p t >= 0, since its parameter
     * is unsigned. This method takes a parameter of time_t which is signed.
     *
     * @return converted time
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
     *                    be an instance of a class derived from KTimeZoneSource.
     * @param name        in system-dependent format. The name must be unique within any
     *                    KTimeZones instance which contains this KTimeZone.
     * @param countryCode ISO 3166 2-character country code, empty if unknown
     * @param latitude    in degrees (between -90 and +90), UNKNOWN if not known
     * @param longitude   in degrees (between -180 and +180), UNKNOWN if not known
     * @param comment     description of the time zone, if any
     */
    KTimeZone(
        KTimeZoneSource *source, const QString &name,
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
    const KTimeZoneData *data(bool create = false) const;

    /**
     * Sets the detailed parsed data for the time zone.
     *
     * @param data parsed data
     */
    void setData(KTimeZoneData *data);

private:
    KTimeZonePrivate *d;
};


/**
 * Base class representing a source of time zone information.
 *
 * Derive subclasses from KTimeZoneSource to read and parse time zone details
 * from a time zone database or other source of time zone information. If can know
 * in advance what KTimeZone instances to create without having to parse the source
 * data, you should reimplement the virtual method parse(const KTimeZone*). Otherwise,
 * you need to define your own parse() methods with appropriate signatures, to both
 * read and parse the new data, and create new KTimeZone instances.
 *
 * KTimeZoneSource itself may be used as a dummy source which returns empty
 * time zone details.
 *
 * @short Base class representing a source of time zone information
 * @see KTimeZone, KTimeZoneData
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @author S.R.Haque <srhaque@iee.org>.
 */
class KDECORE_EXPORT KTimeZoneSource
{
public:
    KTimeZoneSource()  {}
    virtual ~KTimeZoneSource()  {}

    /**
     * Extracts detail information for one time zone from the source database.
     * In this base class, the method always succeeds and returns an empty data
     * instance. Derived classes should reimplement this method to return an
     * appropriate data class derived from KTimeZoneData.
     *
     * @param zone the time zone for which data is to be extracted.
     * @return an instance of a class derived from KTimeZoneData containing
     *         the parsed data. The caller is responsible for deleting the
     *         KTimeZoneData instance.
     *         Null is returned on error.
     */
    virtual KTimeZoneData *parse(const KTimeZone *zone) const;
};


/**
 * Base class for the parsed data returned by a KTimeZoneSource class.
 *
 * It contains all the data available from the KTimeZoneSource class,
 * including, when available, a complete list of daylight savings time
 * changes and leap seconds adjustments.
 *
 * This base class can be instantiated, but contains no data.
 *
 * @short Base class for parsed time zone data
 * @see KTimeZone, KTimeZoneSource
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 */
class KDECORE_EXPORT KTimeZoneData
{
public:
    KTimeZoneData();
    KTimeZoneData(const KTimeZoneData &c);
    virtual ~KTimeZoneData();
    KTimeZoneData &operator=(const KTimeZoneData &c);

    /**
     * Creates a new copy of this object.
     * The caller is responsible for deleting the copy.
     * Derived classes must reimplement this method to return a copy of the
     * calling instance.
     *
     * @return copy of this instance
     */
    virtual KTimeZoneData *clone();

    /**
     * Returns the complete list of time zone abbreviations. This may include
     * translations.
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
    virtual QList<int> utcOffsets() const;

    /**
     * Return all daylight savings time phases.
     *
     * Note that some time zone data sources (such as system time zones accessed
     * via the system libraries) may not allow a list of daylight savings time
     * changes to be compiled easily. In such cases, this method will return an
     * empty list.
     *
     * @return list of phases
     */
    QList<KTimeZonePhase> phases() const;

    /**
     * Find the daylight savings time phase which is current at a given UTC or
     * local time.
     * Because of daylight savings time shifts, a local time may occur twice. Optionally,
     * the phases current at both occurrences of @p dt are calculated.
     *
     * @param dt date/time
     * @param secondPhase if non-null, and the @p dt occurs twice, receives the
     *                     phase for the second occurrence. Otherwise, it is set
     *                     the same as the return value.
     * @return daylight savings time phase with a single start date/time, or
     *         invalid if @p dt is outside the defined range of the phase data
     */
    KTimeZonePhase phase(const QDateTime &dt, KTimeZonePhase *secondPhase = 0) const;

    /**
     * Find the first daylight savings time phase strictly after a given UTC or
     * local time.
     *
     * @param dt date/time. If it is local time, this is interpreted as being
     *           in the time zone to which the KTimeZoneChanges instance belongs.
     * @return daylight savings time phase with a single start date/time, or
     *         invalid if none
     */
    KTimeZonePhase nextPhase(const QDateTime &dt) const;

    /**
     * Find the last daylight savings time phase, at or before a given UTC or
     * local time.
     *
     * @param dt date/time. If it is local time, this is interpreted as being
     *           in the time zone to which the KTimeZoneChanges instance belongs.
     * @return daylight savings time phase with a single start date/time, or
     *         invalid if none
     */
    KTimeZonePhase previousPhase(const QDateTime &dt) const;

    /**
     * Return all leap second adjustments, in time order.
     *
     * Note that some time zone data sources (such as system time zones accessed
     * via the system libraries) may not provide information on leap second
     * adjustments. In such cases, this method will return an empty list.
     *
     * @return list of adjustments
     */
    QList<KTimeZoneLeapSeconds> leapSecondChanges() const;

    /**
     * Find the leap second adjustment which is applicable at a given UTC time.
     *
     * @param utc UTC date/time
     * @return leap second adjustment, or invalid if @p utc is earlier than the
     *         first leap second adjustment or @p utc is a local time
     */
    KTimeZoneLeapSeconds leapSecondChange(const QDateTime &utc) const;

protected:
    /**
     * Initialise the daylight savings time phase list.
     *
     * @param phases list of phases
     */
    void setPhases(const QList<KTimeZonePhase> &phases);

    /**
     * Initialise the leap seconds adjustment list.
     *
     * @param adjusts list of adjustments
     */
    void setLeapSecondChanges(const QList<KTimeZoneLeapSeconds> &adjusts);

private:
    KTimeZoneDataPrivate *d;
};


/**
 * The KSystemTimeZones class represents the system time zone database, consisting
 * of a collection of individual system time zone definitions, indexed by name.
 * Each individual time zone is defined in a KSystemTimeZone instance. Additional
 * time zones (of any class derived from KTimeZone) may be added if desired.
 *
 * At initialisation, KSystemTimeZones reads the zone.tab file to obtain the list
 * of system time zones, and creates a KSystemTimeZone instance for each one.
 *
 * Note that KSystemTimeZones is not derived from KTimeZones, but instead contains
 * a KTimeZones instance which holds the system time zone database. Convenience
 * static methods are defined to access its data, or alternatively you can access
 * the KTimeZones instance directly via the timeZones() method.
 *
 * As an example, find the local time in Oman corresponding to the local system
 * time of 12:15:00 on 13th November 1999:
 * \code
 * QDateTime sampleTime(QDate(1999,11,13), QTime(12,15,0), Qt::LocalTime);
 * const KTimeZone *local = KSystemTimeZones::local();
 * const KTimeZone *oman  = KSystemTimeZones::zone("Asia/Muscat");
 * QDateTime omaniTime = local->convert(oman, sampleTime);
 * \endcode
 *
 * @warning The time zones in the KSystemTimeZones collection are by default
 * instances of the KSystemTimeZone class, which uses the standard system libraries
 * to access time zone data, and whose functionality is limited to what these
 * libraries provide. For guaranteed accuracy for past dates, especially for time
 * zone abbreviations or dates before 1970, you should use the KTzfileTimeZone
 * class instead.
 *
 * @short System time zone access
 * @see KTimeZones, KSystemTimeZone, KSystemTimeZoneSource, KTzfileTimeZone
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 */
class KDECORE_EXPORT KSystemTimeZones
{
public:
    /**
     * Returns all the time zones defined in this collection.
     *
     * @return time zone collection
     */
    static const KTimeZones::ZoneMap zones()   { return timeZones()->zones(); }

    /**
     * Returns the time zone with the given name.
     *
     * @param name name of time zone
     * @return time zone (usually a KSystemTimeZone instance), or 0 if not found
     */
    static const KTimeZone *zone(const QString &name)   { return timeZones()->zone(name); }

    /**
     * Returns the current local system time zone.
     *
     * The idea of this routine is to provide a robust lookup of the local time
     * zone. The problem is that on Unix systems, there are a variety of mechanisms
     * for setting this information, and no well defined way of getting it. For
     * example, if you set your time zone to "Europe/London", then the tzname[]
     * maintained by tzset() typically returns { "GMT", "BST" }. The point of
     * this routine is to actually return "Europe/London" (or rather, the
     * corresponding KTimeZone).
     *
     * @return local system time zone. If necessary, we will use a series of
     *         heuristics which end by returning UTC. We will never return NULL.
     *         Note that if UTC is returned as a default, it may not belong to the
     *         the collection returned by KSystemTimeZones::zones().
     */
    static const KTimeZone *local();

    /**
     * Returns the location of the system time zone zoneinfo database.
     *
     * @return path of directory containing the zoneinfo database
     */
    static QString zoneinfoDir();

    /**
     * Returns the unique KTimeZones instance containing the system time zones
     * collection. It is first created if it does not already exist.
     *
     * @return time zones.
     */
    static KTimeZones *timeZones();

private:
    KSystemTimeZonesPrivate *d;
};


/**
 * The KSystemTimeZone class represents a time zone in the system database.
 *
 * It works in partnership with the KSystemTimeZoneSource class which reads and parses the
 * time zone definition files.
 *
 * Typically, instances are created and accessed via the KSystemTimeZones class.
 *
 * @warning The KSystemTimeZone class uses the standard system libraries to
 * access time zone data, and its functionality is limited to what these libraries
 * provide. On many systems, dates earlier than 1970 are not handled, and on
 * non-GNU systems there is no guarantee that the time zone abbreviation returned
 * for a given date will be correct if the abbreviations applicable then were
 * not those currently in use. Consider using the KTzfileTimeZone class instead,
 * which always provides accurate information from the time zone definition files.
 *
 * @short System time zone
 * @see KSystemTimeZones, KSystemTimeZoneSource, KSystemTimeZoneData, KTzfileTimeZone
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 */
class KDECORE_EXPORT KSystemTimeZone : public KTimeZone
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
    KSystemTimeZone(KSystemTimeZoneSource *source, const QString &name,
        const QString &countryCode = QString(), float latitude = UNKNOWN, float longitude = UNKNOWN,
        const QString &comment = QString());

    ~KSystemTimeZone();

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
    virtual int offsetAtUtc(const QDateTime &utcDateTime) const;

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
    virtual bool isDstAtUtc(const QDateTime &utcDateTime) const;

    /**
     * Returns whether daylight savings time is in operation at a specified UTC time.
     *
     * @param t the UTC time, measured in seconds since 00:00:00 UTC 1st January 1970
     *          (as returned by time(2))
     * @return @c true if daylight savings time is in operation, @c false otherwise
     */
    virtual bool isDst(time_t t) const;

private:
    KSystemTimeZonePrivate *d;
};


/**
 * A class to read and parse system time zone data.
 *
 * Access is performed via the system time zone library functions.
 *
 * @short Reads and parses system time zone data
 * @see KSystemTimeZones, KSystemTimeZone, KSystemTimeZoneData
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 */
class KDECORE_EXPORT KSystemTimeZoneSource : public KTimeZoneSource
{
public:
    /**
     * Constructs a system time zone source.
     */
    KSystemTimeZoneSource();
    virtual ~KSystemTimeZoneSource();

    /**
     * Extract detailed information for one time zone, via the system time zone
     * library functions.
     *
     * @param zone the time zone for which data is to be extracted
     * @return a KSystemTimeZoneData instance containing the parsed data.
     *         The caller is responsible for deleting the KTimeZoneData instance.
     *         Null is returned on error.
     */
    virtual KTimeZoneData *parse(const KTimeZone *zone) const;

    /**
     * Use in conjunction with endParseBlock() to improve efficiency when calling parse()
     * for a group of KSystemTimeZone instances in succession.
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
    KSystemTimeZoneSourcePrivate *d;
};


/**
 * The parsed system time zone data returned by KSystemTimeZoneSource.
 *
 * @short Parsed system time zone data
 * @see KSystemTimeZoneSource, KSystemTimeZone
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 */
class KSystemTimeZoneData : public KTimeZoneData
{
    friend class KSystemTimeZoneSource;

public:
    KSystemTimeZoneData();
    /** Copy constructor; no special ownership assumed. */
    KSystemTimeZoneData(const KSystemTimeZoneData &);
    virtual ~KSystemTimeZoneData();

    /** Assignment; no special ownership assumed. Everything is value based. */
    KSystemTimeZoneData &operator=(const KSystemTimeZoneData &);

    /**
     * Creates a new copy of this object.
     * The caller is responsible for deleting the copy.
     * Derived classes must reimplement this method to return a copy of the
     * calling instance 
     *
     * @return copy of this instance. This is a KSystemTimeZoneData pointer.
     */
    virtual KTimeZoneData *clone();

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
    virtual QList<int> utcOffsets() const;

private:
    KSystemTimeZoneDataPrivate *d;
};

#endif
