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
#include <qptrlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qguardedptr.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <kio/global.h>

class Observer;
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
         * This kills all subjobs and deletes the job.
         * @param quietly if false, Job will emit signal @ref #result
         * and ask kio_uiserver to close the progress window.
         * @p quietly is set to true for subjobs. Whether applications
         * should call with true or false depends on whether they rely
         * on result being emitted or not.
         */
        virtual void kill( bool quietly = true );

        /**
         * @return the error code for this job, 0 if no error
         * Error codes are defined in @ref KIO::Error.
         * Only call this method from the slot connected to @ref result().
         */
        int error() const { return m_error; }

        /**
         * @return the progress id for this job, as returned by uiserver
         */
        int progressId() const { return m_progressId; }

        /**
         * @return a string to help understand the error, usually the url
         * related to the error.
         * Only call if @ref #error is not 0.
         * This is really internal, better use errorString or errorDialog.
         */
        const QString & errorText() const { return m_errorText; }

        /**
         * Converts an error code and a non-i18n error message into an
         * error message in the current language. The low level (non-i18n)
         * error message (usually a url) is put into the translated error
         * message using %1.
         *
         * Example for errid == ERR_CANNOT_OPEN_FOR_READING:
         *   i18n( "Could not read\n%1" ).arg( errortext );
         * Use this to display the error yourself, but for a dialog box
         * use @ref Job::showErrorDialog.
         */
        QString errorString() const;

        /**
         * Converts an error code and a non-i18n error message into i18n
         * strings suitable for presentation in a detailed error message box.
         *
         * @p reqUrl the request URL that generated this error message
         * @p method the method that generated this error message
         * (unimplemented)
         *
         * Returns strings: caption, error + description, causes+solutions
         */
        QStringList detailedErrorStrings(const KURL *reqUrl = 0L,
                                         int method = -1) const;

        /**
         * Display a dialog box to inform the user of the error given by
         * this job.
         * Only call if @ref #error is not 0, and only in the slot connected
         * to @ref #result.
         * @param parent the parent widget for the dialog box
         */
        void showErrorDialog( QWidget * parent = 0L );

        /**
         * Enable or disable the automatic error handling. When automatic
         * error handling is enabled and an error occurs, then showErrorDialog
         * is called with the specified parentWidget (if supplied) , right before
         * the emission of the result signal.
         *
         * The default is false.
         *
         * See also @ref #isAutoErrorHandlingEnabled , @ref #showErrorDialog
         *
         * @param enable enable or disable automatic error handling
         * @param parentWidget the parent widget, passed to @ref #showErrorDialog
         */
        void setAutoErrorHandlingEnabled( bool enable, QWidget *parentWidget = 0 );

        /**
         * Returns whether automatic error handling is enabled or disabled.
         * See also @ref #setAutoErrorHandlingEnabled .
         */
        bool isAutoErrorHandlingEnabled() const;

        /**
         * Associate this job with a window given by @p window.
         */
        void setWindow(QWidget *window);

        /**
         * Returns the window this job is associated with.
         */
        QWidget *window() const;

        /**
         * Set meta data to be sent to the slave.
         */
        void setMetaData( const KIO::MetaData &);

        /**
         * Add key/value pair to the meta data that is sent to the slave.
         */
        void addMetaData(const QString &key, const QString &value);

        /**
         * Add key/value pairs to the meta data that is sent to the slave.
         * If a certain key already existed, it will be overridden.
         */
        void addMetaData(const QMap<QString,QString> &values);

        /**
         * Add key/value pairs to the meta data that is sent to the slave.
         * If a certain key already existed, it will remain unchanged.
         */
        void mergeMetaData(const QMap<QString,QString> &values);

        /**
         * @internal. For the scheduler. Do not use.
         */
        MetaData outgoingMetaData() const;

        /**
         * Get meta data received from the slave.
         * (Valid when first data is received and/or slave is finished)
         */
        MetaData metaData() const;

        /**
         * Query meta data received from the slave.
         * (Valid when first data is received and/or slave is finished)
         */
        QString queryMetaData(const QString &key);

    signals:
        /**
         * Emitted when the job is finished, in any case (completed, canceled,
         * failed...). Use @ref #error to know the result.
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
        // KDE 3.0: Separate rich-text string from plain-text string, for different widgets.

        /**
         * Emitted when the slave successfully connected to the host.
         * There is no guarantee the slave will send this, and this is
         * currently unused (in the applications).
         */
        void connected( KIO::Job * );

        /**
         * Progress signal showing the overall progress of the job
         * This is valid for any kind of job, and allows using a
         * a progress bar very easily. (see @ref KProgress).
	 * Note that this signal is not emitted for finished jobs
	 * (i.e. percent is never 100).
         */
        void percent( KIO::Job *job, unsigned long percent );

        /**
         * Emitted when we know the size of this job (data size for transfers,
         * number of entries for listings).
         */
        void totalSize( KIO::Job *, KIO::filesize_t size );

        /**
         * Regularly emitted to show the progress of this job
         * (current data size for transfers, entries listed).
         */
        void processedSize( KIO::Job *, KIO::filesize_t size );

        /**
         * Emitted to display information about the speed of this job.
         */
        void speed( KIO::Job *, unsigned long bytes_per_second );

    protected slots:
        /**
         * Called whenever a subjob finishes.
         * Default implementation checks for errors and propagates
         * to parent job, then calls @ref #removeSubjob.
         * Override if you don't want subjobs errors to be propagated.
         */
        virtual void slotResult( KIO::Job *job );

        /**
         * Forward signal from subjob.
         */
        void slotSpeed( KIO::Job*, unsigned long bytes_per_second );
        /**
         * Forward signal from subjob.
         */
        void slotInfoMessage( KIO::Job*, const QString & );

        /**
         * Remove speed information.
         */
        void slotSpeedTimeout();

    protected:
        /**
         * Add a job that has to be finished before a result
         * is emitted. This has obviously to be called before
         * the finish signal is emitted by the slave.
         *
         * If @p inheritMetaData is true, the subjob will
         * inherit the meta data from this job.
         */
        virtual void addSubjob( Job *job, bool inheritMetaData=true );

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
        void emitPercent( KIO::filesize_t processedSize, KIO::filesize_t totalSize );

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

        QPtrList<Job> subjobs;
        int m_error;
        QString m_errorText;
        unsigned long m_percent;
        int m_progressId; // for uiserver
        QTimer *m_speedTimer;
        QGuardedPtr<QWidget> m_window;
        MetaData m_outgoingMetaData;
        MetaData m_incomingMetaData;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
        class JobPrivate;
        JobPrivate *d;
    };

    /**
     * A simple job (one url and one command).
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
         * Abort job.
         * This kills all subjobs and deletes the job.
         * @param quietly if true, Job will emit signal @ref #result
         * Should only be set to false when the user kills the job
         * (from kio_uiserver), not when you want to abort a job.
         */
        virtual void kill( bool quietly = true );

        /**
         * Abort job.
         * Suspends slave to be reused by another job for the same request.
         */
        virtual void putOnHold();

        /**
         * Discard suspended slave.
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
         * Slave in use by this job.
         */
        Slave *slave() const { return m_slave; }

        /**
         * @internal
         */
        int command() const { return m_command; }

    protected slots:
        /**
         * Called when the slave marks the job
         * as finished.
         */
        virtual void slotFinished( );

        /**
         * @internal
         * Called on a slave's warning.
         */
        void slotWarning( const QString & );

        /**
         * Called on a slave's info message.
         */
        void slotInfoMessage( const QString & );

        /**
         * Called on a slave's connected signal.
         */
        void slotConnected();

        /**
         * Forward signal from the slave
         */
        void slotTotalSize( KIO::filesize_t data_size );
        /**
         * Forward signal from the slave
         */
        void slotProcessedSize( KIO::filesize_t data_size );
        /**
         * Forward signal from the slave
         */
        void slotSpeed( unsigned long bytes_per_second );

        /**
         * MetaData from the slave is received.
         */
        virtual void slotMetaData( const KIO::MetaData &_metaData);

    public slots:
        /**
         * @internal
         * Called on a slave's error.
         * Made public for the scheduler.
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
        KIO::filesize_t m_totalSize;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class SimpleJobPrivate* d;
    };

    // Stat Job
    class StatJob : public SimpleJob {

    Q_OBJECT

    public:
        StatJob(const KURL& url, int command, const QByteArray &packedArgs, bool showProgressInfo);

        /** A stat() can have two meanings. Either we want to read from this URL,
         * or to check if we can write to it. First case is "source", second is "dest".
         * It is necessary to know what the StatJob is for, to tune the kioslave's behaviour
         * (e.g. with FTP)
         */
        void setSide( bool source ) { m_bSource = source; }

        /**
         * Selects the level of details we want.
         * By default this is 2 (all details wanted, including modification time, size, etc.),
         * setDetails(1) is used when deleting: we don't need all the information if it takes
         * too much time, no need to follow symlinks etc.
         * setDetails(0) is used for very simple probing: we'll only get the answer
         * "it's a file or a directory, or it doesn't exist". This is used by KRun.
         */
        void setDetails( short int details ) { m_details = details; }

        /**
         * Call this in the slot connected to @ref #result,
         * and only after making sure no error happened.
         */
        const UDSEntry & statResult() const { return m_statResult; }

        /**
         * Called by the scheduler when a slave gets to
         * work on this job.
         */
        virtual void start( Slave *slave );

    signals:
        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
         */
        void redirection( KIO::Job *, const KURL &url );

        /**
         * Signals a permanent redirection.
         * The redirection itself is handled internally.
	 * @since 3.1
         */
        void permanentRedirection( KIO::Job *, const KURL &fromUrl, const KURL &toUrl );

    protected slots:
        void slotStatEntry( const KIO::UDSEntry & entry );
        void slotRedirection( const KURL &url);
        virtual void slotFinished();

    protected:
        UDSEntry m_statResult;
        KURL m_redirectionURL;
        bool m_bSource;
        short int m_details;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
        class StatJobPrivate;
        StatJobPrivate *d;
    };

    /**
     * The transfer job pumps data into and/or out of a Slave.
     * Data is sent to the slave on request of the slave (@ref #dataReq).
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
         * Flow control. Returns true if the job is suspended.
         */
	bool isSuspended() const { return m_suspended; }


        /**
         * @return true if we got an (HTML) error page from the server
         * instead of what we asked for. This currently only happens with
         * HTTP urls. Call this from your slot connected to result().
         */
        bool isErrorPage() const { return m_errorPage; }


    signals:
        /**
         * Data from the slave has arrived.
         * @param data data received from the slave.
         * End of data (EOD) has been reached if data.size() == 0
         */
        void data( KIO::Job *, const QByteArray &data);

        /**
         * Request for data.
         * Please note, that you shouldn't put too large chunks
         * of data in it as this requires copies within the frame
         * work, so you should rather split the data you want
         * to pass here in reasonable chunks (about 1MB maximum)
         *
         * @param data buffer to fill with data to send to the
         * slave. An empty buffer indicates end of data. (EOD)
         */

        void dataReq( KIO::Job *, QByteArray &data);

        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
         */
        void redirection( KIO::Job *, const KURL &url );

        /**
         * Signals a permanent redirection.
         * The redirection itself is handled internally.
	 * @since 3.1
         */
        void permanentRedirection( KIO::Job *, const KURL &fromUrl, const KURL &toUrl );

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
        void canResume( KIO::Job *, KIO::filesize_t offset );


    protected slots:
        virtual void slotRedirection( const KURL &url);
        virtual void slotFinished();
        virtual void slotData( const QByteArray &data);
        virtual void slotDataReq();
        virtual void slotMimetype( const QString &mimetype );
        virtual void slotNeedSubURLData();
        virtual void slotSubURLData(KIO::Job*, const QByteArray &);
        void slotErrorPage();
        void slotCanResume( KIO::filesize_t offset );

    protected:
        bool m_suspended;
        bool m_errorPage;
        QByteArray staticData;
        KURL m_redirectionURL;
        KURL::List m_redirectionList;
        QString m_mimetype;
        TransferJob *m_subJob;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class TransferJobPrivate* d;
    };

    // MultiGet Job
    class MultiGetJob : public TransferJob {
    Q_OBJECT

    public:
        MultiGetJob(const KURL& url, bool showProgressInfo);

        virtual void start(Slave *slave);

        void get(long id, const KURL &url, const MetaData &metaData);

    signals:
        /**
         * Data from the slave has arrived.
         * @param data data received from the slave.
         * End of data (EOD) has been reached if data.size() == 0
         */
        void data( long id, const QByteArray &data);

        /**
         * Mimetype determined
         */
        void mimetype( long id, const QString &type );

        /**
         * File transfer completed.
         *
         * When all files have been processed, result(KIO::Job *) gets
         * emitted.
         */
        void result( long id);

    protected slots:
        virtual void slotRedirection( const KURL &url);
        virtual void slotFinished();
        virtual void slotData( const QByteArray &data);
        virtual void slotMimetype( const QString &mimetype );
    private:
        struct GetRequest {
        public:
           GetRequest(long _id, const KURL &_url, const MetaData &_metaData)
             : id(_id), url(_url), metaData(_metaData) { }
           long id;
           KURL url;
           MetaData metaData;
        };
        bool findCurrentEntry();
        void flushQueue(QPtrList<GetRequest> &queue);

        QPtrList<GetRequest> m_waitQueue;
        QPtrList<GetRequest> m_activeQueue;
        bool b_multiGetActive;
        GetRequest *m_currentEntry;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class MultiGetJobPrivate* d;
    };

    // Mimetype Job
    class MimetypeJob : public TransferJob {
    Q_OBJECT

    public:
        MimetypeJob(const KURL& url, int command, const QByteArray &packedArgs, bool showProgressInfo);

        /**
         * Call this in the slot connected to @ref #result,
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
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class MimetypeJobPrivate* d;
    };

    /**
     * The FileCopyJob copies data from one place to another.
     */
    class FileCopyJob : public Job {
    Q_OBJECT

    public:
        FileCopyJob( const KURL& src, const KURL& dest, int permissions,
                     bool move, bool overwrite, bool resume, bool showProgressInfo);

        ~FileCopyJob();
        /**
         * If you know the size of the source file, call this method
         * to inform this job. It will be displayed in the "resume" dialog.
         */
        void setSourceSize( off_t size );

        KURL srcURL() const { return m_src; }
        KURL destURL() const { return m_dest; }

    public slots:
        void slotStart();
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
        void slotProcessedSize( KIO::Job*, KIO::filesize_t size );
        /**
         * Forward signal from subjob
         */
        void slotTotalSize( KIO::Job*, KIO::filesize_t size );
        /**
         * Forward signal from subjob
         */
        void slotPercent( KIO::Job*, unsigned long pct );
        /**
         * Forward signal from subjob
         */
        void slotCanResume( KIO::Job*, KIO::filesize_t offset );

    protected:
        void startCopyJob();
        void startCopyJob(const KURL &slave_url);
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
        KIO::filesize_t m_totalSize;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class FileCopyJobPrivate;
	FileCopyJobPrivate* d;
    };

    class ListJob : public SimpleJob {
    Q_OBJECT

    public:
        ListJob(const KURL& url, bool showProgressInfo,
                bool recursive = false, QString prefix = QString::null,
                bool includeHidden = true);

        virtual void start( Slave *slave );

    signals:
        /**
         * This signal emits the entry found by the job while listing.
         * The progress signals aren't specific to ListJob. It simply
         * uses SimpleJob's @ref #processedSize (number of entries listed) and
         * @ref #totalSize (total number of entries, if known),
         * as well as percent.
         */
        void entries( KIO::Job *, const KIO::UDSEntryList& );

        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
         */
        void redirection( KIO::Job *, const KURL &url );

        /**
         * Signals a permanent redirection.
         * The redirection itself is handled internally.
	 * @since 3.1
         */
        void permanentRedirection( KIO::Job *, const KURL &fromUrl, const KURL &toUrl );

    protected slots:
        virtual void slotFinished( );
        virtual void slotResult( KIO::Job *job );
        void slotListEntries( const KIO::UDSEntryList& list );
        void slotRedirection( const KURL &url );
        void gotEntries( KIO::Job * subjob, const KIO::UDSEntryList& list );

    private:
        bool recursive;
        bool includeHidden;
        QString prefix;
        unsigned long m_processedEntries;
        KURL m_redirectionURL;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class ListJobPrivate* d;
    };

    struct CopyInfo
    {
        KURL uSource;
        KURL uDest;
        QString linkDest; // for symlinks only
        int permissions;
        //mode_t type;
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
         * This signal is mainly for the Undo feature.
         */
        void copyingLinkDone( KIO::Job *, const KURL &from, const QString& target, const KURL& to );

    protected:
        void statNextSrc();

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
        void slotStart();
        void slotEntries( KIO::Job*, const KIO::UDSEntryList& list );
        virtual void slotResult( KIO::Job *job );
        /**
         * Forward signal from subjob
         */
        void slotProcessedSize( KIO::Job*, KIO::filesize_t data_size );
        /**
         * Forward signal from subjob
         */
        void slotTotalSize( KIO::Job*, KIO::filesize_t size );

        void slotReport();
    private:
        CopyMode m_mode;
        bool m_asMethod;
        enum { DEST_NOT_STATED, DEST_IS_DIR, DEST_IS_FILE, DEST_DOESNT_EXIST } destinationState;
        enum { STATE_STATING, STATE_RENAMING, STATE_LISTING, STATE_CREATING_DIRS,
               STATE_CONFLICT_CREATING_DIRS, STATE_COPYING_FILES, STATE_CONFLICT_COPYING_FILES,
               STATE_DELETING_DIRS } state;
        KIO::filesize_t m_totalSize;
        KIO::filesize_t m_processedSize;
        KIO::filesize_t m_fileProcessedSize;
        int m_processedFiles;
        int m_processedDirs;
        QValueList<CopyInfo> files;
        QValueList<CopyInfo> dirs;
        KURL::List dirsToRemove;
        KURL::List m_srcList;
        KURL::List::Iterator m_currentStatSrc;
        bool m_bCurrentSrcIsDir;
        bool m_bCurrentOperationIsLink;
        bool m_bSingleFileCopy;
        bool m_bOnlyRenames;
        KURL m_dest;
        KURL m_currentDest;
        //
        QStringList m_skipList;
        QStringList m_overwriteList;
        bool m_bAutoSkip;
        bool m_bOverwriteAll;
        int m_conflictError;

        QTimer *m_reportTimer;
        //these both are used for progress dialog reporting
        KURL m_currentSrcURL;
        KURL m_currentDestURL;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class CopyJobPrivate* d;
    };

    /*!
     * A more complex Job to delete files and directories.
     */
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

    protected slots:
        void slotStart();
        void slotEntries( KIO::Job*, const KIO::UDSEntryList& list );
        virtual void slotResult( KIO::Job *job );

        /**
         * Forward signal from subjob
         */
        void slotProcessedSize( KIO::Job*, KIO::filesize_t data_size );
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
        KURL m_currentURL;
        KURL::List files;
        KURL::List symlinks;
        KURL::List dirs;
        KURL::List m_srcList;
        KURL::List::Iterator m_currentStat;
	QStringList m_parentDirs;
        bool m_shred;
        QTimer *m_reportTimer;
    protected:
        /// \internal
	virtual void virtual_hook( int id, void* data );
    private:
	class DeleteJobPrivate* d;
    };

};

#endif
