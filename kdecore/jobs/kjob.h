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

#include <kdecore_export.h>
#include <QtCore/QObject>
#include <QtCore/QPair>

class KJobUiDelegate;

class KJobPrivate;
/**
 * The base class for all jobs.
 * For all jobs created in an application, the code looks like
 *
 * \code
 * void SomeClass::methodWithAsynchronousJobCall()
 * {
 *   KJob * job = someoperation( some parameters );
 *   connect( job, SIGNAL( result( KJob * ) ),
 *            this, SLOT( handleResult( KJob * ) ) );
 *   job->start();
 * }
 * \endcode
 *   (other connects, specific to the job)
 *
 * And handleResult is usually at least:
 *
 * \code
 * void SomeClass::handleResult( KJob *job )
 * {
 *   if ( job->error() )
 *       doSomething();
 * }
 * \endcode
 *
 * With the synchronous interface the code looks like
 *
 * \code
 * void SomeClass::methodWithSynchronousJobCall()
 * {
 *   KJob *job = someoperation( some parameters );
 *   if ( !job->exec() )
 *   {
 *       // An error occurred
 *   }
 *   else
 *   {
 *       // Do something
 *   }
 * }
 * \endcode
 *
 * @note: KJob and its subclasses is meant to be used 
 * in a fire-and-forget way. It's deleting itself when
 * it has finished using deleteLater() so the job
 * instance disappears after the next event loop run.
 */
class KDECORE_EXPORT KJob : public QObject
{
    Q_OBJECT
    Q_ENUMS( KillVerbosity Capability Unit )
    Q_FLAGS( Capabilities )

public:
    enum Unit { Bytes, Files, Directories };

    enum Capability { NoCapabilities = 0x0000,
                      Killable       = 0x0001,
                      Suspendable    = 0x0002 };

    Q_DECLARE_FLAGS( Capabilities, Capability )

    /**
     * Creates a new KJob object.
     *
     * @param parent the parent QObject
     */
    explicit KJob( QObject *parent = 0 );

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
     * Returns the capabilities of this job.
     *
     * @return the capabilities that this job supports
     * @see setCapabilities()
     */
    Capabilities capabilities() const;

    /**
     * Returns if the job was suspended with the suspend() call.
     *
     * @return if the job was suspended
     * @see suspend() resume()
     */
    bool isSuspended() const;

    /**
     * Starts the job asynchronously. When the job is finished,
     * result() is emitted.
     *
     * Warning: Never implement any synchronous workload in this method. This method
     * should just trigger the job startup, not do any work itself. It is expected to
     * be non-blocking.
     *
     * This is the method all subclasses need to implement.
     * It should setup and trigger the workload of the job. It should not do any
     * work itself. This includes all signals and terminating the job, e.g. by
     * emitResult(). The workload, which could be another method of the
     * subclass, is to be triggered using the event loop, e.g. by code like:
     * \code
     * void ExampleJob::start()
     * {
     *  QTimer::singleShot( 0, this, SLOT( doWork() ) );
     * }
     * \endcode
     */
    virtual void start() = 0;

    enum KillVerbosity { Quietly, EmitResult };

public Q_SLOTS:
    /**
     * Aborts this job.
     * This kills and deletes the job.
     *
     * @param verbosity if equals to EmitResult, Job will emit signal result
     * and ask uiserver to close the progress window.
     * @p verbosity is set to EmitResult for subjobs. Whether applications
     * should call with Quietly or EmitResult depends on whether they rely
     * on result being emitted or not. Please notice that if @p verbosity is
     * set to Quietly, signal result will NOT be emitted.
     * @return true if the operation is supported and succeeded, false otherwise
     */
    bool kill( KillVerbosity verbosity = Quietly );

    /**
     * Suspends this job.
     * The job should be kept in a state in which it is possible to resume it.
     *
     * @return true if the operation is supported and succeeded, false otherwise
     */
    bool suspend();

    /**
     * Resumes this job.
     *
     * @return true if the operation is supported and succeeded, false otherwise
     */
    bool resume();

protected:
    /**
     * Aborts this job quietly.
     * This simply kills the job, no error reporting or job deletion should be involved.
     *
     * @return true if the operation is supported and succeeded, false otherwise
     */
    virtual bool doKill();

    /**
     * Suspends this job.
     *
     * @return true if the operation is supported and succeeded, false otherwise
     */
    virtual bool doSuspend();

    /**
     * Resumes this job.
     *
     * @return true if the operation is supported and succeeded, false otherwise
     */
    virtual bool doResume();

    /**
     * Sets the capabilities for this job.
     *
     * @param capabilities are the capabilities supported by this job
     * @see capabilities()
     */
    void setCapabilities( Capabilities capabilities );

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
     *   i18n( "Could not read\n%1" , errorText() );
     * \endcode
     * Only call if error is not 0.
     *
     * @return the error message and if there is no error, a message
     *         telling the user that the app is broken, so check with
     *         error() whether there is an error
     */
    virtual QString errorString() const;


    /**
     * Returns the processed amount of a given unit for this job.
     *
     * @param unit the unit of the requested amount
     * @return the processed size
     */
    qulonglong processedAmount(Unit unit) const;

    /**
     * Returns the total amount of a given unit for this job.
     *
     * @param unit the unit of the requested amount
     * @return the total size
     */
    qulonglong totalAmount(Unit unit) const;

    /**
     * Returns the overall progress of this job.
     *
     * @return the overall progress of this job
     */
    unsigned long percent() const;

    /**
     * set the auto-delete property of the job. If @p autodelete is
     * set to false the job will not delete itself once it is finished.
     *
     * The default for any KJob is to automatically delete itself.
     *
     * @param autodelete set to false to disable automatic deletion
     * of the job.
     */
    void setAutoDelete( bool autodelete );

    /**
     * Returns whether this job automatically deletes itself once
     * the job is finished.
     *
     * @return whether the job is deleted automatically after
     * finishing.
     */
    bool isAutoDelete() const;

Q_SIGNALS:
#ifndef Q_MOC_RUN
#ifndef DOXYGEN_SHOULD_SKIP_THIS
private: // don't tell moc or doxygen, but those signals are in fact private
#endif
#endif
    /**
     * Emitted when the job is finished, in any case. It is used to notify
     * observers that the job is terminated and that progress can be hidden.
     *
     * This is a private signal, it can't be emitted directly by subclasses of
     * KJob, use emitResult() instead.
     *
     * Client code is not supposed to connect to this signal, signal result should
     * be used instead.
     *
     * @param job the job that emitted this signal
     * @internal
     */
    void finished(KJob *job);

    /**
     * Emitted when the job is suspended.
     *
     * This is a private signal, it can't be emitted directly by subclasses of
     * KJob.
     *
     * @param job the job that emitted this signal
     */
    void suspended(KJob *job);

    /**
     * Emitted when the job is resumed.
     *
     * This is a private signal, it can't be emitted directly by subclasses of
     * KJob.
     *
     * @param job the job that emitted this signal
     */
    void resumed(KJob *job);

    /**
     * Emitted when the job is finished (except when killed with KJob::Quietly).
     *
     * Use error to know if the job was finished with error.
     *
     * This is a private signal, it can't be emitted directly by subclasses of
     * KJob, use emitResult() instead.
     *
     * Please connect to this signal instead of finished.
     *
     * @param job the job that emitted this signal
     *
     * @see kill
     */
    void result(KJob *job);

Q_SIGNALS:
    /**
     * Emitted to display general description of this job. A description has
     * a title and two optional fields which can be used to complete the
     * description.
     *
     * Examples of titles are "Copying", "Creating resource", etc.
     * The fields of the description can be "Source" with an URL, and,
     * "Destination" with an URL for a "Copying" description.
     * @param job the job that emitted this signal
     * @param title the general description of the job
     * @param field1 first field (localized name and value)
     * @param field2 second field (localized name and value)
     */
    void description(KJob *job, const QString &title,
                     const QPair<QString, QString> &field1 = qMakePair(QString(), QString()),
                     const QPair<QString, QString> &field2 = qMakePair(QString(), QString()));

    /**
     * Emitted to display state information about this job.
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


Q_SIGNALS:
#ifndef Q_MOC_RUN
#ifndef DOXYGEN_SHOULD_SKIP_THIS
private: // don't tell moc, but those signals are in fact private
#endif
#endif
    /**
     * Emitted when we know the amount the job will have to process. The unit of this
     * amount is sent too. It can be emitted several times if the job manages several
     * different units.
     *
     * This is a private signal, it can't be emitted directly by subclasses of
     * KJob, use setTotalAmount() instead.
     *
     * @param job the job that emitted this signal
     * @param unit the unit of the total amount
     * @param amount the total amount
     */
    void totalAmount(KJob *job, KJob::Unit unit, qulonglong amount);

    /**
     * Regularly emitted to show the progress of this job by giving the current amount.
     * The unit of this amount is sent too. It can be emitted several times if the job
     * manages several different units.
     *
     * This is a private signal, it can't be emitted directly by subclasses of
     * KJob, use setProcessedAmount() instead.
     *
     * @param job the job that emitted this signal
     * @param unit the unit of the processed amount
     * @param amount the processed amount
     */
    void processedAmount(KJob *job, KJob::Unit unit, qulonglong amount);

    /**
     * Emitted when we know the size of this job (data size in bytes for transfers,
     * number of entries for listings, etc).
     *
     * This is a private signal, it can't be emitted directly by subclasses of
     * KJob, use setTotalAmount() instead.
     *
     * @param job the job that emitted this signal
     * @param size the total size
     */
    void totalSize(KJob *job, qulonglong size);

    /**
     * Regularly emitted to show the progress of this job
     * (current data size in bytes for transfers, entries listed, etc.).
     *
     * This is a private signal, it can't be emitted directly by subclasses of
     * KJob, use setProcessedAmount() instead.
     *
     * @param job the job that emitted this signal
     * @param size the processed size
     */
    void processedSize(KJob *job, qulonglong size);

    /**
     * Progress signal showing the overall progress of the job
     * This is valid for any kind of job, and allows using a
     * a progress bar very easily. (see KProgressBar).
     * Note that this signal is not emitted for finished jobs.
     *
     * This is a private signal, it can't be emitted directly by subclasses of
     * KJob, use emitPercent(), setPercent() setTotalAmount() or
     * setProcessedAmount() instead.
     *
     * @param job the job that emitted this signal
     * @param percent the percentage
     */
    void percent( KJob *job, unsigned long percent );

    /**
     * Emitted to display information about the speed of this job.
     *
     * This is a private signal, it can't be emitted directly by subclasses of
     * KJob, use emitSpeed() instead.
     *
     * @param job the job that emitted this signal
     * @param speed the speed in bytes/s
     */
    void speed(KJob *job, unsigned long speed);

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
     * Sets the processed size. The processedAmount() and percent() signals
     * are emitted if the values changed. The percent() signal is emitted
     * only for the progress unit.
     *
     * @param unit the unit of the new processed amount
     * @param amount the new processed amount
     */
    void setProcessedAmount(Unit unit, qulonglong amount);

    /**
     * Sets the total size. The totalSize() and percent() signals
     * are emitted if the values changed. The percent() signal is emitted
     * only for the progress unit.
     *
     * @param unit the unit of the new total amount
     * @param amount the new total amount
     */
    void setTotalAmount(Unit unit, qulonglong amount);

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
     * @note: Deletes this job using deleteLater().
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
     * @param processedAmount the processed amount
     * @param totalAmount the total amount
     * @see percent()
     */
    void emitPercent( qulonglong processedAmount, qulonglong totalAmount );

    /**
     * Utility function for inherited jobs.
     * Emits the speed signal and starts the timer for removing that info
     *
     * @param speed the speed in bytes/s
     */
    void emitSpeed(unsigned long speed);

protected:
    KJobPrivate *const d_ptr;
    KJob(KJobPrivate &dd, QObject *parent);

private:
    Q_PRIVATE_SLOT(d_func(), void _k_speedTimeout())
    Q_DECLARE_PRIVATE(KJob)
};

Q_DECLARE_OPERATORS_FOR_FLAGS( KJob::Capabilities )

#endif
