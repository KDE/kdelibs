/**
  * This file is part of the KDE project
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

#ifndef UISERVER_H
#define UISERVER_H

#include <QTimer>
#include <QDateTime>
#include <QListView>
#include <QPersistentModelIndex>
#include <kxmlguiwindow.h>
#include <ksslcertdialog.h>
#include <kio/global.h>
#include <kio/authinfo.h>
#include <kurl.h>
#include <ktoolbar.h>

#include "uiserver_p.h"
#include <kuiserversettings.h>

class ProgressListModel;
class ProgressListDelegate;
class UiServerAdaptor;
class QToolBar;
class QTabWidget;
class KLineEdit;
class OrgKdeUiServerCallbacksInterface;

class UIServer
    : public KXmlGuiWindow
{
  Q_OBJECT

  UIServer();

  virtual ~UIServer();

public:
   static UIServer* createInstance();

    /**
      * Signal a new job
      *
      * @param appServiceName   the DBUS service name
      * @see                    KIO::Observer::newJob
      * @param capabilities		the capabilities that this job accepts
      * @param showProgress     whether to popup the progress for the job.
      *                         Usually true, but may be false when we use kuiserver for
      *                         other things, like SSL dialogs.
      * @param internalAppName  the application name that launched the job (kopete, konqueror...)
      * @param jobIcon          the job icon name
      * @param appName          the translated application name (Kopete, Konqueror...)
      * @return                 the identification number of the job (jobId)
      */
    int newJob(const QString &appServiceName, int capabilities, bool showProgress, const QString &internalAppName, const QString &jobIcon, const QString &appName);

    /**
      * Finishes a job
      *
      * @param jobId     the identification number of the job
      * @param errorCode the error code of the job (succeeded, killed...)
      */
    void jobFinished(int jobId, int errorCode);

    /**
      * Suspends a job
      *
      * @param jobId the identification number of the job
      */
    void jobSuspended(int jobId);

    /**
      * Resumes a job
      *
      * @param jobId the identification number of the job
      */
    void jobResumed(int jobId);

    /**
      * Adds an action (button) to a job
      *
      * @param jobId        the identification number of the job in which the action will be added
      * @param actionId     the identification number of the action
      * @param actionText   the button text
      * @return             the identification number of the action (actionId)
      */
    void newAction(int jobId, int actionId, const QString &actionText);

    /**
      * Sets the total size of a job
      *
      * @param jobId    the identification number of the job
      * @param size     the total size
      */
    void totalSize(int jobId, KIO::filesize_t size);

    /**
      * Sets the total files of a job
      *
      * @param jobId    the identification number of the job
      * @param files    total files to be processed
      */
    void totalFiles(int jobId, unsigned long files);

    /**
      * Sets the total directories of a job
      *
      * @param jobId    the identification number of the job
      * @param dirs     total dirs to be processed
      */
    void totalDirs(int jobId, unsigned long dirs);

    /**
      * Sets the current processed size
      *
      * @param jobId    the identification number of the job
      * @param bytes    the current processed bytes
      */
    void processedSize(int jobId, KIO::filesize_t bytes);

    /**
      * Sets the current processed files
      *
      * @param jobId    the identification number of the job
      * @param files    the current processed files
      */
    void processedFiles(int jobId, unsigned long files);

    /**
      * Sets the current processed dirs
      *
      * @param jobId    the identification number of the job
      * @param dirs     the current processed dirs
      */
    void processedDirs(int jobId, unsigned long dirs);

    /**
      * Sets the current percent
      *
      * @param jobId    the identification number of the job
      * @param ipercent the current percent
      */
    void percent(int jobId, unsigned long ipercent);

    /**
      * Sets the current speed transfer
      *
      * @param jobId            the identification number of the job
      * @param bytes_per_second number of bytes per second on the transfer
      */
    void speed(int jobId, QString bytes_per_second);

    /**
      * Sets the current info message
      *
      * @param jobId    the identification number of the job
      * @param msg      the info message to show
      */
    void infoMessage(int jobId, QString msg);

    /**
      * Sets the current progress info message
      *
      * @param jobId    the identification number of the job
      * @param msg      the progress info message to show
      */
    void progressInfoMessage(int jobId, QString msg);

    /**
     * Set the current job description
     *
     * @param jobId       the identification number of the job
     * @param description the description of the job
     */
    bool setDescription(int jobId, const QString &title);

    /**
     * Set the current first field for the job description
     *
     * @param jobId       the identification number of the job
     * @param name        the name of this field
     * @param value       the value of this field
     */
    bool setDescriptionFirstField(int jobId, const QString &name, const QString &value);

    /**
     * Set the current second field for the job description
     *
     * @param jobId       the identification number of the job
     * @param name        the name of this field
     * @param value       the value of this field
     */
    bool setDescriptionSecondField(int jobId, const QString &name, const QString &value);

    /**
      * Sets a job visible or hidden
      *
      * @param jobId    the job that will be hidden or shown
      * @param visible  whether the job will be shown or not
      */
    void setJobVisible(int jobId, bool visible);

public Q_SLOTS:
    void slotRemoveSystemTrayIcon();
    void updateConfiguration();
    void applySettings();
    void slotActionPerformed(int actionId, int jobId);
    void slotActionPerformedFinishedJob(int actionId, int jobId);

private Q_SLOTS:
    void showConfigurationDialog();

private:
    ProgressListModel *progressListModel;
    ProgressListModel *progressListFinishedModel;
    ProgressListDelegate *progressListDelegate;
    ProgressListDelegate *progressListDelegateFinished;
    QListView *listProgress;
    QListView *listFinished;
    QTabWidget *tabWidget;

    QToolBar *toolBar;
    KLineEdit *searchText;
    UiServerAdaptor *serverAdaptor;
    QHash<int, int> m_hashActions;
    QHash<int, OrgKdeUiServerCallbacksInterface*> m_hashCallbacksInterfaces;

    static int s_jobId;
};

#endif // UISERVER_H
