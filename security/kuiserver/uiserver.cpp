/**
  * This file is part of the KDE project
  * Copyright (C) 2007, 2006 Rafael Fernández López <ereslibre@gmail.com>
  * Copyright (C) 2001 George Staikos <staikos@kde.org>
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
#include <QAction>
#include <QToolBar>
#include <QTabWidget>

#include <ksqueezedtextlabel.h>
#include <kconfig.h>
#include <kconfigdialog.h>
#include <kstandarddirs.h>
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kdebug.h>
#include <kdialog.h>
#include <ksystemtrayicon.h>
#include <kmenu.h>
#include <kaction.h>
#include <klineedit.h>
#include <kio/jobclasses.h>
#include <kjob.h>

#include "uiserver.h"
#include "uiserveradaptor.h"
#include "progresslistmodel.h"
#include "progresslistdelegate.h"
#include "callbacksiface.h"


UIServer::UIServer()
    : KXmlGuiWindow(0)
{
    serverAdaptor = new UiServerAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QLatin1String("/UiServer"), this);

    tabWidget = new QTabWidget();

    QString configure = i18n("Configure...");

    toolBar = addToolBar(configure);
    toolBar->setMovable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    QAction *configureAction = toolBar->addAction(configure);
    configureAction->setIcon(KIcon("configure"));
    configureAction->setIconText(configure);

    connect(configureAction, SIGNAL(triggered(bool)), this,
            SLOT(showConfigurationDialog()));

    toolBar->addSeparator();

    searchText = new KLineEdit(toolBar);
    searchText->setClickMessage(i18n("Search"));
    searchText->setClearButtonShown(true);

    toolBar->addWidget(searchText);

    listProgress = new QListView(tabWidget);
    listProgress->setFrameStyle(QFrame::NoFrame);
    listProgress->setObjectName("progresslist");
    listProgress->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    listFinished = new QListView(tabWidget);
    listFinished->setFrameStyle(QFrame::NoFrame);
    listFinished->setObjectName("progresslistFinished");
    listFinished->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    tabWidget->addTab(listProgress, i18n("In Progress"));
    tabWidget->addTab(listFinished, i18n("Finished"));

    setCentralWidget(tabWidget);

    progressListModel = new ProgressListModel(this);
    progressListFinishedModel = new ProgressListModel(this);

    listProgress->setModel(progressListModel);
    listFinished->setModel(progressListFinishedModel);

    progressListDelegate = new ProgressListDelegate(this, listProgress);
    progressListDelegate->setSeparatorPixels(10);
    progressListDelegate->setLeftMargin(10);
    progressListDelegate->setRightMargin(10);
    progressListDelegate->setProgressBarHeight(20);
    progressListDelegate->setMinimumItemHeight(100);
    progressListDelegate->setMinimumContentWidth(300);
    progressListDelegate->setEditorHeight(20);
    listProgress->setItemDelegate(progressListDelegate);

    progressListDelegateFinished = new ProgressListDelegate(this, listFinished);
    progressListDelegateFinished->setSeparatorPixels(10);
    progressListDelegateFinished->setLeftMargin(10);
    progressListDelegateFinished->setRightMargin(10);
    progressListDelegateFinished->setProgressBarHeight(20);
    progressListDelegateFinished->setMinimumItemHeight(100);
    progressListDelegateFinished->setMinimumContentWidth(300);
    progressListDelegateFinished->setEditorHeight(20);
    listFinished->setItemDelegate(progressListDelegateFinished);

    applySettings();

    connect(progressListDelegate, SIGNAL(actionPerformed(int,int)), this,
            SLOT(slotActionPerformed(int,int)));

    connect(progressListDelegateFinished, SIGNAL(actionPerformed(int,int)), this,
            SLOT(slotActionPerformedFinishedJob(int,int)));

    hide();
}

UIServer::~UIServer()
{
}

UIServer* UIServer::createInstance()
{
    return new UIServer;
}

int UIServer::newJob(const QString &appServiceName, int capabilities, bool showProgress, const QString &internalAppName, const QString &jobIcon, const QString &appName)
{
    // Uncomment if you want to see kuiserver in action (ereslibre)
    // if (isHidden()) show();

    s_jobId++;

    OrgKdeUiServerCallbacksInterface *callbacks = new org::kde::UiServerCallbacks(appServiceName, "/UiServerCallbacks", QDBusConnection::sessionBus());

    m_hashCallbacksInterfaces.insert(s_jobId, callbacks);

    progressListModel->newJob(s_jobId, internalAppName, jobIcon, appName, showProgress);
    progressListModel->setData(progressListModel->indexForJob(s_jobId), s_jobId,
                               ProgressListDelegate::JobId);

    if (capabilities == KJob::NoCapabilities)
        return s_jobId;

    if (capabilities & KJob::Suspendable)
        newAction(s_jobId, KJob::Suspendable, i18n("Pause"));

    if (capabilities & KJob::Killable)
        newAction(s_jobId, KJob::Killable, i18n("Cancel"));

    return s_jobId;
}

void UIServer::jobFinished(int id, int errorCode)
{
    if (errorCode == KJob::NoError)
    {
        if ((id < 1) || !m_hashCallbacksInterfaces.contains(id)) return;

        QModelIndex index = progressListModel->indexForJob(id);

        progressListFinishedModel->newJob(id, progressListModel->data(index, ProgressListDelegate::ApplicationInternalName).toString(),
                                        progressListModel->data(index, ProgressListDelegate::Icon).toString(),
                                        progressListModel->data(index, ProgressListDelegate::ApplicationName).toString(),
                                        true /* showProgress (show or hide) */);

        progressListFinishedModel->newAction(id, KJob::Killable, i18n("Close Information"));
    }

    delete m_hashCallbacksInterfaces[id];
    m_hashCallbacksInterfaces.remove(id);

    progressListModel->finishJob(id);
}

void UIServer::jobSuspended(int id)
{
    if (id < 1) return;

    progressListModel->editAction(id, KJob::Suspendable, i18n("Resume"));
}

void UIServer::jobResumed(int id)
{
    if (id < 1) return;

    progressListModel->editAction(id, KJob::Suspendable, i18n("Pause"));
}


/// ===========================================================


void UIServer::newAction(int jobId, int actionId, const QString &actionText)
{
    m_hashActions.insert(actionId, jobId);

    progressListModel->newAction(jobId, actionId, actionText);
}

/// ===========================================================


void UIServer::totalSize(int id, KIO::filesize_t size)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), KIO::convertSize(size),
                               ProgressListDelegate::SizeTotals);
}

void UIServer::totalFiles(int id, unsigned long files)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), qulonglong(files),
                               ProgressListDelegate::FileTotals);
}

void UIServer::totalDirs(int id, unsigned long dirs)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), qulonglong(dirs),
                               ProgressListDelegate::DirTotals);
}

void UIServer::processedSize(int id, KIO::filesize_t bytes)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), KIO::convertSize(bytes),
                               ProgressListDelegate::SizeProcessed);
}

void UIServer::processedFiles(int id, unsigned long files)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), qulonglong(files),
                               ProgressListDelegate::FilesProcessed);
}

void UIServer::processedDirs(int id, unsigned long dirs)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), qulonglong(dirs),
                               ProgressListDelegate::DirsProcessed);
}

void UIServer::percent(int id, unsigned long ipercent)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), qulonglong(ipercent),
                               ProgressListDelegate::Percent);
}

void UIServer::speed(int id, QString bytes_per_second)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), bytes_per_second,
                               ProgressListDelegate::Speed);
}

void UIServer::infoMessage(int id, QString msg)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), msg,
                               ProgressListDelegate::Message);
}

void UIServer::progressInfoMessage(int id, QString msg)
{
    if (id < 1) return;

    progressListModel->setData(progressListModel->indexForJob(id), msg,
                               ProgressListDelegate::ProgressMessage);
}


bool UIServer::setDescription(int id, const QString &description)
{
    if (id < 1) return false;

    progressListModel->setData(progressListModel->indexForJob(id), description,
                               ProgressListDelegate::Message);

    return true;
}

bool UIServer::setDescriptionFirstField(int id, const QString &name, const QString &value)
{
    if (id < 1) return false;

    progressListModel->setData(progressListModel->indexForJob(id), name,
                               ProgressListDelegate::FromLabel);

    progressListModel->setData(progressListModel->indexForJob(id), value,
                               ProgressListDelegate::From);

    return true;
}

bool UIServer::setDescriptionSecondField(int id, const QString &name, const QString &value)
{
    if (id < 1) return false;

    progressListModel->setData(progressListModel->indexForJob(id), name,
                               ProgressListDelegate::ToLabel);

    progressListModel->setData(progressListModel->indexForJob(id), value,
                               ProgressListDelegate::To);

    return true;
}

void UIServer::setJobVisible(int id, bool visible)
{
    listProgress->setRowHidden(progressListModel->indexForJob(id).row(), !visible);
}

void UIServer::slotRemoveSystemTrayIcon()
{
#ifdef __GNUC__
    #warning implement me (ereslibre)
#endif
    return;
}

void UIServer::updateConfiguration()
{
    Configuration::self()->writeConfig();
}

void UIServer::applySettings()
{
     KSystemTrayIcon *m_systemTray = new KSystemTrayIcon(this);
     m_systemTray->setIcon(KSystemTrayIcon::loadIcon("display")); // found something better ? (ereslibre)
     m_systemTray->show();
}

void UIServer::slotActionPerformed(int actionId, int jobId)
{
    if (!m_hashCallbacksInterfaces.contains(jobId)) return;

    m_hashCallbacksInterfaces[jobId]->slotActionPerformed(actionId, jobId);
}

void UIServer::slotActionPerformedFinishedJob(int /* actionId */, int jobId)
{
    progressListFinishedModel->finishJob(jobId);
}

void UIServer::showConfigurationDialog()
{
    if (KConfigDialog::showDialog("configuration"))
        return;

    KConfigDialog *dialog = new KConfigDialog(this, "configuration",
                                              Configuration::self());

    UIConfigurationDialog *configurationUI = new UIConfigurationDialog(0);

    dialog->addPage(configurationUI, i18n("Behavior"), "display");

    connect(dialog, SIGNAL(settingsChanged(const QString&)), this,
            SLOT(updateConfiguration()));

    dialog->show();
}


/// ===========================================================


UIConfigurationDialog::UIConfigurationDialog(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    adjustSize();
}

UIConfigurationDialog::~UIConfigurationDialog()
{
}


/// ===========================================================


int UIServer::s_jobId = 0;

extern "C" KDE_EXPORT int kdemain(int argc, char **argv)
{
    KLocale::setMainCatalog("kdelibs");
    //  GS 5/2001 - I changed the name to "KDE" to make it look better
    //              in the titles of dialogs which are displayed.
    KAboutData aboutdata("kuiserver", I18N_NOOP("Progress Manager"),
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
      kDebug(7024) << "kuiserver is already running!" << endl;
      return (0);
    }

    KUniqueApplication app;

    // This app is started automatically, no need for session management
    app.disableSessionManagement();
    app.setQuitOnLastWindowClosed( false );
    //app.dcopClient()->setDaemonMode( true );

    UIServer::createInstance();

    return app.exec();
}

#include "uiserver.moc"
#include "uiserver_p.moc"
