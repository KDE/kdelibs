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

#ifndef UISERVER_H
#define UISERVER_H

#include <QTimer>
#include <QDateTime>
#include <QListView>
#include <QPersistentModelIndex>
#include <kmainwindow.h>
#include <ksslcertdialog.h>
#include <kio/global.h>
#include <kio/authinfo.h>
#include <kurl.h>
#include <kmainwindow.h>
#include <k3listview.h>
#include <ksslcertdialog.h>
#include <ktoolbar.h>

class ProgressListModel;
class UIServerAdaptor;

class UIServer
    : public KMainWindow
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
      * @param showProgress     whether to popup the progress for the job.
      *                         Usually true, but may be false when we use kio_uiserver for
      *                         other things, like SSL dialogs.
      * @param internalAppName  the application name that launched the job (kopete, konqueror...)
      * @param jobIcon          the job icon name
      * @param appName          the translated application name (Kopete, Konqueror...)
      * @return                 the identification number of the job (jobId)
      */
    int newJob(const QString &appServiceName, bool showProgress, const QString &internalAppName, const QString &jobIcon, const QString &appName);

    /**
      * Finishes a job
      *
      * @param jobId the identification number of the job
      */
    void jobFinished(int jobId);

    /**
      * Adds an action (button) to a job
      *
      * @param jobId        the identification number of the job in which the action will be added
      * @param actionText   the button text
      * @return             the identification number of the action (actionId)
      */
    int newAction(int jobId, const QString &actionText);

    /**
      * Edits an existing action
      *
      * @param actionId     the identification number of the action to be modified
      * @param actionText   the new button text
      */
    void editAction(int actionId, const QString &actionText);

    /**
      * Removes an existing action
      *
      * @param actionId the identification number of the action to be removed
      */
    void removeAction(int actionId);

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
    void speed(int jobId, unsigned long bytes_per_second);

    /**
      * Sets the current info message
      *
      * @param jobId    the identification number of the job
      * @param msg      the info message to show
      */
    void infoMessage(int jobId, QString msg);

    /**
      * Starts a copying progress
      *
      * @param jobId    the identification number of the job
      * @param from     the source of the copying
      * @param to       the destination of copying transfer
      */
    void copying(int jobId, QString from, QString to);

    /**
      * Starts a moving progress
      *
      * @param jobId    the identification number of the job
      * @param from     the source of the moving
      * @param to       the destination of the moving
      */
    void moving(int jobId, QString from, QString to);

    /**
      * Starts a deleting progress
      *
      * @param jobId    the identification number of the job
      * @param url      the path that is going to be deleted
      */
    void deleting(int jobId, QString url);

    /**
      * Starts a transferring progress
      *
      * @param jobId    the identification number of the job
      * @param url      the path that is going to be transferred
      */
    void transferring(int jobId, QString url);

    /**
      * Starts a dir creation progress
      *
      * @param jobId    the identification number of the job
      * @param dir      the path where the directory is going to be created
      */
    void creatingDir(int jobId, QString dir);

    /**
      * Starts a stating progress
      *
      * @param jobId    the identification number of the job
      * @param url      the path that is going to be stated
      */
    void stating(int jobId, QString url);

    /**
      * Starts a mounting progress
      *
      * @param jobId    the identification number of the job
      * @param dev      the device that is going to be mounted
      * @param point    the mount point where the device will be mounted
      */
    void mounting(int jobId, QString dev, QString point);

    /**
      * Starts an unmounting progress
      *
      * @param jobId    the identification number of the job
      * @param point    the mount point that is going to be unmounted
      */
    void unmounting(int jobId, QString point);

    /**
      * Popup a message box
      *
      * @param type         type of message box: QuestionYesNo, WarningYesNo, WarningContinueCancel...
      *                     This enum is defined in slavebase.h, it currently is:
      *                     QuestionYesNo = 1, WarningYesNo = 2, WarningContinueCancel = 3,
      *                     WarningYesNoCancel = 4, Information = 5, SSLMessageBox = 6
      * @param text         Message string. May contain newlines
      * @param caption      Message box title
      * @param buttonYes    The text for the first button
      *                     The default is i18n("&Yes")
      * @param buttonNo     The text for the second button
      *                     The default is i18n("&No")
      *
      * Note: for ContinueCancel, buttonYes is the continue button and buttonNo is unused.
      * ====  and for Information, none is used
      *
      * @return             a button code, as defined in KMessageBox, or 0 on communication error
      */
    int messageBox(int jobId, int type, const QString &text, const QString &caption,
                   const QString &buttonYes, const QString &buttonNo);

    /**
      * Sets a job visible or hidden
      *
      * @param jobId    the job that will be hidden or shown
      * @param visible  whether the job will be shown or not
      */
    void setJobVisible(int jobId, bool visible);

    /**
      * Show a SSL Information Dialog
      */
    void showSSLInfoDialog(const QString &url, const KIO::MetaData &data, int mainwindow);

    /**
      * Show an SSL Certificate Selection Dialog
      */
    KSSLCertDialogRet showSSLCertDialog(const QString &host, const QStringList &certList, int mainwindow);

public Q_SLOTS:
    void slotRemoveSystemTrayIcon();
    void applySettings();

protected:
    ProgressListModel *progressListModel;
    QListView *listProgress;

private:
    UIServerAdaptor *serverAdaptor;
    QHash<int, int> m_hashActions;

    static int s_jobId;
    static int s_actionId;
};

#endif // UISERVER_H
