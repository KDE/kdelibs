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

#include <qobject.h>
#include <qstringlist.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <kurl.h>

#include <kio/jobclasses.h>

class Observer;
class QTimer;

namespace KIO {

    /// @internal
    struct KIO_EXPORT CopyInfo
    {
        KUrl uSource;
        KUrl uDest;
        QString linkDest; // for symlinks only
        int permissions;
        //mode_t type;
        time_t ctime;
        time_t mtime;
        KIO::filesize_t size; // 0 for dirs
    };

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
        enum CopyMode{ Copy, Move, Link };

	/**
	 * Do not create a CopyJob directly. Use KIO::copy(),
	 * KIO::move(), KIO::link() and friends instead.
	 *
	 * @param src the list of source URLs
	 * @param dest the destination URL
	 * @param mode specifies whether the job should copy, move or link
	 * @param asMethod if true, behaves like KIO::copyAs(),
	 * KIO::moveAs() or KIO::linkAs()
	 * @param showProgressInfo true to show progress information to the user
	 * @see KIO::copy()
	 * @see KIO::copyAs()
	 * @see KIO::move()
	 * @see KIO::moveAs()
	 * @see KIO::link()
	 * @see KIO::linkAs()
	 */
        CopyJob( const KUrl::List& src, const KUrl& dest, CopyMode mode, bool asMethod, bool showProgressInfo );

        virtual ~CopyJob();

	/**
	 * Returns the list of source URLs.
	 * @return the list of source URLs.
	 */
        KUrl::List srcURLs() const { return m_srcList; }

	/**
	 * Returns the destination URL.
	 * @return the destination URL
	 */
        KUrl destURL() const { return m_dest; }

        /**
         * By default the permissions of the copied files will be those of the source files.
         *
         * But when copying "template" files to "new" files, people prefer the umask
         * to apply, rather than the template's permissions.
         * For that case, call setDefaultPermissions(true)
         */
        void setDefaultPermissions( bool b );

        /**
         * Reimplemented for internal reasons
         */
        virtual void suspend();

    Q_SIGNALS:

        /**
	 * Emitted when the total number of files is known.
	 * @param job the job that emitted this signal
	 * @param files the total number of files
	 */
        void totalFiles( KIO::Job *job, unsigned long files );
        /**
	 * Emitted when the toal number of direcotries is known.
	 * @param job the job that emitted this signal
	 * @param dirs the total number of directories
	 */
        void totalDirs( KIO::Job *job, unsigned long dirs );

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
	 * @param job the job that emitted this signal
	 * @param src the URL of the file or directory that is currently
	 *             being copied
	 * @param dest the destination of the current operation
         */
        void copying( KIO::Job *job, const KUrl& src, const KUrl& dest );
        /**
         * The job is creating a symbolic link.
	 * @param job the job that emitted this signal
	 * @param target the URL of the file or directory that is currently
	 *             being linked
	 * @param to the destination of the current operation
         */
        void linking( KIO::Job *job, const QString& target, const KUrl& to );
        /**
         * The job is moving a file or directory.
	 * @param job the job that emitted this signal
	 * @param from the URL of the file or directory that is currently
	 *             being moved
	 * @param to the destination of the current operation
         */
        void moving( KIO::Job *job, const KUrl& from, const KUrl& to );
        /**
         * The job is creating the directory @p dir.
	 * @param job the job that emitted this signal
	 * @param dir the directory that is currently being created
         */
        void creatingDir( KIO::Job *job, const KUrl& dir );
        /**
         * The user chose to rename @p from to @p to.
	 * @param job the job that emitted this signal
	 * @param from the original name
	 * @param to the new name
         */
        void renamed( KIO::Job *job, const KUrl& from, const KUrl& to );

        /**
         * The job emits this signal when copying or moving a file or directory successfully finished.
         * This signal is mainly for the Undo feature.
	 *
	 * @param job the job that emitted this signal
         * @param from the source URL
         * @param to the destination URL
         * @param directory indicates whether a file or directory was successfully copied/moved.
	 *                  true for a directory, false for file
         * @param renamed indicates that the destination URL was created using a
         * rename operation (i.e. fast directory moving). true if is has been renamed
         */
        void copyingDone( KIO::Job *job, const KUrl &from, const KUrl &to, bool directory, bool renamed );
        /**
         * The job is copying or moving a symbolic link, that points to target.
         * The new link is created in @p to. The existing one is/was in @p from.
         * This signal is mainly for the Undo feature.
	 * @param job the job that emitted this signal
         * @param from the source URL
	 * @param target the target
         * @param to the destination URL
         */
        void copyingLinkDone( KIO::Job *job, const KUrl &from, const QString& target, const KUrl& to );

    protected:
        void statCurrentSrc();
        void statNextSrc();

        // Those aren't slots but submethods for slotResult.
        void slotResultStating( KJob * job );
        void startListing( const KUrl & src );
        void slotResultCreatingDirs( KJob * job );
        void slotResultConflictCreatingDirs( KJob * job );
        void createNextDir();
        void slotResultCopyingFiles( KJob * job );
        void slotResultConflictCopyingFiles( KJob * job );
        void copyNextFile();
        void slotResultDeletingDirs( KJob * job );
        void deleteNextDir();
        void skip( const KUrl & sourceURL );
        void slotResultRenaming( KJob * job );
        //void slotResultSettingDirAttributes( KJob * job );
        void setNextDirAttribute();
    private:
        void startRenameJob(const KUrl &slave_url);
        bool shouldOverwrite( const QString& path ) const;
        bool shouldSkip( const QString& path ) const;
        void skipSrc();

    protected Q_SLOTS:
        void slotStart();
        void slotEntries( KIO::Job*, const KIO::UDSEntryList& list );
        virtual void slotResult( KJob *job );
        /**
         * Forward signal from subjob
         */
        void slotProcessedSize( KJob*, qulonglong data_size );
        /**
         * Forward signal from subjob
	 * @param size the total size
         */
        void slotTotalSize( KJob*, qulonglong size );

        void slotReport();

    private:
        CopyMode m_mode;
        bool m_asMethod;
        enum DestinationState { DEST_NOT_STATED, DEST_IS_DIR, DEST_IS_FILE, DEST_DOESNT_EXIST };
        DestinationState destinationState;
        enum { STATE_STATING, STATE_RENAMING, STATE_LISTING, STATE_CREATING_DIRS,
               STATE_CONFLICT_CREATING_DIRS, STATE_COPYING_FILES, STATE_CONFLICT_COPYING_FILES,
               STATE_DELETING_DIRS, STATE_SETTING_DIR_ATTRIBUTES } state;
        KIO::filesize_t m_totalSize;
        KIO::filesize_t m_processedSize;
        KIO::filesize_t m_fileProcessedSize;
        int m_processedFiles;
        int m_processedDirs;
        QList<CopyInfo> files;
        QList<CopyInfo> dirs;
        KUrl::List dirsToRemove;
        KUrl::List m_srcList;
        KUrl::List::Iterator m_currentStatSrc;
        bool m_bCurrentSrcIsDir;
        bool m_bCurrentOperationIsLink;
        bool m_bSingleFileCopy;
        bool m_bOnlyRenames;
        KUrl m_dest;
        KUrl m_currentDest;
        //
        QStringList m_skipList;
        QStringList m_overwriteList;
        bool m_bAutoSkip;
        bool m_bOverwriteAll;
        int m_conflictError;

        QTimer *m_reportTimer;
        //these both are used for progress dialog reporting
        KUrl m_currentSrcURL;
        KUrl m_currentDestURL;
    private:
	class CopyJobPrivate;
        CopyJobPrivate* d;
        friend class CopyJobPrivate; // for DestinationState
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
     */
    KIO_EXPORT CopyJob *trash( const KUrl& src, bool showProgressInfo = true );

    /**
     * Trash a list of files or directories.
     * This is currently only supported for local files and directories.
     *
     * @param src the files to delete
     * @param showProgressInfo true to show progress information
     * @return the job handling the operation
     */
    KIO_EXPORT CopyJob *trash( const KUrl::List& src, bool showProgressInfo = true );

}

#endif
