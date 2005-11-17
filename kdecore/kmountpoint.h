/*
   This file is part of the KDE libraries
   Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

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

#ifndef KMOUNTPOINT_H
#define KMOUNTPOINT_H

#include <qstringlist.h>
#include <ksharedptr.h>

/**
 * The KMountPoint class provides information about mounted and unmounted disks.
 * It provides a system independent interface to fstab.
 *
 * @author Waldo Bastian <bastian@kde.org>
 * @since 3.2
 */
class KDECORE_EXPORT KMountPoint : public KShared
{
public:
  typedef KSharedPtr<KMountPoint> Ptr;
  typedef QList<Ptr> List;
public:
   enum { NeedMountOptions = 1, NeedRealDeviceName = 2 };

   /**
    * This function gives a list of all possible mountpoints. (fstab)
    * @param infoNeeded Flags that specify which additional information
    * should be fetched.
    */
   static List possibleMountPoints(int infoNeeded=0);

   /**
    * This function gives a list of all currently used mountpoints. (mtab)
    * @param infoNeeded Flags that specify which additional information
    * should be fetched.
    */
   static List currentMountPoints(int infoNeeded=0);

   /**
    * Where this filesystem gets mounted from.
    * This can refer to a device, a remote server or something else.
    */
   QString mountedFrom() const { return m_mountedFrom; }

   /**
    * Canonical name of the device where the filesystem got mounted from.
    * (Or empty, if not a device)
    * Only available when the NeedRealDeviceName flag was set.
    */
   QString realDeviceName() const { return m_device; }

   /**
    * Path where the filesystem is mounted or can be mounted.
    */
   QString mountPoint() const { return m_mountPoint; }

   /**
    * Type of filesystem
    */
   QString mountType() const { return m_mountType; }

   /**
    * Options used to mount the filesystem.
    * Only available when the NeedMountOptions flag was set.
    */
   QStringList mountOptions() const { return m_mountOptions; }

   /**
    * When using supermount, the device name is in the options field
    * as dev=/my/device
    * @since 3.4
    */
   static QString devNameFromOptions(const QStringList &options);

   /**
    * Destructor
    */
   ~KMountPoint();

private:
   /**
    * Constructor
    */
   KMountPoint();

   QString m_mountedFrom;
   QString m_device;
   QString m_mountPoint;
   QString m_mountType;
   QStringList m_mountOptions;

   class Private;
   Private * const d;
};

#endif // KMOUNTPOINT_H

