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
#include "jobuidelegate_p.h"

#include <kmessagebox.h>
#include <kjob.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <ktoolinvocation.h>

#include "kio/copyjob.h"
#include "kio/deletejob.h"
#include "kio/scheduler.h"

#include "observeradaptor_p.h"

#if defined Q_WS_X11
#include <QX11Info>
#include <netwm.h>
#endif

K_GLOBAL_STATIC(KIO::SharedUiDelegateProxy, delegateProxy)

KIO::JobUiDelegate::JobUiDelegate( bool showProgressInfo )
    : d( new JobUiDelegatePrivate() )
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
        delegateProxy->jobFinished( job()->progressId() );
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
        const int progressId = delegateProxy->newJob( job, SharedUiDelegateProxy::JobShown );
        job->setProgressId( progressId );
        if (kioJob)
            kioJob->addMetaData("progress-id", QString::number(progressId));

        //kDebug(7007) << "Created job " << this << " with progress info -- m_progressId=" << m_progressId << endl;
        // Connect global progress info signals
        connect( job, SIGNAL( percent( KJob*, unsigned long ) ),
                 this, SLOT( slotPercent( KJob*, unsigned long ) ) );
        connect( job, SIGNAL( infoMessage( KJob*, const QString &, const QString & ) ),
                 this, SLOT( slotInfoMessage( KJob*, const QString & ) ) );
        connect( job, SIGNAL( totalAmount( KJob*, KJob::Unit, qulonglong ) ),
                 this, SLOT( slotTotalAmount( KJob*, KJob::Unit, qulonglong ) ) );
        connect( job, SIGNAL( processedAmount( KJob*, KJob::Unit, qulonglong ) ),
                 this, SLOT( slotProcessedAmount( KJob*, KJob::Unit, qulonglong ) ) );
        connect( job, SIGNAL( speed( KJob*, unsigned long ) ),
                 this, SLOT( slotSpeed( KJob*, unsigned long ) ) );
        connect( job, SIGNAL( description( KJob*, const QString&,
                                           const QPair<QString,QString>&,
                                           const QPair<QString,QString>& ) ),
                 this, SLOT( slotDescription( KJob*, const QString&,
                                              const QPair<QString,QString>&,
                                              const QPair<QString,QString>& ) ) );
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
        delegateProxy->jobFinished( job()->progressId() );
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
        delegateProxy->uiserver().setJobVisible(job->progressId(), false);
    // We now do it in process => KDE4: move this code out of Observer (back to job.cpp), so that
    // opening the rename dialog doesn't start uiserver for nothing if progressId=0 (e.g. F2 in konq)
    RenameDialog_Result res = KIO::open_RenameDialog(caption, src, dest, mode,
                                                     newDest, sizeSrc, sizeDest,
                                                     ctimeSrc, ctimeDest, mtimeSrc,
                                                     mtimeDest);
    if (job && job->progressId())
        delegateProxy->uiserver().setJobVisible(job->progressId(), true);
    return res;
}

KIO::SkipDialog_Result KIO::JobUiDelegate::askSkip(KJob * job,
                                              bool multi,
                                              const QString & error_text)
{
    // Hide existing dialog box if any
    if (job && job->progressId())
        delegateProxy->uiserver().setJobVisible(job->progressId(), false);
    // We now do it in process. So this method is a useless wrapper around KIO::open_RenameDialog.
    SkipDialog_Result res = KIO::open_SkipDialog(multi, error_text);
    if (job && job->progressId())
        delegateProxy->uiserver().setJobVisible(job->progressId(), true);
    return res;
}

void KIO::JobUiDelegate::slotPercent( KJob *job, unsigned long percent )
{
    delegateProxy->uiserver().percent(job->progressId(), percent);
}

void KIO::JobUiDelegate::slotInfoMessage( KJob *job, const QString &msg )
{
    delegateProxy->uiserver().infoMessage(job->progressId(), msg);
}

void KIO::JobUiDelegate::slotDescription( KJob *job, const QString &title,
                                          const QPair<QString, QString> &field1,
                                          const QPair<QString, QString> &field2 )
{
    delegateProxy->uiserver().setDescription(job->progressId(), title);
    delegateProxy->uiserver().setDescriptionFirstField(job->progressId(), field1.first, field1.second);
    delegateProxy->uiserver().setDescriptionSecondField(job->progressId(), field2.first, field2.second);
}

void KIO::JobUiDelegate::slotTotalAmount( KJob *job, KJob::Unit unit, qulonglong total )
{
    switch (unit)
    {
    case KJob::Bytes:
        delegateProxy->uiserver().totalSize(job->progressId(), total);
        break;
    case KJob::Files:
        delegateProxy->uiserver().totalFiles(job->progressId(), total);
        break;
    case KJob::Directories:
        delegateProxy->uiserver().totalDirs(job->progressId(), total);
        break;
    }
}

void KIO::JobUiDelegate::slotProcessedAmount( KJob *job, KJob::Unit unit, qulonglong amount )
{
    switch (unit)
    {
    case KJob::Bytes:
        delegateProxy->uiserver().processedSize(job->progressId(), amount);
        break;
    case KJob::Files:
        delegateProxy->uiserver().processedFiles(job->progressId(), amount);
        break;
    case KJob::Directories:
        delegateProxy->uiserver().processedDirs(job->progressId(), amount);
        break;
    }
}

void KIO::JobUiDelegate::slotSpeed( KJob *job, unsigned long speed )
{
    if (speed)
        delegateProxy->uiserver().speed(job->progressId(), KIO::convertSize(speed) + QString("/s"));
    else
        delegateProxy->uiserver().speed(job->progressId(), QString());
}



KIO::SharedUiDelegateProxy::SharedUiDelegateProxy()
    : m_uiserver("org.kde.kuiserver", "/UIServer", QDBusConnection::sessionBus())
{
    QDBusConnection::sessionBus().registerObject("/KIO/Observer", this, QDBusConnection::ExportScriptableSlots);

    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kuiserver"))
    {
        //kDebug(KDEBUG_OBSERVER) << "Starting kuiserver" << endl;
        QString error;
        int ret = KToolInvocation::startServiceByDesktopPath("kuiserver.desktop",
                                                             QStringList(), &error);
        if (ret > 0)
        {
            kError() << "Couldn't start kuiserver from kuiserver.desktop: " << error << endl;
        } //else
          //  kDebug(KDEBUG_OBSERVER) << "startServiceByDesktopPath returned " << ret << endl;
    }

    //if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kuiserver"))
    //    kDebug(KDEBUG_OBSERVER) << "The application kuiserver is STILL NOT REGISTERED" << endl;
    //else
    //    kDebug(KDEBUG_OBSERVER) << "kuiserver registered" << endl;

    observerAdaptor = new ObserverAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QLatin1String("/Observer"), this);
}

KIO::SharedUiDelegateProxy::~SharedUiDelegateProxy()
{
}

org::kde::KIO::UIServer & KIO::SharedUiDelegateProxy::uiserver()
{
    return m_uiserver;
}

int KIO::SharedUiDelegateProxy::newJob(KJob *job, JobVisibility visibility, const QString &icon)
{
    if (!job) return 0;

    KComponentData componentData = KGlobal::mainComponent();

    QString jobIcon;
    if (icon.isEmpty())
    {
        if (job->uiDelegate()->jobIcon().isEmpty())
            kWarning() << "No icon set for a job launched from " << componentData.aboutData()->appName()
                       << ". No associated icon will be shown on kuiserver" << endl;

        jobIcon = job->uiDelegate()->jobIcon();
    }
    else
    {
        jobIcon = icon;
    }

    // Notify the kuiserver about the new job

    int progressId = m_uiserver.newJob(QDBusConnection::sessionBus().baseService(), job->capabilities(),
                                       visibility, componentData.aboutData()->appName(),
                                       jobIcon, componentData.aboutData()->programName());

    m_dctJobs.insert(progressId, job);

    job->setProgressId(progressId); // Just to make sure this attribute is set

    return progressId;
}

void KIO::SharedUiDelegateProxy::jobFinished(int progressId)
{
    m_uiserver.jobFinished(progressId);

    m_dctJobs.remove(progressId);
}

void KIO::SharedUiDelegateProxy::slotActionPerformed(int actionId, int jobId)
{
    KJob *job = m_dctJobs[jobId];

    if (job) {
        switch (actionId)
        {
        case KJob::Suspendable:
            if (job->isSuspended())
                job->resume();
            else
                job->suspend();
            break;
        case KJob::Killable:
            job->kill();
            break;
        default:
            break;
        }
    }
}


#include "jobuidelegate.moc"
#include "jobuidelegate_p.moc"
