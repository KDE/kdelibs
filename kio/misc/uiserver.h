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

class KIO_EXPORT UIServer
    : public KMainWindow
{
  Q_OBJECT

  UIServer();

  virtual ~UIServer();

public:
   static UIServer* createInstance();

    /**
      * Signal a new job
      * @param appId the DCOP application id of the job's parent application
      * @see KIO::Observer::newJob
      * @param showProgress whether to popup the progress for the job.
      *   Usually true, but may be false when we use kio_uiserver for
      *   other things, like SSL dialogs.
      * @return the job id
      */
    int newJob(const QString &appServiceName, bool showProgress, const QString &internalAppName, const QString &jobIcon, const QString &appName);

    void jobFinished(int jobId);

    int newAction(int jobId, const QString &actionText);

    void totalSize(int jobId, KIO::filesize_t size);
    void totalFiles(int jobId, unsigned long files);
    void totalDirs(int jobId, unsigned long dirs);

    void processedSize(int jobId, KIO::filesize_t bytes);
    void processedFiles(int jobId, unsigned long files);
    void processedDirs(int jobId, unsigned long dirs);

    void percent(int jobId, unsigned long ipercent);
    void speed(int jobId, unsigned long bytes_per_second);
    void infoMessage(int jobId, QString msg);

    void copying(int jobId, QString from, QString to);
    void moving(int jobId, QString from, QString to);
    void deleting(int jobId, QString url);
    void transferring(int jobId, QString url);
    void creatingDir(int jobId, QString dir);
    void stating(int jobId, QString url);

    void mounting(int jobId, QString dev, QString point);
    void unmounting(int jobId, QString point);

    void canResume(int jobId, KIO::filesize_t offset);

    /**
      * Popup a message box.
      * @param type type of message box: QuestionYesNo, WarningYesNo, WarningContinueCancel...
      *   This enum is defined in slavebase.h, it currently is:
      *   QuestionYesNo = 1, WarningYesNo = 2, WarningContinueCancel = 3,
      *   WarningYesNoCancel = 4, Information = 5, SSLMessageBox = 6
      * @param text Message string. May contain newlines.
      * @param caption Message box title.
      * @param buttonYes The text for the first button.
      *                  The default is i18n("&Yes").
      * @param buttonNo  The text for the second button.
      *                  The default is i18n("&No").
      * Note: for ContinueCancel, buttonYes is the continue button and buttonNo is unused.
      *       and for Information, none is used.
      * @return a button code, as defined in KMessageBox, or 0 on communication error.
      */
    int messageBox(int jobId, int type, const QString &text, const QString &caption,
                   const QString &buttonYes, const QString &buttonNo);

    void setJobVisible(int id, bool visible);

    /**
      * Show a SSL Information Dialog
      */
    void showSSLInfoDialog(const QString &url, const KIO::MetaData &data, int mainwindow);

    /**
      * @deprecated
      */
    void showSSLInfoDialog(const QString &url, const KIO::MetaData &data);

    /**
      * Show an SSL Certificate Selection Dialog
      */
    KSSLCertDialogRet showSSLCertDialog(const QString &host, const QStringList &certList, int mainwindow);

    /**
      * @deprecated
      */
    KSSLCertDialogRet showSSLCertDialog(const QString &host, const QStringList &certList);

public Q_SLOTS:
    void slotRemoveSystemTrayIcon();
    void applySettings();

protected:
    ProgressListModel *progressListModel;
    QListView *listProgress;

private:
    UIServerAdaptor *serverAdaptor;

    static int s_jobId;
    static int s_actionId;
};

#endif // UISERVER_H
