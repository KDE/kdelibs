// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                  2000-2009 David Faure <faure@kde.org>

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

#ifndef KIO_JOB_H
#define KIO_JOB_H

#include <kio/jobclasses.h>

namespace KIO {

    enum LoadType { Reload, NoReload };

    class FileJob;

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
    KIO_EXPORT SimpleJob * mkdir( const KUrl& url, int permissions = -1 ); // TODO KDE5: return a MkdirJob and make that class public again

    /**
     * Removes a single directory.
     *
     * The directory is assumed to be empty.
     * The job will fail if the directory is not empty.
     * Use KIO::del() (DeleteJob) to delete non-empty directories.
     *
     * @param url The URL of the directory to remove.
     * @return A pointer to the job handling the operation.
     */
    KIO_EXPORT SimpleJob * rmdir( const KUrl& url );

    /**
     * Changes permissions on a file or directory.
     * See the other chmod in chmodjob.h for changing many files
     * or directories.
     *
     * @param url The URL of file or directory.
     * @param permissions The permissions to set.
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob * chmod( const KUrl& url, int permissions );

    /**
     * Changes ownership and group of a file or directory.
     *
     * @param url The URL of file or directory.
     * @param owner the new owner
     * @param group the new group
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob * chown( const KUrl& url, const QString& owner, const QString& group );

    /**
     * Changes the modification time on a file or directory.
     *
     * @param url The URL of file or directory.
     * @param permissions The permissions to set.
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob *setModificationTime( const KUrl& url, const QDateTime& mtime );


    /**
     * Rename a file or directory.
     * Warning: this operation fails if a direct renaming is not
     * possible (like with files or dirs on separate partitions)
     * Use move or file_move in this case.
     *
     * @param src The original URL
     * @param dest The final URL
     * @param flags Can be Overwrite here
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob * rename( const KUrl& src, const KUrl & dest, JobFlags flags = DefaultFlags );

    /**
     * Create or move a symlink.
     * This is the lowlevel operation, similar to file_copy and file_move.
     * It doesn't do any check (other than those the slave does)
     * and it doesn't show rename and skip dialogs - use KIO::link for that.
     * @param target The string that will become the "target" of the link (can be relative)
     * @param dest The symlink to create.
     * @param flags Can be Overwrite and HideProgressInfo
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob * symlink( const QString & target, const KUrl& dest, JobFlags flags = DefaultFlags );

    /**
     * Execute any command that is specific to one slave (protocol).
     *
     * Examples are : HTTP POST, mount and unmount (kio_file)
     *
     * @param url The URL isn't passed to the slave, but is used to know
     *        which slave to send it to :-)
     * @param data Packed data.  The meaning is completely dependent on the
     *        slave, but usually starts with an int for the command number.
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob * special( const KUrl& url, const QByteArray & data, JobFlags flags = DefaultFlags );

    /**
     * Mount filesystem.
     *
     * Special job for @p kio_file.
     *
     * @param ro Mount read-only if @p true.
     * @param fstype File system type (e.g. "ext2", can be empty).
     * @param dev Device (e.g. /dev/sda0).
     * @param point Mount point, can be @p null.
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob *mount( bool ro, const QByteArray& fstype, const QString& dev, const QString& point, JobFlags flags = DefaultFlags );

    /**
     * Unmount filesystem.
     *
     * Special job for @p kio_file.
     *
     * @param point Point to unmount.
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob *unmount( const QString & point, JobFlags flags = DefaultFlags );

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
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT StatJob * stat( const KUrl& url, JobFlags flags = DefaultFlags );
    /**
     * Find all details for one file or directory.
     * This version of the call includes two additional booleans, @p sideIsSource and @p details.
     *
     * @param url the URL of the file
     * @param side is SourceSide when stating a source file (we will do a get on it if
     * the stat works) and DestinationSide when stating a destination file (target of a copy).
     * The reason for this parameter is that in some cases the kioslave might not
     * be able to determine a file's existence (e.g. HTTP doesn't allow it, FTP
     * has issues with case-sensitivity on some systems).
     * When the slave can't reliably determine the existence of a file, it will:
     * @li be optimistic if SourceSide, i.e. it will assume the file exists,
     * and if it doesn't this will appear when actually trying to download it
     * @li be pessimistic if DestinationSide, i.e. it will assume the file
     * doesn't exist, to prevent showing "about to overwrite" errors to the user.
     * If you simply want to check for existence without downloading/uploading afterwards,
     * then you should use DestinationSide.
     *
     * @param details selects the level of details we want.
     * By default this is 2 (all details wanted, including modification time, size, etc.),
     * setDetails(1) is used when deleting: we don't need all the information if it takes
     * too much time, no need to follow symlinks etc.
     * setDetails(0) is used for very simple probing: we'll only get the answer
     * "it's a file or a directory or a symlink, or it doesn't exist". This is used by KRun and DeleteJob.
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT StatJob * stat( const KUrl& url, KIO::StatJob::StatSide side,
                               short int details, JobFlags flags = DefaultFlags );
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
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT_DEPRECATED StatJob * stat( const KUrl& url, bool sideIsSource,
                                          short int details, JobFlags flags = DefaultFlags );

    /**
     * Get (a.k.a. read).
     * This is the job to use in order to "download" a file into memory.
     * The slave emits the data through the data() signal.
     *
     * Special case: if you want to determine the mimetype of the file first,
     * and then read it with the appropriate component, you can still use
     * a KIO::get() directly. When that job emits the mimeType signal, (which is
     * guaranteed to happen before it emits any data), put the job on hold:
     * <code>
     *   job->putOnHold();
     *   KIO::Scheduler::publishSlaveOnHold();
     * </code>
     * and forget about the job. The next time someone does a KIO::get() on the
     * same URL (even in another process) this job will be resumed. This saves KIO
     * from doing two requests to the server.
     *
     * @param url the URL of the file
     * @param reload: Reload to reload the file, NoReload if it can be taken from the cache
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT TransferJob *get( const KUrl& url, LoadType reload = NoReload, JobFlags flags = DefaultFlags );

    /**
     * Open ( random access I/O )
     *
     * The file-job emits open() when opened
     * @param url the URL of the file
     * @param mode the access privileges: see \ref OpenMode
     *
     * @return The file-handling job. It will never return 0. Errors are handled asynchronously
     * (emitted as signals).
     */
    KIO_EXPORT FileJob *open(const KUrl &url, QIODevice::OpenMode mode);

    /**
     * Put (a.k.a. write)
     *
     * @param url Where to write data.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param flags Can be HideProgressInfo, Overwrite and Resume here. WARNING:
     * Setting Resume means that the data will be appended to @p dest if @p dest exists.
     * @return the job handling the operation.
     * @see multi_get()
     */
    KIO_EXPORT TransferJob *put( const KUrl& url, int permissions,
                                 JobFlags flags = DefaultFlags );

    /**
     * HTTP POST (for form data).
     *
     * Example:
     * \code
     *    job = KIO::http_post( url, postData, KIO::HideProgressInfo );
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
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT TransferJob *http_post( const KUrl& url, const QByteArray &postData,
                                       JobFlags flags = DefaultFlags );

    /**
     * Get (a.k.a. read), into a single QByteArray.
     * @see StoredTransferJob
     *
     * @param url the URL of the file
     * @param reload: Reload to reload the file, NoReload if it can be taken from the cache
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT StoredTransferJob *storedGet( const KUrl& url, LoadType reload = NoReload, JobFlags flags = DefaultFlags );

    /**
     * Put (a.k.a. write) data from a single QByteArray.
     * @see StoredTransferJob
     *
     * @param arr The data to write
     * @param url Where to write data.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param flags Can be HideProgressInfo, Overwrite and Resume here. WARNING:
     * Setting Resume means that the data will be appended to @p dest if @p dest exists.
     * @return the job handling the operation.
     */
    KIO_EXPORT StoredTransferJob *storedPut( const QByteArray& arr, const KUrl& url, int permissions,
                                             JobFlags flags = DefaultFlags );

    /**
     * HTTP POST (a.k.a. write) data from a single QByteArray.
     * @see StoredTransferJob
     *
     * @param arr The data to write
     * @param url Where to write data.
     * @param flags Can be HideProgressInfo here.
     * @return the job handling the operation.
     * @since 4.2
     */
    KIO_EXPORT StoredTransferJob *storedHttpPost( const QByteArray& arr, const KUrl& url,
                                                  JobFlags flags = DefaultFlags );

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
     * If you are going to download the file right after determining its mimetype,
     * then don't use this, prefer using a KIO::get() job instead. See the note
     * about putting the job on hold once the mimetype is determined.
     *
     * @param url the URL of the file
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT MimetypeJob * mimetype( const KUrl& url,
                                       JobFlags flags = DefaultFlags );

    /**
     * Copy a single file.
     *
     * Uses either SlaveBase::copy() if the slave supports that
     * or get() and put() otherwise.
     * @param src Where to get the file.
     * @param dest Where to put the file.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param flags Can be HideProgressInfo, Overwrite and Resume here. WARNING:
     * Setting Resume means that the data will be appended to @p dest if @p dest exists.
     * @return the job handling the operation.
     */
    KIO_EXPORT FileCopyJob *file_copy( const KUrl& src, const KUrl& dest, int permissions=-1,
                                       JobFlags flags = DefaultFlags );


    /**
     * Move a single file.
     *
     * Use either SlaveBase::rename() if the slave supports that,
     * or copy() and del() otherwise, or eventually get() & put() & del()
     * @param src Where to get the file.
     * @param dest Where to put the file.
     * @param permissions May be -1. In this case no special permission mode is set.
     * @param flags Can be HideProgressInfo, Overwrite and Resume here. WARNING:
     * Setting Resume means that the data will be appended to @p dest if @p dest exists.
     * @return the job handling the operation.
     */
    KIO_EXPORT FileCopyJob *file_move( const KUrl& src, const KUrl& dest, int permissions=-1,
                                       JobFlags flags = DefaultFlags );

    /**
     * Overload for catching code mistakes. Do NOT call this method (it is not implemented),
     * insert a value for permissions (-1 by default) before the JobFlags.
     * @since 4.3
     */
    FileCopyJob *file_move( const KUrl& src, const KUrl& dest, JobFlags flags ); // not implemented - on purpose.


    /**
     * Delete a single file.
     *
     * @param src File to delete.
     * @param flags Can be HideProgressInfo here
     * @return the job handling the operation.
     */
    KIO_EXPORT SimpleJob *file_delete( const KUrl& src, JobFlags flags = DefaultFlags );

    /**
     * List the contents of @p url, which is assumed to be a directory.
     *
     * "." and ".." are returned, filter them out if you don't want them.
     *
     *
     * @param url the url of the directory
     * @param flags Can be HideProgressInfo here
     * @param includeHidden true for all files, false to cull out UNIX hidden
     *                      files/dirs (whose names start with dot)
     * @return the job handling the operation.
     */
    KIO_EXPORT ListJob *listDir( const KUrl& url, JobFlags flags = DefaultFlags,
                                 bool includeHidden = true );

    /**
     * The same as the previous method, but recurses subdirectories.
     * Directory links are not followed.
     *
     * "." and ".." are returned but only for the toplevel directory.
     * Filter them out if you don't want them.
     *
     * @param url the url of the directory
     * @param flags Can be HideProgressInfo here
     * @param includeHidden true for all files, false to cull out UNIX hidden
     *                      files/dirs (whose names start with dot)
     * @return the job handling the operation.
     */
    KIO_EXPORT ListJob *listRecursive( const KUrl& url, JobFlags flags = DefaultFlags,
                            bool includeHidden = true );

    /**
     * Tries to map a local URL for the given URL, using a KIO job.
     *
     * Starts a (stat) job for determining the "most local URL" for a given URL.
     * Retrieve the result with StatJob::mostLocalUrl in the result slot.
     * @param url The URL we are testing.
     * \since 4.4
     */
    KIO_EXPORT StatJob* mostLocalUrl(const KUrl& url, JobFlags flags = DefaultFlags);

}

#endif

