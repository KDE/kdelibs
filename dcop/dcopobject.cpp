/*****************************************************************

Copyright (c) 1999 Preston Brown <pbrown@kde.org>
Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
******************************************************************/

#include <dcopobject.h>
#include <dcopclient.h>

template QList<DCOPObjectProxy>;

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

bool DCOPObject::process(const QCString &fun, const QByteArray &data,
			 QCString& replyType, QByteArray &replyData)
{
    return processDynamic( fun, data, replyType, replyData );
}

bool DCOPObject::processDynamic( const QCString&, const QByteArray&, QCString&, QByteArray& )
{
    return FALSE;
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

