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

#include "autostart.h"

#include <sys/types.h>
#include <unistd.h>

#include <config-kdeinit.h> // HAVE_X11

#if HAVE_X11
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
#define USE_KPROCESS_FOR_KIOSLAVES
#endif

#include <QtCore/QString>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QDateTime>
#include <QUrl>
#include <QDBusConnection>
#include <QDBusMessage>

#include <kservice.h>
#include <kio/idleslave.h>
#include <connection_p.h> // TODO: remove (ConnectionServer into own header)

using KIO::IdleSlave;

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
   QString tolerant_dbus_name;
   enum status_t { Init = 0, Launching, Running, Error, Done };
   pid_t pid;
   status_t status;
   QDBusMessage transaction;
   KService::DBusStartupType dbus_startup_type;
   bool autoStart;
   QString errorMsg;
#if HAVE_X11
   QByteArray startup_id; // "" is the default, "0" for none
   QByteArray startup_dpy; // Display to send startup notification to.
#endif
   QStringList envs; // env. variables to be app's environment
   QString cwd;
#ifdef USE_KPROCESS_FOR_KIOSLAVES
protected:
   QProcess *process;
   friend class KLauncher;
#endif
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
#ifndef USE_KPROCESS_FOR_KIOSLAVES
   KLauncher(int kdeinitSocket);
#else
   KLauncher();
#endif

   ~KLauncher();

   void close();

public Q_SLOTS:
   void destruct(); // exit!

protected:
   void processDied(pid_t pid, long exitStatus);

   void requestStart(KLaunchRequest *request);
   void requestDone(KLaunchRequest *request);

   bool start_service(KService::Ptr service, const QStringList &urls,
       const QStringList &envs, const QByteArray &startup_id,
       bool blind, bool autoStart, const QDBusMessage &msg );

   void createArgs(KLaunchRequest *request, const KService::Ptr service,
                   const QList<QUrl> &url);

   void queueRequest(KLaunchRequest *);

   void send_service_startup_info( KLaunchRequest *request, KService::Ptr service, const QByteArray &startup_id,
       const QStringList &envs );
   void cancel_service_startup_info( KLaunchRequest *request, const QByteArray& startup_id,
       const QStringList &envs );

Q_SIGNALS:
    void autoStart0Done();
    void autoStart1Done();
    void autoStart2Done();

public: // remote methods, called by KLauncherAdaptor
    void autoStart(int phase = 1);

    /**
     * Starts a program.
     * 'envs' are environment variables that will be added
     *   to this program's environment before starting it
     * 'startup_id' is for application startup notification,
     * "" is the default, "0" for none
     */
    void exec_blind(const QString &name, const QStringList &arg_list, const QStringList &envs, const QString &startup_id);
    inline void exec_blind(const QString &name, const QStringList &arg_list)
    { exec_blind(name, arg_list, QStringList(), QLatin1String("0")); }

    bool kdeinit_exec(const QString &app, const QStringList &args,
                      const QString& workdir, const QStringList &envs,
                      const QString &startup_id, bool wait, const QDBusMessage &msg);

    void reparseConfiguration();
    void setLaunchEnv(const QString &name, const QString &value);

    /**
     * Start a service by desktop name.
     *
     * 'serviceName' refers to a desktop file describing the service.
     * The service is looked up anywhere in the applications and services directories
     * (see KStandardDirs resource "xdgdata-apps" and "services").
     * E.g. it should have the form "korganizer".
     *
     * 'url', if not empty, will be passed to the service as
     * argument.
     *
     * 'envs' are environment variables that will be added
     *   to this program's environment before starting it
     *
     * 'startup_id' is for application startup notification,
     * "" is the default, "0" for none
     */
    bool start_service_by_desktop_name(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind, const QDBusMessage &msg);

    /**
     * Start a service by desktop path.
     *
     * 'serviceName' refers to a desktop file describing the service.
     * This may be an absolute path or a path relative to the applications
     * and/or services directories (see KStandardDirs)
     * E.g. it should have the form "korganizer.desktop" or
     * "/opt/kde/share/applications/korganizer.desktop".
     * Note that for absolute paths the restrictions of
     * KDesktopFile::isAuthorizedDesktopFile() are obeyed for security.
     *
     * 'url', if not empty, will be passed to the service as
     * argument.
     *
     * 'envs' are environment variables that will be added
     *   to this program's environment before starting it
     *
     * 'startup_id' is for application startup notification,
     * "" is the default, "0" for none
     */
    bool start_service_by_desktop_path(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind, const QDBusMessage &msg);

    /**
     * Start a service by (translated) name - deprecated
     *
     * 'serviceName' refers to the service name as given by·
     * the Name field in the desktop file describing the service.
     *
     * 'url', if not empty, will be passed to the service as
     * argument.
     *
     * 'envs' are environment variables that will be added
     *   to this program's environment before starting it
     *
     * 'startup_id' is for application startup notification,
     * "" is the default, "0" for none
     *
     * @deprecated use start_service_by_desktop_path
     */
    bool start_service_by_name(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind, const QDBusMessage &msg); // KDE5: remove

    pid_t requestHoldSlave(const QUrl &url, const QString &app_socket);

    pid_t requestSlave(const QString &protocol, const QString &host,
                       const QString &app_socket, QString &error);
    /**
     * Return true of there is a slave held for @p url.
     * @since 4.7
     */
    bool checkForHeldSlave(const QString &url);
    void waitForSlave(int pid, const QDBusMessage &msg);
    void terminate_kdeinit();

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
#ifndef USE_KPROCESS_FOR_KIOSLAVES
   int kdeinitSocket;
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
#if HAVE_X11
   Display *mCached_dpy;
#endif
   void processRequestReturn(int status, const QByteArray &requestData);

protected Q_SLOTS:
    void slotGotOutput();
    void slotFinished(int exitCode, QProcess::ExitStatus exitStatus);
};
#endif
