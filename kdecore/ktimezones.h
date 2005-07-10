/*
   This file is part of the KDE libraries
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KTIMEZONES_H
#define _KTIMEZONES_H

#include "kdelibs_export.h"
#include <qnamespace.h>
#include <qmap.h>
#include <qstring.h>
class KTimezonePrivate;
class KTimezoneDetailsPrivate;
class KTimezonesPrivate;

/**
 * The KTimezone class contains core functions related to a system timezone.
 *
 * @since KTimezoneDetails
 * @since 3.5
 * @author S.R.Haque <srhaque@iee.org>.
 */
class KDECORE_EXPORT KTimezone
{
public:
    static const float UNKNOWN;

    KTimezone(
        class KTimezones *db, const QString &name,
        const QString &countryCode = "??", float latitude = UNKNOWN, float longitude = UNKNOWN,
        const QString &comment = "");

    /**
     * Returns the name of the timezone.
     *
     * @return name in system-dependent format.
     */
    QString name() const;

    /**
     * Returns the two-letter country code of the timezone.
     *
     * @return name in ISO 3166 2-character country code, "??" if unknown.
     */
    QString countryCode() const;

    /**
     * Returns the latitude of the timezone.
     *
     * @return latitude in degrees, UNKNOWN if not known.
     */
    float latitude() const;

    /**
     * Returns the latitude of the timezone.
     *
     * @return latitude in degrees, UNKNOWN if not known.
     */
    float longitude() const;

    /**
     * Returns the current offset of the given timezone to UTC or the local
     * timezone in seconds.
     *
     * Take care if you cache the results of this routine; that would
     * break if the result were stored across a daylight savings change.
     *
     * @return offset in seconds, INT_MAX on error.
     */
    int offset(Qt::TimeSpec basisSpec = Qt::UTC) const;

    /**
     * Returns any comment for the timezone.
     *
     * @return comment, may be empty.
     */
    QString comment() const;

private:
    friend class KTimezoneDetails;
    class KTimezones *m_db;
    QString m_name;
    QString m_countryCode;
    float m_latitude;
    float m_longitude;
    QString m_comment;
    KTimezonePrivate *d;
};

/**
 * The KTimezoneDetails class contains extended functions related to a system
 * timezone. It consists basically of a parser for zoneinfo files in tzfile(5).
 *
 * The parser must be customised by overriding the given virtual callbacks:
 *<ul>
 *    <li>{@link parseEnded() }
 *    <li>{@link gotHeader() }
 *    <li>{@link gotTransitionTime() }
 *    <li>{@link gotLocalTimeIndex() }
 *    <li>{@link gotLocalTime() }
 *    <li>{@link gotAbbreviation() }
 *    <li>{@link gotLeapAdjustment() }
 *    <li>{@link gotIsStandard() }
 *    <li>{@link gotIsUTC() }
 *</ul>
 *
 * @since KTimezone
 * @since 3.5
 * @author S.R.Haque <srhaque@iee.org>.
 */
class KDECORE_EXPORT KTimezoneDetails
{
public:
    KTimezoneDetails(KTimezone *zone);
    virtual ~KTimezoneDetails();

    /**
     * Retrieve the details of the timezone using the callbacks for:
     * @return true if file was a valid timezoneinfo file.
     */
    bool parseStart();

protected:

    /**
     * Always called before {@link parseStart() } returns.
     */
    virtual void parseEnded();

    /**
     * Called when the header is seen.
     */
    virtual void gotHeader(
        unsigned ttIsGmtCnt, unsigned ttIsStdCnt, unsigned leapCnt,
        unsigned timeCnt, unsigned typeCnt, unsigned charCnt);

    /**
     * Called when a transition time is seen.
     */
    virtual void gotTransitionTime(unsigned transitionTime);

    /**
     * Called when a local time index is seen.
     */
    virtual void gotLocalTimeIndex(unsigned localTimeIndex);

    /**
     * Called when a local time is seen.
     */
    virtual void gotLocalTime(int gmtOff, int isDst, unsigned abbrInd);

    /**
     * Called when a timezone abbreviation is seen.
     */
    virtual void gotAbbreviation(const QString &abbr);

    /**
     * Called when a leap second adjustment is seen.
     */
    virtual void gotLeapAdjustment(unsigned leapTime, unsigned leapSeconds);

    /**
     * Called when a standard/wall time indicator is seen.
     */
    virtual void gotIsStandard(unsigned isStandard);

    /**
     * Called when a UTC/local time indicator is seen.
     */
    virtual void gotIsUTC(unsigned isUTC);

private:
    KTimezone *m_zone;
    KTimezoneDetailsPrivate *d;
};

/**
 * The KTimezones class contains functions related to the use of system timezones.
 *
 * @since 3.5
 * @author S.R.Haque <srhaque@iee.org>.
 */
class KDECORE_EXPORT KTimezones
{
public:
    KTimezones();
    ~KTimezones();

    /**
     * Returns the local timezone. The idea of this routine is to provide a
     * robust lookup of the local timezone.
     *
     * The problem is that on Unix systems, there are a variety of mechanisms
     * for setting this information, and no real way of getting it. For example,
     * if you set your timezone to "Europe/London", then the tzname[]
     * maintained by tzset() typically returns { "GMT", "BST" }. The point of
     * this routine is to actually return "Europe/London" (or rather, the
     * corresponding KTimezone).
     *
     * @return local timezone. If necessary, we will return a guess, NULL means
     *         "UTC".
     */
    const KTimezone *local();

    /**
     * Returns the given timezone.
     * @return named timezone, NULL on error.
     */
    const KTimezone *zone(const QString &name);

    typedef QMap<QString, KTimezone *> ZoneMap;

    /**
     * Parse system timezone database.
     * @return known timezones.
     */
    const ZoneMap allZones();

    /**
     * Location of system timezone information.
     * @return value which can be combined with zone name to retrieve timezone info.
     */
    QString db();

private:
    float convertCoordinate(const QString &coordinate);

    QString m_zoneinfoDir;
    ZoneMap *m_zones;
    KTimezonesPrivate *d;
};

#endif
