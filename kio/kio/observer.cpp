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

#include <Q3CString>

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

#include "slavebase.h"
#include <kmessagebox.h>
#include <ksslinfodialog.h>
#include <ksslcertificate.h>
#include <ksslcertchain.h>
#include <klocale.h>

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


void Observer::slotTotalSize(KJob *job, qulonglong size)
{
    m_uiserver->totalSize(job->progressId(), size);
}

void Observer::slotTotalFiles(KJob *job, unsigned long files)
{
    m_uiserver->totalFiles(job->progressId(), files);
}

void Observer::slotTotalDirs(KJob *job, unsigned long dirs)
{
    m_uiserver->totalDirs(job->progressId(), dirs);
}

void Observer::slotProcessedSize(KJob *job, qulonglong size)
{
    m_uiserver->processedSize(job->progressId(), size);
}

void Observer::slotProcessedFiles(KJob *job, unsigned long files)
{
    m_uiserver->processedFiles(job->progressId(), files);
}

void Observer::slotProcessedDirs(KJob *job, unsigned long dirs)
{
    m_uiserver->processedDirs(job->progressId(), dirs);
}

void Observer::slotSpeed(KJob *job, unsigned long speed)
{
    if (speed)
        m_uiserver->speed(job->progressId(), KIO::convertSize(speed) + QString("/s"));
    else
        m_uiserver->speed(job->progressId(), QString());
}

void Observer::slotPercent(KJob *job, unsigned long percent)
{
    m_uiserver->percent(job->progressId(), percent);
}

void Observer::slotInfoMessage(KJob *job, const QString &msg)
{
    m_uiserver->infoMessage(job->progressId(), msg);
}

void Observer::slotProgressMessage(KJob *job, const QString &msg)
{
    m_uiserver->progressInfoMessage(job->progressId(), msg);
}


/// ===========================================================


void Observer::slotCopying(KJob *job, const KUrl &from, const KUrl &to)
{
    m_uiserver->copying(job->progressId(), from.url(), to.url());
}

void Observer::slotMoving(KJob *job, const KUrl &from, const KUrl &to)
{
    m_uiserver->moving(job->progressId(), from.url(), to.url());
}

void Observer::slotDeleting(KJob *job, const KUrl &url)
{
    m_uiserver->deleting(job->progressId(), url.url());
}

void Observer::slotTransferring(KJob *job, const KUrl &url)
{
    m_uiserver->transferring(job->progressId(), url.url());
}

void Observer::slotCreatingDir(KJob *job, const KUrl &dir)
{
    m_uiserver->creatingDir(job->progressId(), dir.url());
}

void Observer::stating(KJob *job, const KUrl &url)
{
    m_uiserver->stating(job->progressId(), url.url());
}

void Observer::mounting(KJob *job, const QString &dev, const QString &point)
{
    m_uiserver->mounting(job->progressId(), dev, point);
}

void Observer::unmounting(KJob *job, const QString &point)
{
    m_uiserver->unmounting(job->progressId(), point);
}

/// ===========================================================

int Observer::messageBox(int progressId, int type, const QString &text,
                          const QString &caption, const QString &buttonYes,
                          const QString &buttonNo)
{
    return messageBox(progressId, type, text, caption, buttonYes, buttonNo, QString());
}

int Observer::messageBox(int progressId, int type, const QString &text,
                          const QString &caption, const QString &buttonYes,
                          const QString &buttonNo, const QString &dontAskAgainName)
{
    kDebug() << "Observer::messageBox " << type << " " << text << " - " << caption << endl;
    int result = -1;
    KConfig *config = new KConfig("kioslaverc");
    KMessageBox::setDontShowAskAgainConfig(config);

    switch (type) {
        case KIO::SlaveBase::QuestionYesNo:
            result = KMessageBox::questionYesNo(0L, // parent ?
                                               text, caption, KGuiItem(buttonYes), KGuiItem(buttonNo), dontAskAgainName);
            break;
        case KIO::SlaveBase::WarningYesNo:
            result = KMessageBox::warningYesNo(0L, // parent ?
                                              text, caption, KGuiItem(buttonYes), KGuiItem(buttonNo), dontAskAgainName);
            break;
        case KIO::SlaveBase::WarningContinueCancel:
            result = KMessageBox::warningContinueCancel(0L, // parent ?
                                              text, caption, KGuiItem(buttonYes), dontAskAgainName);
            break;
        case KIO::SlaveBase::WarningYesNoCancel:
            result = KMessageBox::warningYesNoCancel(0L, // parent ?
                                              text, caption, KGuiItem(buttonYes), KGuiItem(buttonNo), dontAskAgainName);
            break;
        case KIO::SlaveBase::Information:
            KMessageBox::information(0L, // parent ?
                                      text, caption, dontAskAgainName);
            result = 1; // whatever
            break;
        case KIO::SlaveBase::SSLMessageBox:
        {
#ifdef __GNUC__
# warning FIXME This will never work
#endif
            QString observerAppId = caption; // hack, see slaveinterface.cpp
            // Contact the object "KIO::Observer" in the application <appId>
            // Yes, this could be the same application we are, but not necessarily.
            QDBusInterface observer(observerAppId, "/KIO/Observer", "org.kde.KIO.Observer");

            QDBusReply<QVariantMap> reply =
                observer.call(QDBus::BlockWithGui, "metadata", progressId);
            const QVariantMap &meta = reply;
            KSSLInfoDialog *kid = new KSSLInfoDialog(meta["ssl_in_use"].toString().toUpper()=="TRUE", 0L /*parent?*/, 0L, true);
            KSSLCertificate *x = KSSLCertificate::fromString(meta["ssl_peer_certificate"].toString().toLocal8Bit());
            if (x) {
               // Set the chain back onto the certificate
               QStringList cl = meta["ssl_peer_chain"].toString().split('\n', QString::SkipEmptyParts);
               Q3PtrList<KSSLCertificate> ncl;

               ncl.setAutoDelete(true);
               for (QStringList::Iterator it = cl.begin(); it != cl.end(); ++it) {
                  KSSLCertificate *y = KSSLCertificate::fromString((*it).toLocal8Bit());
                  if (y) ncl.append(y);
               }

               if (ncl.count() > 0)
                  x->chain().setChain(ncl);

               kid->setup(x,
                           meta["ssl_peer_ip"].toString(),
                           text, // the URL
                           meta["ssl_cipher"].toString(),
                           meta["ssl_cipher_desc"].toString(),
                           meta["ssl_cipher_version"].toString(),
                           meta["ssl_cipher_used_bits"].toInt(),
                           meta["ssl_cipher_bits"].toInt(),
                           KSSLCertificate::KSSLValidation(meta["ssl_cert_state"].toInt()));
               kDebug(7024) << "Showing SSL Info dialog" << endl;
               kid->exec();
               delete x;
               kDebug(7024) << "SSL Info dialog closed" << endl;
            } else {
               KMessageBox::information(0L, // parent ?
                                         i18n("The peer SSL certificate appears to be corrupt."), i18n("SSL"));
            }
            // This doesn't have to get deleted.  It deletes on it's own.
            result = 1; // whatever
            break;
        }
        default:
            kWarning() << "Observer::messageBox: unknown type " << type << endl;
            result = 0;
            break;
    }
    KMessageBox::setDontShowAskAgainConfig(0);
    delete config;
    return result;
}

#include "observer.moc"
