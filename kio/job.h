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
     * @param @p url the URL of the directory to create
     * @param @p permissions the permissions to set after creating the directory
     * @return a pointer to the job handling the operation.
     */
    SimpleJob * mkdir( const KURL& url, int permissions = -1 );

    /**
     * Removes a single directory.
     *
     * assumed to be empty
     *
     *
     *
     * @param @p url the URL of the directory to remove
     * @return a pointer to the job handling the operation.
     */
    SimpleJob * rmdir( const KURL& url );

    /**
     * Change permissions on a file or directory
     *
     *
     *
     * @param url the URL of file or directory
     * @param permissions the permissions to set
     * @return the job handling the operation
     */
    SimpleJob * chmod( const KURL& url, int permissions );

    /**
     * Any command that is specific to one slave (protocol)
     * Examples are : HTTP POST, mount and unmount (kio_file)
     *
     * @param url the url isn't passed to the slave, but is used to know 
     *        which slave to send it to :-)
     * @param data packed data; the meaning is completely dependent on the
     *        slave, but usually starts with an int for the command number.
     */
    SimpleJob * special( const KURL& url, const QByteArray & data );

    /**
     * Mount, special job for kio_file
     *
     * @param ro mount read-only if true
     * @param fstype file system type (e.g. "ext2", can be 0L)
     * @param dev device (e.g. /dev/sda0)
     * @param point mount point, can be null
     */
    SimpleJob *mount( bool ro, const char *fstype, const QString& dev, const QString& point );

    /**
     * Unmount, special job for kio_file
     *
     * @param mount point to unmount
     */
    SimpleJob *unmount( const QString & point );

    /**
     * Finds all details for one file or directory.
     */
    StatJob * stat( const KURL& url );

    /**
     * get, aka read.
     * The slave emits the data through @ref data
     */
    TransferJob *get( const KURL& url, bool reload=false );

    /**
     * put, aka write.
     * @param url where to write data
     * @param permissions may be -1. In this case no special permission mode is set.
     * @param overwrite if true, any existing file will be overwritten
     * @param resume
     */
    TransferJob *put( const KURL& url, int permissions,
                      bool overwrite, bool resume );

    /**
     * HTTP POST (for form data)
     * @param url where to write data
     * @param postData encoded data to post
     */
    TransferJob *http_post( const KURL& url, const QByteArray &postData );

    /**
     * Finds mimetype for one file or directory.
     */
    MimetypeJob * mimetype( const KURL& url );

    /**
     * Copies a single file
     * Uses either SlaveBase::copy() if the slave supports that
     * or get() & put() otherwise.
     * @param src where to get the file
     * @param dest where to put the file
     * @param permissions may be -1. In this case no special permission mode is set.
     * @param overwrite if true, any existing file will be overwritten
     * @param resume
     */
    FileCopyJob *file_copy( const KURL& src, const KURL& dest, int permissions=-1,
                            bool overwrite=false, bool resume=false);

    /**
     * Moves a single file.
     * Uses either SlaveBase::rename() if the slave supports that,
     * copy() & del() otherwise, or eventually get() & put() & del()
     * @param src where to get the file
     * @param dest where to put the file
     * @param permissions may be -1. In this case no special permission mode is set.
     * @param overwrite if true, any existing file will be overwritten
     * @param resume
     */
    FileCopyJob *file_move( const KURL& src, const KURL& dest, int permissions=-1,
                            bool overwrite=false, bool resume=false);

    /**
     * Delete a single file.
     * @param src file to delete
     */
    SimpleJob *file_delete( const KURL& src);

    /**
     * Create a link
     * This not yet a job, and will become only if at least one other
     * protocol than file has support for it :)
     * @param src local files will result in symlinks,
     *            remote files will result in Type=Link .desktop files
     * @param dest destination, has to be a local file currently.
     */
    void link( const KURL::List& src, const KURL& dest );

    /**
     * Lists the contents of @p url, which is assumed to be a directory.
     * "." and ".." are returned, filter them out if you don't want them.
     */
    ListJob *listDir( const KURL& url );

    /**
     * The same, recursive.
     * "." and ".." are returned but only for the toplevel directory.
     * Filter them out if you don't want them.
     */
    ListJob *listRecursive( const KURL& url );

    CopyJob *copy( const KURL& src, const KURL& dest );
    CopyJob *copy( const KURL::List& src, const KURL& dest );

    CopyJob *move( const KURL& src, const KURL& dest );
    CopyJob *move( const KURL::List& src, const KURL& dest );

    /**
     * Delete a file or directory
     * @param src file to delete
     * @param shred if true, delete in a way that makes recovery impossible
     * (only supported for local files currently)
     */
    DeleteJob *del( const KURL& src, bool shred = false );
    DeleteJob *del( const KURL::List& src, bool shred = false );
};

#endif

