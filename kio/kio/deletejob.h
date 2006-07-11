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

#ifndef KIO_DELETEJOB_H
#define KIO_DELETEJOB_H

#include <qstringlist.h>

#include <kurl.h>
#include <kio/global.h>

#include <kio/jobclasses.h>

class Observer;
class QTimer;

namespace KIO {

    /**
     * A more complex Job to delete files and directories.
     * Don't create the job directly, but use KIO::del() instead.
     *
     * @see KIO::del()
     */
    class KIO_EXPORT DeleteJob : public Job {
    Q_OBJECT

    public:
	/**
	 * Do not create a DeleteJob directly. Use KIO::del()
	 * instead.
	 *
	 * @param src the list of URLs to delete
	 * @param shred true to shred (make sure that data is not recoverable)a
	 * @param showProgressInfo true to show progress information to the user
	 * @see KIO::del()
	 */
        DeleteJob( const KUrl::List& src, bool shred, bool showProgressInfo );

	/**
	 * Returns the list of URLs.
	 * @return the list of URLs.
	 */
        KUrl::List urls() const { return m_srcList; }

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
	 * Sends the URL of the file that is currently being deleted.
	 * @param job the job that emitted this signal
	 * @param file the URL of the file or directory that is being
	 *        deleted
	 */
        void deleting( KIO::Job *job, const KUrl& file );

    protected Q_SLOTS:
        void slotStart();
        void slotEntries( KIO::Job*, const KIO::UDSEntryList& list );
        virtual void slotResult( KJob *job );

        /**
         * Forward signal from subjob
         */
        void slotProcessedSize( KJob*, qulonglong data_size );
        void slotReport();

    private:
        void statNextSrc();
        void deleteNextFile();
        void deleteNextDir();

    private:
        enum { STATE_STATING, STATE_LISTING,
               STATE_DELETING_FILES, STATE_DELETING_DIRS } state;
        KIO::filesize_t m_totalSize;
        KIO::filesize_t m_processedSize;
        KIO::filesize_t m_fileProcessedSize;
        int m_processedFiles;
        int m_processedDirs;
        int m_totalFilesDirs;
        KUrl m_currentURL;
        KUrl::List files;
        KUrl::List symlinks;
        KUrl::List dirs;
        KUrl::List m_srcList;
        KUrl::List::Iterator m_currentStat;
	QStringList m_parentDirs;
        QTimer *m_reportTimer;
    private:
	class DeleteJobPrivate* d;
    };

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
