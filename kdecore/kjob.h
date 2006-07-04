/*  This file is part of the KDE project
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef KJOB_H
#define KJOB_H

#include <QObject>
#include <kdelibs_export.h>

class KJobUiDelegate;

/**
 * The base class for all jobs.
 * For all jobs created in an application, the code looks like
 *
 * \code
 *   KJob * job = someoperation( some parameters );
 *   connect( job, SIGNAL( result( KJob * ) ),
 *            this, SLOT( slotResult( KJob * ) ) );
 *   job->start();
 * \endcode
 *   (other connects, specific to the job)
 *
 * And slotResult is usually at least:
 *
 * \code
 *  if ( job->error() )
 *      doSomething();
 * \endcode
 *
 * With the synchronous interface the code looks like
 *
 * \code
 *  KJob *job = someoperation( some parameters );
 *  if ( !job->exec() )
 *  {
 *      // An error occurred
 *  }
 *  else
 *  {
 *      // Do something
 *  }
 * \endcode
 *
 */
class KDECORE_EXPORT KJob : public QObject
{
    Q_OBJECT
    Q_ENUMS( KillVerbosity )

public:
    /**
     * Creates a new KJob object.
     *
     * @param parent the parent QObject
     */
    KJob( QObject *parent = 0 );

    /**
     * Destroys a KJob object.
     */
    virtual ~KJob();

    /**
     * Attach a UI delegate to this job.
     *
     * If the job had another UI delegate, it's automatically deleted. Once
     * attached to the job, the UI delegate will be deleted with the job.
     *
     * @param delegate the new UI delegate to use
     * @see KJobUiDelegate
     */
    void setUiDelegate( KJobUiDelegate *delegate );

    /**
     * Retrieves the delegate attached to this job.
     *
     * @return the delegate attached to this job, or 0 if there's no such delegate
     */
    KJobUiDelegate *uiDelegate() const;

    /**
     * Starts the job asynchronously. When the job is finished,
     * result() is emitted.
     */
    virtual void start() = 0;

    enum KillVerbosity { Quietly, EmitResult };

    /**
     * Aborts this job.
     * This kills and deletes the job.
     *
     * @param verbosity if equals to EmitResult, Job will emit signal result
     * and ask uiserver to close the progress window.
     * @p verbosity is set to EmitResult for subjobs. Whether applications
     * should call with Quietly or EmitResult depends on whether they rely
     * on result being emitted or not.
     * @return true if the operation is supported and succeeded, false otherwise
     */
    bool kill( KillVerbosity verbosity = Quietly );

protected:
    /**
     * Aborts this job quietly.
     * This simply kills the job, no error reporting or job deletion should be involved.
     *
     * @return true if the operation is supported and succeeded, false otherwise
     */
    virtual bool doKill() { return false; }

public:
    /**
     * Executes the job synchronously.
     *
     * @return true if the job has been executed without error, false otherwise
     */
    bool exec();

    enum
    {
        NoError = 0,
        KilledJobError = 1,
        UserDefinedError = 100
    };


    /**
     * Returns the error code, if there has been an error.
     * Only call this method from the slot connected to result().
     *
     * @return the error code for this job, 0 if no error.
     */
    int error() const;

    /**
     * Returns the error text if there has been an error.
     * Only call if error is not 0.
     * This is really internal, better use errorString.
     *
     * @return a string to help understand the error, usually the url
     * related to the error. Only valid if error() is not 0.
     */
    QString errorText() const;

    /**
     * Converts an error code and a non-i18n error message into an
     * error message in the current language. The low level (non-i18n)
     * error message (usually a url) is put into the translated error
     * message using %1.
     *
     * Example for errid == ERR_CANNOT_OPEN_FOR_READING:
     * \code
     *   i18n( "Could not read\n%1" ).arg( errorText() );
     * \endcode
     * Do not call it if error() is not 0.
     *
     * @return the error message and if there is no error, a message
     *         telling the user that the app is broken, so check with
     *         error() whether there is an error
     */
    virtual QString errorString() const;


    /**
     * Sets the progress id for this job.
     * Call this with the value given by uiserver.
     *
     * @param id the progress id for this job, as returned by uiserver
     * @internal
     */
    void setProgressId( int id );

    /**
     * Returns the progress id for this job.
     *
     * @return the progress id for this job, as returned by uiserver
     */
    int progressId() const;


    /**
     * Returns the processed size for this job.
     *
     * @return the processed size
     */
    qulonglong processedSize() const;

    /**
     * Returns the total size for this job.
     *
     * @return the total size
     */
    qulonglong totalSize() const;

    /**
     * Returns the overall progress of this job.
     *
     * @return the overall progress of this job
     */
    unsigned long percent() const;

Q_SIGNALS:
    /**
     * Emitted when the job is finished, in any case. It is used to notify
     * observers that the job is terminated and that progress can be hidden.
     *
     * @param job the job that emitted this signal
     * @param jobId the progress id for this job as returned by uiserver
     * @internal
     */
    void finished( KJob *job, int jobId );

    /**
     * Emitted when the job is finished, in any case (completed, canceled,
     * failed...). Use error to know the result.
     *
     * @param job the job that emitted this signal
     */
    void result( KJob *job );


    /**
     * Emitted to display information about this job.
     * Examples of message are "Resolving host", "Connecting to host...", etc.
     *
     * @param job the job that emitted this signal
     * @param plain the info message
     * @param rich the rich text version of the message, or QString() is none is available
     */
    void infoMessage( KJob *job, const QString &plain, const QString &rich = QString() );

    /**
     * Emitted to display a warning about this job.
     *
     * @param job the job that emitted this signal
     * @param plain the warning message
     * @param rich the rich text version of the message, or QString() is none is available
     */
    void warning( KJob *job, const QString &plain, const QString &rich = QString() );


    /**
     * Emitted when we know the size of this job (data size in bytes for transfers,
     * number of entries for listings).
     * @param job the job that emitted this signal
     * @param size the total size
     */
    void totalSize( KJob *job, qulonglong size );

    /**
     * Regularly emitted to show the progress of this job
     * (current data size in bytes for transfers, entries listed).
     *
     * @param job the job that emitted this signal
     * @param size the processed size
     */
    void processedSize( KJob *job, qulonglong size );

    /**
     * Progress signal showing the overall progress of the job
     * This is valid for any kind of job, and allows using a
     * a progress bar very easily. (see KProgressBar).
     * Note that this signal is not emitted for finished jobs.
     * @param job the job that emitted this signal
     * @param percent the percentage
     */
    void percent( KJob *job, unsigned long percent );

protected:
    /**
     * Sets the error code. It should be called when an error
     * is encountered in the job, just before calling emitResult().
     *
     * @param errorCode the error code
     * @see emitResult()
     */
    void setError( int errorCode );

    /**
     * Sets the error text. It should be called when an error
     * is encountered in the job, just before calling emitResult().
     *
     * @param errorText the error text
     * @see emitResult()
     */
    void setErrorText( const QString &errorText );


    /**
     * Sets the processed size. The processedSize() and percent() signals
     * are emitted if the values changed.
     *
     * @param size the new processed size
     */
    void setProcessedSize( qulonglong size );

    /**
     * Sets the total size. The totalSize() and percent() signals
     * are emitted if the values changed.
     *
     * @param size the new total size
     */
    void setTotalSize( qulonglong size );

    /**
     * Sets the overall progress of the job. The percent() signal
     * is emitted if the value changed.
     *
     * @param percentage the new overall progress
     */
    void setPercent( unsigned long percentage );


    /**
     * Utility function to emit the result signal, and suicide this job.
     * It first notifies the observers to hide the progress for this job using
     * the finished() signal.
     *
     * @see result()
     * @see finished()
     */
    void emitResult();

    /**
     * Utility function for inherited jobs.
     * Emits the percent signal if bigger than previous value,
     * after calculating it from the parameters.
     *
     * @param processedSize the processed size
     * @param totalSize the total size
     * @see percent()
     */
    void emitPercent( qulonglong processedSize, qulonglong totalSize );

private:
    class Private;
    Private *const d;
};

#endif
