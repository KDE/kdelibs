/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>
 
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

#include <kapp.h>
#include <qbitarray.h>
#include <dcopclient.h>
#include <dcopobject.h>

/**
 g++ -o testdcop testdcop.cpp -I$KDEDIR/include -I$QTDIR/include -L$KDEDIR/lib -L$QTDIR/lib -lkdecore

**/


class MyDCOPObject : public DCOPObject
{
public:
  MyDCOPObject(const QCString &name) : DCOPObject(name) {}
  bool process(const QCString &fun, const QByteArray &data,
	       QByteArray &replyData);
  void function(const QCString &arg) { qDebug("function got arg: %s",arg.data()); }
};

bool MyDCOPObject::process(const QCString &fun, const QByteArray &data,
			   QByteArray &replyData)
{
  qDebug("in MyDCOPObject::process");
  if (fun == "aFunction") {
    QDataStream args(data, IO_ReadOnly);
    QCString arg;
    args >> arg;
    function(arg);
    return true;
  }

  return false;
}

int main(int argc, char **argv)
{
  KApplication app(argc, argv);

  QByteArray data, reply;
  DCOPClient *client; client = app.dcopClient();
//   client->attach();
  client->attach(app.name());

  if (!client->call(app.name(), "unknownObj", "unknownFunction", data, reply))
    qDebug("I couldn't call myself.");

  DCOPObject *obj1 = new MyDCOPObject("object1");

  QDataStream ds(data, IO_WriteOnly);
  ds << QString("This is the argument string");

  if (!client->call(app.name(), "object1", "aFunction", data, reply))
    qDebug("I couldn't call myself");

  if ( !client->isApplicationAttached("gubel") ) //app.name()) )
    qDebug("not attached");
  else
    qDebug("attached");

  int n = client->attachedApplications().count();
  qDebug("number of attached applications = %d", n );

  return app.exec();

  client->detach();
}
