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

#include <dcopclient.moc>
#include <dcopclient.h>
#include <ctype.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/file.h>

#define INT32 QINT32

#include <X11/Xmd.h>
extern "C" {
#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICEutil.h>
#include <X11/ICE/ICEmsg.h>
#include <X11/ICE/ICEproto.h>
}

#include <qtextstream.h>
#include <qfile.h>
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

  static QCString serverAddr; // location of server in ICE-friendly format.
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

QCString DCOPClientPrivate::serverAddr = 0;

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
	qWarning("DCOP failure in application %s:\n   object '%s' has no function '%s'", app.data(), objId.data(), fun.data() );

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
  DCOPClientPrivate::serverAddr = addr;
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
      // here, we obtain the list of possible DCOP connections, 
      // and attach to them.
      QString fName = ::getenv("HOME");
      fName += "/.DCOPserver";
      QFile f(fName);
      if (!f.open(IO_ReadOnly)) {
	emit attachFailed("Could not read network connection list.");
	return false;
      }
      QTextStream t(&f);
      d->serverAddr = t.readLine().latin1();
      f.close();
    }

    if ((d->iceConn = IceOpenConnection(d->serverAddr.data(), 
					0, False, d->majorOpcode,
					sizeof(errBuf), errBuf)) == 0L) {
	emit attachFailed(errBuf);
	d->iceConn = 0;
	return false;
    }

    IceSetShutdownNegotiation(d->iceConn, False);

    int setupstat;
    setupstat = IceProtocolSetup(d->iceConn, d->majorOpcode,
				 (IcePointer) d,
				 True, /* must authenticate */
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

  //  IceFlush(d->iceConn);

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
  if ( !app.isEmpty() && app != d->appId && app != "*") {
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
      return false;

  } else {
    DCOPObject *objPtr = DCOPObject::find(objId);
    if (!objPtr->process(fun, data, replyType, replyData)) {
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
