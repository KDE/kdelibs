/* This file is part of the KDE libraries
    Copyright (C) 2000, 2006 David Faure <faure@kde.org>

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

#ifndef DIRECTORYSIZEJOB_H
#define DIRECTORYSIZEJOB_H

#include "kio_export.h"
#include "job.h"
#include <kfileitem.h>

namespace KIO {

class DirectorySizeJobPrivate;
/**
 * Computes a directory size (similar to "du", but doesn't give the same results
 * since we simply sum up the dir and file sizes, whereas du speaks disk blocks)
 *
 * Usage: see KIO::directorySize.
 */
class KIO_EXPORT DirectorySizeJob : public KIO::Job
{
    Q_OBJECT

public:
    ~DirectorySizeJob();

public:
    /**
     * @return the size we found
     */
    KIO::filesize_t totalSize() const;

    /**
     * @return the total number of files (counting symlinks to files, sockets
     * and character devices as files) in this directory and all sub-directories
     */
    KIO::filesize_t totalFiles() const;

    /**
     * @return the total number of sub-directories found (not including the
     * directory the search started from and treating symlinks to directories
     * as directories)
     */
    KIO::filesize_t totalSubdirs() const;

protected Q_SLOTS:
    virtual void slotResult( KJob *job );

protected:
    DirectorySizeJob(DirectorySizeJobPrivate &dd);

private:
    Q_PRIVATE_SLOT(d_func(), void slotEntries( KIO::Job * , const KIO::UDSEntryList &))
    Q_PRIVATE_SLOT(d_func(), void processNextItem())
    Q_DECLARE_PRIVATE(DirectorySizeJob)
};

/**
 * Computes a directory size (by doing a recursive listing).
 * Connect to the result signal (this is the preferred solution to avoid blocking the GUI),
 * or use exec() for a synchronous (blocking) calculation.
 *
 * This one lists a single directory.
 */
KIO_EXPORT DirectorySizeJob * directorySize(const QUrl & directory);

/**
 * Computes a directory size (by doing a recursive listing).
 * Connect to the result signal (this is the preferred solution to avoid blocking the GUI),
 * or use exec() for a synchronous (blocking) calculation.
 *
 * This one lists the items from @p lstItems.
 * The reason we asks for items instead of just urls, is so that
 * we directly know if the item is a file or a directory,
 * and in case of a file, we already have its size.
 */
KIO_EXPORT DirectorySizeJob * directorySize( const KFileItemList & lstItems );

}

#endif
