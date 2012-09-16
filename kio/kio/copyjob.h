// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright 2000       Stephan Kulow <coolo@kde.org>
    Copyright 2000-2006  David Faure <faure@kde.org>

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

#ifndef KIO_COPYJOB_H
#define KIO_COPYJOB_H

#include <QtCore/QObject>
#include <QtCore/QStringList>

#include <sys/types.h> // time_t

#include <QUrl>

#include "jobclasses.h"

class QTimer;

namespace KIO {

    /// @internal
    /// FIXME: If this is internal, why is being used in a public signal below?
    /// (aboutToCreate, see also konq_operations.h/cpp
    struct CopyInfo
    {
        QUrl uSource;
        QUrl uDest;
        QString linkDest; // for symlinks only
        int permissions;
        time_t ctime;
        time_t mtime;
        KIO::filesize_t size; // 0 for dirs
    };

    class CopyJobPrivate;
    /**
     * CopyJob is used to move, copy or symlink files and directories.
     * Don't create the job directly, but use KIO::copy(),
     * KIO::move(), KIO::link() and friends.
     *
     * @see KIO::copy()
     * @see KIO::copyAs()
     * @see KIO::move()
     * @see KIO::moveAs()
     * @see KIO::link()
     * @see KIO::linkAs()
     */
    class KIO_EXPORT CopyJob : public Job {

        Q_OBJECT

    public:
	/**
	 * Defines the mode of the operation
	 */
        enum CopyMode { Copy, Move, Link };

        virtual ~CopyJob();

	/**
	 * Returns the mode of the operation (copy, move, or link),
	 * depending on whether KIO::copy(), KIO::move() or KIO::link() was called.
	 */
        CopyMode operationMode() const;

	/**
	 * Returns the list of source URLs.
	 * @return the list of source URLs.
	 */
        QList<QUrl> srcUrls() const;

	/**
	 * Returns the destination URL.
	 * @return the destination URL
	 */
        QUrl destUrl() const;

        /**
         * By default the permissions of the copied files will be those of the source files.
         *
         * But when copying "template" files to "new" files, people prefer the umask
         * to apply, rather than the template's permissions.
         * For that case, call setDefaultPermissions(true)
         */
        void setDefaultPermissions( bool b );

        /**
         * Skip copying or moving any file when the destination already exists,
         * instead of the default behavior (interactive mode: showing a dialog to the user,
         * non-interactive mode: aborting with an error).
         * Initially added for a unit test.
         * \since 4.2
         */
        void setAutoSkip(bool autoSkip);

        /**
         * Rename files automatically when the destination already exists,
         * instead of the default behavior (interactive mode: showing a dialog to the user,
         * non-interactive mode: aborting with an error).
         * Initially added for a unit test.
         * \since 4.7
         */
        void setAutoRename(bool autoRename);

        /**
         * Reuse any directory that already exists, instead of the default behavior
         * (interactive mode: showing a dialog to the user,
         * non-interactive mode: aborting with an error).
         * \since 4.2
         */
        void setWriteIntoExistingDirectories(bool overwriteAllDirs);

        /**
         * Reimplemented for internal reasons
         */
        virtual bool doSuspend();

    Q_SIGNALS:

        /**
	 * Emitted when the total number of files is known.
	 * @param job the job that emitted this signal
	 * @param files the total number of files
	 */
        void totalFiles( KJob *job, unsigned long files );
        /**
	 * Emitted when the toal number of direcotries is known.
	 * @param job the job that emitted this signal
	 * @param dirs the total number of directories
	 */
        void totalDirs( KJob *job, unsigned long dirs );

        /**
	 * Emitted when it is known which files / directories are going
	 * to be created. Note that this may still change e.g. when
	 * existing files with the same name are discovered.
	 * @param job the job that emitted this signal
	 * @param files a list of items that are about to be created.
	 */
        void aboutToCreate( KIO::Job *job, const QList<KIO::CopyInfo> &files);

        /**
	 * Sends the number of processed files.
	 * @param job the job that emitted this signal
	 * @param files the number of processed files
	 */
        void processedFiles( KIO::Job *job, unsigned long files );
        /**
	 * Sends the number of processed directories.
	 * @param job the job that emitted this signal
	 * @param dirs the number of processed dirs
	 */
        void processedDirs( KIO::Job *job, unsigned long dirs );

        /**
         * The job is copying a file or directory.
         *
         * Note: This signal is used for progress dialogs, it's not emitted for
         * every file or directory (this would be too slow), but every 200ms.
         *
	 * @param job the job that emitted this signal
	 * @param src the URL of the file or directory that is currently
	 *             being copied
	 * @param dest the destination of the current operation
         */
        void copying(KIO::Job *job, const QUrl& src, const QUrl& dest);
        /**
         * The job is creating a symbolic link.
         *
         * Note: This signal is used for progress dialogs, it's not emitted for
         * every file or directory (this would be too slow), but every 200ms.
         *
	 * @param job the job that emitted this signal
	 * @param target the URL of the file or directory that is currently
	 *             being linked
	 * @param to the destination of the current operation
         */
        void linking(KIO::Job *job, const QString& target, const QUrl& to);
        /**
         * The job is moving a file or directory.
         *
         * Note: This signal is used for progress dialogs, it's not emitted for
         * every file or directory (this would be too slow), but every 200ms.
         *
	 * @param job the job that emitted this signal
	 * @param from the URL of the file or directory that is currently
	 *             being moved
	 * @param to the destination of the current operation
         */
        void moving(KIO::Job *job, const QUrl& from, const QUrl& to);
        /**
         * The job is creating the directory @p dir.
         *
         * This signal is emitted for every directory being created.
         *
	 * @param job the job that emitted this signal
	 * @param dir the directory that is currently being created
         */
        void creatingDir(KIO::Job *job, const QUrl& dir);
        /**
         * The user chose to rename @p from to @p to.
         *
	 * @param job the job that emitted this signal
	 * @param from the original name
	 * @param to the new name
         */
        void renamed(KIO::Job *job, const QUrl& from, const QUrl& to);

        /**
         * The job emits this signal when copying or moving a file or directory successfully finished.
         * This signal is mainly for the Undo feature.
         * If you simply want to know when a copy job is done, use result().
	 *
	 * @param job the job that emitted this signal
         * @param from the source URL
         * @param to the destination URL
         * @param mtime the modification time of the source file, hopefully set on the destination file
         * too (when the kioslave supports it).
         * @param directory indicates whether a file or directory was successfully copied/moved.
	 *                  true for a directory, false for file
         * @param renamed indicates that the destination URL was created using a
         * rename operation (i.e. fast directory moving). true if is has been renamed
         */
        void copyingDone(KIO::Job *job, const QUrl &from, const QUrl &to, time_t mtime, bool directory, bool renamed);
        /**
         * The job is copying or moving a symbolic link, that points to target.
         * The new link is created in @p to. The existing one is/was in @p from.
         * This signal is mainly for the Undo feature.
	 * @param job the job that emitted this signal
         * @param from the source URL
	 * @param target the target
         * @param to the destination URL
         */
        void copyingLinkDone(KIO::Job *job, const QUrl &from, const QString& target, const QUrl& to);
    protected Q_SLOTS:
        virtual void slotResult( KJob *job );

    protected:
        CopyJob(CopyJobPrivate &dd);
        void emitResult();

    private:
        Q_PRIVATE_SLOT(d_func(), void slotStart())
        Q_PRIVATE_SLOT(d_func(), void slotEntries( KIO::Job*, const KIO::UDSEntryList& list ))
        Q_PRIVATE_SLOT(d_func(), void slotSubError( KIO::ListJob*, KIO::ListJob* ))
        Q_PRIVATE_SLOT(d_func(), void slotProcessedSize( KJob*, qulonglong data_size ))
        Q_PRIVATE_SLOT(d_func(), void slotTotalSize( KJob*, qulonglong size ))
        Q_PRIVATE_SLOT(d_func(), void slotReport())

        Q_DECLARE_PRIVATE(CopyJob)
    };

    /**
     * Copy a file or directory @p src into the destination @p dest,
     * which can be a file (including the final filename) or a directory
     * (into which @p src will be copied).
     *
     * This emulates the cp command completely.
     *
     * @param src the file or directory to copy
     * @param dest the destination
     * @param flags: copy() supports HideProgressInfo and Overwrite.
     * Note: Overwrite has the meaning of both "write into existing directories" and
     * "overwrite existing files". However if "dest" exists, then src is copied
     * into a subdir of dest, just like "cp" does. Use copyAs if you don't want that.
     *
     * @return the job handling the operation
     * @see copyAs()
     */
    KIO_EXPORT CopyJob *copy(const QUrl& src, const QUrl& dest, JobFlags flags = DefaultFlags);

    /**
     * Copy a file or directory @p src into the destination @p dest,
     * which is the destination name in any case, even for a directory.
     *
     * As opposed to copy(), this doesn't emulate cp, but is the only
     * way to copy a directory, giving it a new name and getting an error
     * box if a directory already exists with the same name (or writing the
     * contents of @p src into @p dest, when using Overwrite).
     *
     * @param src the file or directory to copy
     * @param dest the destination
     * @param flags: copyAs() supports HideProgressInfo and Overwrite.
     * Note: Overwrite has the meaning of both "write into existing directories" and
     * "overwrite existing files".
     *
     * * @return the job handling the operation
     */
    KIO_EXPORT CopyJob *copyAs(const QUrl& src, const QUrl& dest, JobFlags flags = DefaultFlags);

    /**
     * Copy a list of file/dirs @p src into a destination directory @p dest.
     *
     * @param src the list of files and/or directories
     * @param dest the destination
     * @param flags: copy() supports HideProgressInfo and Overwrite.
     * Note: Overwrite has the meaning of both "write into existing directories" and
     * "overwrite existing files". However if "dest" exists, then src is copied
     * into a subdir of dest, just like "cp" does.
     * @return the job handling the operation
     */
    KIO_EXPORT CopyJob *copy(const QList<QUrl>& src, const QUrl& dest, JobFlags flags = DefaultFlags);

    /**
     * Moves a file or directory @p src to the given destination @p dest.
     *
     * @param src the file or directory to copy
     * @param dest the destination
     * @param flags: move() supports HideProgressInfo and Overwrite.
     * Note: Overwrite has the meaning of both "write into existing directories" and
     * "overwrite existing files". However if "dest" exists, then src is copied
     * into a subdir of dest, just like "cp" does.
     * @return the job handling the operation
     * @see copy()
     * @see moveAs()
     */
    KIO_EXPORT CopyJob *move(const QUrl& src, const QUrl& dest, JobFlags flags = DefaultFlags);
    /**
     * Moves a file or directory @p src to the given destination @p dest. Unlike move()
     * this operation will not move @p src into @p dest when @p dest exists: it will
     * either fail, or move the contents of @p src into it if Overwrite is set.
     *
     * @param src the file or directory to copy
     * @param dest the destination
     * @param flags: moveAs() supports HideProgressInfo and Overwrite.
     * Note: Overwrite has the meaning of both "write into existing directories" and
     * "overwrite existing files".
     * @return the job handling the operation
     * @see copyAs()
     */
    KIO_EXPORT CopyJob *moveAs(const QUrl& src, const QUrl& dest, JobFlags flags = DefaultFlags);
    /**
     * Moves a list of files or directories @p src to the given destination @p dest.
     *
     * @param src the list of files or directories to copy
     * @param dest the destination
     * @param flags: move() supports HideProgressInfo and Overwrite.
     * Note: Overwrite has the meaning of both "write into existing directories" and
     * "overwrite existing files". However if "dest" exists, then src is copied
     * into a subdir of dest, just like "cp" does.
     * @return the job handling the operation
     * @see copy()
     */
    KIO_EXPORT CopyJob *move(const QList<QUrl>& src, const QUrl& dest, JobFlags flags = DefaultFlags);

    /**
     * Create a link.
     * If the protocols and hosts are the same, a Unix symlink will be created.
     * Otherwise, a .desktop file of Type Link and pointing to the src URL will be created.
     *
     * @param src The existing file or directory, 'target' of the link.
     * @param destDir Destination directory where the link will be created.
     * @param flags: link() supports HideProgressInfo only
     * @return the job handling the operation
     */
    KIO_EXPORT CopyJob *link(const QUrl& src, const QUrl& destDir, JobFlags flags = DefaultFlags);

    /**
     * Create several links
     * If the protocols and hosts are the same, a Unix symlink will be created.
     * Otherwise, a .desktop file of Type Link and pointing to the src URL will be created.
     *
     * @param src The existing files or directories, 'targets' of the link.
     * @param destDir Destination directory where the links will be created.
     * @param flags: link() supports HideProgressInfo only
     * @return the job handling the operation
     * @see link()
     */
    KIO_EXPORT CopyJob *link(const QList<QUrl>& src, const QUrl& destDir, JobFlags flags = DefaultFlags);

    /**
     * Create a link. Unlike link() this operation will fail when the directory already
     * exists.
     * If the protocols and hosts are the same, a Unix symlink will be created.
     * Otherwise, a .desktop file of Type Link and pointing to the src URL will be created.
     *
     * @param src The existing file or directory, 'target' of the link.
     * @param dest Destination directory where the link will be created.
     * @param flags: linkAs() supports HideProgressInfo only
     * @return the job handling the operation
     * @see link ()
     * @see copyAs()
     */
    KIO_EXPORT CopyJob *linkAs(const QUrl& src, const QUrl& dest, JobFlags flags = DefaultFlags);

    /**
     * Trash a file or directory.
     * This is currently only supported for local files and directories.
     * Use QUrl::fromLocalFile to create a URL from a local file path.
     *
     * @param src file to delete
     * @param flags: trash() supports HideProgressInfo only
     * @return the job handling the operation
     */
    KIO_EXPORT CopyJob *trash(const QUrl& src, JobFlags flags = DefaultFlags);

    /**
     * Trash a list of files or directories.
     * This is currently only supported for local files and directories.
     *
     * @param src the files to delete
     * @param flags: trash() supports HideProgressInfo only
     * @return the job handling the operation
     */
    KIO_EXPORT CopyJob *trash(const QList<QUrl>& src, JobFlags flags = DefaultFlags);

}

#endif
