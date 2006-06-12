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
#include <qlist.h>

#include <kurl.h>
#include <kuniqueapplication.h>
#include <kstreamsocket.h>
#include <kserversocket.h>

#include <kio/connection.h>

#include <kservice.h>

#include <dbus/qdbus.h>

#include "autostart.h"

// ### FIXME: output values for D-BUS calls are missing!

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
   bool onHold(const KUrl &url);
   QString protocol() const   {return mProtocol;}

Q_SIGNALS:
   void statusUpdate(IdleSlave *);

protected Q_SLOTS:
   void gotInput();

protected:
   KIO::Connection mConn;
   QString mProtocol;
   QString mHost;
   bool mConnected;
   pid_t mPid;
   time_t mBirthDate;
   bool mOnHold;
   KUrl mUrl;
};

class SlaveWaitRequest
{
public:
   pid_t pid;
   QDBusMessage transaction;
};

class KLaunchRequest
{
public:
   QString name;
   QStringList arg_list;
   QString dcop_name;
   enum status_t { Init = 0, Launching, Running, Error, Done };
   pid_t pid;
   status_t status;
   QDBusMessage transaction;
   KService::DCOPServiceType_t dcop_service_type;
   bool autoStart;
   QString errorMsg;
#ifdef Q_WS_X11
   QString startup_id; // "" is the default, "0" for none
   QString startup_dpy; // Display to send startup notification to.
#endif
   QStringList envs; // env. variables to be app's environment
   QString cwd;
};

struct serviceResult
{
  int result;        // 0 means success. > 0 means error (-1 means pending)
  QString dcopName; // Contains DCOP name on success
  QString error;     // Contains error description on failure.
  pid_t pid;
};

class KLauncher : public KApplication
{
   Q_OBJECT

public:
   KLauncher(int kdeinitSocket);

   ~KLauncher();

   void close();
   static void destruct(int exit_code); // exit!

protected:
   void processDied(pid_t pid, long exitStatus);

   void requestStart(KLaunchRequest *request);
   void requestDone(KLaunchRequest *request);

   bool start_service(KService::Ptr service, const QStringList &urls,
       const QStringList &envs, const QString &startup_id,
       bool blind, bool autoStart, const QDBusMessage &msg );
#if 0
   bool start_service_by_name(const QString &serviceName, const QStringList &urls,
       const QStringList &envs, const QString& startup_id, bool blind, const QDBusMessage &msg);
   bool start_service_by_desktop_path(const QString &serviceName, const QStringList &urls,
       const QStringList &envs, const QString& startup_id, bool blind, const QDBusMessage &msg);
   bool start_service_by_desktop_name(const QString &serviceName, const QStringList &urls,
       const DCOPCStringList &envs, const DCOPCString& startup_id, bool blind, const QDBusMessage &msg);
#endif
   bool kdeinit_exec(const QString &app, const QStringList &args,
       const QStringList &envs, const QString &startup_id, bool wait, const QDBusMessage &msg);

   void createArgs( KLaunchRequest *request, const KService::Ptr service,
                    const QStringList &url);

   pid_t requestHoldSlave(const KUrl &url, const QString &app_socket);

   void queueRequest(KLaunchRequest *);

   void send_service_startup_info( KLaunchRequest *request, KService::Ptr service, const QString &startup_id,
       const QStringList &envs );
   void cancel_service_startup_info( KLaunchRequest *request, const QString& startup_id,
       const QStringList &envs );

Q_SIGNALS:
    void autoStartDone(int phase);

public Q_SLOTS: // remote slots
    void autoStart(int phase = 1);
    void exec_blind(const QString &name, const QStringList &arg_list, const QStringList &envs, const QString &startup_id);
    inline void exec_blind(const QString &name, const QStringList &arg_list)
    { exec_blind(name, arg_list, QStringList(), QLatin1String("0")); }
    inline void kdeinit_exec(const QString &app, const QStringList &args, const QStringList &env, const QDBusMessage &msg)
    { kdeinit_exec(app, args, env, QString(), false, msg); }
    inline void kdeinit_exec(const QString &app, const QStringList &args, const QDBusMessage &msg)
    { kdeinit_exec(app, args, QStringList(), QString(), false, msg); }
    inline void kdeinit_exec_wait(const QString &app, const QStringList &args, const QStringList &env, const QString& startup_id, const QDBusMessage &msg)
    { kdeinit_exec(app, args, env, startup_id, true, msg); }

    void reparseConfiguration();
    inline int requestHoldSlave(const QString &url, const QString &app_socket)
    { return requestHoldSlave(KUrl(url), app_socket); }
    inline int requestSlave(const QString &protocol, const QString &host, const QString &app_socket)
    { return requestSlave(protocol, host, app_socket); }
    void setLaunchEnv(const QString &name, const QString &value);
    bool start_service_by_desktop_name(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind, const QDBusMessage &msg);
    inline bool start_service_by_desktop_name(const QString &serviceName, const QStringList &urls, const QDBusMessage &msg)
    { return start_service_by_desktop_name(serviceName, urls, QStringList(), QString(), false, msg); }
    bool start_service_by_desktop_path(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind, const QDBusMessage &msg);
    inline bool start_service_by_desktop_path(const QString &serviceName, const QStringList &urls, const QDBusMessage &msg)
    { return start_service_by_desktop_path(serviceName, urls, QStringList(), QString(), false, msg); }
    bool start_service_by_name(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind, const QDBusMessage &msg);
    inline bool start_service_by_name(const QString &serviceName, const QStringList &urls, const QDBusMessage &msg)
    { return start_service_by_name(serviceName, urls, QStringList(), QString(), false, msg); }
    pid_t requestSlave(const QString &protocol, const QString &host,
                       const QString &app_socket, QString &error);
    void waitForSlave(int pid, const QDBusMessage &msg);

public Q_SLOTS:
   void slotAutoStart();
   void slotDequeue();
   void slotKDEInitData(int);
   void slotNameOwnerChanged(const QString &name, const QString &oldOnwer, const QString &newOwner);
   void slotSlaveStatus(IdleSlave *);
   void acceptSlave();
   void slotSlaveGone();
   void idleTimeout();

public:
   serviceResult requestResult; // accessed by the adaptor
protected:
   QList<KLaunchRequest*> requestList; // Requests being handled
   QList<KLaunchRequest*> requestQueue; // Requests waiting to being handled
   KLaunchRequest *lastRequest;
   QList<SlaveWaitRequest*> mSlaveWaitRequest;
   QString mPoolSocketName;
   int kdeinitSocket;
   QSocketNotifier *kdeinitNotifier;
   KNetwork::KServerSocket mPoolSocket;
   QList<IdleSlave*> mSlaveList;
   QTimer mTimer;
   QTimer mAutoTimer;
   bool bProcessingQueue;
   AutoStart mAutoStart;
   QString mSlaveDebug;
   QString mSlaveValgrind;
   QString mSlaveValgrindSkin;
   bool dontBlockReading;
#ifdef Q_WS_X11
   Display *mCached_dpy;
#endif
};
#endif
