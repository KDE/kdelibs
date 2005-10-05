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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _KLAUNCHER_H_
#define _KLAUNCHER_H_

#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <qstring.h>
#include <qsocketnotifier.h>
#include <qtimer.h>
#include <q3ptrlist.h>

#include <dcopclient.h>

#include <kurl.h>
#include <kuniqueapplication.h>
#include <kstreamsocket.h>
#include <kserversocket.h>

#include <kio/connection.h>

#include <kservice.h>

#include "autostart.h"

class IdleSlave : public QObject
{
   Q_OBJECT
public:
   IdleSlave(KNetwork::KStreamSocket *socket);
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
   DCOPCString name;
   DCOPCStringList arg_list;
   DCOPCString dcop_name;
   enum status_t { Init = 0, Launching, Running, Error, Done };
   pid_t pid;
   status_t status;
   DCOPClientTransaction *transaction;
   KService::DCOPServiceType_t dcop_service_type;
   bool autoStart;
   QString errorMsg;
#ifdef Q_WS_X11
   DCOPCString startup_id; // "" is the default, "0" for none
   DCOPCString startup_dpy; // Display to send startup notification to.
#endif
   DCOPCStringList envs; // env. variables to be app's environment
   DCOPCString cwd;
};

struct serviceResult
{
  int result;        // 0 means success. > 0 means error (-1 means pending)
  DCOPCString dcopName; // Contains DCOP name on success
  QString error;     // Contains error description on failure.
  pid_t pid;
};

class KLauncher : public KApplication, public DCOPObject
{
   Q_OBJECT

public:
   KLauncher(int kdeinitSocket);

   ~KLauncher();

   void close();
   static void destruct(int exit_code); // exit!

   // DCOP
   virtual bool process(const DCOPCString &fun, const QByteArray &data,
                DCOPCString &replyType, QByteArray &replyData);
   virtual DCOPCStringList functions();
   virtual DCOPCStringList interfaces();

protected:
   void processDied(pid_t pid, long exitStatus);

   void requestStart(KLaunchRequest *request);
   void requestDone(KLaunchRequest *request);

   void setLaunchEnv(const DCOPCString &name, const DCOPCString &value);
   void exec_blind(const DCOPCString &name, const DCOPCStringList &arg_list,
       const DCOPCStringList &envs, const DCOPCString& startup_id = "" );
   bool start_service(KService::Ptr service, const QStringList &urls,
       const DCOPCStringList &envs, const DCOPCString& startup_id = "",
       bool blind = false, bool autoStart = false );
   bool start_service_by_name(const QString &serviceName, const QStringList &urls,
       const DCOPCStringList &envs, const DCOPCString& startup_id, bool blind);
   bool start_service_by_desktop_path(const QString &serviceName, const QStringList &urls,
       const DCOPCStringList &envs, const DCOPCString& startup_id, bool blind);
   bool start_service_by_desktop_name(const QString &serviceName, const QStringList &urls,
       const DCOPCStringList &envs, const DCOPCString& startup_id, bool blind);
   bool kdeinit_exec(const QString &app, const QStringList &args,
       const DCOPCStringList &envs, DCOPCString startup_id, bool wait);

   void waitForSlave(pid_t pid);

   void autoStart(int phase);

   void createArgs( KLaunchRequest *request, const KService::Ptr service,
                    const QStringList &url);

   pid_t requestHoldSlave(const KURL &url, const QString &app_socket);
   pid_t requestSlave(const QString &protocol, const QString &host,
                      const QString &app_socket, QString &error);


   void queueRequest(KLaunchRequest *);

   void send_service_startup_info( KLaunchRequest *request, KService::Ptr service, const DCOPCString& startup_id,
       const DCOPCStringList &envs );
   void cancel_service_startup_info( KLaunchRequest *request, const DCOPCString& startup_id,
       const DCOPCStringList &envs );

public slots:
   void slotAutoStart();
   void slotDequeue();
   void slotKDEInitData(int);
   void slotAppRegistered(const QByteArray &appId);
   void slotSlaveStatus(IdleSlave *);
   void acceptSlave();
   void slotSlaveGone();
   void idleTimeout();

protected:
   Q3PtrList<KLaunchRequest> requestList; // Requests being handled
   Q3PtrList<KLaunchRequest> requestQueue; // Requests waiting to being handled
   serviceResult DCOPresult;
   KLaunchRequest *lastRequest;
   Q3PtrList<SlaveWaitRequest> mSlaveWaitRequest;
   QString mPoolSocketName;
   int kdeinitSocket;
   QSocketNotifier *kdeinitNotifier;
   KNetwork::KServerSocket mPoolSocket;
   Q3PtrList<IdleSlave> mSlaveList;
   QTimer mTimer;
   QTimer mAutoTimer;
   bool bProcessingQueue;
   AutoStart mAutoStart;
   DCOPCString mSlaveDebug;
   DCOPCString mSlaveValgrind;
   DCOPCString mSlaveValgrindSkin;
   bool dontBlockReading;
#ifdef Q_WS_X11
   Display *mCached_dpy;
#endif
};
#endif
