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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KMOUNTPOINT_H_
#define _KMOUNTPOINT_H_

#include <qptrlist.h>
#include <qstringlist.h>

/**
 * The KMountPoint class provides information about mounted and unmounted disks.
 * It provides a system independant interface to fstab.
 *
 * @author Waldo Bastian <bastian@kde.org>
 * @since 3.2
 */
class KMountPoint
{
   typedef signed long long int filesize_t;
public:
   enum { NeedMountOptions = 1, NeedRealDeviceName = 2 };

   /**
    * This function gives a list of all possible mountpoints. (fstab)
    * @param infoNeeded Flags that specify which additional information 
    * should be fetched.
    */
   static QPtrList<KMountPoint> possibleMountPoints(int infoNeeded=0);

   /**
    * This function gives a list of all currently used mountpoints. (mtab)
    * @param infoNeeded Flags that specify which additional information 
    * should be fetched.
    */
   static QPtrList<KMountPoint> currentMountPoints(int infoNeeded=0);

   /**
    * Where this filesystem gets mounted from.
    * This can refer to a device, a remote server or something else.
    */
   QString mountedFrom() { return m_mountedFrom; }

   /**
    * Canonical name of the device where the filesystem got mounted from.
    * (Or empty, if not a device)
    * Only available when the @ref NeedRealDeviceName flag was set.
    */
   QString realDeviceName() { return m_device; }

   /**
    * Path where the filesystem is mounted or can be mounted.
    */
   QString mountPoint() { return m_mountPoint; }
   
   /**
    * Type of filesystem
    */
   QString mountType() { return m_mountType; }
   
   /**
    * Options used to mount the filesystem.
    * Only available when the @ref NeedMountOptions flag was set.
    */
   QStringList mountOptions() { return m_mountOptions; }

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
};

#endif
