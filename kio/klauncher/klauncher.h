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
#include <qstring.h>
#include <qvaluelist.h>
#include <qsocketnotifier.h>
#include <qlist.h>

#include <dcopclient.h>
#include <kio/connection.h>
#include <ksock.h>
#include <kuniqueapp.h>

#include <kservice.h>

class IdleSlave : public QObject
{
   Q_OBJECT
public:
   IdleSlave(KSocket *socket);
   bool match( const QString &protocol, const QString &host, bool connected);
   void connect( const QString &app_socket);
   pid_t pid() { return mPid;}

protected slots:
   void gotInput();      

protected:
   KIO::Connection mConn;
   QString mProtocol;
   QString mHost;
   bool mConnected;
   pid_t mPid;
};

class KLaunchRequest
{
public:
   QCString name;
   QValueList<QCString> arg_list;
   QCString dcop_name;
   enum status_t { Init = 0, Launching, Running, Error };
   pid_t pid;
   status_t status;
   DCOPClientTransaction *transaction;
   KService::DCOPServiceType_t dcop_service_type;
};

struct serviceResult
{
  int result;        // 0 means success. > 0 means error (-1 means pending)
  QCString dcopName; // Contains DCOP name on success
  QString error;     // Contains error description on failure.
};

class KLauncher : public KUniqueApplication
{
   Q_OBJECT

public:
   KLauncher(int _kdeinitSocket);

protected:
   bool process(const QCString &fun, const QByteArray &data,
                QCString &replyType, QByteArray &replyData);

   void processDied(pid_t pid, long exitStatus);

   void requestStart(KLaunchRequest *request);
   void requestDone(KLaunchRequest *request);

   void exec_blind(const QCString &name, const QValueList<QCString> &arg_list);
   bool start_service(KService::Ptr service, const QString &filename);
   bool start_service_by_name(const QString &serviceName, const QString &filename);
   bool start_service_by_desktop_path(const QString &serviceName, const QString &filename);
   bool start_service_by_desktop_name(const QString &serviceName, const QString &filename);

   void createArgs( KLaunchRequest *request, const KService::Ptr service,
                    const QString &url);

   void replaceArg( QValueList<QCString> &args, const QCString &target, 
                    const QCString &replace, const char *replacePrefix = 0);

   void removeArg( QValueList<QCString> &args, const QCString &target);

   pid_t requestSlave(const QString &protocol, const QString &host,
                      const QString &app_socket, QString &error);

public slots:
   void slotKInitData(int);
   void slotAppRegistered(const QCString &appId);
   void acceptSlave( KSocket *);
   void slotSlaveGone();

protected:
   QList<KLaunchRequest> requestList;
   int kdeinitSocket;
   QSocketNotifier *kdeinitNotifier;
   serviceResult DCOPresult;
   KLaunchRequest *lastRequest;
   QString mPoolSocketName;                         
   KServerSocket *mPoolSocket;
   QList<IdleSlave> mSlaveList;
};
#endif
