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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
     * @param url The URL of the directory to create.
     * @param permissions The permissions to set after creating the
     *                    directory (unix-style), -1 for default permissions.
     * @return A pointer to the job handling the operation.
     */
    KIO_EXPORT SimpleJob * mkdir( const KUrl& url, int permissions = -1 );

    /**
     * Removes a single directory.
     *
     * The directory is assumed to be empty.
     *
     *
     *
     * @param url The URL of the directory to remove.
     * @return A pointer to the job handling the operation.
     */
    KIO_EXPORT SimpleJob * rmdir( const KUrl& url );

    /**
     * Changes permissions on a file or directory.
     * See the other chmod below for changing many files
     * or directories.
     *
     * @param url The URL of file or directory.
     * @param permissions The permissions to set.
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob * chmod( const KUrl& url, int permissions );

    /**
     * Rename a file or directory.
     * Warning: this operation fails if a direct renaming is not
     * possible (like with files or dirs on separate partitions)
     * Use move or file_move in this case.
     *
     * @param src The original URL
     * @param dest The final URL
     * @param overwrite whether to automatically overwrite if the dest exists
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob * rename( const KUrl& src, const KUrl & dest, bool overwrite );

    /**
     * Create or move a symlink.
     * This is the lowlevel operation, similar to file_copy and file_move.
     * It doesn't do any check (other than those the slave does)
     * and it doesn't show rename and skip dialogs - use KIO::link for that.
     * @param target The string that will become the "target" of the link (can be relative)
     * @param dest The symlink to create.
     * @param overwrite whether to automatically overwrite if the dest exists
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob * symlink( const QString & target, const KUrl& dest, bool overwrite, bool showProgressInfo = true );

    /**
     * Execute any command that is specific to one slave (protocol).
     *
     * Examples are : HTTP POST, mount and unmount (kio_file)
     *
     * @param url The URL isn't passed to the slave, but is used to know
     *        which slave to send it to :-)
     * @param data Packed data.  The meaning is completely dependent on the
     *        slave, but usually starts with an int for the command number.
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob * special( const KUrl& url, const QByteArray & data, bool showProgressInfo = true );

    /**
     * Mount filesystem.
     *
     * Special job for @p kio_file.
     *
     * @param ro Mount read-only if @p true.
     * @param fstype File system type (e.g. "ext2", can be empty).
     * @param dev Device (e.g. /dev/sda0).
     * @param point Mount point, can be @p null.
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob *mount( bool ro, const QByteArray& fstype, const QString& dev, const QString& point, bool showProgressInfo = true );

    /**
     * Unmount filesystem.
     *
     * Special job for @p kio_file.
     *
     * @param point Point to unmount.
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob *unmount( const QString & point, bool showProgressInfo = true );

    /**
     * HTTP cache update
     *
     * @param url Url to update, protocol must be "http".
     * @param no_cache If true, cache entry for @p url is deleted.
     * @param expireDate Local machine time indicating when the entry is
     * supposed to expire.
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob *http_update_cache( const KUrl& url, bool no_cache, time_t expireDate);

    /**
     * Find all details for one file or directory.
     *
     * @param url the URL of the file
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     */
    KIO_EXPORT StatJob * stat( const KUrl& url, bool showProgressInfo = true );
    /**
     * Find all details for one file or directory.
     * This version of the call includes two additional booleans, @p sideIsSource and @p details.
     *
     * @param url the URL of the file
     * @param sideIsSource is true when stating a source file (we will do a get on it if
     * the stat works) and false when stating a destination file (target of a copy).
     * The reason for this parameter is that in some cases the kioslave might not
     * be able to determine a file's existence (e.g. HTTP doesn't allow it, FTP
     * has issues with case-sensitivity on some systems).
     * When the slave can't reliably determine the existence of a file, it will:
     * @li be optimistic if sideIsSource=true, i.e. it will assume the file exists,
     * and if it doesn't this will appear when actually trying to download it
     * @li be pessimistic if sideIsSource=false, i.e. it will assume the file
     * doesn't exist, to prevent showing "about to overwrite" errors to the user.
     * If you simply want to check for existence without downloading/uploading afterwards,
     * then you should use sideIsSource=false.
     *
     * @param details selects the level of details we want.
     * By default this is 2 (all details wanted, including modification time, size, etc.),
     * setDetails(1) is used when deleting: we don't need all the information if it takes
     * too much time, no need to follow symlinks etc.
     * setDetails(0) is used for very simple probing: we'll only get the answer
     * "it's a file or a directory, or it doesn't exist". This is used by KRun.
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     */
    KIO_EXPORT StatJob * stat( const KUrl& url, bool sideIsSource, short int details, bool showProgressInfo = true );

    /**
     * Get (a.k.a. read).
     *
     * The slave emits the data through data().
     * @param url the URL of the file
     * @param reload true to reload the file, false if it can be taken from the cache
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     */
    KIO_EXPORT TransferJob *get( const KUrl& url, bool reload=false, bool showProgressInfo = true );

    /**
     * Put (a.k.a. write)
     *
     * @param url Where to write data.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param overwrite If true, any existing file will be overwritten.
     * @param resume true to resume an operation. Warning, setting this to true means
     *               that the data will be appended to @p dest if @p dest exists.
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     * @see multi_get()
     */
    KIO_EXPORT TransferJob *put( const KUrl& url, int permissions,
                      bool overwrite, bool resume,
                      // KDE4 TODO: add 'long mtime = 0' for #79937
                      bool showProgressInfo = true );

    /**
     * HTTP POST (for form data).
     *
     * Example:
     * \code
     *    job = KIO::http_post( url, postData, false );
     *    job->addMetaData("content-type", contentType );
     *    job->addMetaData("referrer", referrerURL);
     * \endcode
     *
     * @p postData is the data that you want to send and
     * @p contentType is the complete HTTP header line that
     * specifies the content's MIME type, for example
     * "Content-Type: text/xml".
     *
     * You MUST specify content-type!
     *
     * Often @p contentType is
     * "Content-Type: application/x-www-form-urlencoded" and
     * the @p postData is then an ASCII string (without null-termination!)
     * with characters like space, linefeed and percent escaped like %20,
     * %0A and %25.
     *
     * @param url Where to write the data.
     * @param postData Encoded data to post.
     * @param showProgressInfo true to display
     * @return the job handling the operation.
     */
    KIO_EXPORT TransferJob *http_post( const KUrl& url, const QByteArray &postData,
                            bool showProgressInfo = true );

    /**
     * Get (a.k.a. read), into a single QByteArray.
     * @see StoredTransferJob
     *
     * @param url the URL of the file
     * @param reload true to reload the file, false if it can be taken from the cache
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     * @since 3.3
     */
    KIO_EXPORT StoredTransferJob *storedGet( const KUrl& url, bool reload=false, bool showProgressInfo = true );

    /**
     * Put (a.k.a. write) data from a single QByteArray.
     * @see StoredTransferJob
     *
     * @param arr The data to write
     * @param url Where to write data.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param overwrite If true, any existing file will be overwritten.
     * @param resume true to resume an operation. Warning, setting this to true means
     *               that the data will be appended to @p dest if @p dest exists.
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     * @since 3.3
     */
    KIO_EXPORT StoredTransferJob *storedPut( const QByteArray& arr, const KUrl& url, int permissions,
                                  bool overwrite, bool resume, bool showProgressInfo = true );

    /**
     * Creates a new multiple get job.
     *
     * @param id the id of the get operation
     * @param url the URL of the file
     * @param metaData the MetaData associated with the file
     *
     * @return the job handling the operation.
     * @see get()
     */
    KIO_EXPORT MultiGetJob *multi_get( long id, const KUrl &url, const MetaData &metaData);

    /**
     * Find mimetype for one file or directory.
     *
     * @param url the URL of the file
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     */
    KIO_EXPORT MimetypeJob * mimetype( const KUrl& url,
                            bool showProgressInfo = true );

    /**
     * Copy a single file.
     *
     * Uses either SlaveBase::copy() if the slave supports that
     * or get() and put() otherwise.
     * @param src Where to get the file.
     * @param dest Where to put the file.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param overwrite If true, any existing file will be overwritten.
     * @param resume true to resume an operation. Warning, setting this to true means
     *               that @p src will be appended to @p dest if @p dest exists.
     *               You probably don't want that, so leave it to false :)
     *
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     */
    KIO_EXPORT FileCopyJob *file_copy( const KUrl& src, const KUrl& dest, int permissions=-1,
                            bool overwrite=false, bool resume=false,
                            bool showProgressInfo = true);

    /**
     * Move a single file.
     *
     * Use either SlaveBase::rename() if the slave supports that,
     * or copy() and del() otherwise, or eventually get() & put() & del()
     * @param src Where to get the file.
     * @param dest Where to put the file.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param overwrite If @p true, any existing file will be overwritten.
     * @param resume true to resume an operation. Warning, setting this to true means
     *               that @p src will be appended to @p dest if @p dest exists.
     *               You probably don't want that, so leave it to false :)
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     */
    KIO_EXPORT FileCopyJob *file_move( const KUrl& src, const KUrl& dest, int permissions=-1,
                            bool overwrite=false, bool resume=false,
                            bool showProgressInfo = true);

    /**
     * Delete a single file.
     *
     * @param src File to delete.
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob *file_delete( const KUrl& src, bool showProgressInfo = true);

    /**
     * List the contents of @p url, which is assumed to be a directory.
     *
     * "." and ".." are returned, filter them out if you don't want them.
     *
     *
     * @param url the url of the directory
     * @param showProgressInfo true to show progress information
     * @param includeHidden true for all files, false to cull out UNIX hidden
     *                      files/dirs (whose names start with dot)
     * @return the job handling the operation.
     */
    KIO_EXPORT ListJob *listDir( const KUrl& url, bool showProgressInfo = true,
                      bool includeHidden = true );

    /**
     * The same as the previous method, but recurses subdirectories.
     * Directory links are not followed.
     *
     * "." and ".." are returned but only for the toplevel directory.
     * Filter them out if you don't want them.
     *
     * @param url the url of the directory
     * @param showProgressInfo true to show progress information
     * @param includeHidden true for all files, false to cull out UNIX hidden
     *                      files/dirs (whose names start with dot)
     * @return the job handling the operation.
     */
    KIO_EXPORT ListJob *listRecursive( const KUrl& url, bool showProgressInfo = true,
                            bool includeHidden = true );

    /**
     * Copy a file or directory @p src into the destination @p dest,
     * which can be a file (including the final filename) or a directory
     * (into which @p src will be copied).
     *
     * This emulates the cp command completely.
     *
     * @param src the file or directory to copy
     * @param dest the destination
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation
     * @see copyAs()
     */
    KIO_EXPORT CopyJob *copy( const KUrl& src, const KUrl& dest, bool showProgressInfo = true );

    /**
     * Copy a file or directory @p src into the destination @p dest,
     * which is the destination name in any case, even for a directory.
     *
     * As opposed to copy(), this doesn't emulate cp, but is the only
     * way to copy a directory, giving it a new name and getting an error
     * box if a directory already exists with the same name.
     *
     * @param src the file or directory to copy
     * @param dest the destination
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation
     */
    KIO_EXPORT CopyJob *copyAs( const KUrl& src, const KUrl& dest, bool showProgressInfo = true );

    /**
     * Copy a list of file/dirs @p src into a destination directory @p dest.
     *
     * @param src the list of files and/or directories
     * @param dest the destination
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation
     */
    KIO_EXPORT CopyJob *copy( const KUrl::List& src, const KUrl& dest, bool showProgressInfo = true );

    /**
     * Moves a file or directory @p src to the given destination @p dest.
     *
     * @param src the file or directory to copy
     * @param dest the destination
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation
     * @see copy()
     * @see moveAs()
     */
    KIO_EXPORT CopyJob *move( const KUrl& src, const KUrl& dest, bool showProgressInfo = true );
    /**
     * Moves a file or directory @p src to the given destination @p dest. Unlike move()
     * this operation will fail when the directory already exists.
     *
     * @param src the file or directory to copy
     * @param dest the destination
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation
     * @see copyAs()
     */
    KIO_EXPORT CopyJob *moveAs( const KUrl& src, const KUrl& dest, bool showProgressInfo = true );
    /**
     * Moves a list of files or directories @p src to the given destination @p dest.
     *
     * @param src the list of files or directories to copy
     * @param dest the destination
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation
     * @see copy()
     */
    KIO_EXPORT CopyJob *move( const KUrl::List& src, const KUrl& dest, bool showProgressInfo = true );

    /**
     * Create a link.
     * If the protocols and hosts are the same, a Unix symlink will be created.
     * Otherwise, a .desktop file of Type Link and pointing to the src URL will be created.
     *
     * @param src The existing file or directory, 'target' of the link.
     * @param destDir Destination directory where the link will be created.
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation
     */
    KIO_EXPORT CopyJob *link( const KUrl& src, const KUrl& destDir, bool showProgressInfo = true );

    /**
     * Create several links
     * If the protocols and hosts are the same, a Unix symlink will be created.
     * Otherwise, a .desktop file of Type Link and pointing to the src URL will be created.
     *
     * @param src The existing files or directories, 'targets' of the link.
     * @param destDir Destination directory where the links will be created.
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation
     * @see link()
     */
    KIO_EXPORT CopyJob *link( const KUrl::List& src, const KUrl& destDir, bool showProgressInfo = true );

    /**
     * Create a link. Unlike link() this operation will fail when the directory already
     * exists.
     * If the protocols and hosts are the same, a Unix symlink will be created.
     * Otherwise, a .desktop file of Type Link and pointing to the src URL will be created.
     *
     * @param src The existing file or directory, 'target' of the link.
     * @param dest Destination directory where the link will be created.
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation
     * @see link ()
     * @see copyAs()
     */
    KIO_EXPORT CopyJob *linkAs( const KUrl& src, const KUrl& dest, bool showProgressInfo = true );

    /**
     * Trash a file or directory.
     * This is currently only supported for local files and directories.
     * Use "KUrl src; src.setPath( path );" to create a URL from a path.
     *
     * @param src file to delete
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation
     * @since 3.4
     */
    KIO_EXPORT CopyJob *trash( const KUrl& src, bool showProgressInfo = true );

    /**
     * Trash a list of files or directories.
     * This is currently only supported for local files and directories.
     *
     * @param src the files to delete
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation
     * @since 3.4
     */
    KIO_EXPORT CopyJob *trash( const KUrl::List& src, bool showProgressInfo = true );

    /**
     * Delete a file or directory.
     *
     * @param src file to delete
     * @param shred obsolete (TODO remove in KDE4)
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation
     */
    KIO_EXPORT DeleteJob *del( const KUrl& src, bool shred = false, bool showProgressInfo = true );

    /**
     * Deletes a list of files or directories.
     *
     * @param src the files to delete
     * @param shred obsolete (TODO remove in KDE4)
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation
     */
    KIO_EXPORT DeleteJob *del( const KUrl::List& src, bool shred = false, bool showProgressInfo = true );
}

#endif

