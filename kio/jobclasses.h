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
#include <qguardedptr.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <kio/global.h>

class QTimer;

namespace KIO {

    class Slave;
    class SlaveInterface;


    /**
     * The base class for all jobs.
     * For all jobs created in an application, the code looks like
     *
     * <pre>
     *   KIO::Job * job = KIO::someoperation( some parameters );
     *   connect( job, SIGNAL( result( KIO::Job * ) ),
     *            this, SLOT( slotResult( KIO::Job * ) ) );
     * </pre>
     *   (other connects, specific to the job)
     *
     * And slotResult is usually at least:
     *
     * <pre>
     *  if ( job->error() )
     *      job->showErrorDialog( this or 0L  );
     * </pre>
     *
     */
    class Job : public QObject {
        Q_OBJECT

    protected:
        Job( bool showProgressInfo );

    public:
        virtual ~Job();

        /**
        * Abort this job.
        * This kills all subjobs and deletes the job
        * @param quietly if false, Job will emit signal @ref result
        * and ask kio_uiserver to close the progress window.
        * @p quietly is set to true for subjobs. Whether applications
        * should call with true or false depends on whether they rely
        * on result being emitted or not.
        */
        virtual void kill( bool quietly = true );

        /**
         * @return the error code for this job, 0 if no error
         * Error codes are defined in @ref KIO::Error.
         * Only call this method from the slot connected to @ref result.
         */
        int error() { return m_error; }

        /**
         * @return the progress id for this job, as returned by uiserver
         */
        int progressId() { return m_progressId; }

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

        /**
         * Associate this job with a window given by @p window
         */
        void setWindow(QWidget *window);

        /**
         * Returns the window this job is associated with.
         */
        QWidget *window() const;

    signals:
        /**
         * Emitted when the job is finished, in any case (completed, canceled,
         * failed...). Use @ref error to know the result.
         */
        void result( KIO::Job *job );

        /**
         * Emitted when the job is canceled.
         * @deprecated. Don't use !
         * Signal @p result is emitted as well, and error() is,
         * in this case, ERR_USER_CANCELED.
         */
        void canceled( KIO::Job *job );

        /**
         * Emitted to display information about this job, as sent by the slave.
         * Examples of message are "Resolving host", "Connecting to host...", etc.
         */
        void infoMessage( KIO::Job *, const QString & msg );

        /**
         * Emitted when the slave successfully connected to the host.
         * There is no guarantee the slave will send this, and this is
         * currently unused (in the applications).
         */
        void connected( KIO::Job * );

        /**
         * Progress signal showing the overall progress of the job
         * This is valid for any kind of job, and allows using a
         * a progress bar very easily (see @ref KProgress)
         */
        void percent( KIO::Job *job, unsigned long percent );

        /**
         * Emitted when we know the size of this job (data size for transfers,
         * number of entries for listings).
         */
        void totalSize( KIO::Job *, unsigned long size );

        /**
         * Regularly emitted to show the progress of this job
         * (current data size for transfers, entries listed).
         */
        void processedSize( KIO::Job *, unsigned long size );

        /**
         * Emitted to display information about the speed of this job.
         */
        void speed( KIO::Job *, unsigned long bytes_per_second );

    protected slots:
        /**
         * Called whenever a subjob finishes.
         * Default implementation checks for errors and propagates
         * to parent job, then calls @ref removeSubjob.
         * Override if you don't want subjobs errors to be propagated.
         */
        virtual void slotResult( KIO::Job *job );

        /**
         * Forward signal from subjob
         */
        void slotSpeed( KIO::Job*, unsigned long bytes_per_second );
        /**
         * Forward signal from subjob
         */
        void slotInfoMessage( KIO::Job*, const QString & );

        /**
         * Remove speed information
         */
        void slotSpeedTimeout();

    protected:
        /**
         * Add a job that has to be finished before a result
         * is emitted. This has obviously to be called before
         * the finish signal is emitted by the slave.
         */
        virtual void addSubjob( Job *job );

        /**
         * Mark a sub job as being done. If it's the last to
         * wait on the job will emit a result - jobs with
         * two steps might want to override slotResult
         * in order to avoid calling this method.
         */
        virtual void removeSubjob( Job *job );

        /**
         * Utility function for inherited jobs.
         * Emits the percent signal if bigger than m_percent,
         * after calculating it from the parameters.
         */
        void emitPercent( unsigned long processedSize, unsigned long totalSize );

        /**
         * Utility function for inherited jobs.
         * Emits the speed signal and starts the timer for removing that info
         */
        void emitSpeed( unsigned long bytes_per_second );

        /**
         * Utility function to emit the result signal, and suicide this job.
         * It first tells the observer to hide the progress dialog for this job.
         */
        void emitResult();

        QList<Job> subjobs;
        int m_error;
        QString m_errorText;
        unsigned long m_percent;
        int m_progressId; // for uiserver
        QTimer *m_speedTimer;
        QGuardedPtr<QWidget> m_window;
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
                  bool showProgressInfo);

        ~SimpleJob();

        const KURL& url() const { return m_url; }

        /**
         * Abort job
         * This kills all subjobs and deletes the job
         * @param quietly if true, Job will emit signal @ref result
         * Should only be set to false when the user kills the job
         * (from kio_uiserver), not when you want to abort a job.
         */
        virtual void kill( bool quietly = true );

        /**
         * Abort job
         * Suspends slave to be reused by another job for the same request.
         */
        virtual void putOnHold();

        /**
         * Discard suspended slave
         */
        static void removeOnHold();

        /**
         * @internal
         * Called by the scheduler when a slave gets to
         * work on this job.
         **/
        virtual void start( Slave *slave );

        /**
         * @internal
         * Called to detach a slave from a job.
         **/
        void slaveDone();

        /**
         * @internal
         * Slave in use by this job
         */
        Slave *slave() { return m_slave; }

    protected slots:
        /**
         * Called when the slave marks the job
         * as finished.
         */
        virtual void slotFinished( );

        /**
         * @internal
         * Called on a slave's warning
         */
        void slotWarning( const QString & );

        /**
         * Called on a slave's info message
         */
        void slotInfoMessage( const QString & );

        /**
         * Called on a slave's connected signal
         */
        void slotConnected();

        /**
         * Forward signal from the slave
         */
        void slotTotalSize( unsigned long data_size );
        /**
         * Forward signal from the slave
         */
        void slotProcessedSize( unsigned long data_size );
        /**
         * Forward signal from the slave
         */
        void slotSpeed( unsigned long bytes_per_second );

    public slots:
        /**
         * @internal
         * Called on a slave's error
         * Made public for the scheduler
         */
        virtual void slotError( int , const QString & );

    protected slots:
        /**
         * @internal
         */
        void slotNeedProgressId();

    protected:
        Slave * m_slave;
        QByteArray m_packedArgs;
        KURL m_url;
        KURL m_subUrl;
        int m_command;
        unsigned long m_totalSize;
    };

    // Stat Job
    class StatJob : public SimpleJob {

    Q_OBJECT

    public:
        StatJob(const KURL& url, int command, const QByteArray &packedArgs, bool showProgressInfo);

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
                    const QByteArray &_staticData,
                    bool showProgressInfo);

        virtual void start(Slave *slave);

        /**
         * Called when m_subJob finishes.
         */
        virtual void slotResult( KIO::Job *job );

        /**
         * Flow control. Suspend data processing from the slave.
         */
        void suspend();

        /**
         * Flow control. Resume data processing from the slave.
         */
        void resume();

        /**
         * Set meta data to be sent to the slave.
         */
        void setMetaData( const KIO::MetaData &);

        /**
         * Add key/value pair to the meta data that is sent to the slave
         */
        void addMetaData(const QString &key, const QString &value);

        /**
         * Add key/value pairs to the meta data that is sent to the slave
         */
        void addMetaData(const QMap<QString,QString> &values);

        /**
         * Get meta data received from the slave.
         * (Valid when first data is received and/or slave is finished)
         */
        MetaData metaData();

        /**
         * Query meta data received from the slave.
         * (Valid when first data is received and/or slave is finished)
         */
        QString queryMetaData(const QString &key);

        /**
         * @return true if we got an (HTML) error page from the server
         * instead of what we asked for. This currently only happens with
         * HTTP urls. Call this from your slot connected to result().
         */
        bool isErrorPage() const { return m_errorPage; }


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
        void redirection( KIO::Job *, const KURL &url );

        /**
         * Mimetype determined
         */
        void mimetype( KIO::Job *, const QString &type );

        /**
         * @internal
         * Emitted if the "put" job found an existing partial file
         * (in which case offset is the size of that file)
         * and emitted by the "get" job if it supports resuming to
         * the given offset - in this case @p offset is unused)
         */
        void canResume( KIO::Job *, unsigned long offset );


    protected slots:
        virtual void slotRedirection( const KURL &url);
        virtual void slotFinished();
        virtual void slotData( const QByteArray &data);
        virtual void slotDataReq();
        virtual void slotMimetype( const QString &mimetype );
        virtual void slotMetaData( const KIO::MetaData &_metaData);
        virtual void slotNeedSubURLData();
        virtual void slotSubURLData(KIO::Job*, const QByteArray &);
        void slotErrorPage();
        void slotCanResume( unsigned long offset );

    protected:
        bool m_suspended;
        bool m_errorPage;
        QByteArray staticData;
        KURL m_redirectionURL;
        KURL::List m_redirectionList;
        QString m_mimetype;
        MetaData m_outgoingMetaData;
        MetaData m_incomingMetaData;
        TransferJob *m_subJob;
    };

    // Mimetype Job
    class MimetypeJob : public TransferJob {
    Q_OBJECT

    public:
        MimetypeJob(const KURL& url, int command, const QByteArray &packedArgs, bool showProgressInfo);

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
        virtual void slotFinished( );
    };

    /**
     * The FileCopyJob copies data from one place to another.
     */
    class FileCopyJob : public Job {
    Q_OBJECT

    public:
        FileCopyJob( const KURL& src, const KURL& dest, int permissions,
                     bool move, bool overwrite, bool resume, bool showProgressInfo);

        KURL srcURL() const { return m_src; }
        KURL destURL() const { return m_dest; }

    public slots:
        void slotData( KIO::Job *, const QByteArray &data);
        void slotDataReq( KIO::Job *, QByteArray &data);

    protected slots:
        /**
         * Called whenever a subjob finishes.
         */
        virtual void slotResult( KIO::Job *job );

        /**
         * Forward signal from subjob
         */
        void slotProcessedSize( KIO::Job*, unsigned long size );
        /**
         * Forward signal from subjob
         */
        void slotTotalSize( KIO::Job*, unsigned long size );
        /**
         * Forward signal from subjob
         */
        void slotPercent( KIO::Job*, unsigned long pct );
        /**
         * Forward signal from subjob
         */
        void slotCanResume( KIO::Job*, unsigned long offset );

    protected:
        void startCopyJob();
        void startDataPump();
        void connectSubjob( SimpleJob * job );

        KURL m_src;
        KURL m_dest;
        int m_permissions;
        bool m_move:1;
        bool m_overwrite:1;
        bool m_resume:1;
        bool m_canResume:1;
        bool m_resumeAnswerSent:1;
        QByteArray m_buffer;
        SimpleJob *m_moveJob;
        SimpleJob *m_copyJob;
        TransferJob *m_getJob;
        TransferJob *m_putJob;
        SimpleJob *m_delJob;
        unsigned long m_totalSize;
    };

    class ListJob : public SimpleJob {
    Q_OBJECT

    public:
        ListJob(const KURL& url, bool showProgressInfo,
                bool recursive = false, QString prefix = QString::null);

        virtual void start( Slave *slave );

    signals:
        /**
         * This signal emits the entry found by the job while listing.
         * The progress signals aren't specific to ListJob. It simply
         * uses SimpleJob's @ref processedSize (number of entries listed) and
         * @ref totalSize (total number of entries, if known),
         * as well as percent.
         */
        void entries( KIO::Job *, const KIO::UDSEntryList& );

        /**
         * Signals a redirection
         * Use to update the URL shown to the user
         * The redirection itself is handled internally
         */
        void redirection( KIO::Job *, const KURL &url );

    protected slots:
        virtual void slotFinished( );
        virtual void slotResult( KIO::Job *job );
        void slotListEntries( const KIO::UDSEntryList& list );
        void slotRedirection( const KURL &url );
        void gotEntries( KIO::Job * subjob, const KIO::UDSEntryList& list );

    private:
        bool recursive;
        QString prefix;
        unsigned long m_processedEntries;
        KURL m_redirectionURL;
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

    // Copy or Move, files or directories
    class CopyJob : public Job {
    Q_OBJECT

    public:
        enum CopyMode{ Copy, Move, Link };
        CopyJob( const KURL::List& src, const KURL& dest, CopyMode mode, bool asMethod, bool showProgressInfo );

        KURL::List srcURLs() const { return m_srcList; }
        KURL destURL() const { return m_dest; }

    signals:

        void totalFiles( KIO::Job *, unsigned long files );
        void totalDirs( KIO::Job *, unsigned long dirs );

        void processedFiles( KIO::Job *, unsigned long files );
        void processedDirs( KIO::Job *, unsigned long dirs );

        /**
         * The job is copying a file or directory
         */
        void copying( KIO::Job *, const KURL& from, const KURL& to );
        /**
         * The job is creating a symbolic link
         */
        void linking( KIO::Job *, const QString& target, const KURL& to );
        /**
         * The job is moving a file or directory
         */
        void moving( KIO::Job *, const KURL& from, const KURL& to );
        /**
         * The job is creating the directory @p dir
         */
        void creatingDir( KIO::Job *, const KURL& dir );
        /**
         * The user chose to rename 'from' to 'to'
         */
        void renamed( KIO::Job *, const KURL& from, const KURL& to );

        /**
         * The job emits this signal when copying or moving a file or directory successfully finished.
         * @param src the source URL
         * @param dst the destination URL
         * @param direction indicates whether a file or directory was successfully copied/moved
         * @param renamed indicates that the destination URL was created using a
         * rename operation (i.e. fast directory moving).
         * This signal is mainly for the Undo feature.
         */
        void copyingDone( KIO::Job *, const KURL &from, const KURL &to, bool directory, bool renamed );
        /**
         * The job is copying or moving a symbolic link, that points to target.
         * The new link is created in @p to. The existing one is/was in @p from.
         */
        void copyingLinkDone( KIO::Job *, const KURL &from, const QString& target, const KURL& to );

    protected:
        void startNextJob();

        // Those aren't slots but submethods for slotResult.
        void slotResultStating( KIO::Job * job );
        void startListing( const KURL & src );
        void slotResultCreatingDirs( KIO::Job * job );
        void slotResultConflictCreatingDirs( KIO::Job * job );
        void createNextDir();
        void slotResultCopyingFiles( KIO::Job * job );
        void slotResultConflictCopyingFiles( KIO::Job * job );
        void copyNextFile();
        void slotResultDeletingDirs( KIO::Job * job );
        void deleteNextDir();
        void skip( const KURL & sourceURL );

    protected slots:
        void slotEntries( KIO::Job*, const KIO::UDSEntryList& list );
        virtual void slotResult( KIO::Job *job );
        /**
         * Forward signal from subjob
         */
        void slotProcessedSize( KIO::Job*, unsigned long data_size );
        /**
         * Forward signal from subjob
         */
        void slotTotalSize( KIO::Job*, unsigned long size );

    private:
        CopyMode m_mode;
        bool m_asMethod;
        enum { DEST_NOT_STATED, DEST_IS_DIR, DEST_IS_FILE, DEST_DOESNT_EXIST } destinationState;
        enum { STATE_STATING, STATE_RENAMING, STATE_LISTING, STATE_CREATING_DIRS,
               STATE_CONFLICT_CREATING_DIRS, STATE_COPYING_FILES, STATE_CONFLICT_COPYING_FILES,
               STATE_DELETING_DIRS } state;
        unsigned long m_totalSize;
        unsigned long m_processedSize;
        unsigned long m_fileProcessedSize;
        int m_processedFiles;
        int m_processedDirs;
        QValueList<CopyInfo> files;
        QValueList<CopyInfo> dirs;
        KURL::List dirsToRemove;
        KURL::List m_srcList; // is emptied while copying
        KURL::List m_srcListCopy;
        bool m_bCurrentSrcIsDir;
        bool m_bCurrentOperationIsLink;
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
        DeleteJob( const KURL::List& src, bool shred, bool showProgressInfo );

        KURL::List urls() const { return m_srcList; }

    signals:

        void totalFiles( KIO::Job *, unsigned long files );
        void totalDirs( KIO::Job *, unsigned long dirs );

        void processedFiles( KIO::Job *, unsigned long files );
        void processedDirs( KIO::Job *, unsigned long dirs );

        void deleting( KIO::Job *, const KURL& file );

    protected:
        void startNextJob();
        void deleteNextFile();
        void deleteNextDir();

    protected slots:
        void slotEntries( KIO::Job*, const KIO::UDSEntryList& list );
        virtual void slotResult( KIO::Job *job );

        /**
         * Forward signal from subjob
         */
        void slotProcessedSize( KIO::Job*, unsigned long data_size );

    private:
        enum { STATE_STATING, STATE_LISTING,
               STATE_DELETING_FILES, STATE_DELETING_DIRS } state;
        unsigned long m_totalSize;
        unsigned long m_processedSize;
        unsigned long m_fileProcessedSize;
        int m_processedFiles;
        int m_processedDirs;
        int m_totalFilesDirs;
        KURL::List files;
        KURL::List symlinks;
        KURL::List dirs;
        KURL::List m_srcList; // is emptied while deleting
        KURL::List m_srcListCopy;
        bool m_shred;
    };

};

#endif
