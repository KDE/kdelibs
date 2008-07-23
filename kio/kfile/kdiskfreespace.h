/*
 * kdiskfreespace.h
 *
 * Copyright 2007 David Faure <faure@kde.org>
 * Copyright 2008 Dirk Mueller <mueller@kde.org>
 * Copyright 2008 Sebastian Trug <trueg@kde.org>
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
#include <kio/global.h>


/**
 * \class KDiskFreeSpacePrivate kdiskfreespace.h KDiskFreeSpace
 *
 * \brief Determine the space left on an arbitrary partition.
 *
 * This class wraps around the system calls to detemermine the
 * free space left in a specific partition. It supports arbitrary
 * paths and the mount point of the partition in question.
 *
 * The class is intended to be used as a fire-and-forget object
 * very much like KJob. It is recommended to use the static
 * findUsageInfo method instead of creating ones own instance:
 *
 * \code
 * connect( KDiskFreeSpace::findUsageInfo( myPath ), SIGNAL(finished(KDiskFreeSpace*)),
 *          this, SLOT(mySlot(KDiskFreeSpace*)) );
 *
 * void mySlot( KDiskFreeSpace* df ) {
 *     if (!df->error())
 *         doSomethingWithTheDFInfo( df->mountPoint(),
 *                                   df->size(),
 *                                   df->used(),
 *                                   df->avail() );
 * }
 * \code
 */
class KIO_EXPORT KDiskFreeSpace : public QObject // KDE 5 -> make it a KIO::Job
{
    Q_OBJECT

public:

    /**
     * Constructor
     *
     * It is recommended to use findUsageInfo instead.
     */
    explicit KDiskFreeSpace( QObject *parent = 0 );

    /**
     * Destructor - this object autodeletes itself when it's done
     */
    ~KDiskFreeSpace();

    /**
     * The mount point the requested partition is mounted on.
     *
     * Only valid after a successful run of readDF() or findUsageInfo()
     * (ie. error() returns 0).
     */
    QString mountPoint() const;

    /**
     * The size of the requested partition.
     *
     * Only valid after a successful run of readDF() or findUsageInfo()
     * (ie. error() returns 0).
     */
    KIO::filesize_t size() const;

    /**
     * The used space of the requested partition.
     *
     * Only valid after a successful run of readDF() or findUsageInfo()
     * (ie. error() returns 0).
     */
    KIO::filesize_t used() const;

    /**
     * The available space of the requested partition.
     *
     * Only valid after a successful run of readDF() or findUsageInfo()
     * (ie. error() returns 0).
     */
    KIO::filesize_t avail() const;

    /**
     * Returns the error code, if there has been an error. 
     * Only call this method from the slot connected to finished().
     *
     * \return the error code for this job, 0 is no error.
     */
    int error() const;

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
     *
     * It is recommended to use findUsageInfo instead.
     */
    bool readDF( const QString & mountPoint );

    /**
     * Call this to fire a search on the disk usage information
     * for the mount point containing @p path.
     * The foundMountPoint() signal will be emitted
     * if this mount point is found, with the info requested.
     * The done() signal is emitted in any case.
     */
    static KDiskFreeSpace * findUsageInfo( const QString & path ); // KDE 5 -> move this into the KIO namespace

Q_SIGNALS:
    /**
     * Emitted when the information about the requested mount point was found.
     * @param mountPoint the requested mount point
     * @param kibSize the total size of the partition in KiB
     * @param kibUsed the amount of KiB being used on the partition
     * @param kibAvail the available space on the partition in KiB
     * \deprecated use finished
     */
    void foundMountPoint( const QString & mountPoint, quint64 kibSize, quint64 kibUsed, quint64 kibAvail );

    /**
     * Emitted when the request made via readDF is over, whether foundMountPoint was emitted or not.
     * \deprecated use finished
     */
    void done();

    /**
     * Emitted once the request is done. Check error() to see if the call was
     * successful.
     */
    void finished( KDiskFreeSpace* );

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT( d, bool _k_calculateFreeSpace() )
};

#endif
