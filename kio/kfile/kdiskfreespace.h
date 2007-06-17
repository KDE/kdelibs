/*
 * kdiskfreesp.h
 *
 * Copyright (c) 1999 Michael Kropfberger <michael.kropfberger@gmx.net>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */


#ifndef KDISKFREESP_H
#define KDISKFREESP_H

#include <QtCore/QObject>
#include <QtCore/QString>

#include <kio/kio_export.h>

class K3Process;

/**
 * This class parses the output of "df" to find the disk usage
 * information for a given partition (mount point).
 */
class KIO_EXPORT KDiskFreeSpace : public QObject
{
Q_OBJECT
public:
   KDiskFreeSpace( QObject *parent=0);
   /**
    * Destructor - this object autodeletes itself when it's done
    */
   ~KDiskFreeSpace();
   /**
    * Call this to fire a search on the disk usage information
    * for @p mountPoint. foundMountPoint will be emitted
    * if this mount point is found, with the info requested.
    * done is emitted in any case.
    */
   int readDF( const QString & mountPoint );

   /**
    * Call this to fire a search on the disk usage information
    * for the mount point containing @p path.
    * foundMountPoint will be emitted
    * if this mount point is found, with the info requested.
    * done is emitted in any case.
    */
   static KDiskFreeSpace * findUsageInfo( const QString & path );

Q_SIGNALS:
   void foundMountPoint( const QString & mountPoint, quint64 kBSize, quint64 kBUsed, quint64 kBAvail );

   // This one is a hack around a weird (compiler?) bug. In the former signal,
   // the slot in KPropertiessDialog would get 0L, 0L as the last two parameters.
   // When using const ulong& instead, all is ok.
   void foundMountPoint( const quint64&, const quint64&, const quint64&, const QString& );
   void done();

private Q_SLOTS:
   void receivedDFStdErrOut(K3Process *, char *data, int len);
   void dfDone();

private:
  class Private;
  Private * const d;
};
/***************************************************************************/


#endif
