/**
  * This file is part of the KDE project
  * Copyright (C) 2006 Rafael Fernández López <ereslibre@gmail.com>
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

#include <QWidget>
#include <QBoxLayout>

#include <ksqueezedtextlabel.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kdesu/client.h>
#include <kwin.h>
#include <kdialog.h>
#include <ksystemtrayicon.h>
#include <kmenu.h>
#include <kaction.h>
#include <kdialog.h>
#include <kstandardaction.h>

#include "uiserver.h"
#include "uiserveradaptor_p.h"
#include "uiserveriface.h"
#include "progresslistmodel.h"
#include "progresslistdelegate.h"

#include "observer.h" // for static methods only
#include "kio/defaultprogress.h"
#include "kio/jobclasses.h"
#include "uiserver.h"
#include "passworddialog.h"
#include "kio/renamedialog.h"
#include "kio/skipdialog.h"
#include "slavebase.h" // for QuestionYesNo etc.
#include <ksslinfodialog.h>
#include <ksslcertdialog.h>
#include <ksslcertificate.h>
#include <ksslcertchain.h>

UIServer::UIServer()
    : KMainWindow(0)
{
    listProgress = new QListView(this);
    listProgress->setObjectName("progresslist");
    listProgress->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    progressListModel = new ProgressListModel(this);

    serverAdaptor = new UIServerAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QLatin1String("/UIServer"), this);

    listProgress->setModel(progressListModel);

    setCentralWidget(listProgress);

    ProgressListDelegate *progressListDelegate = new ProgressListDelegate(this, listProgress);
    progressListDelegate->setSeparatorPixels(5);
    progressListDelegate->setLeftMargin(5);
    progressListDelegate->setRightMargin(5);
    progressListDelegate->setProgressBarHeight(20);
    progressListDelegate->setMinimumItemHeight(100);
    progressListDelegate->setMinimumContentWidth(200);
    //progressListDelegate->setEditorHeight(20);
    #warning no editor yet, change height when implemented (ereslibre)
    progressListDelegate->setEditorHeight(0);
    listProgress->setItemDelegate(progressListDelegate);

    connect(progressListModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            listProgress, SLOT(rowsInserted(const QModelIndex&, int, int)));

    connect(progressListModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            listProgress, SLOT(rowsRemoved(const QModelIndex&, int, int)));

    connect(progressListModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            listProgress, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));

    connect(progressListDelegate, SIGNAL(actionPerformed(int)), serverAdaptor,
            SIGNAL(actionPerformed(int)));

    applySettings();

    hide();
}

UIServer::~UIServer()
{
}

UIServer* UIServer::createInstance()
{
    return new UIServer;
}

int UIServer::newJob(const QString &appServiceName, bool showProgress, const QString &internalAppName, const QString &jobIcon, const QString &appName)
{
    s_jobId++;

    progressListModel->newJob(s_jobId, internalAppName, jobIcon, appName);
    progressListModel->setData(progressListModel->indexForJob(s_jobId), s_jobId,
                               ProgressListDelegate::jobId);

    #warning Opening persistent editor leads to painting problems (ereslibre)
    //listProgress->openPersistentEditor(progressListModel->indexForJob(s_jobId));

    return s_jobId;
}

void UIServer::jobFinished(int id)
{
    if (id < 1) return;

    progressListModel->finishJob(id);
}

int UIServer::newAction(int jobId, const QString &actionText)
{
    s_actionId++;

    progressListModel->newAction(jobId, s_actionId, actionText);

    return s_actionId;
}

void UIServer::totalSize(int id, KIO::filesize_t size)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), KIO::convertSize(size),
                               ProgressListDelegate::sizeTotals);
}

void UIServer::totalFiles(int id, unsigned long files)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), qulonglong(files),
                               ProgressListDelegate::fileTotals);
}

void UIServer::totalDirs(int id, unsigned long dirs)
{
    if (id < 1) return;

    #warning implement me (ereslibre)
}

void UIServer::processedSize(int id, KIO::filesize_t bytes)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), KIO::convertSize(bytes),
                               ProgressListDelegate::sizeProcessed);
}

void UIServer::processedFiles(int id, unsigned long files)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), qulonglong(files),
                               ProgressListDelegate::filesProcessed);
}

void UIServer::processedDirs(int id, unsigned long dirs)
{
    if (id < 1) return;

    #warning implement me (ereslibre)
}

void UIServer::percent(int id, unsigned long ipercent)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), qulonglong(ipercent),
                               ProgressListDelegate::percent);
}

void UIServer::speed(int id, unsigned long bytes_per_second)
{
    if (id < 1) return;

    #warning implement me (ereslibre)
}

void UIServer::infoMessage(int id, QString msg)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), msg,
                               ProgressListDelegate::message);
}

void UIServer::copying(int id, QString from, QString to)
{
    if (id < 1) return;

    QString delegateMessage(i18n("Copying"));

    progressListModel->setData(progressListModel->indexForJob(id), delegateMessage,
                               ProgressListDelegate::message);

    progressListModel->setData(progressListModel->indexForJob(id), from,
                               ProgressListDelegate::from);

    progressListModel->setData(progressListModel->indexForJob(id), to,
                               ProgressListDelegate::to);

    progressListModel->setData(progressListModel->indexForJob(id), i18n("From"),
                               ProgressListDelegate::fromLabel);

    progressListModel->setData(progressListModel->indexForJob(id), i18n("To"),
                               ProgressListDelegate::toLabel);
}

void UIServer::moving(int id, QString from, QString to)
{
    if (id < 1) return;

    QString delegateMessage(i18n("Moving"));

    progressListModel->setData(progressListModel->indexForJob(id), delegateMessage,
                               ProgressListDelegate::message);

    progressListModel->setData(progressListModel->indexForJob(id), from,
                               ProgressListDelegate::from);

    progressListModel->setData(progressListModel->indexForJob(id), to,
                               ProgressListDelegate::to);

    progressListModel->setData(progressListModel->indexForJob(id), i18n("From"),
                               ProgressListDelegate::fromLabel);

    progressListModel->setData(progressListModel->indexForJob(id), i18n("To"),
                               ProgressListDelegate::toLabel);
}

void UIServer::deleting(int id, QString url)
{
    if (id < 1) return;

    QString delegateMessage(i18n("Deleting"));

    progressListModel->setData(progressListModel->indexForJob(id), delegateMessage,
                               ProgressListDelegate::message);

    progressListModel->setData(progressListModel->indexForJob(id), url,
                               ProgressListDelegate::from);

    progressListModel->setData(progressListModel->indexForJob(id), i18n("File"),
                               ProgressListDelegate::fromLabel);
}

void UIServer::transferring(int id, QString url)
{
    if (id < 1) return;

    QString delegateMessage(i18n("Transferring"));

    progressListModel->setData(progressListModel->indexForJob(id), delegateMessage,
                               ProgressListDelegate::message);

    progressListModel->setData(progressListModel->indexForJob(id), url,
                               ProgressListDelegate::from);

    progressListModel->setData(progressListModel->indexForJob(id), i18n("Source on"),
                               ProgressListDelegate::fromLabel);
}

void UIServer::creatingDir(int id, QString dir)
{
    if (id < 1) return;

    QString delegateMessage(i18n("Creating directory"));

    progressListModel->setData(progressListModel->indexForJob(id), delegateMessage,
                               ProgressListDelegate::message);

    progressListModel->setData(progressListModel->indexForJob(id), dir,
                               ProgressListDelegate::from);

    progressListModel->setData(progressListModel->indexForJob(id), i18n("New directory"),
                               ProgressListDelegate::fromLabel);
}

void UIServer::stating(int id, QString url)
{
    if (id < 1) return;

    QString delegateMessage(i18n("Stating"));

    progressListModel->setData(progressListModel->indexForJob(id), delegateMessage,
                               ProgressListDelegate::message);

    progressListModel->setData(progressListModel->indexForJob(id), url,
                               ProgressListDelegate::from);

    progressListModel->setData(progressListModel->indexForJob(id), i18n("Stating"),
                               ProgressListDelegate::fromLabel);
}

void UIServer::mounting(int id, QString dev, QString point)
{
    if (id < 1) return;

    QString delegateMessage(i18n("Mounting device"));

    progressListModel->setData(progressListModel->indexForJob(id), delegateMessage,
                               ProgressListDelegate::message);

    progressListModel->setData(progressListModel->indexForJob(id), dev,
                               ProgressListDelegate::from);

    progressListModel->setData(progressListModel->indexForJob(id), point,
                               ProgressListDelegate::to);

    progressListModel->setData(progressListModel->indexForJob(id), i18n("Device"),
                               ProgressListDelegate::fromLabel);

    progressListModel->setData(progressListModel->indexForJob(id), i18n("Mount point"),
                               ProgressListDelegate::toLabel);
}

void UIServer::unmounting(int id, QString point)
{
    if (id < 1) return;

    QString delegateMessage(i18n("Unmounting device"));

    progressListModel->setData(progressListModel->indexForJob(id), delegateMessage,
                               ProgressListDelegate::message);

    progressListModel->setData(progressListModel->indexForJob(id), point,
                               ProgressListDelegate::from);

    progressListModel->setData(progressListModel->indexForJob(id), i18n("Mount point"),
                               ProgressListDelegate::fromLabel);
}

void UIServer::canResume(int id, KIO::filesize_t offset)
{
    #warning implement me (ereslibre)

    return;
}

int UIServer::messageBox(int id, int type, const QString &text, const QString &caption,
               const QString &buttonYes, const QString &buttonNo)
{
    return Observer::messageBox(id, type, text, caption, buttonYes, buttonNo);
}

void UIServer::setJobVisible(int id, bool visible)
{
    listProgress->setRowHidden(progressListModel->indexForJob(id).row(), !visible);
}

void UIServer::showSSLInfoDialog(const QString &url, const KIO::MetaData &data, int mainwindow)
{
   KSSLInfoDialog *kid = new KSSLInfoDialog(data["ssl_in_use"].toUpper()=="TRUE", 0L /*parent?*/, 0L, true);
   KSSLCertificate *x = KSSLCertificate::fromString(data["ssl_peer_certificate"].toLocal8Bit());

   if (x) {
      // Set the chain back onto the certificate
      QStringList cl =
                      data["ssl_peer_chain"].split(QString("\n"), QString::SkipEmptyParts);
      Q3PtrList<KSSLCertificate> ncl;

      ncl.setAutoDelete(true);
      for (QStringList::Iterator it = cl.begin(); it != cl.end(); ++it) {
         KSSLCertificate *y = KSSLCertificate::fromString((*it).toLocal8Bit());
         if (y) ncl.append(y);
      }

      if (ncl.count() > 0)
         x->chain().setChain(ncl);

      kDebug(7024) << "ssl_cert_errors=" << data["ssl_cert_errors"] << endl;
      kid->setCertState(data["ssl_cert_errors"]);
      QString ip = data.contains("ssl_proxied") ? "" : data["ssl_peer_ip"];
      kid->setup( x,
                  ip,
                  url, // the URL
                  data["ssl_cipher"],
                  data["ssl_cipher_desc"],
                  data["ssl_cipher_version"],
                  data["ssl_cipher_used_bits"].toInt(),
                  data["ssl_cipher_bits"].toInt(),
                  KSSLCertificate::KSSLValidation(data["ssl_cert_state"].toInt()));
      kDebug(7024) << "Showing SSL Info dialog" << endl;
#ifndef Q_WS_WIN
      if( mainwindow != 0 )
          KWin::setMainWindow( kid, mainwindow );
#endif
      kid->exec();
      delete x;
      kDebug(7024) << "SSL Info dialog closed" << endl;
   } else {
      KMessageBox::information( 0L, // parent ?
                              i18n("The peer SSL certificate appears to be corrupt."), i18n("SSL") );
   }
   // Don't delete kid!!
}

KSSLCertDialogRet UIServer::showSSLCertDialog(const QString& host, const QStringList& certList)
{
    return showSSLCertDialog(host, certList, 0);
}

KSSLCertDialogRet UIServer::showSSLCertDialog(const QString& host, const QStringList& certList, int mainwindow)
{
    KSSLCertDialogRet rc;
    rc.ok = false;
    if (!certList.isEmpty())
    {
        KSSLCertDialog *kcd = new KSSLCertDialog(0L, 0L, true);
        kcd->setupDialog(certList);
        kcd->setHost(host);
        kDebug(7024) << "Showing SSL certificate dialog" << endl;

#ifndef Q_WS_WIN
        if( mainwindow != 0 )
            KWin::setMainWindow( kcd, mainwindow );
#endif

        kcd->exec();
        rc.ok = true;
        rc.choice = kcd->getChoice();
        rc.save = kcd->saveChoice();
        rc.send = kcd->wantsToSend();
        kDebug(7024) << "SSL certificate dialog closed" << endl;
        delete kcd;
    }
    return rc;
}

void UIServer::slotRemoveSystemTrayIcon()
{
    #warning implement me (ereslibre)

    return;
}

void UIServer::applySettings()
{
     KSystemTrayIcon *m_systemTray = new KSystemTrayIcon(this);
     m_systemTray->setIcon(KSystemTrayIcon::loadIcon("launch")); // found something better ? (ereslibre)
     m_systemTray->show();
}

int UIServer::s_jobId = 0;
int UIServer::s_actionId = 0;

extern "C" KDE_EXPORT int kdemain(int argc, char **argv)
{
    KLocale::setMainCatalog("kdelibs");
    //  GS 5/2001 - I changed the name to "KDE" to make it look better
    //              in the titles of dialogs which are displayed.
    KAboutData aboutdata("kio_uiserver", I18N_NOOP("KDE Task Manager"),
                         "0.8", I18N_NOOP("KDE Progress Information UI Server"),
                         KAboutData::License_GPL, "(C) 2000-2005, David Faure & Matt Koss");
    aboutdata.addAuthor("David Faure",I18N_NOOP("Maintainer"),"faure@kde.org");
    aboutdata.addAuthor("Matej Koss",I18N_NOOP("Developer"),"koss@miesto.sk");
    aboutdata.addAuthor("Rafael Fernández López",I18N_NOOP("Developer"),"ereslibre@gmail.com");

    KCmdLineArgs::init( argc, argv, &aboutdata );
    // KCmdLineArgs::addCmdLineOptions( options );
    KUniqueApplication::addCmdLineOptions();

    if (!KUniqueApplication::start())
    {
      kDebug(7024) << "kio_uiserver is already running!" << endl;
      return (0);
    }

    KUniqueApplication app;

    // This app is started automatically, no need for session management
    app.disableSessionManagement();
    app.setQuitOnLastWindowClosed( false );
    //app.dcopClient()->setDaemonMode( true );

    UIServer *uiserver = UIServer::createInstance();

    return app.exec();
}

#include "uiserver.moc"
