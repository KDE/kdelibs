#/*****************************************************************

Copyright (c) 1999,2000 Preston Brown <pbrown@kde.org>
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
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <dcopobject.h>
#include <dcopclient.h>

QMap<QCString, DCOPObject *> *dcopObjMap = 0;

inline QMap<QCString, DCOPObject *> *objMap()
{
  if (!dcopObjMap)
    dcopObjMap = new QMap<QCString, DCOPObject *>;
  return dcopObjMap;
}

class DCOPObject::DCOPObjectPrivate
{
public:
    DCOPObjectPrivate()
        { m_signalConnections = 0; m_dcopClient = 0; }

    unsigned int m_signalConnections;
    DCOPClient *m_dcopClient;
};

DCOPObject::DCOPObject()
{
    d = new DCOPObjectPrivate;
    ident.sprintf("%p", (void *)this );
    objMap()->insert(ident, this );
}

DCOPObject::DCOPObject(QObject *obj)
{
    d = new DCOPObjectPrivate;
    QObject *currentObj = obj;
    while (currentObj != 0L) {
        ident.prepend( currentObj->name() );
        ident.prepend("/");
        currentObj = currentObj->parent();
    }
    if ( ident[0] == '/' )
        ident = ident.mid(1);

    objMap()->insert(ident, this);
}

DCOPObject::DCOPObject(const QCString &_objId)
  : ident(_objId)
{
    d = new DCOPObjectPrivate;
    if ( ident.isEmpty() )
        ident.sprintf("%p", (void *)this );
    objMap()->insert(ident, this);
}

DCOPObject::~DCOPObject()
{
    DCOPClient *client = DCOPClient::mainClient();
    if ( d->m_signalConnections > 0 && client )
         client->disconnectDCOPSignal( 0, 0, 0, objId(), 0 );

    objMap()->remove(ident);
    delete d;
}

DCOPClient *DCOPObject::callingDcopClient()
{
    return d->m_dcopClient;
}

void DCOPObject::setCallingDcopClient(DCOPClient *client)
{
    d->m_dcopClient = client;
}

bool DCOPObject::setObjId(const QCString &objId)
{
  if (objMap()->find(objId)!=objMap()->end()) return false;

  DCOPClient *client = DCOPClient::mainClient();
    if ( d->m_signalConnections > 0 && client )
         client->disconnectDCOPSignal( 0, 0, 0, ident, 0 );

    objMap()->remove(ident);
    ident=objId;
    objMap()->insert(ident,this);
    return true;
}

QCString DCOPObject::objId() const
{
  return ident;
}

bool DCOPObject::hasObject(const QCString &_objId)
{
  if (objMap()->contains(_objId))
    return true;
  else
    return false;
}

DCOPObject *DCOPObject::find(const QCString &_objId)
{
  QMap<QCString, DCOPObject *>::ConstIterator it;
  it = objMap()->find(_objId);
  if (it != objMap()->end())
    return *it;
  else
    return 0L;
}

QPtrList<DCOPObject> DCOPObject::match(const QCString &partialId)
{
    QPtrList<DCOPObject> mlist;
    QMap<QCString, DCOPObject *>::ConstIterator it(objMap()->begin());
    for (; it != objMap()->end(); ++it)
	if (it.key().left(partialId.length()) == partialId) // found it?
	    mlist.append(it.data());
    return mlist;
}


QCString DCOPObject::objectName( QObject* obj )
{
    if ( obj == 0 )
	return QCString();

    QCString identity;

    QObject *currentObj = obj;
    while (currentObj != 0 )
    {
	identity.prepend( currentObj->name() );
	identity.prepend("/");
	currentObj = currentObj->parent();
    }
    if ( identity[0] == '/' )
	identity = identity.mid(1);

    return identity;
}

bool DCOPObject::process(const QCString &fun, const QByteArray &data,
			 QCString& replyType, QByteArray &replyData)
{
    if ( fun == "interfaces()" ) {
	replyType = "QCStringList";
	QDataStream reply( replyData, IO_WriteOnly );
	reply << interfaces();
	return TRUE;
    } else  if ( fun == "functions()" ) {
	replyType = "QCStringList";
	QDataStream reply( replyData, IO_WriteOnly );
	reply << functions();
	return TRUE;
    }
    return processDynamic( fun, data, replyType, replyData );
}

bool DCOPObject::processDynamic( const QCString&, const QByteArray&, QCString&, QByteArray& )
{
    return FALSE;
}
QCStringList DCOPObject::interfacesDynamic()
{
    QCStringList result;
    return result;
}

QCStringList DCOPObject::functionsDynamic()
{
    QCStringList result;
    return result;
}
QCStringList DCOPObject::interfaces()
{
    QCStringList result = interfacesDynamic();
    result << "DCOPObject";
    return result;
}

QCStringList DCOPObject::functions()
{
    QCStringList result = functionsDynamic();
    result.prepend("QCStringList functions()");
    result.prepend("QCStringList interfaces()");
    return result;
}

void DCOPObject::emitDCOPSignal( const QCString &signal, const QByteArray &data)
{
    DCOPClient *client = DCOPClient::mainClient();
    if ( client )
        client->emitDCOPSignal( objId(), signal, data );
}

bool DCOPObject::connectDCOPSignal( const QCString &sender, const QCString &senderObj,
                                    const QCString &signal,
                                    const QCString &slot,
                                    bool Volatile)
{
    DCOPClient *client = DCOPClient::mainClient();

    if ( !client )
        return false;

    d->m_signalConnections++;
    return client->connectDCOPSignal( sender, senderObj, signal, objId(), slot, Volatile );
}

bool DCOPObject::disconnectDCOPSignal( const QCString &sender, const QCString &senderObj,
                                       const QCString &signal,
                                       const QCString &slot)
{
    DCOPClient *client = DCOPClient::mainClient();

    if ( !client )
        return false;

    d->m_signalConnections--;
    return client->disconnectDCOPSignal( sender, senderObj, signal, objId(), slot );
}


QPtrList<DCOPObjectProxy>* DCOPObjectProxy::proxies = 0;

DCOPObjectProxy::DCOPObjectProxy()
{
    if ( !proxies )
	proxies = new QPtrList<DCOPObjectProxy>;
    proxies->append( this );
}

DCOPObjectProxy::DCOPObjectProxy( DCOPClient*)
{
    if ( !proxies )
	proxies = new QPtrList<DCOPObjectProxy>;
    proxies->append( this );
}

DCOPObjectProxy:: ~DCOPObjectProxy()
{
    if ( proxies )
	proxies->removeRef( this );
}

bool DCOPObjectProxy::process( const QCString& /*obj*/,
			       const QCString& /*fun*/,
			       const QByteArray& /*data*/,
			       QCString& /*replyType*/,
			       QByteArray &/*replyData*/ )
{
    return FALSE;
}

void DCOPObject::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void DCOPObjectProxy::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }
