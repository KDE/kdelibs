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
#include <qstring.h>
#include <qvaluelist.h>
#include <kuniqueapp.h>
#include <dcopclient.h>
#include <qsocketnotifier.h>

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
};

class LaunchApp : public KUniqueApplication
{
   Q_OBJECT

public:
   LaunchApp(int argc, char **argv, const QCString &appName, int _kinitSocket);

protected:
   bool process(const QCString &fun, const QByteArray &data,
                QCString &replyType, QByteArray &replyData);

   void processDied(pid_t pid, long exitStatus);

   void requestStart(KLaunchRequest *request);
   void requestDone(KLaunchRequest *request);

public slots:
   void slotKInitData(int);
   void slotAppRegistered(const QCString &appId);

protected:
   QList<KLaunchRequest> requestList;
   int kinitSocket;
   QSocketNotifier *kinitNotifier;
};
#endif
