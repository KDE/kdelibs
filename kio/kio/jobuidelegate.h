/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef KIO_JOBUIDELEGATE_H
#define KIO_JOBUIDELEGATE_H

#include <kjobuidelegate.h>
#include <kio/skipdialog.h>
#include <kio/renamedialog.h>

class KJob;
namespace KIO
{
class Job;
class JobUiDelegatePrivate;

/**
 * A UI delegate tuned to be used with KIO Jobs.
 *
 * It uses KIO::Observer to do the tracking.
 */
class KIO_EXPORT JobUiDelegate : public KJobUiDelegate
{
    Q_OBJECT

public:
    /**
     * Constructs a new KIO Job UI delegate.
     *
     * @param showProgressInfo indicates if this delegate should
     * show the progress of the job or not
     */
    JobUiDelegate( bool showProgressInfo );

    /**
     * Destroys the KIO Job UI delegate.
     */
    virtual ~JobUiDelegate();

public:

    /**
     * Associate this job with a window given by @p window.
     * @param window the window to associate to
     * @see window()
     */
    void setWindow(QWidget *window);

    /**
     * Returns the window this job is associated with.
     * @return the associated window
     * @see setWindow()
     */
    QWidget *window() const;

    /**
     * Updates the last user action timestamp to the given time.
     * See KApplication::updateUserTimestamp().
     */
    void updateUserTimestamp( unsigned long time );

    /**
     * @internal
     */
    unsigned long userTimestamp() const;

    virtual void showErrorMessage();

    /**
     * \relates KIO::RenameDialog
     * Construct a modal, parent-less "rename" dialog, and return
     * a result code, as well as the new dest. Much easier to use than the
     * class RenameDialog directly.
     *
     * @param caption the caption for the dialog box
     * @param src the URL of the file/dir we're trying to copy, as it's part of the text message
     * @param dest the URL of the destination file/dir, i.e. the one that already exists
     * @param mode parameters for the dialog (which buttons to show...),
     *             see RenameDialog_Mode
     * @param newDestPath the new destination path, valid if R_RENAME was returned.
     * @param sizeSrc size of source file
     * @param sizeDest size of destination file
     * @param ctimeSrc creation time of source file
     * @param ctimeDest creation time of destination file
     * @param mtimeSrc modification time of source file
     * @param mtimeDest modification time of destination file
     * @return the result
     */
    virtual RenameDialog_Result askFileRename(KJob * job,
                                              const QString & caption,
                                              const QString& src,
                                              const QString & dest,
                                              KIO::RenameDialog_Mode mode,
                                              QString& newDest,
                                              KIO::filesize_t sizeSrc = (KIO::filesize_t) -1,
                                              KIO::filesize_t sizeDest = (KIO::filesize_t) -1,
                                              time_t ctimeSrc = (time_t) -1,
                                              time_t ctimeDest = (time_t) -1,
                                              time_t mtimeSrc = (time_t) -1,
                                              time_t mtimeDest = (time_t) -1);

    /**
     * @internal
     * See skipdialog.h
     */
    virtual SkipDialog_Result askSkip(KJob * job,
                                      bool multi,
                                      const QString & error_text);

private Q_SLOTS:
    void slotPercent( KJob *job, unsigned long percent );
    void slotInfoMessage( KJob *job, const QString &msg );
    void slotDescription( KJob *job, const QString &title,
                          const QPair<QString, QString> &field1,
                          const QPair<QString, QString> &field2 );
    void slotTotalSize( KJob *job, qulonglong totalSize );
    void slotProcessedSize( KJob *job, qulonglong size );
    void slotSpeed( KJob *job, unsigned long speed );
    void slotProcessedFiles(KIO::Job *job, unsigned long files);
    void slotTotalFiles(KJob *job, unsigned long files);
    void slotProcessedDirs(KIO::Job *job, unsigned long dirs);
    void slotTotalDirs(KJob *job, unsigned long dirs);

protected:
     virtual void connectJob( KJob *job );

protected Q_SLOTS:
    void slotFinished( KJob *job, int jobId );
    void slotWarning( KJob *job, const QString &errorText );

private:
    JobUiDelegatePrivate * const d;
};
}

#endif
