/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qobject.h>
#include <qcstring.h>
#include <qstring.h>
#include <qsocketnotifier.h>
#include <qlist.h>
#include <qvaluelist.h>
#include <qcstring.h>
#include <qdict.h>
#include <qptrdict.h>
#include <qapplication.h>

#define INT32 QINT32
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <X11/ICE/ICElib.h>
extern "C" {
#include <X11/ICE/ICEutil.h>
#include <X11/ICE/ICEmsg.h>
#include <X11/ICE/ICEproto.h>
}


class DCOPConnection;
class DCOPListener;

typedef QValueList<QCString> QCStringList;

class DCOPServer : public QObject
{
Q_OBJECT
 public:
  DCOPServer();
 ~DCOPServer();

 void* watchConnection( IceConn iceConn );
 void removeConnection( void* data );
 void processMessage( IceConn iceConn, int opcode, unsigned long length, Bool swap);

 virtual bool receive(const QCString &app, const QCString &obj,
		      const QCString &fun, const QByteArray& data,
		      QCString& replyType, QByteArray &replyData, IceConn iceConn);

private slots:
  void newClient( int socket );
  void processData( int socket );

 private:
  int majorOpcode;
  QList<DCOPListener> listener;
  QDict<DCOPConnection> appIds;
  QPtrDict<DCOPConnection> clients;
};
