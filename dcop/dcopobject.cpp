/*****************************************************************

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

#include <qobject.h>
#include <qmap.h>

typedef QMap<DCOPCString, DCOPObject*> ObjectMap;
ObjectMap* kde_dcopObjMap = 0;

static inline ObjectMap* objMap()
{
  if (!kde_dcopObjMap)
    kde_dcopObjMap = new ObjectMap;
  return kde_dcopObjMap;
}

class DCOPObject::Private
{
public:
    Private() : m_signalConnections(0), m_dcopClient(0) { }

    unsigned int m_signalConnections;
    DCOPClient *m_dcopClient;
};

DCOPObject::DCOPObject() : d(new Private)
{
    qsnprintf(ident.data(), ident.size(), "%p", this );
    objMap()->insert(ident, this );
}

DCOPObject::DCOPObject(QObject *obj) : d(new Private)
{
    QObject *currentObj = obj;
    while (currentObj != 0L) {
#ifdef _GNUC
#warning might make sense updating ident from DCOPCString to something that uses QString as objectName is a QString now and not a const char *
#endif
        ident.prepend( currentObj->objectName().toLatin1().constData() );
        ident.prepend("/");
        currentObj = currentObj->parent();
    }
    if ( ident[0] == '/' )
        ident = ident.mid(1);

    objMap()->insert(ident, this);
}

DCOPObject::DCOPObject(const DCOPCString &_objId)
  : ident(_objId), d(new Private)
{
    if ( ident.isEmpty() )
        qsnprintf(ident.data(), ident.size(), "%p", this );
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

bool DCOPObject::setObjId(const DCOPCString &objId)
{
  if (objMap()->contains(objId)) return false;

  DCOPClient *client = DCOPClient::mainClient();
    if ( d->m_signalConnections > 0 && client )
         client->disconnectDCOPSignal( 0, 0, 0, ident, 0 );

    objMap()->remove(ident);
    ident=objId;
    objMap()->insert(ident,this);
    return true;
}

DCOPCString DCOPObject::objId() const
{
  return ident;
}

bool DCOPObject::hasObject(const DCOPCString &_objId)
{
  return objMap()->contains(_objId);
}

DCOPObject *DCOPObject::find(const DCOPCString &_objId)
{
  ObjectMap::ConstIterator it = objMap()->find(_objId);
  if (it != objMap()->end())
    return *it;

  return 0L;
}

QList<DCOPObject*> DCOPObject::match(const DCOPCString &partialId)
{
    QList<DCOPObject*> mlist;
    ObjectMap::ConstIterator it(objMap()->begin());
    for (; it != objMap()->end(); ++it)
	if (it.key().startsWith(partialId)) // found it?
	    mlist << it.value();
    return mlist;
}


DCOPCString DCOPObject::objectName( QObject* obj )
{
    if ( obj == 0 )
	return DCOPCString();

    DCOPCString identity;

    QObject *currentObj = obj;
    while (currentObj != 0 )
    {
#ifdef _GNUC
#warning might make sense updating ident from DCOPCString to something that uses QString as objectName is a QString now and not a const char *
#endif
	identity.prepend( currentObj->objectName().toLatin1().constData() );
	identity.prepend("/");
	currentObj = currentObj->parent();
    }
    if ( identity[0] == '/' )
	identity = identity.mid(1);

    return identity;
}

bool DCOPObject::process(const DCOPCString &fun, const QByteArray &data,
			 DCOPCString& replyType, QByteArray &replyData)
{
    if ( fun == "interfaces()" ) {
	replyType = "QCStringList";
	QDataStream reply( &replyData, QIODevice::WriteOnly );
	reply.setVersion(QDataStream::Qt_3_1);
	reply << interfaces();
	return true;
    } else  if ( fun == "functions()" ) {
	replyType = "QCStringList";
	QDataStream reply( &replyData, QIODevice::WriteOnly );
	reply.setVersion(QDataStream::Qt_3_1);
	reply << functions();
	return true;
    }
    return processDynamic( fun, data, replyType, replyData );
}

bool DCOPObject::processDynamic( const DCOPCString&, const QByteArray&, DCOPCString&, QByteArray& )
{
    return false;
}
DCOPCStringList DCOPObject::interfacesDynamic()
{
    DCOPCStringList result;
    return result;
}

DCOPCStringList DCOPObject::functionsDynamic()
{
    DCOPCStringList result;
    return result;
}
DCOPCStringList DCOPObject::interfaces()
{
    DCOPCStringList result = interfacesDynamic();
    result << "DCOPObject";
    return result;
}

DCOPCStringList DCOPObject::functions()
{
    DCOPCStringList result = functionsDynamic();
    result.prepend("QCStringList functions()");
    result.prepend("QCStringList interfaces()");
    return result;
}

void DCOPObject::emitDCOPSignal( const DCOPCString &signal, const QByteArray &data)
{
    DCOPClient *client = DCOPClient::mainClient();
    if ( client )
        client->emitDCOPSignal( objId(), signal, data );
}

bool DCOPObject::connectDCOPSignal( const DCOPCString &sender, const DCOPCString &senderObj,
                                    const DCOPCString &signal,
                                    const DCOPCString &slot,
                                    bool Volatile)
{
    DCOPClient *client = DCOPClient::mainClient();

    if ( !client )
        return false;

    d->m_signalConnections++;
    return client->connectDCOPSignal( sender, senderObj, signal, objId(), slot, Volatile );
}

bool DCOPObject::disconnectDCOPSignal( const DCOPCString &sender, const DCOPCString &senderObj,
                                       const DCOPCString &signal,
                                       const DCOPCString &slot)
{
    DCOPClient *client = DCOPClient::mainClient();

    if ( !client )
        return false;

    d->m_signalConnections--;
    return client->disconnectDCOPSignal( sender, senderObj, signal, objId(), slot );
}


QList<DCOPObjectProxy*>* DCOPObjectProxy::proxies = 0;

DCOPObjectProxy::DCOPObjectProxy()
{
    if ( !proxies )
	proxies = new QList<DCOPObjectProxy*>;
    proxies->append( this );
}

DCOPObjectProxy:: ~DCOPObjectProxy()
{
    if ( proxies )
	proxies->removeAll( this );
}

bool DCOPObjectProxy::process( const DCOPCString& /*obj*/,
			       const DCOPCString& /*fun*/,
			       const QByteArray& /*data*/,
			       DCOPCString& /*replyType*/,
			       QByteArray &/*replyData*/ )
{
    return false;
}

void DCOPObject::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void DCOPObjectProxy::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }
