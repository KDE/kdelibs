// -*- c++ -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
 *
 * $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef KIO_DAEMON_H
#define KIO_DAEMON_H

#include <qlist.h>
#include <ksock.h>
#include <kuniqueapp.h>
#include <kio/connection.h>

class IdleSlave : public QObject
{
   Q_OBJECT
public:
   IdleSlave(KSocket *socket);
   bool match( const QString &protocol, const QString &host, bool connected);
   void connect( const QString &app_socket);

protected slots:
   void gotInput();      

protected:
   KIO::Connection mConn;
   QString mProtocol;
   QString mHost;
   bool mConnected;
};

class KIODaemon : public KUniqueApplication {
    Q_OBJECT

public:
    KIODaemon(int& argc, char** argv);

    virtual bool process(const QCString &fun, const QByteArray &data,
			 QCString &replyType, QByteArray &replyData);

protected slots:
    void acceptSlave( KSocket *);
    void slotSlaveGone();

protected:
    QString requestSlave(const QString &protocol, const QString &host,
                         const QString &app_socket);
                         
    QString mPoolSocketName;                         
    KServerSocket *mPoolSocket;
    QList<IdleSlave> mSlaveList;
};

#endif
