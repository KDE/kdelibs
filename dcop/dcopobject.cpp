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

#include <dcopobject.h>
#include <dcopclient.h>

static QMap<QCString, DCOPObject *> *objMap_ = 0;

inline QMap<QCString, DCOPObject *> *objMap()
{
  if (!objMap_)
    objMap_ = new QMap<QCString, DCOPObject *>;
  return objMap_;
}

DCOPObject::DCOPObject()
{
    ident.sprintf("%p", this );
    objMap()->insert(ident, this );
}

DCOPObject::DCOPObject(QObject *obj)
{
  QObject *currentObj = obj;
  while (currentObj != 0L) {
    ident.prepend( currentObj->name() );
    ident.prepend("/");
    currentObj = currentObj->parent();
  }
  if ( ident[0] == '/' )
      ident = ident.mid(1);
  qDebug("set dcopobject id to %s",ident.data());

  objMap()->insert(ident, this);
}

DCOPObject::DCOPObject(const QCString &objId)
  : ident(objId)
{
  ident = objId;
  objMap()->insert(ident, this);
}

DCOPObject::~DCOPObject()
{
  objMap()->remove(ident);
}

QCString DCOPObject::objId() const
{
  return ident;
}

bool DCOPObject::hasObject(const QCString &objId)
{
  if (objMap()->contains(objId))
    return true;
  else
    return false;
}

DCOPObject *DCOPObject::find(const QCString &objId)
{
  QMap<QCString, DCOPObject *>::ConstIterator it;
  it = objMap()->find(objId);
  if (it != objMap()->end())
    return *it;
  else
    return 0L;
}

QCString DCOPObject::objectName( QObject* obj )
{
    if ( obj == 0 )
	return QCString();

    QCString ident;

    QObject *currentObj = obj;
    while (currentObj != 0 )
    {
	ident.prepend( currentObj->name() );
	ident.prepend("/");
	currentObj = currentObj->parent();
    }
    if ( ident[0] == '/' )
	ident = ident.mid(1);

    return ident;
}

DCOPObjectProxy::DCOPObjectProxy( DCOPClient* client )
{
    parent = client;
    if ( !parent ) {
	qWarning("DCOPObjectProxy: no client specified" );
	return;
    }
    parent->installObjectProxy( this );
}

DCOPObjectProxy:: ~DCOPObjectProxy()
{
    if ( parent )
	parent->removeObjectProxy( this );
}

bool DCOPObjectProxy::process( const QCString& /*obj*/, 
			       const QCString& /*fun*/, 
			       const QByteArray& /*data*/,
			       QCString& /*replyType*/, 
			       QByteArray &/*replyData*/ )
{
    return FALSE;
}

