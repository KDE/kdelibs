/*
   This file is part of the KDE libraries
   Copyright (c) 2005,2006 David Jarvie <software@astrojar.org.uk>

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

#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include "kdelibs_export.h"
#include <ktimezones.h>

class KTzfileTimeZoneSource;
class KTzfileTimeZonePrivate;
class KTzfileTimeZoneDataPrivate;
class KTzfileTimeZoneSourcePrivate;

/**
 * The KTzfileTimeZone class represents a time zone defined in tzfile(5) format.
 *
 * It works in partnership with the KTzfileTimeZoneSource class which reads and parses the
 * time zone definition files.
 *
 * @short Represents a time zone defined in tzfile(5) format
 * @see KTzfileTimeZoneSource, KTzfileTimeZoneData
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 */
class KDECORE_EXPORT KTzfileTimeZone : public KTimeZone
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
    KTzfileTimeZone(KTzfileTimeZoneSource *source, const QString &name,
        const QString &countryCode = QString(), float latitude = UNKNOWN, float longitude = UNKNOWN,
        const QString &comment = QString());

    ~KTzfileTimeZone();

    /**
     * Return whether daylight saving transitions are available for the time zone.
     *
     * @return @c true
     */
    virtual bool hasTransitions() const;

private:
    KTzfileTimeZonePrivate *d;
};


/**
 * A class to read and parse tzfile time zone definition files.
 *
 * tzfile is the format used by zoneinfo files in the system time zone database.
 * The format is documented in the tzfile(5) manpage.
 *
 * @short Reads and parses tzfile(5) time zone definition files
 * @see KTzfileTimeZone, KTzfileTimeZoneData
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 */
class KDECORE_EXPORT KTzfileTimeZoneSource : public KTimeZoneSource
{
public:
    /**
     * Constructs a time zone source.
     *
     * @param location the local directory containing the time zone definition files
     */
    explicit KTzfileTimeZoneSource(const QString &location);
    virtual ~KTzfileTimeZoneSource();

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
     * @return a KTzfileTimeZoneData instance containing the parsed data.
     *         The caller is responsible for deleting the KTimeZoneData instance.
     *         Null is returned on error.
     */
    virtual KTimeZoneData *parse(const KTimeZone *zone) const;

private:
    KTzfileTimeZoneSourcePrivate *d;
};


/**
 * The parsed data returned by KTzfileTimeZoneSource.
 *
 * @short Parsed data from tzfile(5) time zone definition files
 * @see KTzfileTimeZoneSource, KTzfileTimeZone
 * @ingroup timezones
 * @author David Jarvie <software@astrojar.org.uk>.
 */
class KTzfileTimeZoneData : public KTimeZoneData
{
    friend class KTzfileTimeZoneSource;

public:
    KTzfileTimeZoneData();
    KTzfileTimeZoneData(const KTzfileTimeZoneData &);
    virtual ~KTzfileTimeZoneData();

    KTzfileTimeZoneData &operator=(const KTzfileTimeZoneData &);

    /**
     * Creates a new copy of this object.
     * The caller is responsible for deleting the copy.
     * Derived classes must reimplement this method to return a copy of the
     * calling instance 
     *
     * @return copy of this instance. This is a KTzfileTimeZoneData pointer.
     */
    virtual KTimeZoneData *clone();

    /**
     * Return whether daylight saving transitions are available for the time zone.
     *
     * @return @c true
     */
    virtual bool hasTransitions() const;

private:
    KTzfileTimeZoneDataPrivate *d;
};

#endif
