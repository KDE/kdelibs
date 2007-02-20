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

#include "jobuidelegate.h"

#include <qpointer.h>
#include <kmessagebox.h>
#include <kjob.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include "kio/observer.h"
#include "kio/scheduler.h"
#include "uiserveriface.h"

#if defined Q_WS_X11
#include <QX11Info>
#include <netwm.h>
#endif

class KIO::JobUiDelegate::Private
{
public:
    Private() : uiserver("org.kde.kuiserver",
                         "/UIServer",
                         QDBusConnection::sessionBus()) { }

    bool showProgressInfo;
    QPointer<QWidget> errorParentWidget;
    unsigned long userTimestamp;
    org::kde::KIO::UIServer uiserver;
};

KIO::JobUiDelegate::JobUiDelegate( bool showProgressInfo )
    : d( new Private() )
{
    d->showProgressInfo = showProgressInfo;
    d->errorParentWidget = 0L;
#if defined Q_WS_X11
    d->userTimestamp = QX11Info::appUserTime();
#endif
}

KIO::JobUiDelegate::~JobUiDelegate()
{
    if ( d->showProgressInfo && job() )
    {
        Observer::self()->jobFinished( job()->progressId() );
    }

    delete d;
}

void KIO::JobUiDelegate::setWindow(QWidget *window)
{
    d->errorParentWidget = window;
    KIO::Scheduler::registerWindow(window);
}

QWidget *KIO::JobUiDelegate::window() const
{
    return d->errorParentWidget;
}

void KIO::JobUiDelegate::updateUserTimestamp( unsigned long time )
{
#if defined Q_WS_X11
  if( d->userTimestamp == 0 || NET::timestampCompare( time, d->userTimestamp ) > 0 )
      d->userTimestamp = time;
#endif
}

unsigned long KIO::JobUiDelegate::userTimestamp() const
{
    return d->userTimestamp;
}

void KIO::JobUiDelegate::connectJob( KJob *job )
{
    // Notify the UI Server and get a progress id
    if ( d->showProgressInfo )
    {
        KIO::Job* kioJob = dynamic_cast<KIO::Job*>( job );
        const int progressId = Observer::self()->newJob( job, Observer::JobShown );
        job->setProgressId( progressId );
        if (kioJob)
            kioJob->addMetaData("progress-id", QString::number(progressId));

        //kDebug(7007) << "Created job " << this << " with progress info -- m_progressId=" << m_progressId << endl;
        // Connect global progress info signals
        connect( job, SIGNAL( percent( KJob*, unsigned long ) ),
                 this, SLOT( slotPercent( KJob*, unsigned long ) ) );
        connect( job, SIGNAL( infoMessage( KJob*, const QString &, const QString & ) ),
                 this, SLOT( slotInfoMessage( KJob*, const QString & ) ) );
        connect( job, SIGNAL( totalSize( KJob*, qulonglong ) ),
                 this, SLOT( slotTotalSize( KJob*, qulonglong ) ) );
        connect( job, SIGNAL( processedSize( KJob*, qulonglong ) ),
                 this, SLOT( slotProcessedSize( KJob*, qulonglong ) ) );
        connect( job, SIGNAL( speed( KJob*, unsigned long ) ),
                 this, SLOT( slotSpeed( KJob*, unsigned long ) ) );
        connect( job, SIGNAL( totalFiles( KJob*, unsigned long ) ),
                 this, SLOT( totalFiles( KJob*, unsigned long ) ) );
        connect( job, SIGNAL( totalDirs( KJob*, unsigned long ) ),
                 this, SLOT( totalDirs( KJob*, unsigned long ) ) );

        connect( job, SIGNAL( finished( KJob*, int ) ),
                 this, SLOT( slotFinished( KJob*, int ) ) );
    }

    connect( job, SIGNAL( warning( KJob*, const QString& ) ),
             this, SLOT( slotWarning( KJob*, const QString& ) ) );
}

void KIO::JobUiDelegate::showErrorMessage()
{
    if ( job()->error() != KJob::KilledJobError )
    {
        KMessageBox::queuedMessageBox( d->errorParentWidget, KMessageBox::Error, job()->errorString() );
    }
}

void KIO::JobUiDelegate::slotFinished( KJob * /*job*/, int /*jobId*/ )
{
    // If we are displaying a progress dialog, remove it first.
    if ( job()->progressId() ) // Did we get an ID from the observer ?
        Observer::self()->jobFinished( job()->progressId() );
    if ( job()->error() && isAutoErrorHandlingEnabled() )
        showErrorMessage();
}

void KIO::JobUiDelegate::slotWarning( KJob * /*job*/, const QString &errorText )
{
    if (isAutoWarningHandlingEnabled())
    {
        static uint msgBoxDisplayed = 0;
        if ( msgBoxDisplayed == 0 ) // don't bomb the user with message boxes, only one at a time
        {
            msgBoxDisplayed++;
            KMessageBox::information( d->errorParentWidget, errorText );
            msgBoxDisplayed--;
        }
        // otherwise just discard it.
    }
}

KIO::RenameDialog_Result KIO::JobUiDelegate::askFileRename(KJob * job,
                                                           const QString & caption,
                                                           const QString& src,
                                                           const QString & dest,
                                                           KIO::RenameDialog_Mode mode,
                                                           QString& newDest,
                                                           KIO::filesize_t sizeSrc,
                                                           KIO::filesize_t sizeDest,
                                                           time_t ctimeSrc,
                                                           time_t ctimeDest,
                                                           time_t mtimeSrc,
                                                           time_t mtimeDest)
{
    kDebug() << "Observer::open_RenameDialog job=" << job << endl;
    if (job)
        kDebug() << "                        progressId=" << job->progressId() << endl;
    // Hide existing dialog box if any
    if (job && job->progressId())
        d->uiserver.setJobVisible(job->progressId(), false);
    // We now do it in process => KDE4: move this code out of Observer (back to job.cpp), so that
    // opening the rename dialog doesn't start uiserver for nothing if progressId=0 (e.g. F2 in konq)
    RenameDialog_Result res = KIO::open_RenameDialog(caption, src, dest, mode,
                                                     newDest, sizeSrc, sizeDest,
                                                     ctimeSrc, ctimeDest, mtimeSrc,
                                                     mtimeDest);
    if (job && job->progressId())
        d->uiserver.setJobVisible(job->progressId(), true);
    return res;
}

KIO::SkipDialog_Result KIO::JobUiDelegate::askSkip(KJob * job,
                                              bool multi,
                                              const QString & error_text)
{
    // Hide existing dialog box if any
    if (job && job->progressId())
        d->uiserver.setJobVisible(job->progressId(), false);
    // We now do it in process. So this method is a useless wrapper around KIO::open_RenameDialog.
    SkipDialog_Result res = KIO::open_SkipDialog(multi, error_text);
    if (job && job->progressId())
        d->uiserver.setJobVisible(job->progressId(), true);
    return res;
}

void KIO::JobUiDelegate::processedFiles(unsigned long files)
{
    d->uiserver.processedFiles(job()->progressId(), files);
}

void KIO::JobUiDelegate::processedDirs(unsigned long dirs)
{
    d->uiserver.processedDirs(job()->progressId(), dirs);
}

void KIO::JobUiDelegate::totalFiles(unsigned long files)
{
    d->uiserver.totalFiles(job()->progressId(), files);
}

void KIO::JobUiDelegate::totalDirs(unsigned long dirs)
{
    d->uiserver.totalDirs(job()->progressId(), dirs);
}

void KIO::JobUiDelegate::moving(const KUrl &src, const KUrl &dest)
{
    d->uiserver.moving(job()->progressId(), src.url(), dest.url());
}

void KIO::JobUiDelegate::copying(const KUrl &src, const KUrl &dest)
{
    d->uiserver.copying(job()->progressId(), src.url(), dest.url());
}

void KIO::JobUiDelegate::creatingDir(const KUrl &dir)
{
    d->uiserver.creatingDir(job()->progressId(), dir.url());
}

void KIO::JobUiDelegate::deleting(const KUrl &url)
{
    d->uiserver.deleting(job()->progressId(), url.url());
}

void KIO::JobUiDelegate::stating(const KUrl &url)
{
    d->uiserver.stating(job()->progressId(), url.url());
}

void KIO::JobUiDelegate::transferring(const KUrl &url)
{
    d->uiserver.transferring(job()->progressId(), url.url());
}

void KIO::JobUiDelegate::mounting(const QString &dev, const QString &point)
{
    d->uiserver.mounting(job()->progressId(), dev, point);
}

void KIO::JobUiDelegate::unmounting(const QString &point)
{
    d->uiserver.unmounting(job()->progressId(), point);
}

void KIO::JobUiDelegate::slotPercent( KJob */*job*/, unsigned long percent )
{
    d->uiserver.percent(job()->progressId(), percent);
}

void KIO::JobUiDelegate::slotInfoMessage( KJob */*job*/, const QString &msg )
{
    d->uiserver.infoMessage(job()->progressId(), msg);
}

void KIO::JobUiDelegate::slotTotalSize( KJob */*job*/, qulonglong totalSize )
{
    d->uiserver.totalSize(job()->progressId(), totalSize);
}

void KIO::JobUiDelegate::slotProcessedSize( KJob */*job*/, qulonglong size )
{
    d->uiserver.processedSize(job()->progressId(), size);
}

void KIO::JobUiDelegate::slotSpeed( KJob */*job*/, unsigned long speed )
{
    if (speed)
        d->uiserver.speed(job()->progressId(), KIO::convertSize(speed) + QString("/s"));
    else
        d->uiserver.speed(job()->progressId(), QString());
}

void KIO::JobUiDelegate::slotTotalFiles(KJob */*job*/, unsigned long files)
{
    totalFiles(files);
}

void KIO::JobUiDelegate::slotTotalDirs(KJob */*job*/, unsigned long dirs)
{
    totalDirs(dirs);
}

#include "jobuidelegate.moc"
