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
class KTimezonesPrivate;

/**
 * The KTimezone class contains functions related to a system timezone.
 *
 * @since 3.5
 * @author S.R.Haque <srhaque@iee.org>.
 */
class KDECORE_EXPORT KTimezone
{
public:
    static const float UNKNOWN;

    KTimezone(const QString& name, const QString& countryCode = "??", float latitude = UNKNOWN, float longitude = UNKNOWN, const QString& comment = "");

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
    QString m_name;
    QString m_countryCode;
    float m_latitude;
    float m_longitude;
    QString m_comment;
    KTimezonePrivate *d;
};


/**
 * The KTimezones class contains functions related to the use of system timezones.
 *
 * @author S.R.Haque <srhaque@iee.org>.
 */
class KDECORE_EXPORT KTimezones
{
public:
    KTimezones();
    ~KTimezones();

    /**
     * Returns the local timezone.
     * @return local timezone. If necessary, we will return a guess.
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

private:
    float convertOrdinate(const QString& ordinate);
    bool matchAbbreviations(const QString& zoneFile, const QString& stdZone, const QString& dstZone);

private:
    QString m_zoneinfoDir;
    ZoneMap * m_zones;
    KTimezonesPrivate *d;
};

#endif
