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

#ifndef __kio_jobclasses_h__
#define __kio_jobclasses_h__

#include <kurl.h>

#include <qobject.h>
#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <kio/global.h>

namespace KIO {

    class Slave;
    class SlaveInterface;


    /**
     * The base class for all jobs.
     * For all jobs created in an application, the code looks like
     *
     *   KIO::Job * job = KIO::someoperation( some parameters );
     *   connect( job, SIGNAL( result( KIO::Job * ) ),
     *            this, SLOT( slotResult( KIO::Job * ) ) );
     *   (other connects, specific to the job)
     *
     * And slotResult is usually at least:
     *
     *  if ( job->error() )
     *      job->showErrorDialog( this or 0L  );
     *
     */
    class Job : public QObject {
        Q_OBJECT

    protected:
        Job( bool showProgressInfo = true );

    public:
        virtual ~Job() {}

        /**
        * Abort job
        * This kills all subjobs and deletes the job
        */
        virtual void kill();

        /**
         * @return the error code for this job, 0 if no error
         * Error codes are defined in @ref KIO::Error.
         * Only call this method from the slot connected to @ref result.
         */
        int error() { return m_error; }

        /**
         * @return a string to help understand the error, usually the url
         * related to the error.
         * Only call if @ref error is not 0.
         * This is really internal, better use errorString or errorDialog.
         */
        const QString & errorText() { return m_errorText; }

        /**
         * Converts an error code and a non-i18n error message into an
         * error message in the current language. The low level (non-i18n)
         * error message (usually a url) is put into the translated error
         * message using %1.
         * Example for errid == ERR_CANNOT_OPEN_FOR_READING:
         *   i18n( "Could not read\n%1" ).arg( errortext );
         * Use this to display the error yourself, but for a dialog box
         * use @ref KIO::ErrorDialog.
         */
        QString errorString();

        /**
         * Display a dialog box to inform the user of the error given by
         * this job.
         * Only call if @ref error is not 0, and only in the slot connected
         * to @ref result.
         * @param parent the parent widget for the dialog box
         */
        void showErrorDialog( QWidget * parent = 0L );

    signals:
        /**
         * Emitted when the job is finished, in any case (completed, canceled,
         * failed...). Use @ref error to know the result.
         */
        void result( KIO::Job *job );

    protected slots:
        /**
         * Called whenever a subjob finishes.
         * Default implementation checks for errors and propagates
         * to parent job, then calls @ref removeSubjob.
         * Override if you don't want subjobs errors to be propagated.
         */
        virtual void slotResult( KIO::Job *job );

    protected:
        /**
         * Add a job that has to be finished before a result
         * is emitted. This has obviously to be called before
         * the finish signal is emitted by the slave.
         */
        virtual void addSubjob( Job *job );

        /**
         * Mark a sub job as beeing done. If it's the last to
         * wait on the job will emit a result - jobs with
         * two steps might want to override slotResult
         * in order to avoid calling this method.
         */
        virtual void removeSubjob( Job *job );

        QList<Job> subjobs;
        int m_error;
        QString m_errorText;
        int id; // for uiserver
    };

    /**
     * A simple job (one url and one command)
     * This is the base class for all jobs that are scheduled.
     * Other jobs are high-level jobs (CopyJob, DeleteJob, FileCopyJob...)
     * that manage subjobs but aren't scheduled directly.
     */
    class SimpleJob : public KIO::Job {
    Q_OBJECT

    public:
        SimpleJob(const KURL& url, int command, const QByteArray &packedArgs,
                  bool showProgressInfo = true);

        ~SimpleJob();

        const KURL& url() const { return m_url; }

        /**
         * Abort job
         * Reimplemented (to cancel the job in the scheduler as well)
         */
        virtual void kill();

        /**
         * @internal
         * Called by the scheduler when a slave gets to
         * work on this job.
         **/
        virtual void start( Slave *slave );

        /**
         * @internal
         * Slave in use by this job
         */
        Slave *slave() { return m_slave; }

    signals:
        void totalSize( KIO::Job *, unsigned long size );
        void processedSize( KIO::Job *, unsigned long size );
        void percent( KIO::Job *, unsigned long percent );
        void speed( KIO::Job *, unsigned long bytes_per_second );

    protected slots:
        /**
         * Called when the slave marks the job
         * as finished.
         */
        virtual void slotFinished( );

    public slots:
        /**
         * @internal
         * Called on a slave's error
         * Made public for the scheduler
         */
        virtual void slotError( int , const QString & );

        void slotTotalSize( unsigned long data_size );
        void slotProcessedSize( unsigned long data_size );
        void slotSpeed( unsigned long bytes_per_second );

    protected:
        Slave * m_slave;
        QByteArray m_packedArgs;
        KURL m_url;
        int m_command;
        unsigned long m_totalSize;
        unsigned long m_percent;
    };

    // Stat Job
    class StatJob : public SimpleJob {

    Q_OBJECT

    public:
        StatJob(const KURL& url, int command, const QByteArray &packedArgs);

        /**
         * Call this in the slot connected to @ref result,
         * and only after making sure no error happened.
         */
        const UDSEntry & statResult() const { return m_statResult; }

        /**
         * Called by the scheduler when a slave gets to
         * work on this job.
         */
        virtual void start( Slave *slave );

    protected slots:
        void slotStatEntry( const KIO::UDSEntry & entry );

    protected:
        UDSEntry m_statResult;
    };

    /**
     * The tranfer job pumps data into and/or out of a Slave.
     * Data is sent to the slave on request of the slave (@ref dataReq).
     * If data coming from the slave can not be handled, the
     * reading of data from the slave should be suspended.
     */
    class TransferJob : public SimpleJob {
    Q_OBJECT

    public:
        TransferJob(const KURL& url, int command,
                    const QByteArray &packedArgs,
                    const QByteArray &_staticData = QByteArray());

        virtual void start(Slave *slave);

        /**
         * Flow control. Suspend data processing from the slave.
         */
        void suspend();

        /**
         * Flow control. Resume data processing from the slave.
         */
        void resume();

    signals:
        /**
         * Data from the slave has arrived.
         * @param data data received from the slave
         * End of data (EOD) has been reached if data.size() == 0
         */
        void data( KIO::Job *, const QByteArray &data);

        /**
         * Request for data.
         *
         * @param data buffer to fill with data to send to the
         * slave. An empty buffer indicates end of data. (EOD)
         */

        void dataReq( KIO::Job *, QByteArray &data);

        /**
         * Signals a redirection
         * Use to update the URL shown to the user
         * The redirection itself is handled internally
         */
        void redirection( const KURL &url );

    protected slots:
        virtual void slotRedirection( const KURL &url);
        virtual void slotFinished();
        virtual void slotData( const QByteArray &data);
        virtual void slotDataReq();

    protected:
        bool m_suspended;
        QByteArray staticData;
        KURL m_redirectionURL;
    };

    // Mimetype Job
    class MimetypeJob : public TransferJob {
    Q_OBJECT

    public:
        MimetypeJob(const KURL& url, int command, const QByteArray &packedArgs);

        /**
         * Call this in the slot connected to @ref result,
         * and only after making sure no error happened.
         */
         QString mimetype() const { return m_mimetype; }

        /**
         * Called by the scheduler when a slave gets to
         * work on this job.
         */
        virtual void start( Slave *slave );

    protected slots:
        virtual void slotData( KIO::Job *, const QByteArray &data);
        void slotMimetype( const QString &mimetype );
        virtual void slotFinished( );

    protected:
        QString m_mimetype;
    };

    /**
     * The FileCopyJob copies data from one place to another.
     */
    class FileCopyJob : public Job {
    Q_OBJECT

    public:
        FileCopyJob( const KURL& src, const KURL& dest, int permissions,
                     bool move, bool overwrite, bool resume);

    public slots:
        void slotData( KIO::Job *, const QByteArray &data);
        void slotDataReq( KIO::Job *, QByteArray &data);

    protected slots:
        /**
         * Called whenever a subjob finishes.
         */
        virtual void slotResult( KIO::Job *job );

    signals:
        void totalSize( KIO::Job *, unsigned long size );
        void processedSize( KIO::Job *, unsigned long size );
        void percent( KIO::Job *, unsigned long percent );
        void speed( KIO::Job *, unsigned long bytes_per_second );


    protected:
        void startCopyJob();
        void startDataPump();

        KURL m_src;
        KURL m_dest;
        int m_permissions;
        bool m_move;
        bool m_overwrite;
        bool m_resume;
        QByteArray m_buffer;
        SimpleJob *m_moveJob;
        SimpleJob *m_copyJob;
        TransferJob *m_getJob;
        TransferJob *m_putJob;
        SimpleJob *m_delJob;
    };

    class ListJob : public SimpleJob {
    Q_OBJECT

    public:
        ListJob(const KURL& url, bool showProgressInfo = true,
                bool recursive = false, QString prefix = QString::null);

        virtual void start( Slave *slave );

    signals:
        /**
         * This signal emits the entry found by the job while listing.
         */
        void entries( KIO::Job *, const KIO::UDSEntryList& );

    protected slots:
        virtual void slotResult( KIO::Job *job );
        void slotListEntries( const KIO::UDSEntryList& list );
        void slotTotalEntries( unsigned long count );
        void gotEntries( KIO::Job * subjob, const KIO::UDSEntryList& list );

    private:
        bool recursive;
        QString prefix;
    };

    struct CopyInfo
    {
        KURL uSource;
        KURL uDest;
        QString linkDest; // for symlinks only
        mode_t permissions;
        mode_t type;
        time_t ctime;
        time_t mtime;
        off_t size; // 0 for dirs
    };

    // Copy or Move
    class CopyJob : public Job {
    Q_OBJECT

    public:
        CopyJob( const KURL::List& src, const KURL& dest, bool move = false, bool observe = true );

    signals:

        void totalSize( KIO::Job *, unsigned long size );
        void totalFiles( KIO::Job *, unsigned long files );
        void totalDirs( KIO::Job *, unsigned long dirs );

        void processedSize( KIO::Job *, unsigned long size );
        void processedFiles( KIO::Job *, unsigned long files );
        void processedDirs( KIO::Job *, unsigned long dirs );

        void percent( KIO::Job *, unsigned long percent );

        void speed( KIO::Job *, unsigned long bytes_per_second );

        void copying( KIO::Job *, const KURL& from, const KURL& to );
        void moving( KIO::Job *, const KURL& from, const KURL& to );
        /**
         * The @p job is creating the directory @dir
         */
        void creatingDir( KIO::Job *, const KURL& dir );
        void renaming( KIO::Job *, const KURL& old_name, const KURL& new_name );

        void canResume( KIO::Job *, bool can_resume );

    protected:
        void startNextJob();

        // Those aren't slots but submethods for slotResult.
        void slotResultStating( KIO::Job * job );
        void slotResultCreatingDirs( KIO::Job * job );
        void slotResultConflictCreatingDirs( KIO::Job * job );
        void createNextDir();
        void slotResultCopyingFiles( KIO::Job * job );
        void slotResultConflictCopyingFiles( KIO::Job * job );
        void copyNextFile();
        void slotResultDeletingDirs( KIO::Job * job );
        void deleteNextDir();

    protected slots:
        void slotEntries( KIO::Job*, const KIO::UDSEntryList& list );
        virtual void slotResult( KIO::Job *job );

        void slotProcessedSize( KIO::Job*, unsigned long data_size );
        void slotSpeed( KIO::Job*, unsigned long bytes_per_second );

    private:
        bool m_move;
        enum { DEST_NOT_STATED, DEST_IS_DIR, DEST_IS_FILE, DEST_DOESNT_EXIST } destinationState;
        enum { STATE_STATING, STATE_LISTING, STATE_CREATING_DIRS, STATE_CONFLICT_CREATING_DIRS,
               STATE_COPYING_FILES, STATE_CONFLICT_COPYING_FILES, STATE_DELETING_DIRS } state;
        unsigned long m_totalSize;
        unsigned long m_processedSize;
        unsigned long m_fileProcessedSize;
        unsigned long m_percent;
        QValueList<CopyInfo> files;
        QValueList<CopyInfo> dirs;
        KURL::List dirsToRemove;
        KURL::List m_srcList;
        bool m_bCurrentSrcIsDir;
        KURL m_dest;
        KURL m_currentDest;
        //
        QStringList m_skipList;
        QStringList m_overwriteList;
        bool m_bAutoSkip;
        bool m_bOverwriteAll;
        int m_conflictError;
    };

    class DeleteJob : public Job {
    Q_OBJECT

    public:
        DeleteJob( const KURL::List& src, bool shred, bool observe = true );

    signals:

        void totalSize( KIO::Job *, unsigned long size );
        void totalFiles( KIO::Job *, unsigned long files );
        void totalDirs( KIO::Job *, unsigned long dirs );

        void processedSize( KIO::Job *, unsigned long size );
        void processedFiles( KIO::Job *, unsigned long files );
        void processedDirs( KIO::Job *, unsigned long dirs );

        void percent( KIO::Job *, unsigned long percent );

        void deleting( KIO::Job *, const KURL& file );

    protected:
        void startNextJob();
        void deleteNextFile();
        void deleteNextDir();

    protected slots:
        void slotEntries( KIO::Job*, const KIO::UDSEntryList& list );
        virtual void slotResult( KIO::Job *job );

        void slotProcessedSize( KIO::Job*, unsigned long data_size );

    private:
        enum { STATE_STATING, STATE_LISTING,
               STATE_DELETING_FILES, STATE_DELETING_DIRS } state;
        unsigned long m_totalSize;
        unsigned long m_processedSize;
        unsigned long m_fileProcessedSize;
        unsigned long m_percent;
        KURL::List files;
        KURL::List symlinks;
        KURL::List dirs;
        KURL::List m_srcList;
        bool m_shred;
    };

};

#endif
