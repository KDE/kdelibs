/*  This file is part of the KDE project
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#include "kuiserverjobtracker.h"
#include "kuiserverjobtracker_p.h"

#include <klocale.h>
#include <kdebug.h>
#include <ktoolinvocation.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kjob.h>

#include "uiservercallbacks.h"

K_GLOBAL_STATIC(KSharedUiServerProxy, serverProxy)

class KUiServerJobTracker::Private
{
public:
    Private() { }

    QMap<KJob*, int> progressIds;
};

KUiServerJobTracker::KUiServerJobTracker(QObject *parent)
    : KJobTrackerInterface(parent), d(new Private)
{

}

KUiServerJobTracker::~KUiServerJobTracker()
{
    if (!d->progressIds.isEmpty()) {
        qWarning() << "A KUiServerJobTracker instance contains"
                   << d->progressIds.size() << "stalled jobs";
    }

    delete d;
}

void KUiServerJobTracker::registerJob(KJob *job)
{
    // Already registered?
    if (d->progressIds.contains(job)) return;

    int id = serverProxy->newJob(job, KSharedUiServerProxy::JobShown);
    d->progressIds.insert(job, id);

    KJobTrackerInterface::registerJob(job);
}

void KUiServerJobTracker::unregisterJob(KJob *job)
{
    int id = d->progressIds.take(job);
    serverProxy->jobFinished(id, job->error());
    KJobTrackerInterface::unregisterJob(job);
}

void KUiServerJobTracker::finished(KJob *job)
{
    int id = d->progressIds.take(job);
    serverProxy->jobFinished(id, job->error());
}

void KUiServerJobTracker::suspended(KJob *job)
{
    if (!d->progressIds.contains(job)) return;
    int id = d->progressIds[job];
    serverProxy->uiserver().jobSuspended(id);
}

void KUiServerJobTracker::resumed(KJob *job)
{
    if (!d->progressIds.contains(job)) return;
    int id = d->progressIds[job];
    serverProxy->uiserver().jobResumed(id);
}

void KUiServerJobTracker::description(KJob *job, const QString &title,
                                      const QPair<QString, QString> &field1,
                                      const QPair<QString, QString> &field2)
{
    int id = d->progressIds[job];

    serverProxy->uiserver().setDescription(id, title);
    serverProxy->uiserver().setDescriptionFirstField(id, field1.first, field1.second);
    serverProxy->uiserver().setDescriptionSecondField(id, field2.first, field2.second);
}

void KUiServerJobTracker::infoMessage(KJob *job, const QString &plain, const QString &/*rich*/)
{
    serverProxy->uiserver().infoMessage(d->progressIds[job], plain);
}

void KUiServerJobTracker::totalAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    switch (unit)
    {
    case KJob::Bytes:
        serverProxy->uiserver().totalSize(d->progressIds[job], amount);
        break;
    case KJob::Files:
        serverProxy->uiserver().totalFiles(d->progressIds[job], amount);
        break;
    case KJob::Directories:
        serverProxy->uiserver().totalDirs(d->progressIds[job], amount);
        break;
    }
}

void KUiServerJobTracker::processedAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    switch (unit)
    {
    case KJob::Bytes:
        serverProxy->uiserver().processedSize(d->progressIds[job], amount);
        break;
    case KJob::Files:
        serverProxy->uiserver().processedFiles(d->progressIds[job], amount);
        break;
    case KJob::Directories:
        serverProxy->uiserver().processedDirs(d->progressIds[job], amount);
        break;
    }
}

void KUiServerJobTracker::percent(KJob *job, unsigned long percent)
{
    serverProxy->uiserver().percent(d->progressIds[job], percent);
}

void KUiServerJobTracker::speed(KJob *job, unsigned long value)
{
    if (value)
        serverProxy->uiserver().speed(d->progressIds[job], KGlobal::locale()->formatByteSize(value) + QString("/s"));
    else
        serverProxy->uiserver().speed(d->progressIds[job], QString());
}

KSharedUiServerProxy::KSharedUiServerProxy()
    : m_uiserver("org.kde.kuiserver", "/UiServer", QDBusConnection::sessionBus())
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kuiserver"))
    {
        //kDebug(KDEBUG_OBSERVER) << "Starting kuiserver";
        QString error;
        int ret = KToolInvocation::startServiceByDesktopPath("kuiserver.desktop",
                                                             QStringList(), &error);
        if (ret > 0)
        {
            kError() << "Couldn't start kuiserver from kuiserver.desktop: " << error << endl;
        } //else
          //  kDebug(KDEBUG_OBSERVER) << "startServiceByDesktopPath returned " << ret;
    }

    //if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kuiserver"))
    //    kDebug(KDEBUG_OBSERVER) << "The application kuiserver is STILL NOT REGISTERED";
    //else
    //    kDebug(KDEBUG_OBSERVER) << "kuiserver registered";

    new UiServerCallbacksAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QLatin1String("/UiServerCallbacks"), this);
}

KSharedUiServerProxy::~KSharedUiServerProxy()
{
}

org::kde::UiServer &KSharedUiServerProxy::uiserver()
{
    return m_uiserver;
}

int KSharedUiServerProxy::newJob(KJob *job, JobVisibility visibility, const QString &icon)
{
    if (!job) return 0;

    KComponentData componentData = KGlobal::mainComponent();

    // Notify the kuiserver about the new job
    int progressId = m_uiserver.newJob(QDBusConnection::sessionBus().baseService(), job->capabilities(),
                                       visibility, componentData.aboutData()->appName(),
                                       icon, componentData.aboutData()->programName());

    m_jobs.insert(progressId, job);

    return progressId;
}

void KSharedUiServerProxy::jobFinished(int progressId, int errorCode)
{
    m_uiserver.jobFinished(progressId, errorCode);
    m_jobs.remove(progressId);
}

void KSharedUiServerProxy::slotActionPerformed(int actionId, int jobId)
{
    KJob *job = m_jobs[jobId];

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
            job->kill( KJob::EmitResult ); // notify that the job has been killed
            break;
        default:
            kWarning() << "Unknown actionId (" << actionId << ") for jobId " << jobId;
            break;
        }
    }
}


#include "kuiserverjobtracker.moc"
#include "kuiserverjobtracker_p.moc"
