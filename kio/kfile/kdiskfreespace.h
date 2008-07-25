/*
 * kdiskfreespace.h
 *
 * Copyright 2007 David Faure <faure@kde.org>
 * Copyright 2008 Dirk Mueller <mueller@kde.org>
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

/**
 * \deprecated Use KDiskFreeSpaceInfo
 */
class KIO_EXPORT KDiskFreeSpace : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor
     */
    KDE_DEPRECATED explicit KDiskFreeSpace( QObject *parent = 0 );

    /**
     * Destructor - this object autodeletes itself when it's done
     */
    ~KDiskFreeSpace();

    /**
     * Call this to fire a search on the disk usage information
     * for @p mountPoint.
     * The foundMountPoint() signal will be emitted
     * if this mount point is found, with the info requested.
     * The done() signal is emitted in any case.
     *
     * @return true if the request could be handled, false if another
     * request is happening already. readDF() can only be called once
     * on a given instance of KDiskFreeSpace, given that it handles only
     * the request for one mount point and then auto-deletes itself.
     * Suicidal objects are not reusable...
     */
    KDE_DEPRECATED bool readDF( const QString & mountPoint );

    /**
     * Call this to fire a search on the disk usage information
     * for the mount point containing @p path.
     * The foundMountPoint() signal will be emitted
     * if this mount point is found, with the info requested.
     * The done() signal is emitted in any case.
     */
     KDE_DEPRECATED static KDiskFreeSpace * findUsageInfo( const QString & path );

Q_SIGNALS:
    /**
     * Emitted when the information about the requested mount point was found.
     * @param mountPoint the requested mount point
     * @param kibSize the total size of the partition in KiB
     * @param kibUsed the amount of KiB being used on the partition
     * @param kibAvail the available space on the partition in KiB
     */
    void foundMountPoint( const QString & mountPoint, quint64 kibSize, quint64 kibUsed, quint64 kibAvail );

    /**
     * Emitted when the request made via readDF is over, whether foundMountPoint was emitted or not.
     */
    void done();

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT( d, bool _k_calculateFreeSpace() )
};

#endif
