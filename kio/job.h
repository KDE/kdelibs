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

#include <kurl.h>

#include <qobject.h>
//#include <qintdict.h>
#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>
//#include <qdatetime.h>
//#include <time.h>

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
     *      job->showErrorDialog();
     *
     */
    class Job : public QObject {
	Q_OBJECT
	
    protected:
	Job();

    public:
	virtual ~Job() {}

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
   	 * Use this to display the error yourself, but for a dialog box use @ref KIO::ErrorDialog.
   	 */
  	QString errorString();

  	/**
   	 * Display a dialog box to inform the user of the error given by
         * this job.
	 * Only call if @ref error is not 0, and only in the slot connected
	 * to @ref result.
   	 */
  	void showErrorDialog();

    signals:
	/**
	 * Emitted when the job is finished, in any case (completed, canceled,
	 * failed...). Use @ref error to know the result.
	 */
	void result( KIO::Job *job );
	
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

    protected slots:
	/**
	 * Called whenever a subjob finishes.
	 * Default implementation checks for errors and propagates
	 * to parent job, then calls @ref removeSubjob.
	 * Override if you don't want subjobs errors to be propagated.
	 */
        virtual void slotResult( KIO::Job *job );

    protected:
	QList<Job> subjobs;
	int m_error;
	QString m_errorText;
    };

    /**
     * A simple job (one url and one command)
     * This is the base class for all jobs that are scheduled.
     * Other jobs are high-level jobs (CopyJob, DeleteJob, FIleCopyJob...)
     * that manage subjobs but aren't scheduled directly.
     * This is why you can @ref kill a SimpleJob, but not high-level jobs.
     */
    class SimpleJob : public KIO::Job {
	Q_OBJECT
    public:
	SimpleJob(const KURL& url, int command, const QByteArray &packedArgs);
        ~SimpleJob();
	const KURL& url() const { return m_url; }

        /**
         * Abort job
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
         **/
	Slave *slave() { return m_slave; }


    protected slots:
        /**
	 * Called when the slave marks the job
	 * as finished.
	 */
        virtual void slotFinished( );

        virtual void slotError( int , const QString & );
    private:
	KURL m_url;
	int m_command;

    protected:
	Slave * m_slave;
	QByteArray m_packedArgs;
    };


    /**
     * Create a directory
     * @param url url to the directory to create
     * @param permissions the permissions to set after creating the directory
     * @return the job handling the operation
     */
    SimpleJob * mkdir( const KURL& url, int permissions = -1 );

    /**
     * Remove a directory, assumed to be empty
     * @param url url to the directory to remove
     * @return the job handling the operation
     */
    SimpleJob * rmdir( const KURL& url );

    /**
     * Change permissions on a file or directory
     * @param url file or directory
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
	 **/
	virtual void start( Slave *slave );
	
    protected slots:
        void slotStatEntry( const KIO::UDSEntry & entry );

    protected:
	UDSEntry m_statResult;
    };

    /**
     * Finds all details for one file or directory.
     */
    StatJob * stat( const KURL& url );


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

    protected slots:
        virtual void slotRedirection( const KURL &url);
        virtual void slotData( const QByteArray &data);
        virtual void slotDataReq();

    protected:
        bool m_suspended;
        QByteArray staticData;
    };

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
	 **/
	virtual void start( Slave *slave );
	
    protected slots:
        virtual void slotData( KIO::Job *, const QByteArray &data);
        void slotMimetype( const QString &mimetype );
        virtual void slotFinished( );

    protected:
	QString m_mimetype;
    };

    /**
     * Finds mimetype for one file or directory.
     */
    MimetypeJob * mimetype( const KURL& url );


    /**
     * The FileCopyJob copies data from one place to another.
     */
    class FileCopyJob : public Job {
        Q_OBJECT

    public:
        FileCopyJob( const KURL& src, const KURL& dest, int permissions, bool move,
                     bool overwrite, bool resume);

        /**
         * Abort job
         */
        virtual void kill();

    protected:
        void startCopyJob();
        void startDataPump();

    public slots:
        void slotData( KIO::Job *, const QByteArray &data);
        void slotDataReq( KIO::Job *, QByteArray &data);

    protected slots:
	/**
	 * Called whenever a subjob finishes.
	 */
        virtual void slotResult( KIO::Job *job );

    protected:
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

    class ListJob : public SimpleJob {
	Q_OBJECT
	
    public:
	ListJob(const KURL& url, bool recursive = false, QString prefix = QString::null);
        virtual void start( Slave *slave );

    signals:
	void entries( KIO::Job *, const KIO::UDSEntryList& );

    protected slots:
        virtual void slotResult( KIO::Job *job );
	void slotListEntries( const KIO::UDSEntryList& list );
	void gotEntries( KIO::Job *, const KIO::UDSEntryList& );

    private:
	bool recursive;
	QString prefix;
    };

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


    struct CopyInfo
    {
        KURL uSource;
        KURL uDest;
        mode_t permissions;
        mode_t type;
        time_t mtime;
        off_t size; // 0 for dirs
    };

    // Copy or Move
    class CopyJob : public Job {
	Q_OBJECT
	
    public:
	CopyJob( const KURL::List& src, const KURL& dest, bool move = false );

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

    protected slots:
        void slotEntries( KIO::Job*, const KIO::UDSEntryList& list );
        virtual void slotResult( KIO::Job *job );

    private:
	bool m_move;
	enum { DEST_NOT_STATED, DEST_IS_DIR, DEST_IS_FILE, DEST_DOESNT_EXIST } destinationState;
	enum { STATE_STATING, STATE_LISTING, STATE_CREATING_DIRS, STATE_CONFLICT_CREATING_DIRS,
               STATE_COPYING_FILES, STATE_CONFLICT_COPYING_FILES } state;
	long int m_totalSize;
	QValueList<CopyInfo> files;
	QValueList<CopyInfo> dirs;
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

    CopyJob *copy( const KURL& src, const KURL& dest );
    CopyJob *copy( const KURL::List& src, const KURL& dest );

    CopyJob *move( const KURL& src, const KURL& dest );
    CopyJob *move( const KURL::List& src, const KURL& dest );



    class DeleteJob : public Job {
        Q_OBJECT

    public:
	DeleteJob( const KURL::List& src, bool shred );

    protected:
        void startNextJob();
        void deleteNextFile();
        void deleteNextDir();

    protected slots:
        void slotEntries( KIO::Job*, const KIO::UDSEntryList& list );
        virtual void slotResult( KIO::Job *job );

    private:
        enum { STATE_STATING, STATE_LISTING,
               STATE_DELETING_FILES, STATE_DELETING_DIRS } state;
        KURL::List files;
        KURL::List dirs;
        KURL::List m_srcList;
        bool m_shred;
    };

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

