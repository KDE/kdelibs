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

#ifndef __kio_jobclasses_h__
#define __kio_jobclasses_h__

#include <qobject.h>
#include <qlinkedlist.h>
#include <qstringlist.h>
#include <qpointer.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <kurl.h>
#include <kio/global.h>

#include <kcompositejob.h>

class Observer;
class QTimer;

namespace KIO {

    class JobUiDelegate;
    class Slave;
    class SlaveInterface;

    /**
     * The base class for all jobs.
     * For all jobs created in an application, the code looks like
     *
     * \code
     *   KIO::Job * job = KIO::someoperation( some parameters );
     *   connect( job, SIGNAL( result( KJob * ) ),
     *            this, SLOT( slotResult( KJob * ) ) );
     * \endcode
     *   (other connects, specific to the job)
     *
     * And slotResult is usually at least:
     *
     * \code
     *  if ( job->error() )
     *      job->ui()->showErrorDialog();
     * \endcode
     * @see KIO::Scheduler
     * @see KIO::Slave
     */
    class KIO_EXPORT Job : public KCompositeJob {
        Q_OBJECT

    protected:
        Job( bool showProgressInfo );

    public:
        virtual ~Job();
        void start() {} // Since KIO autostarts its jobs

        /**
         * Retrieves the UI delegate of this job.
         *
         * This method is basically a convenience for static_cast<KIO::JobUiDelegate*>(uiDelegate()).
         *
         * @return the delegate used by the job to communicate with the UI
         */
        JobUiDelegate *ui() const;

        /**
         * Abort this job.
         * This kills all subjobs and deletes the job.
	 *
         * @param quietly if false, Job will emit signal result
         * and ask kio_uiserver to close the progress window.
         * @p quietly is set to true for subjobs. Whether applications
         * should call with true or false depends on whether they rely
         * on result being emitted or not.
         */
        virtual bool doKill();

        /**
         * Suspend this job
         * @see resume
         */
        virtual void suspend();

        /**
         * Resume this job
         * @see suspend
         */
        virtual void resume();

        /**
         * @return true if this job was suspended by suspend()
         */
        bool isSuspended() const;

        /**
         * Converts an error code and a non-i18n error message into an
         * error message in the current language. The low level (non-i18n)
         * error message (usually a url) is put into the translated error
         * message using %1.
         *
         * Example for errid == ERR_CANNOT_OPEN_FOR_READING:
	 * \code
         *   i18n( "Could not read\n%1" ).arg( errortext );
	 * \endcode
         * Use this to display the error yourself, but for a dialog box
         * use Job::showErrorDialog. Do not call it if error()
	 * is not 0.
	 * @return the error message and if there is no error, a message
	 *         telling the user that the app is broken, so check with
	 *         error() whether there is an error
         */
        QString errorString() const;

        /**
         * Converts an error code and a non-i18n error message into i18n
         * strings suitable for presentation in a detailed error message box.
         *
         * @param reqUrl the request URL that generated this error message
         * @param method the method that generated this error message
         * (unimplemented)
         * @return the following strings: caption, error + description,
	 *         causes+solutions
         */
        QStringList detailedErrorStrings(const KUrl *reqUrl = 0L,
                                         int method = -1) const;

        /**
         * Display a dialog box to inform the user of the error given by
         * this job.
         * Only call if error is not 0, and only in the slot connected
         * to result.
         * @param parent the parent widget for the dialog box, can be 0 for
         *        top-level
         * @deprecated you should use job->ui()->setWindow(parent)
         *             and job->ui()->showErrorMessage() instead
         */
        KDE_DEPRECATED void showErrorDialog( QWidget *parent = 0 );

        /**
         * Returns whether message display is enabled or disabled.
         * See also setInteractive .
         * @return true if message display is enabled
         * @see setInteractive()
         */
        bool isInteractive() const;

        /**
         * Set the parent Job.
         * One example use of this is when FileCopyJob calls open_RenameDlg,
         * it must pass the correct progress ID of the parent CopyJob
         * (to hide the progress dialog).
	 * You can set the parent job only once. By default a job does not
	 * have a parent job.
	 * @param parentJob the new parent job
         */
        void setParentJob( Job* parentJob );

        /**
	 * Returns the parent job, if there is one.
	 * @return the parent job, or 0 if there is none
         * @see setParentJob
         */
        Job* parentJob() const;

        /**
         * Set meta data to be sent to the slave, replacing existing
	 * meta data.
	 * @param metaData the meta data to set
	 * @see addMetaData()
	 * @see mergeMetaData()
         */
        void setMetaData( const KIO::MetaData &metaData);

        /**
         * Add key/value pair to the meta data that is sent to the slave.
	 * @param key the key of the meta data
	 * @param value the value of the meta data
	 * @see setMetaData()
	 * @see mergeMetaData()
         */
        void addMetaData(const QString &key, const QString &value);

        /**
         * Add key/value pairs to the meta data that is sent to the slave.
         * If a certain key already existed, it will be overridden.
	 * @param values the meta data to add
	 * @see setMetaData()
	 * @see mergeMetaData()
         */
        void addMetaData(const QMap<QString,QString> &values);

        /**
         * Add key/value pairs to the meta data that is sent to the slave.
         * If a certain key already existed, it will remain unchanged.
	 * @param values the meta data to merge
	 * @see setMetaData()
	 * @see addMetaData()
         */
        void mergeMetaData(const QMap<QString,QString> &values);

        /**
         * @internal. For the scheduler. Do not use.
         */
        MetaData outgoingMetaData() const;

        /**
         * Get meta data received from the slave.
         * (Valid when first data is received and/or slave is finished)
	 * @return the job's meta data
         */
        MetaData metaData() const;

        /**
         * Query meta data received from the slave.
         * (Valid when first data is received and/or slave is finished)
	 * @param key the key of the meta data to retrieve
	 * @return the value of the meta data, or QString() if the
	 *         @p key does not exist
         */
        QString queryMetaData(const QString &key);

    Q_SIGNALS:
        /**
         * @deprecated. Don't use !
         * Emitted when the job is canceled.
         * Signal result() is emitted as well, and error() is,
         * in this case, ERR_USER_CANCELED.
	 * @param job the job that emitted this signal
         */
        void canceled( KJob *job );

        /**
         * Emitted when the slave successfully connected to the host.
         * There is no guarantee the slave will send this, and this is
         * currently unused (in the applications).
	 * @param job the job that emitted this signal
         */
        void connected( KIO::Job *job );

        /**
         * Emitted to display information about the speed of this job.
	 * @param job the job that emitted this signal
	 * @param speed the speed in bytes/s
         */
        void speed( KIO::Job *job, unsigned long speed );

    protected Q_SLOTS:
        /**
         * Forward signal from subjob.
	 * @param job the subjob
	 * @param speed the speed in bytes/s
	 * @see speed()
         */
        void slotSpeed( KIO::Job *job, unsigned long speed );

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
	 * @param job the subjob to add
         * @param inheritMetaData if true, the subjob will
         * inherit the meta data from this job.
         */
        virtual void addSubjob( Job *job, bool inheritMetaData=true );

        /**
         * Mark a sub job as being done.
         *
         * KDE4 change: this doesn't terminate the parent job anymore, call emitResult to do that.
	 *
	 * @param job the subjob to remove
         * @param mergeMetaData if set, the metadata received by the subjob is
         *                      merged into this job.
         */
        void removeSubjob( KJob *job, bool mergeMetaData = false );

        /**
         * Utility function for inherited jobs.
         * Emits the speed signal and starts the timer for removing that info
	 *
	 * @param speed the speed in bytes/s
         */
        void emitSpeed( unsigned long speed );

        /**
         * @internal
         * Some extra storage space for jobs that don't have their own
         * private d pointer.
         */
        enum { EF_TransferJobAsync    = (1 << 0),
               EF_TransferJobNeedData = (1 << 1),
               EF_TransferJobDataSent = (1 << 2),
               EF_ListJobUnrestricted = (1 << 3) };
        int &extraFlags();

        MetaData m_incomingMetaData;
        MetaData m_outgoingMetaData;

    private:
        QTimer *m_speedTimer;
        QPointer<QWidget> m_window;

        class JobPrivate;
        JobPrivate *d;
    };


    /**
     * A simple job (one url and one command).
     * This is the base class for all jobs that are scheduled.
     * Other jobs are high-level jobs (CopyJob, DeleteJob, FileCopyJob...)
     * that manage subjobs but aren't scheduled directly.
     */
    class KIO_EXPORT SimpleJob : public KIO::Job {
    Q_OBJECT

    public:
        /**
	 * Creates a new simple job. You don't need to use this constructor,
	 * unless you create a new job that inherits from SimpleJob.
	 * @param url the url of the job
	 * @param command the command of the job
	 * @param packedArgs the arguments
	 * @param showProgressInfo true to show progress information to the user
	 */
        SimpleJob(const KUrl& url, int command, const QByteArray &packedArgs,
                  bool showProgressInfo);

        ~SimpleJob();

        virtual void start();

        /**
         * Suspend this job
         * @see resume
         */
        virtual void suspend();

        /**
         * Resume this job
         * @see suspend
         */
        virtual void resume();

        /**
	 * Returns the SimpleJob's URL
	 * @return the url
	 */
        const KUrl& url() const { return m_url; }

        /**
         * Abort job.
         * This kills all subjobs and deletes the job.
         */
        virtual bool doKill();

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

    public Q_SLOTS:
        /**
         * Forward signal from the slave
         * Can also be called by the parent job, when it knows the size.
	 * @param data_size the total size
         */
        void slotTotalSize( KIO::filesize_t data_size );

    protected Q_SLOTS:
        /**
         * Called when the slave marks the job
         * as finished.
         */
        virtual void slotFinished( );

        /**
         * @internal
         * Called on a slave's warning.
         */
        virtual void slotWarning( const QString & );

        /**
         * Called on a slave's info message.
	 * @param s the info message
	 * @see infoMessage()
         */
        virtual void slotInfoMessage( const QString &s );

        /**
         * Called on a slave's connected signal.
	 * @see connected()
         */
        void slotConnected();

        /**
         * Forward signal from the slave.
	 * @param data_size the processed size in bytes
	 * @see processedSize()
         */
        void slotProcessedSize( KIO::filesize_t data_size );
        /**
         * Forward signal from the slave.
	 * @param speed the speed in bytes/s
	 * @see speed()
         */
        void slotSpeed( unsigned long speed );

        /**
         * MetaData from the slave is received.
	 * @param _metaData the meta data
	 * @see metaData()
         */
        virtual void slotMetaData( const KIO::MetaData &_metaData);

    public Q_SLOTS:
        /**
         * @internal
         * Called on a slave's error.
         * Made public for the scheduler.
         */
        virtual void slotError( int , const QString & );

    protected Q_SLOTS:
        /**
         * @internal
         */
        void slotNeedProgressId();

    protected:
        Slave * m_slave;
        QByteArray m_packedArgs;
        KUrl m_url;
        KUrl m_subUrl;
        int m_command;
        KIO::filesize_t m_totalSize;
    protected:
	/*
	 * Allow jobs that inherit SimpleJob and are aware
	 * of redirections to store the SSL session used.
	 * Retrieval is handled by SimpleJob::start
	 * @param m_redirectionURL Reference to redirection URL,
	 * used instead of m_url if not empty
	 */
	void storeSSLSessionFromJob(const KUrl &m_redirectionURL);
    private:
	class SimpleJobPrivate* d;
    };

    /**
     * A KIO job that retrieves information about a file or directory.
     * @see KIO::stat()
     */
    class KIO_EXPORT StatJob : public SimpleJob {

    Q_OBJECT

    public:
        /**
	 * Do not use this constructor to create a StatJob, use KIO::stat() instead.
	 * @param url the url of the file or directory to check
	 * @param command the command to issue
	 * @param packedArgs the arguments
	 * @param showProgressInfo true to show progress information to the user
	 */
        StatJob(const KUrl& url, int command, const QByteArray &packedArgs, bool showProgressInfo);

        /**
	 * A stat() can have two meanings. Either we want to read from this URL,
         * or to check if we can write to it. First case is "source", second is "dest".
         * It is necessary to know what the StatJob is for, to tune the kioslave's behavior
         * (e.g. with FTP).
	 * @param source true for "source" mode, false for "dest" mode
         */
        void setSide( bool source ) { m_bSource = source; }

        /**
         * Selects the level of @p details we want.
         * By default this is 2 (all details wanted, including modification time, size, etc.),
         * setDetails(1) is used when deleting: we don't need all the information if it takes
         * too much time, no need to follow symlinks etc.
         * setDetails(0) is used for very simple probing: we'll only get the answer
         * "it's a file or a directory, or it doesn't exist". This is used by KRun.
	 * @param details 2 for all details, 1 for simple, 0 for very simple
         */
        void setDetails( short int details ) { m_details = details; }

        /**
         * Call this in the slot connected to result,
         * and only after making sure no error happened.
	 * @return the result of the stat
         */
        const UDSEntry & statResult() const { return m_statResult; }

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start( Slave *slave );

    Q_SIGNALS:
        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
	 * @param job the job that is redirected
	 * @param url the new url
         */
        void redirection( KIO::Job *job, const KUrl &url );

        /**
         * Signals a permanent redirection.
         * The redirection itself is handled internally.
	 * @param job the job that is redirected
	 * @param fromUrl the original URL
	 * @param toUrl the new URL
         */
        void permanentRedirection( KIO::Job *job, const KUrl &fromUrl, const KUrl &toUrl );

    protected Q_SLOTS:
        void slotStatEntry( const KIO::UDSEntry & entry );
        void slotRedirection( const KUrl &url);
        virtual void slotFinished();
        virtual void slotMetaData( const KIO::MetaData &_metaData);

    protected:
        UDSEntry m_statResult;
        KUrl m_redirectionURL;
        bool m_bSource;
        short int m_details;
    private:
        class StatJobPrivate;
        StatJobPrivate *d;
    };

    /**
     * A KIO job that creates a directory
     * @see KIO::mkdir()
     */
    class KIO_EXPORT MkdirJob : public SimpleJob {

    Q_OBJECT

    public:
        /**
	 * Do not use this constructor to create a MkdirJob, use KIO::mkdir() instead.
	 * @param url the url of the file or directory to check
	 * @param command the command to issue
	 * @param packedArgs the arguments
	 * @param showProgressInfo true to show progress information to the user
	 */
        MkdirJob(const KUrl& url, int command, const QByteArray &packedArgs, bool showProgressInfo);

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start( Slave *slave );

    Q_SIGNALS:
        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
	 * @param job the job that is redirected
	 * @param url the new url
         */
        void redirection( KIO::Job *job, const KUrl &url );

        /**
         * Signals a permanent redirection.
         * The redirection itself is handled internally.
	 * @param job the job that is redirected
	 * @param fromUrl the original URL
	 * @param toUrl the new URL
         */
        void permanentRedirection( KIO::Job *job, const KUrl &fromUrl, const KUrl &toUrl );

    protected Q_SLOTS:
        void slotRedirection( const KUrl &url);
        virtual void slotFinished();

    protected:
        KUrl m_redirectionURL;

    private:
        class MkdirJobPrivate;
        MkdirJobPrivate *d;
    };

    /**
     * @internal
     * Used for direct copy from or to the local filesystem (i.e. SlaveBase::copy())
     */
    class KIO_EXPORT DirectCopyJob : public SimpleJob {
    Q_OBJECT

    public:
        /**
         * Do not create a DirectCopyJob. Use KIO::copy() or KIO::file_copy() instead.
         */
        DirectCopyJob(const KUrl& url, int command, const QByteArray &packedArgs,
                      bool showProgressInfo);
        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start(Slave *slave);

    Q_SIGNALS:
        /**
         * @internal
         * Emitted if the job found an existing partial file
         * and supports resuming. Used by FileCopyJob.
         */
        void canResume( KIO::Job *job, KIO::filesize_t offset );

    private Q_SLOTS:
        void slotCanResume( KIO::filesize_t offset );
    };


    /**
     * The transfer job pumps data into and/or out of a Slave.
     * Data is sent to the slave on request of the slave ( dataReq).
     * If data coming from the slave can not be handled, the
     * reading of data from the slave should be suspended.
     */
    class KIO_EXPORT TransferJob : public SimpleJob {
    Q_OBJECT

    public:
       /**
	* Do not create a TransferJob. Use KIO::get() or KIO::put()
	* instead.
	* @param url the url to get or put
	* @param command the command to issue
	* @param packedArgs the arguments
	* @param _staticData additional data to transmit (e.g. in a HTTP Post)
	* @param showProgressInfo true to show progress information to the user
	*/
        TransferJob(const KUrl& url, int command,
                    const QByteArray &packedArgs,
                    const QByteArray &_staticData,
                    bool showProgressInfo);

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start(Slave *slave);

        /**
         * Called when m_subJob finishes.
	 * @param job the job that finished
         */
        virtual void slotResult( KJob *job );

        /**
         * Flow control. Suspend data processing from the slave.
         */
        void internalSuspend();

        /**
         * Flow control. Resume data processing from the slave.
         */
        void internalResume();

        /**
         * Reimplemented for internal reasons
         */
        virtual void resume();

        /**
	 * Checks whether we got an error page. This currently only happens
	 * with HTTP urls. Call this from your slot connected to result().
	 *
         * @return true if we got an (HTML) error page from the server
         * instead of what we asked for.
         */
        bool isErrorPage() const { return m_errorPage; }

        /**
         * Enable the async data mode.
         * When async data is enabled, data should be provided to the job by
         * calling sendAsyncData() instead of returning data in the
         * dataReq() signal.
         */
        void setAsyncDataEnabled(bool enabled);

        /**
         * Provide data to the job when async data is enabled.
         * Should be called exactly once after receiving a dataReq signal
         * Sending an empty block indicates end of data.
         */
        void sendAsyncData(const QByteArray &data);

        /**
         * When enabled, the job reports the amount of data that has been sent,
         * instead of the amount of data that that has been received.
         * @see slotProcessedSize
         * @see slotSpeed
         */
        void setReportDataSent(bool enabled);

        /**
         *  Returns whether the job reports the amount of data that has been
         *  sent (true), or whether the job reports the amount of data that
         * has been received (false)
         */
        bool reportDataSent();

    Q_SIGNALS:
        /**
         * Data from the slave has arrived.
         * @param job the job that emitted this signal
         * @param data data received from the slave.
         *
         * End of data (EOD) has been reached if data.size() == 0, however, you
         * should not be certain of data.size() == 0 ever happening (e.g. in case
         * of an error), so you should rely on result() instead.
         */
        void data( KIO::Job *job, const QByteArray &data );

        /**
         * Request for data.
         * Please note, that you shouldn't put too large chunks
         * of data in it as this requires copies within the frame
         * work, so you should rather split the data you want
         * to pass here in reasonable chunks (about 1MB maximum)
         *
	 * @param job the job that emitted this signal
         * @param data buffer to fill with data to send to the
         * slave. An empty buffer indicates end of data. (EOD)
         */
        void dataReq( KIO::Job *job, QByteArray &data );

        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
	 * @param job the job that emitted this signal
	 * @param url the new URL
         */
        void redirection( KIO::Job *job, const KUrl &url );

        /**
         * Signals a permanent redirection.
         * The redirection itself is handled internally.
	 * @param job the job that emitted this signal
	 * @param fromUrl the original URL
	 * @param toUrl the new URL
         */
        void permanentRedirection( KIO::Job *job, const KUrl &fromUrl, const KUrl &toUrl );

        /**
         * Mimetype determined.
	 * @param job the job that emitted this signal
	 * @param type the mime type
         */
        void mimetype( KIO::Job *job, const QString &type );

        /**
         * @internal
         * Emitted if the "put" job found an existing partial file
         * (in which case offset is the size of that file)
         * and emitted by the "get" job if it supports resuming to
         * the given offset - in this case @p offset is unused)
         */
        void canResume( KIO::Job *job, KIO::filesize_t offset );


    protected Q_SLOTS:
        virtual void slotRedirection( const KUrl &url);
        virtual void slotFinished();
        virtual void slotData( const QByteArray &data);
        virtual void slotDataReq();
        virtual void slotMimetype( const QString &mimetype );
        virtual void slotNeedSubUrlData();
        virtual void slotSubUrlData(KIO::Job*, const QByteArray &);
        virtual void slotMetaData( const KIO::MetaData &_metaData);
        void slotErrorPage();
        void slotCanResume( KIO::filesize_t offset );
        void slotPostRedirection();

    protected:
        bool m_internalSuspended;
        bool m_errorPage;
        bool m_unused1;
        bool m_unused2;
        QByteArray staticData;
        KUrl m_redirectionURL;
        KUrl::List m_redirectionList;
        QString m_mimetype;
        TransferJob *m_subJob;
    private:
	class TransferJobPrivate *d;
    };

    /**
     * StoredTransferJob is a TransferJob (for downloading or uploading data) that
     * also stores a QByteArray with the data, making it simpler to use than the
     * standard TransferJob.
     *
     * For KIO::storedGet it puts the data into the member QByteArray, so the user
     * of this class can get hold of the whole data at once by calling data()
     * when the result signal is emitted.
     * You should only use StoredTransferJob to download data if you cannot
     * process the data by chunks while it's being downloaded, since storing
     * everything in a QByteArray can potentially require a lot of memory.
     *
     * For KIO::storedPut the user of this class simply provides the bytearray from
     * the start, and the job takes care of uploading it.
     * You should only use StoredTransferJob to upload data if you cannot
     * provide the in chunks while it's being uploaded, since storing
     * everything in a QByteArray can potentially require a lot of memory.
     */
    class KIO_EXPORT StoredTransferJob : public KIO::TransferJob {
        Q_OBJECT

    public:
       /**
	* Do not create a StoredTransferJob. Use storedGet() or storedPut()
	* instead.
	* @param url the url to get or put
	* @param command the command to issue
	* @param packedArgs the arguments
	* @param _staticData additional data to transmit (e.g. in a HTTP Post)
	* @param showProgressInfo true to show progress information to the user
	*/
        StoredTransferJob(const KUrl& url, int command,
                          const QByteArray &packedArgs,
                          const QByteArray &_staticData,
                          bool showProgressInfo);

        /**
         * Set data to be uploaded. This is for put jobs.
         * Automatically called by KIO::storedPut(const QByteArray &, ...),
         * do not call this yourself.
         */
        void setData( const QByteArray& arr );

        /**
         * Get hold of the downloaded data. This is for get jobs.
         * You're supposed to call this only from the slot connected to the result() signal.
         */
        QByteArray data() const { return m_data; }

    private Q_SLOTS:
        void slotStoredData( KIO::Job *job, const QByteArray &data );
        void slotStoredDataReq( KIO::Job *job, QByteArray &data );
    private:
        QByteArray m_data;
        int m_uploadOffset;
    };

    /**
     * The MultiGetJob is a TransferJob that allows you to get
     * several files from a single server. Don't create directly,
     * but use KIO::multi_get() instead.
     * @see KIO::multi_get()
     */
    class KIO_EXPORT MultiGetJob : public TransferJob {
    Q_OBJECT

    public:
        /**
	 * Do not create a MultiGetJob directly, use KIO::multi_get()
	 * instead.
	 *
	 * @param url the first url to get
	 * @param showProgressInfo true to show progress information to the user
	 */
        MultiGetJob(const KUrl& url, bool showProgressInfo);

        virtual ~MultiGetJob();

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
         virtual void start(Slave *slave);

	/**
	 * Get an additional file.
	 *
	 * @param id the id of the file
	 * @param url the url of the file to get
	 * @param metaData the meta data for this request
	 */
        void get(long id, const KUrl &url, const MetaData &metaData);

    Q_SIGNALS:
        /**
         * Data from the slave has arrived.
	 * @param id the id of the request
         * @param data data received from the slave.
         * End of data (EOD) has been reached if data.size() == 0
         */
        void data( long id, const QByteArray &data);

        /**
         * Mimetype determined
	 * @param id the id of the request
	 * @param type the mime type
         */
        void mimetype( long id, const QString &type );

        /**
         * File transfer completed.
         *
         * When all files have been processed, result(KJob *) gets
         * emitted.
	 * @param id the id of the request
         */
        void result( long id);

    protected Q_SLOTS:
        virtual void slotRedirection( const KUrl &url);
        virtual void slotFinished();
        virtual void slotData( const QByteArray &data);
        virtual void slotMimetype( const QString &mimetype );
    private:
        struct GetRequest {
        public:
           GetRequest(long _id, const KUrl &_url, const MetaData &_metaData)
             : id(_id), url(_url), metaData(_metaData) { }
           long id;
           KUrl url;
           MetaData metaData;
           bool operator==( const GetRequest& req ) const { return req.id == id; }
        };
        bool findCurrentEntry();
        void flushQueue(QLinkedList<GetRequest> &queue);

        typedef QLinkedList<GetRequest> RequestQueue;
        RequestQueue m_waitQueue;
        RequestQueue m_activeQueue;
        bool b_multiGetActive;
        GetRequest m_currentEntry;
    private:
	class MultiGetJobPrivate* d;
    };

    /**
     * A MimetypeJob is a TransferJob that  allows you to get
     * the mime type of an URL. Don't create directly,
     * but use KIO::mimetype() instead.
     * @see KIO::mimetype()
     */
    class KIO_EXPORT MimetypeJob : public TransferJob {
    Q_OBJECT

    public:
       /**
	* Do not create a MimetypeJob directly. Use KIO::mimetype()
	* instead.
	* @param url the url to get
	* @param command the command to issue
	* @param packedArgs the arguments
	* @param showProgressInfo true to show progress information to the user
	*/
        MimetypeJob(const KUrl& url, int command, const QByteArray &packedArgs, bool showProgressInfo);

        /**
         * Call this in the slot connected to result,
         * and only after making sure no error happened.
	 * @return the mimetype of the URL
         */
         QString mimetype() const { return m_mimetype; }

        /**
	 * @internal
         * Called by the scheduler when a slave gets to
         * work on this job.
	 * @param slave the slave that works on the job
         */
        virtual void start( Slave *slave );

    protected Q_SLOTS:
        virtual void slotFinished( );
    private:
	class MimetypeJobPrivate* d;
    };

    /**
     * The FileCopyJob copies data from one place to another.
     * @see KIO::file_copy()
     * @see KIO::file_move()
     */
    class KIO_EXPORT FileCopyJob : public Job {
    Q_OBJECT

    public:
	/**
	* Do not create a FileCopyJob directly. Use KIO::file_move()
	* or KIO::file_copy() instead.
	* @param src the source URL
	* @param dest the destination URL
	* @param permissions the permissions of the resulting resource
	* @param move true to move, false to copy
	* @param overwrite true to allow overwriting, false otherwise
	* @param resume true to resume an operation, false otherwise
	* @param showProgressInfo true to show progress information to the user
	 */
        FileCopyJob( const KUrl& src, const KUrl& dest, int permissions,
                     bool move, bool overwrite, bool resume, bool showProgressInfo);

        ~FileCopyJob();
        /**
         * If you know the size of the source file, call this method
         * to inform this job. It will be displayed in the "resume" dialog.
	 * @param size the size of the source file
         */
        void setSourceSize(KIO::filesize_t size);

        /**
         * Sets the modification time of the file
         *
         * Note that this is ignored if a direct copy (SlaveBase::copy) can be done,
         * in which case the mtime of the source is applied to the destination (if the protocol
         * supports the concept).
         */
        void setModificationTime( time_t mtime );

	/**
	 * Returns the source URL.
	 * @return the source URL
	 */
        KUrl srcUrl() const { return m_src; }

	/**
	 * Returns the destination URL.
	 * @return the destination URL
	 */
        KUrl destUrl() const { return m_dest; }

    public Q_SLOTS:
        void slotStart();
        void slotData( KIO::Job *, const QByteArray &data);
        void slotDataReq( KIO::Job *, QByteArray &data);

    protected Q_SLOTS:
        /**
         * Called whenever a subjob finishes.
	 * @param job the job that emitted this signal
         */
        virtual void slotResult( KJob *job );

        /**
         * Forward signal from subjob
	 * @param job the job that emitted this signal
	 * @param size the processed size in bytes
         */
        void slotProcessedSize( KJob *job, qulonglong size );
        /**
         * Forward signal from subjob
	 * @param job the job that emitted this signal
	 * @param size the total size
         */
        void slotTotalSize( KJob *job, qulonglong size );
        /**
         * Forward signal from subjob
	 * @param job the job that emitted this signal
	 * @param pct the percentage
         */
        void slotPercent( KJob *job, unsigned long pct );
        /**
         * Forward signal from subjob
	 * @param job the job that emitted this signal
	 * @param offset the offset to resume from
         */
        void slotCanResume( KIO::Job *job, KIO::filesize_t offset );

    protected:
        void startCopyJob();
        void startCopyJob(const KUrl &slave_url);
        void startRenameJob(const KUrl &slave_url);
        void startDataPump();
        void connectSubjob( SimpleJob * job );

    private:
        void startBestCopyMethod();

    protected:
        KUrl m_src;
        KUrl m_dest;
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
    private:
	class FileCopyJobPrivate;
	FileCopyJobPrivate* d;
    };

    /**
     * A ListJob is allows you to get the get the content of a directory.
     * Don't create the job directly, but use KIO::listRecursive() or
     * KIO::listDir() instead.
     * @see KIO::listRecursive()
     * @see KIO::listDir()
     */
    class KIO_EXPORT ListJob : public SimpleJob {
    Q_OBJECT

    public:
       /**
	* Do not create a ListJob directly. Use KIO::listDir() or
	* KIO::listRecursive() instead.
	* @param url the url of the directory
	* @param showProgressInfo true to show progress information to the user
	* @param recursive true to get the data recursively from child directories,
	*        false to get only the content of the specified dir
	* @param prefix the prefix of the files, or QString() for no prefix
	* @param includeHidden true to include hidden files (those starting with '.')
	*/
        ListJob(const KUrl& url, bool showProgressInfo,
                bool recursive = false, const QString &prefix = QString(),
                bool includeHidden = true);

        /**
	 * @internal
         * Called by the scheduler when a @p slave gets to
         * work on this job.
	 * @param slave the slave that starts working on this job
         */
        virtual void start( Slave *slave );

        /**
         * Returns the ListJob's redirection URL. This will be invalid if there
         * was no redirection.
         * @return the redirection url
         */
        const KUrl& redirectionUrl() const { return m_redirectionURL; }

        /**
         * Do not apply any KIOSK restrictions to this job.
         */
        void setUnrestricted(bool unrestricted);

    Q_SIGNALS:
        /**
         * This signal emits the entry found by the job while listing.
         * The progress signals aren't specific to ListJob. It simply
         * uses SimpleJob's processedSize (number of entries listed) and
         * totalSize (total number of entries, if known),
         * as well as percent.
	 * @param job the job that emitted this signal
	 * @param list the list of UDSEntries
         */
        void entries( KIO::Job *job, const KIO::UDSEntryList& list);

        /**
         * Signals a redirection.
         * Use to update the URL shown to the user.
         * The redirection itself is handled internally.
	 * @param job the job that is redirected
	 * @param url the new url
         */
        void redirection( KIO::Job *job, const KUrl &url );

        /**
         * Signals a permanent redirection.
         * The redirection itself is handled internally.
	 * @param job the job that emitted this signal
	 * @param fromUrl the original URL
	 * @param toUrl the new URL
         */
        void permanentRedirection( KIO::Job *job, const KUrl &fromUrl, const KUrl &toUrl );

    protected Q_SLOTS:
        virtual void slotFinished( );
        virtual void slotMetaData( const KIO::MetaData &_metaData);
        virtual void slotResult( KJob *job );
        void slotListEntries( const KIO::UDSEntryList& list );
        void slotRedirection( const KUrl &url );
        void gotEntries( KIO::Job * subjob, const KIO::UDSEntryList& list );

    private:
        bool recursive;
        bool includeHidden;
        QString prefix;
        unsigned long m_processedEntries;
        KUrl m_redirectionURL;
    private:
	class ListJobPrivate* d;
    };
}

// Source compat includes

#include <kio/copyjob.h>
#include <kio/deletejob.h>

#endif
