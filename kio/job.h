// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __kio_job_h__
#define __kio_job_h__

#include <kio/jobclasses.h>

namespace KIO {

    /**
     * Creates a single directory.
     *
     *
     *
     *
     * @param @p url The URL of the directory to create.
     * @param @p permissions The permissions to set after creating the directory.
     * @return A pointer to the job handling the operation.
     */
    SimpleJob * mkdir( const KURL& url, int permissions = -1 );

    /**
     * Remove a single directory.
     *
     * The directory is assumed to be empty.
     *
     *
     *
     * @param @p url The URL of the directory to remove.
     * @return A pointer to the job handling the operation.
     */
    SimpleJob * rmdir( const KURL& url );

    /**
     * Change permissions on a file or directory.
     *
     *
     *
     * @param url The URL of file or directory.
     * @param permissions The permissions to set.
     * @return The job handling the operation.
     */
    SimpleJob * chmod( const KURL& url, int permissions );

    /**
     * Execute any command that is specific to one slave (protocol).
     *
     * Examples are : HTTP POST, mount and unmount (kio_file)
     *
     * @param url The URL isn't passed to the slave, but is used to know
     *        which slave to send it to :-)
     * @param data Packed data.  The meaning is completely dependent on the
     *        slave, but usually starts with an @tt int for the command number.
     */
    SimpleJob * special( const KURL& url, const QByteArray & data, bool showProgressInfo = true );

    /**
     * Mount filesystem.
     *
     * Special job for @tt kio_file.
     *
     * @param ro Mount read-only if @tt true.
     * @param fstype File system type (e.g. "ext2", can be 0L).
     * @param dev Device (e.g. /dev/sda0).
     * @param point Mount point, can be @tt null.
     */
    SimpleJob *mount( bool ro, const char *fstype, const QString& dev, const QString& point, bool showProgressInfo = true );

    /**
     * Unmount filesystem.
     *
     * Special job for @tt kio_file.
     *
     * @param mount Point to unmount.
     */
    SimpleJob *unmount( const QString & point, bool showProgressInfo = true );

    /**
     * Find all details for one file or directory.
     */
    StatJob * stat( const KURL& url, bool showProgressInfo = true );

    /**
     * Get (a.k.a. read).
     *
     * The slave emits the data through @ref data().
     */
    TransferJob *get( const KURL& url, bool reload=false, bool showProgressInfo = true );

    /**
     * Put (a.k.a. write)
     *
     * @param url Where to write data.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param overwrite If @tt true, any existing file will be overwritten.
     * @param resume
     */
    TransferJob *put( const KURL& url, int permissions,
                      bool overwrite, bool resume, bool showProgressInfo = true );

    /**
     * HTTP POST (for form data).
     *
     * @param url Where to write data.
     * @param postData Encoded data to post.
     */
    TransferJob *http_post( const KURL& url, const QByteArray &postData,
                            bool showProgressInfo = true );

    /**
     * HTTP POST (for form data) with arbitrary header additions.
     * (e.g., special content types, etc). 
     *
     * This should replace the
     * other http_post, eventually.
     *
     * @param url Where to write data.
     * @param postData Encoded data to post.
     * @param contentType Special content type (if needed).
     */
    TransferJob *http_post( const KURL& url, const QByteArray &postData,
                            const QString& headers,
                            bool showProgressInfo = true );

    /**
     * Find mimetype for one file or directory.
     */
    MimetypeJob * mimetype( const KURL& url,
                            bool showProgressInfo = true );

    /**
     * Copy a single file.
     *
     * Uses either @ref SlaveBase::copy() if the slave supports that
     * or @ref get() and @ref put() otherwise.
     * @param src Where to get the file.
     * @param dest Where to put the file.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param overwrite If @tt true, any existing file will be overwritten.
     * @param resume
     */
    FileCopyJob *file_copy( const KURL& src, const KURL& dest, int permissions=-1,
                            bool overwrite=false, bool resume=false,
                            bool showProgressInfo = true);

    /**
     * Move a single file.
     *
     * Use either @ref SlaveBase::rename() if the slave supports that,
     * or @ref copy() and @ref del() otherwise, or eventually get() & put() & del()
     * @param src Where to get the file.
     * @param dest Where to put the file.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param overwrite If @tt true, any existing file will be overwritten.
     * @param resume
     */
    FileCopyJob *file_move( const KURL& src, const KURL& dest, int permissions=-1,
                            bool overwrite=false, bool resume=false,
                            bool showProgressInfo = true);

    /**
     * Delete a single file.
     *
     * @param src File to delete.
     */
    SimpleJob *file_delete( const KURL& src, bool showProgressInfo = true);

    /**
     * @internal
     * Create a local symlink
     * Mostly a wrapper around symlink(2).
     * @param linkDest the existing file, destination of the link to be created
     * @param destUrl name of the link to be created
     * @param overwriteExistingFiles set to true to enforce overwriting if dest exists
     * @param overwriteAll matches the corresponding button in RenameDlg, passed from one call to the next one
     * @param autoSkip matches the corresponding button in RenameDlg, passed from one call to the next one
     * @param cancelAll matches the corresponding button in RenameDlg, passed from one call to the next one
     */
    bool link( const QString & linkDest, const KURL & destUrl, bool overwriteExistingFiles,
               bool & overwriteAll, bool & autoSkip, bool & cancelAll );

    /**
     * Create one or more links.
     *
     * This not yet a job, and will become one only if at least one other
     * protocol than file has support for it :)
     *
     * @param src Local files will result in symlinks,
     *            remote files will result in Type=Link .desktop files
     * @param dest Destination, has to be a local dir currently.
     * @return true On success, false on failure.
     */
    bool link( const KURL::List& src, const KURL& destDir );

    /**
     * List the contents of @p url, which is assumed to be a directory.
     *
     * "." and ".." are returned, filter them out if you don't want them.
     */
    ListJob *listDir( const KURL& url, bool showProgressInfo = true );

    /**
     * The same as the previous method, but recurses subdirectories.
     *
     * "." and ".." are returned but only for the toplevel directory.
     * Filter them out if you don't want them.
     */
    ListJob *listRecursive( const KURL& url, bool showProgressInfo = true );

    /**
     * Copy a file or directory @p src into the destination @dest,
     * which can be a file (including the final filename) or a directory
     * (into which @p src will be copied).
     *
     * This emulates the @tt cp command completely.
     */
    CopyJob *copy( const KURL& src, const KURL& dest, bool showProgressInfo = true );
    /**
     * Copy a file or directory @p src into the destination @dest,
     * which is the destination name in any case, even for a directory.
     *
     * As opposed to @ref copy, this doesn't emulate @tt cp, but is the only
     * way to copy a directory, giving it a new name and getting an error
     * box if a directory already exists with the same name.
     */
    CopyJob *copyAs( const KURL& src, const KURL& dest, bool showProgressInfo = true );

    /**
     * Copy a list of file/dirs @p src into a destination directory @p dest.
     */
    CopyJob *copy( const KURL::List& src, const KURL& dest, bool showProgressInfo = true );

    /**
     * @see copy
     */
    CopyJob *move( const KURL& src, const KURL& dest, bool showProgressInfo = true );
    /**
     * @see copyAs
     */
    CopyJob *moveAs( const KURL& src, const KURL& dest, bool showProgressInfo = true );
    /**
     * @see copy
     */
    CopyJob *move( const KURL::List& src, const KURL& dest, bool showProgressInfo = true );

    /**
     * Delete a file or directory.
     *
     * @param src file to delete
     * @param shred If @tt true, delete in a way that makes recovery impossible
     * (only supported for local files currently)
     */
    DeleteJob *del( const KURL& src, bool shred = false, bool showProgressInfo = true );
    DeleteJob *del( const KURL::List& src, bool shred = false, bool showProgressInfo = true );
}

#endif

