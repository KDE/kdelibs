/*
   This file is part of the KDE libraries
   Copyright (c) 2005 David Jarvie <software@astrojar.org.uk>

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
 * TZFILE time zone functions
 * @author David Jarvie <software@astrojar.org.uk>.
 */

#ifndef _KTZFILETIMEZONE_H
#define _KTZFILETIMEZONE_H

#include <QDateTime>
#include <QList>
#include <QString>
#include <QByteArray>
#include "kdelibs_export.h"
#include <ktimezones.h>

class KTzfileTimezoneSource;
class KTzfileTimezonePrivate;
class KTzfileTimezoneDataPrivate;
class KTzfileTimezoneSourcePrivate;

/**
 * The KTzfileTimezone class represents a time zone defined in tzfile(5) format.
 *
 * It works in partnership with the KTzfileTimezoneSource class which reads and parses the
 * time zone definition files.
 *
 * @short Represents a time zone defined in tzfile(5) format
 * @see KTzfileTimezoneSource, KTzfileTimezoneData
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
     * Creates a time zone.
     *
     * @param source      tzfile reader and parser
     * @param name        time zone's unique name
     * @param countryCode ISO 3166 2-character country code, empty if unknown
     * @param latitude    in degrees (between -90 and +90), UNKNOWN if not known
     * @param longitude   in degrees (between -180 and +180), UNKNOWN if not known
     * @param comment     description of the time zone, if any
     */
    KTzfileTimezone(KTzfileTimezoneSource *source, const QString &name,
        const QString &countryCode = QString(), float latitude = UNKNOWN, float longitude = UNKNOWN,
        const QString &comment = QString());

    ~KTzfileTimezone();

    /**
     * Returns the offset of this time zone to UTC at the given local date/time.
     * Because of daylight savings time shifts, the date/time may occur twice. Optionally,
     * the offsets at both occurrences of @p dateTime are calculated.
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
    virtual int offsetAtZoneTime(const QDateTime &zoneDateTime, int *secondOffset) const;

    /**
     * Returns the offset of this time zone to UTC at a specified UTC time.
     *
     * @param t the UTC time at which the offset is to be calculated, measured in seconds
     *          since 00:00:00 UTC 1st January 1970 (as returned by time(2))
     * @return offset in seconds, or 0 if error
     */
    virtual int offset(time_t t) const;
    virtual bool isDst(time_t t) const;

    /**
     * Returns time adjustment details for a given date and time.
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
 * @short Reads and parses tzfile(5) time zone definition files
 * @see KTzfileTimezone, KTzfileTimezoneData
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 * @since 4.0
 */
class KDECORE_EXPORT KTzfileTimezoneSource : public KTimezoneSource
{
public:
    /**
     * Constructs a time zone source.
     *
     * @param location the local directory containing the time zone definition files
     */
    KTzfileTimezoneSource(const QString &location);
    virtual ~KTzfileTimezoneSource();

    /**
     * Returns the local directory containing the time zone definition files.
     *
     * @return path to time zone definition files
     */
    QString location();

    /**
     * Parses a tzfile file to extract detailed information for one time zone.
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
 * @short Parsed data from tzfile(5) time zone definition files
 * @see KTzfileTimezoneSource, KTzfileTimezone
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
     * Creates a new copy of this object.
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
     * Returns the m_transitionTimes details for a given UTC time.
     *
     * @param t UTC time, measured in seconds since 00:00:00 UTC 1st January 1970
     *          (as returned by time(2))
     * @return pointer to details, or 0 if the time is before the start of data
     */
    const TransitionTime *getTransitionTime(time_t t) const;

    /**
     * Returns the m_localTimeTypes details for a given UTC time.
     *
     * @param t UTC time, measured in seconds since 00:00:00 UTC 1st January 1970
     *          (as returned by time(2))
     * @return pointer to details, or 0 if the time is before the start of data
     */
    const LocalTimeType *getLocalTime(time_t t) const;

    /**
     * Returns the number of leap seconds to be applied after a given UTC time.
     *
     * @param t UTC time, measured in seconds since 00:00:00 UTC 1st January 1970
     *          (as returned by time(2))
     * @return adjustment, or 0 if the time is before the start of data
     */
    Q_UINT32 getLeapSeconds(time_t t) const;

    /** Returns the list of time zone abbreviations.
     *
     * @return the list of abbreviations
     */
    virtual QList<QByteArray> abbreviations() const;
    virtual QByteArray abbreviation(const QDateTime &utcDateTime) const;

    /**
     * Returns the time zone abbreviation at a given index.
     *
     * @param index abbreviation's index, as contained in LocalTimeType's abbrIndex element
     * @return time zone abbreviation, or empty string if invalid index
     */
    QByteArray abbreviation(int index) const;

protected:
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

private:
    KTzfileTimezoneDataPrivate *d;
};

#endif
