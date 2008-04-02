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
#include <QtCore/QString>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>
#include <QtCore/QList>
#include <QtCore/QObject>

#include <kurl.h>
#include <kio/connection.h>

#include <kservice.h>

#include <QtDBus/QtDBus>
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

#include "autostart.h"

#ifdef Q_WS_WIN
class QProcess;
#endif

class IdleSlave : public QObject
{
   Q_OBJECT
public:
   explicit IdleSlave(QObject *parent);
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

public:
   KIO::Connection mConn;
protected:
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
   QString dbus_name;
   enum status_t { Init = 0, Launching, Running, Error, Done };
   pid_t pid;
   status_t status;
   QDBusMessage transaction;
   KService::DBusStartupType dbus_startup_type;
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
  QString dbusName; // Contains DBUS name on success
  QString error;     // Contains error description on failure.
  pid_t pid;
};

class KLauncher : public QObject
{
   Q_OBJECT

public:
   KLauncher(int kdeinitSocket);

   ~KLauncher();

   void close();

public slots:
   void destruct(); // exit!

protected:
   void processDied(pid_t pid, long exitStatus);

   void requestStart(KLaunchRequest *request);
   void requestDone(KLaunchRequest *request);

   bool start_service(KService::Ptr service, const QStringList &urls,
       const QStringList &envs, const QString &startup_id,
       bool blind, bool autoStart, const QDBusMessage &msg );
   bool kdeinit_exec(const QString &app, const QStringList &args,
       const QStringList &envs, const QString &startup_id, bool wait, const QDBusMessage &msg);

   void createArgs( KLaunchRequest *request, const KService::Ptr service,
                    const QStringList &url);

   void queueRequest(KLaunchRequest *);

   void send_service_startup_info( KLaunchRequest *request, KService::Ptr service, const QString &startup_id,
       const QStringList &envs );
   void cancel_service_startup_info( KLaunchRequest *request, const QString& startup_id,
       const QStringList &envs );

Q_SIGNALS:
    void autoStart0Done();
    void autoStart1Done();
    void autoStart2Done();

public: // remote methods, called by KLauncherAdaptor
    void autoStart(int phase = 1);
    void exec_blind(const QString &name, const QStringList &arg_list, const QStringList &envs, const QString &startup_id);
    inline void exec_blind(const QString &name, const QStringList &arg_list)
    { exec_blind(name, arg_list, QStringList(), QLatin1String("0")); }
    inline void kdeinit_exec(const QString &app, const QStringList &args, const QStringList &env, const QString& startup_id, const QDBusMessage &msg)
    { kdeinit_exec(app, args, env, startup_id, false, msg); }
    inline void kdeinit_exec_wait(const QString &app, const QStringList &args, const QStringList &env, const QString& startup_id, const QDBusMessage &msg)
    { kdeinit_exec(app, args, env, startup_id, true, msg); }

    void reparseConfiguration();
    void setLaunchEnv(const QString &name, const QString &value);
    bool start_service_by_desktop_name(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind, const QDBusMessage &msg);
    bool start_service_by_desktop_path(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind, const QDBusMessage &msg);
    bool start_service_by_name(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind, const QDBusMessage &msg);
    pid_t requestHoldSlave(const KUrl &url, const QString &app_socket);

    pid_t requestSlave(const QString &protocol, const QString &host,
                       const QString &app_socket, QString &error);
    void waitForSlave(int pid, const QDBusMessage &msg);

public Q_SLOTS:
   void slotAutoStart();
   void slotDequeue();
#ifndef Q_WS_WIN
   void slotKDEInitData(int);
#endif
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
   int kdeinitSocket;
#ifdef Q_WS_WIN
   QList<QProcess *>processList;
#else
   QSocketNotifier *kdeinitNotifier;
#endif
   KIO::ConnectionServer mConnectionServer;
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
   void processRequestReturn(int status, const QByteArray &requestData);

protected Q_SLOTS:
    void slotGotOutput();
};
#endif
