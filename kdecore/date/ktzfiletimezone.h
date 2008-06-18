/*
   This file is part of the KDE libraries
   Copyright (c) 2005-2008 David Jarvie <djarvie@kde.org>

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
 * TZFILE time zone functions
 * @author David Jarvie <djarvie@kde.org>.
 */

#ifndef _KTZFILETIMEZONE_H
#define _KTZFILETIMEZONE_H

#include <kdecore_export.h>
#include <ktimezone.h>

#include <QtCore/QString>

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
 * @see KTzfileTimeZoneBackend, KTzfileTimeZoneSource, KTzfileTimeZoneData
 * @ingroup timezones
 * @author David Jarvie <djarvie@kde.org>.
 */
class KDECORE_EXPORT KTzfileTimeZone : public KTimeZone  //krazy:exclude=dpointer (no d-pointer for KTimeZone derived classes)
{
public:
    /**
     * Creates a time zone.
     *
     * @param source      tzfile reader and parser
     * @param name        time zone's unique name, which must be the tzfile path relative
     *                    to the location specified for @p source
     * @param countryCode ISO 3166 2-character country code, empty if unknown
     * @param latitude    in degrees (between -90 and +90), UNKNOWN if not known
     * @param longitude   in degrees (between -180 and +180), UNKNOWN if not known
     * @param comment     description of the time zone, if any
     */
    KTzfileTimeZone(KTzfileTimeZoneSource *source, const QString &name,
        const QString &countryCode = QString(), float latitude = UNKNOWN, float longitude = UNKNOWN,
        const QString &comment = QString());

    ~KTzfileTimeZone();

private:
    // d-pointer is in KTzfileTimeZoneBackend.
    // This is a requirement for classes inherited from KTimeZone.
};


/**
 * Backend class for KTzfileTimeZone class.
 *
 * This class implements KTzfileTimeZone's constructors and virtual methods. A
 * backend class is required for all classes inherited from KTimeZone to
 * allow KTimeZone virtual methods to work together with reference counting of
 * private data.
 *
 * @short Backend class for KTzfileTimeZone class
 * @see KTimeZoneBackend, KTzfileTimeZone, KTimeZone
 * @ingroup timezones
 * @author David Jarvie <djarvie@kde.org>.
 */
class KDECORE_EXPORT KTzfileTimeZoneBackend : public KTimeZoneBackend  //krazy:exclude=dpointer (non-const d-pointer for KTimeZoneBackend-derived classes)
{
public:
    /** Implements KTzfileTimeZone::KTzfileTimeZone(). */
    KTzfileTimeZoneBackend(KTzfileTimeZoneSource *source, const QString &name,
        const QString &countryCode, float latitude, float longitude, const QString &comment);

    ~KTzfileTimeZoneBackend();

    /**
     * Creates a copy of this instance.
     *
     * @return new copy
     */
    virtual KTimeZoneBackend *clone() const;

    /**
     * Returns the class name of the data represented by this instance.
     *
     * @return "KTzfileTimeZone"
     */
    virtual QByteArray type() const;

    /**
     * Implements KTzfileTimeZone::hasTransitions().
     *
     * Returns whether daylight saving transitions are available for the time zone.
     *
     * @param caller calling KTzfileTimeZone object
     * @return @c true
     */
    virtual bool hasTransitions(const KTimeZone* caller) const;

private:
    KTzfileTimeZonePrivate *d;   // non-const
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
 * @author David Jarvie <djarvie@kde.org>.
 */
class KDECORE_EXPORT KTzfileTimeZoneSource : public KTimeZoneSource
{
public:
    /**
     * Constructs a time zone source.
     *
     * The directory containing the time zone definition files is given by the
     * @p location parameter, which will usually be the zoneinfo directory. For
     * tzfile files in other locations, bear in mind that the name generated
     * for each KTzfileTimeZone is its file path relative to @p location.
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
    QString location() const;

    /**
     * Parses a tzfile file to extract detailed information for one time zone.
     *
     * @param zone the time zone for which data is to be extracted
     * @return a KTzfileTimeZoneData instance containing the parsed data.
     *         The caller is responsible for deleting the KTimeZoneData instance.
     *         Null is returned on error.
     */
    virtual KTimeZoneData *parse(const KTimeZone &zone) const;

private:
    KTzfileTimeZoneSourcePrivate * const d;
};


/**
 * @internal
 * The parsed data returned by KTzfileTimeZoneSource.
 *
 * @short Parsed data from tzfile(5) time zone definition files
 * @see KTzfileTimeZoneSource, KTzfileTimeZone
 * @ingroup timezones
 * @author David Jarvie <djarvie@kde.org>.
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
    virtual KTimeZoneData *clone() const;

    /**
     * Return whether daylight saving transitions are available for the time zone.
     *
     * @return @c true
     */
    virtual bool hasTransitions() const;

private:
    // Enable this if you add KDECORE_EXPORT to this class
    //KTzfileTimeZoneDataPrivate * const d;
};

#endif
