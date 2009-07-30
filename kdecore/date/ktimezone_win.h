/*
   This file is part of the KDE libraries
   Copyright (c) 2008 Marc Mutz <mutz@kde.org>, Till Adam <adam@kde.org>

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

#ifndef _KTIMEZONE_WINDOWS_H
#define _KTIMEZONE_WINDOWS_H

#include <kdecore_export.h>

#include <ktimezone.h>
#include <ksystemtimezone.h>
#include <windows.h>
  
class KSystemTimeZoneSourceWindowsPrivate;

/**
 * A class to read and parse the timezone information from the Windows registry.
 *
 * The registry is queried for the currently active and for all available timezones
 * on the system, and those are provided to the KystemTimeZone framework.
 *
 * @short Reads and parses the Windows registry timezone information
 * @see KSystemTimeZoneWindows, KSystemTimeZoneDataWindows
 * @ingroup timezones
 * @author Marc Mutz <mutz@kde.org>, Till Adam <adam@kde.org>.
 */
class KDECORE_EXPORT KSystemTimeZoneSourceWindows : public KSystemTimeZoneSource
{
public:
   /**
     * Constructs a time zone source.
     */
  KSystemTimeZoneSourceWindows();

   /**
    * Retrieves and parses the Windows registry information to extract
    * detailed information for one time zone.
    *
    * @param zone the time zone for which data is to be extracted
    * @return a KSystemTimeZoneDataWindows instance containing the parsed data.
    *         The caller is responsible for deleting the KTimeZoneData instance.
    *         Null is returned on error.
    */
  KTimeZoneData* parse(const KTimeZone &zone) const;
private:
  KSystemTimeZoneSourceWindowsPrivate * const d;
};


/**
 * The KSystemTimeZoneWindows class represents a time zone defined in the Windows registry.
 *
 * It works in partnership with the KSystemTimeZoneSourceWindows class which reads and parses the
 * time zone definitions from the Windows registry.
 *
 * @short Represents a time zone defined in the Windows registry 
 * @see KSystemTimeZoneBackendWindows, KSystemTimeZoneSourceWindows, KSystemTimeZoneDataWindows
 * @ingroup timezones
 * @author Marc Mutz <mutz@kde.org>, Till Adam <adam@kde.org>.
 */
class KDECORE_EXPORT KSystemTimeZoneWindows : public KTimeZone  //krazy:exclude=dpointer (no d-pointer for KTimeZone derived classes)
{
public:
   /**
    * Creates a time zone.
    *
    * @param source      Windows registry reader and parser
    * @param name        time zone's unique name, which must be the tzfile path relative
    *                    to the location specified for @p source
    */
  KSystemTimeZoneWindows(KTimeZoneSource *source, const QString &name);
    
  ~KSystemTimeZoneWindows() {}

  /**
   * Static helper method that lists all availalbe timezones on the system
   * as per the information in the Windows registry.
   */
  static QStringList listTimeZones();
private:
   // d-pointer is in backend class.
   // This is a requirement for classes inherited from KTimeZone.
};

#endif // _KTIMEZONE_WINDOWS_H

