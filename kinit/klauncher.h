/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KLAUNCHER_H_
#define _KLAUNCHER_H_

#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qsocketnotifier.h>
#include <qlist.h>
#include <qtimer.h>

#include <dcopclient.h>
#include <kio/connection.h>
#include <ksock.h>
#include <kurl.h>
#include <kuniqueapplication.h>

#include <kservice.h>

#include "autostart.h"

class IdleSlave : public QObject
{
   Q_OBJECT
public:
   IdleSlave(KSocket *socket);
   bool match( const QString &protocol, const QString &host, bool connected);
   void connect( const QString &app_socket);
   pid_t pid() const { return mPid;}
   int age(time_t now);
   void reparseConfiguration();
   bool onHold(const KURL &url);
   QString protocol() const   {return mProtocol;}

signals:
   void statusUpdate(IdleSlave *);
   
protected slots:
   void gotInput();

protected:
   KIO::Connection mConn;
   QString mProtocol;
   QString mHost;
   bool mConnected;
   pid_t mPid;
   time_t mBirthDate;
   bool mOnHold;
   KURL mUrl;
};

class SlaveWaitRequest
{
public:
   pid_t pid;
   DCOPClientTransaction *transaction;
};

class KLaunchRequest
{
public:
   QCString name;
   QValueList<QCString> arg_list;
   QCString dcop_name;
   enum status_t { Init = 0, Launching, Running, Error, Done };
   pid_t pid;
   status_t status;
   DCOPClientTransaction *transaction;
   KService::DCOPServiceType_t dcop_service_type;
   bool autoStart;
   QString errorMsg;
#ifdef Q_WS_X11
   QCString startup_id; // "" is the default, "0" for none
   QCString startup_dpy; // Display to send startup notification to.
#endif
   QValueList<QCString> envs; // env. variables to be app's environment
};

struct serviceResult
{
  int result;        // 0 means success. > 0 means error (-1 means pending)
  QCString dcopName; // Contains DCOP name on success
  QString error;     // Contains error description on failure.
  pid_t pid;
};

class KLauncher : public KUniqueApplication
{
   Q_OBJECT

public:
   KLauncher(int _kdeinitSocket);

   ~KLauncher();

   static void destruct(int exit_code); // exit!

   // DCOP
   virtual bool process(const QCString &fun, const QByteArray &data,
                QCString &replyType, QByteArray &replyData);
   virtual QCStringList functions();
   virtual QCStringList interfaces();

protected:
   void processDied(pid_t pid, long exitStatus);

   void requestStart(KLaunchRequest *request);
   void requestDone(KLaunchRequest *request);

   void setLaunchEnv(const QCString &name, const QCString &value);
   void exec_blind(const QCString &name, const QValueList<QCString> &arg_list,
       const QValueList<QCString> &envs, const QCString& startup_id = "" );
   bool start_service(KService::Ptr service, const QStringList &urls,
       const QValueList<QCString> &envs, const QCString& startup_id = "",
       bool blind = false, bool autoStart = false );
   bool start_service_by_name(const QString &serviceName, const QStringList &urls,
       const QValueList<QCString> &envs, const QCString& startup_id, bool blind);
   bool start_service_by_desktop_path(const QString &serviceName, const QStringList &urls,
       const QValueList<QCString> &envs, const QCString& startup_id, bool blind);
   bool start_service_by_desktop_name(const QString &serviceName, const QStringList &urls,
       const QValueList<QCString> &envs, const QCString& startup_id, bool blind);
   bool kdeinit_exec(const QString &app, const QStringList &args, const QValueList<QCString> &envs, bool wait);

   void waitForSlave(pid_t pid);

   void autoStart(int phase);

   void createArgs( KLaunchRequest *request, const KService::Ptr service,
                    const QStringList &url);

   pid_t requestHoldSlave(const KURL &url, const QString &app_socket);
   pid_t requestSlave(const QString &protocol, const QString &host,
                      const QString &app_socket, QString &error);


   void queueRequest(KLaunchRequest *);
   
   void send_service_startup_info( KLaunchRequest *request, KService::Ptr service, const QCString& startup_id,
       const QValueList<QCString> &envs );
   void cancel_service_startup_info( KLaunchRequest *request, const QCString& startup_id,
       const QValueList<QCString> &envs );

public slots:
   void slotAutoStart();
   void slotDequeue();
   void slotKDEInitData(int);
   void slotAppRegistered(const QCString &appId);
   void slotSlaveStatus(IdleSlave *);
   void acceptSlave( KSocket *);
   void slotSlaveGone();
   void idleTimeout();

protected:
   QPtrList<KLaunchRequest> requestList; // Requests being handled
   QPtrList<KLaunchRequest> requestQueue; // Requests waiting to being handled
   int kdeinitSocket;
   QSocketNotifier *kdeinitNotifier;
   serviceResult DCOPresult;
   KLaunchRequest *lastRequest;
   QPtrList<SlaveWaitRequest> mSlaveWaitRequest;
   QString mPoolSocketName;
   KServerSocket *mPoolSocket;
   QPtrList<IdleSlave> mSlaveList;
   QTimer mTimer;
   QTimer mAutoTimer;
   bool bProcessingQueue;
   AutoStart mAutoStart;
   QCString mSlaveDebug;
   bool dontBlockReading;
#ifdef Q_WS_X11
   Display *mCached_dpy;
#endif
};
#endif
