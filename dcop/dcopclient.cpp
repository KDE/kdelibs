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

#include <dcopclient.moc>
#include <dcopclient.h>
#include <ctype.h>

#include <unistd.h>

#define INT32 QINT32

#include <X11/Xmd.h>
extern "C" {
#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICEutil.h>
#include <X11/ICE/ICEmsg.h>
#include <X11/ICE/ICEproto.h>
}

#include <qdatastream.h>
#include <qsocketnotifier.h>

#include <dcopglobal.h>
#include <dcopobject.h>

class DCOPClientPrivate
{
public:
  DCOPClient *parent;
  QCString appId;
  IceConn iceConn;
  int majorOpcode; // major opcode negotiated w/server and used to tag all comms.

  int majorVersion, minorVersion; // protocol versions negotiated w/server
  char *vendor, *release; // information from server

  static char* serverAddr; // location of server in ICE-friendly format.
  QSocketNotifier *notifier;
  bool registered;

  QCString senderId;
    
   QList<DCOPObjectProxy> proxies;
};

struct ReplyStruct
{
  bool result;
  QCString* replyType;
  QByteArray* replyData;
};

char* DCOPClientPrivate::serverAddr = 0;

/**
 * Callback for ICE.
 */
void DCOPProcessMessage(IceConn iceConn, IcePointer clientObject,
			int opcode, unsigned long length, Bool /*swap*/,
			IceReplyWaitInfo *replyWait,
			Bool *replyWaitRet)
{
  DCOPMsg *pMsg = 0;
  DCOPClientPrivate *d = (DCOPClientPrivate *) clientObject;
  DCOPClient *c = d->parent;

  if (opcode == DCOPReply || opcode == DCOPReplyFailed) {
    if ( replyWait ) {
      QByteArray* b = ((ReplyStruct*) replyWait->reply)->replyData;
      QCString* t =  ((ReplyStruct*) replyWait->reply)->replyType;
      ((ReplyStruct*) replyWait->reply)->result = opcode == DCOPReply;

      IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
      QByteArray tmp( length );
      IceReadData(iceConn, length, tmp.data() );
      
      // TODO: avoid this data copying
      QDataStream tmpStream( tmp, IO_ReadOnly );
      tmpStream >> *t >> *b;
      
      *replyWaitRet = True;
      return;
    } else {
      qDebug("Very strange! got a DCOPReply opcode, but we were not waiting for a reply!");
      return;
    }
  } else if (opcode == DCOPCall || opcode == DCOPSend) {
    IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
    QByteArray ba( length );
    IceReadData(iceConn, length, ba.data() );
    QDataStream ds( ba, IO_ReadOnly );
    QCString app, objId, fun;
    QByteArray data;
    ds >> d->senderId >> app >> objId >> fun >> data;

    QCString replyType;
    QByteArray replyData;
    bool b = c->receive( app, objId, fun,
			 data, replyType, replyData );

    if ( !b )
	qWarning("DCOP failure in applicaton %s:\n   object '%s' has no function '%s'", app.data(), objId.data(), fun.data() );

    if (opcode != DCOPCall)
      return;

    QByteArray reply;
    QDataStream replyStream( reply, IO_WriteOnly );
    replyStream << replyType << replyData.size();
    
    // we are calling, so we need to set up reply data
    IceGetHeader( iceConn, 1, b ? DCOPReply : DCOPReplyFailed,
		  sizeof(DCOPMsg), DCOPMsg, pMsg );
    int datalen = reply.size() + replyData.size();
    pMsg->length += datalen;
    // use IceSendData not IceWriteData to avoid a copy.  Output buffer
    // shouldn't need to be flushed.
    IceSendData( iceConn, reply.size(), (char *) reply.data());
    IceSendData( iceConn, replyData.size(), (char *) replyData.data());
  }
}

static IcePoVersionRec DCOPVersions[] = {
  { DCOPVersionMajor, DCOPVersionMinor,  DCOPProcessMessage }
};

DCOPClient::DCOPClient()
{
  d = new DCOPClientPrivate;
  d->parent = this;
  d->iceConn = 0L;
  d->majorOpcode = 0;
  d->appId = 0;
  d->notifier = 0L;
  d->registered = false;
}

DCOPClient::~DCOPClient()
{
  if (IceConnectionStatus(d->iceConn) == IceConnectAccepted) {
    detach();
  }

  delete d->notifier;
  delete d;
}

void DCOPClient::setServerAddress(const QCString &addr)
{
  DCOPClientPrivate::serverAddr = qstrdup(addr);
}

bool DCOPClient::attach()
{
    return attachInternal();
}

bool DCOPClient::attachInternal( bool registerAsAnonymous )
{
    char errBuf[1024];

    if ( isAttached() )
	detach();

    if ((d->majorOpcode = IceRegisterForProtocolSetup((char *) "DCOP", (char *) DCOPVendorString,
						      (char *) DCOPReleaseString, 1, DCOPVersions,
						      DCOPAuthCount, (char **) DCOPAuthNames,
						      DCOPClientAuthProcs, 0L)) < 0) {
	emit attachFailed("Communications could not be established.");
	return false;
    }

    // first, check if serverAddr was ever set.
    if (!d->serverAddr) {
	// here, we will check some environment variable and find the
	// DCOP server.  Now, we hardcode it.  CHANGE ME
	char buff[1024];

	gethostname(buff, 1023);
	d->serverAddr = qstrdup( (QCString("local/") + buff + ":/tmp/.ICE-unix/5432").data() );
	//    serverAddr = "tcp/faui06e:5000";
    }

    if ((d->iceConn = IceOpenConnection(d->serverAddr, 0, 0, d->majorOpcode,
					sizeof(errBuf), errBuf)) == 0L) {
	emit attachFailed(errBuf);
	d->iceConn = 0;
	return false;
    }

    IceSetShutdownNegotiation(d->iceConn, False);

    int setupstat;
    setupstat = IceProtocolSetup(d->iceConn, d->majorOpcode,
				 (IcePointer) d,
				 False, /* must authenticate */
				 &(d->majorVersion), &(d->minorVersion),
				 &(d->vendor), &(d->release), 1024, errBuf);

    if (setupstat == IceProtocolSetupFailure ||
	setupstat == IceProtocolSetupIOError) {
	IceCloseConnection(d->iceConn);
	emit attachFailed(errBuf);
	return false;
    } else if (setupstat == IceProtocolAlreadyActive) {
	/* should not happen because 3rd arg to IceOpenConnection was 0. */
	emit attachFailed("internal error in IceOpenConnection");
	return false;
    }


    if (IceConnectionStatus(d->iceConn) != IceConnectAccepted) {
	emit attachFailed("DCOP server did not accept the connection.");
	return false;
    }
    
    // check if we have a qApp instantiated.  If we do,
    // we can create a QSocketNotifier and use it for receiving data.
    if (qApp) {
	if ( d->notifier )
	    delete d->notifier;
	d->notifier = new QSocketNotifier(socket(),
					  QSocketNotifier::Read, 0, 0);
	connect(d->notifier, SIGNAL(activated(int)),
		SLOT(processSocketData(int)));
    }

    if ( registerAsAnonymous )
	registerAs( "anonymous" );

    return true;
}


bool DCOPClient::detach()
{
  int status;

  if (d->iceConn) {
    IceProtocolShutdown(d->iceConn, d->majorOpcode);
    status = IceCloseConnection(d->iceConn);
    if (status != IceClosedNow)
      return false;
  }
  delete d->notifier;
  d->notifier = 0L;
  d->registered = false;
  return true;
}

bool DCOPClient::isAttached() const
{
  if (!d->iceConn)
    return false;

  return (IceConnectionStatus(d->iceConn) == IceConnectAccepted);
}


QCString DCOPClient::registerAs( const QCString& appId )
{
    qDebug("registeras %s", appId.data() );
    QCString result;
    if ( !isAttached() ) {
	if ( !attachInternal( FALSE ) ) {
	    return result;
	}
    }
	
    // register the application identifier with the server
    QCString replyType;
    QByteArray data, replyData;
    QDataStream arg( data, IO_WriteOnly );
    arg <<appId;
    if ( call( "DCOPServer", "", "registerAs(QCString)", data, replyType, replyData ) ) {
	QDataStream reply( replyData, IO_ReadOnly );
	reply >> result;
    }
    d->appId = result;
    d->registered = !result.isNull();
    return result;
}

bool DCOPClient::isRegistered() const
{
    return d->registered;
}


QCString DCOPClient::appId() const
{
    return d->appId;
}


int DCOPClient::socket() const
{
  if (d->iceConn)
    return IceConnectionNumber(d->iceConn);
  else
    return 0;
}

static inline bool isIdentChar( char x )
{						// Avoid bug in isalnum
    return x == '_' || (x >= '0' && x <= '9') ||
	 (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z');
}

QCString DCOPClient::normalizeFunctionSignature( const QCString& fun ) {
    if ( fun.isEmpty() )				// nothing to do
	return fun.copy();
    QCString result( fun.size() );
    char *from	= fun.data();
    char *to	= result.data();
    char *first = to;
    char last = 0;
    while ( TRUE ) {
	while ( *from && isspace(*from) )
	    from++;
	if ( last && isIdentChar( last ) && isIdentChar( *from ) )
	    *to++ = 0x20;
	while ( *from && !isspace(*from) ) {
	    last = *from++;
	    *to++ = last;
	}
	if ( !*from )
	    break;
    }
    if ( to > first && *(to-1) == 0x20 )
	to--;
    *to = '\0';
    result.resize( (int)((long)to - (long)result.data()) + 1 );
    return result;
}


QCString DCOPClient::senderId() const
{
    return d->senderId;
}


bool DCOPClient::send(const QCString &remApp, const QCString &remObjId,
		      const QCString &remFun, const QByteArray &data,
		      bool)
{
  if ( !isAttached() )
      return false;


  DCOPMsg *pMsg;

  QByteArray ba;
  QDataStream ds(ba, IO_WriteOnly);
  ds << d->appId << remApp << remObjId << normalizeFunctionSignature(remFun) << data.size();

  IceGetHeader(d->iceConn, d->majorOpcode, DCOPSend,
	       sizeof(DCOPMsg), DCOPMsg, pMsg);

  int datalen = ba.size() + data.size();
  pMsg->length += datalen;

  IceSendData( d->iceConn, ba.size(), (char *) ba.data() );
  IceSendData( d->iceConn, data.size(), (char *) data.data() );

  if (IceConnectionStatus(d->iceConn) != IceConnectAccepted)
    return false;
  else
    return true;
}

bool DCOPClient::send(const QCString &remApp, const QCString &remObjId,
		      const QCString &remFun, const QString &data,
		      bool)
{
  QByteArray ba;
  QDataStream ds(ba, IO_WriteOnly);
  ds << data;
  return send(remApp, remObjId, remFun, ba);
}

bool DCOPClient::process(const QCString &, const QByteArray &,
			 QCString&, QByteArray &)
{
  return false;
}

bool DCOPClient::isApplicationRegistered( const QCString& remApp)
{
  QCString replyType;
  QByteArray data, replyData;
  QDataStream arg( data, IO_WriteOnly );
  arg << remApp;
  int result = false;
  if ( call( "DCOPServer", "", "isApplicationRegistered(QCString)", data, replyType, replyData ) ) {
    QDataStream reply( replyData, IO_ReadOnly );
    reply >> result;
  }
  return result;
}

QCStringList DCOPClient::registeredApplications()
{
  QCString replyType;
  QByteArray data, replyData;
  QCStringList result;
  if ( call( "DCOPServer", "", "registeredApplications()", data, replyType, replyData ) ) {
    QDataStream reply( replyData, IO_ReadOnly );
    reply >> result;
  }
  return result;
}

bool DCOPClient::receive(const QCString &app, const QCString &objId,
			 const QCString &fun, const QByteArray &data,
			 QCString& replyType, QByteArray &replyData)
{
  if ( !app.isEmpty() && app != d->appId) {
      qDebug("WEIRD! we somehow received a DCOP message w/a different appId");
    return false;
  }

  if ( objId.isEmpty() ) {
      if ( fun == "applicationRegistered(QCString)" ) {
	  QDataStream ds( data, IO_ReadOnly );
	  QCString r;
	  ds >> r;
	  emit applicationRegistered( r );
	  return TRUE;
      } else if ( fun == "applicationRemoved(QCString)" ) {
	  QDataStream ds( data, IO_ReadOnly );
	  QCString r;
	  ds >> r;
	  emit applicationRemoved( r );
	  return TRUE;
      }
      return process( fun, data, replyType, replyData );
  }
  if (!DCOPObject::hasObject(objId)) {
      
      for ( DCOPObjectProxy* proxy = d->proxies.first(); proxy; proxy = d->proxies.next() ) {
	  if ( proxy->process( objId, fun, data, replyType, replyData ) )
	      return TRUE;
      }
      qDebug("we received a DCOP message for an object '%s' we don't know about!", objId.data());
      return false;
      
  } else {
    DCOPObject *objPtr = DCOPObject::find(objId);
    if (!objPtr->process(fun, data, replyType, replyData)) {
      qDebug("for some reason, the function didn't process the DCOP request.");
      // obj doesn't understand function or some other error.
      return false;
    }
  }

  return true;
}

bool DCOPClient::call(const QCString &remApp, const QCString &remObjId,
		      const QCString &remFun, const QByteArray &data,
		      QCString& replyType, QByteArray &replyData, bool)
{
  if ( !isAttached() )
      return false;

  DCOPMsg *pMsg;

  QByteArray ba;
  QDataStream ds(ba, IO_WriteOnly);
  ds << d->appId << remApp << remObjId << normalizeFunctionSignature(remFun) << data.size();

  IceGetHeader(d->iceConn, d->majorOpcode, DCOPCall,
	       sizeof(DCOPMsg), DCOPMsg, pMsg);

  int datalen = ba.size() + data.size();
  pMsg->length += datalen;

  IceSendData(d->iceConn, ba.size(), (char *) ba.data());
  IceSendData(d->iceConn, data.size(), (char *) data.data());


  if (IceConnectionStatus(d->iceConn) != IceConnectAccepted)
    return false;

  IceReplyWaitInfo waitInfo;
  waitInfo.sequence_of_request = IceLastSentSequenceNumber(d->iceConn);
  waitInfo.major_opcode_of_request = d->majorOpcode;
  waitInfo.minor_opcode_of_request = DCOPCall;
  ReplyStruct tmp;
  tmp.replyType = &replyType;
  tmp.replyData = &replyData;
  waitInfo.reply = (IcePointer) &tmp;

  Bool readyRet = False;
  IceProcessMessagesStatus s;

  do {
    s = IceProcessMessages(d->iceConn, &waitInfo,
			   &readyRet);
  } while (!readyRet);

  if (s == IceProcessMessagesIOError) {
    IceCloseConnection(d->iceConn);
    qDebug("received an error processing data from DCOP server!");
    return false;
  }

  return tmp.result;
}

void DCOPClient::processSocketData(int)
{
  IceProcessMessagesStatus s =  IceProcessMessages(d->iceConn, 0, 0);

  if (s == IceProcessMessagesIOError) {
    IceCloseConnection(d->iceConn);
    qDebug("received an error processing data from the DCOP server!");
    return;
  }
}

void DCOPClient::installObjectProxy( DCOPObjectProxy* obj)
{
    d->proxies.append( obj );
}

void DCOPClient::removeObjectProxy( DCOPObjectProxy* obj)
{
    d->proxies.removeRef( obj );
}
