/**
  * This file is part of the KDE libraries
  * Copyright (C) 2007, 2006 Rafael Fernández López <ereslibre@gmail.com>
  * Copyright (C) 2000 Matej Koss <koss@miesto.sk>
  *                    David Faure <faure@kde.org>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License version 2 as published by the Free Software Foundation.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#include <assert.h>

#include <kdebug.h>
#include <kapplication.h>
#include <ktoolinvocation.h>
#include <kurl.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>

#include "jobclasses.h"
#include "jobuidelegate.h"
#include "observer.h"
#include "observeradaptor_p.h"
#include "uiserveriface.h"

using namespace KIO;

Observer *Observer::s_pObserver = 0L;

const int KDEBUG_OBSERVER = 7007; // Should be 7028

Observer::Observer()
{
    QDBusConnection::sessionBus().registerObject("/KIO/Observer", this, QDBusConnection::ExportScriptableSlots);

    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kuiserver"))
    {
        kDebug(KDEBUG_OBSERVER) << "Starting kuiserver" << endl;
        QString error;
        int ret = KToolInvocation::startServiceByDesktopPath("kuiserver.desktop",
                                                             QStringList(), &error);
        if (ret > 0)
        {
            kError() << "Couldn't start kuiserver from kuiserver.desktop: " << error << endl;
        } else
            kDebug(KDEBUG_OBSERVER) << "startServiceByDesktopPath returned " << ret << endl;

    }
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kuiserver"))
        kDebug(KDEBUG_OBSERVER) << "The application kuiserver is STILL NOT REGISTERED" << endl;
    else
        kDebug(KDEBUG_OBSERVER) << "kuiserver registered" << endl;

    observerAdaptor = new ObserverAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QLatin1String("/Observer"), this);

    m_uiserver = new org::kde::KIO::UIServer("org.kde.kuiserver", "/UIServer", QDBusConnection::sessionBus());
}

int Observer::newJob(KJob *job, JobVisibility visibility, const QString &icon)
{
    if (!job) return 0;

    KComponentData componentData = KGlobal::mainComponent();

    QString jobIcon;
    if (icon.isEmpty())
    {
        if (job->uiDelegate()->jobIcon().isEmpty())
            kWarning() << "No icon set for a job launched from " << componentData.aboutData()->appName() << ". No associated icon will be shown on kuiserver" << endl;

        jobIcon = job->uiDelegate()->jobIcon();
    }
    else
    {
        jobIcon = icon;
    }

    // Notify the kuiserver about the new job

    int progressId = m_uiserver->newJob(QDBusConnection::sessionBus().baseService(), job->capabilities(),
                                        visibility, componentData.aboutData()->appName(),
                                        jobIcon, componentData.aboutData()->programName());

    m_dctJobs.insert(progressId, job);

    job->setProgressId(progressId); // Just to make sure this attribute is set

    return progressId;
}

void Observer::jobFinished(int progressId)
{
    m_uiserver->jobFinished(progressId);

    m_dctJobs.remove(progressId);
}

void Observer::slotActionPerformed(int actionId, int jobId)
{
    QByteArray slotName;

    switch (actionId)
    {
        case KJob::Suspendable:
                slotName = SLOT(suspend());
            break;
        case KJob::Killable:
                slotName = SLOT(kill());
            break;
        default:
            return;
    }

    connect(this, SIGNAL(actionPerformed(KJob*,int)), m_dctJobs[jobId], slotName);
    emit actionPerformed(m_dctJobs[jobId], actionId);
    disconnect(this, SIGNAL(actionPerformed(KJob*,int)), m_dctJobs[jobId], slotName);
}


/// ===========================================================

#include "observer.moc"
