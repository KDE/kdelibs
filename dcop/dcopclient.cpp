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
};

struct ReplyStruct
{
  bool result;
  QByteArray* replyData;
};

char* DCOPClientPrivate::serverAddr = 0;

/**
 * Callback for ICE.
 */
void DCOPProcessMessage(IceConn iceConn, IcePointer clientObject,
			int opcode, unsigned long length, Bool swap,
			IceReplyWaitInfo *replyWait,
			Bool *replyWaitRet)
{
  DCOPMsg *pMsg = 0;
  DCOPClientPrivate *d = (DCOPClientPrivate *) clientObject;
  DCOPClient *c = d->parent;

  if (opcode == DCOPReply || opcode == DCOPReplyFailed) {
    if ( replyWait ) {
      QByteArray* b = ((ReplyStruct*) replyWait->reply)->replyData;
      ((ReplyStruct*) replyWait->reply)->result = opcode == DCOPReply;

      IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
      b->resize( length );
      IceReadData(iceConn, length, b->data() );
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
    ds >> app >> objId >> fun >> data;

    QByteArray replyData;
    bool b = c->receive( app, objId, fun,
			 data, replyData );

    if ( !b )
	qWarning("DCOP failure in applicaton %s:\n   object '%s' has no function '%s'", app.data(), objId.data(), fun.data() );

    if (opcode != DCOPCall)
      return;

    // we are calling, so we need to set up reply data
    IceGetHeader( iceConn, 1, b ? DCOPReply : DCOPReplyFailed,
		  sizeof(DCOPMsg), DCOPMsg, pMsg );
    int datalen = replyData.size();
    pMsg->length += datalen;
    IceWriteData( iceConn, datalen, (char *) replyData.data());
    IceFlush( iceConn );
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
  char errBuf[1024];

  if ( isAttached() )
      detach();

  if ((d->majorOpcode = IceRegisterForProtocolSetup("DCOP", DCOPVendorString,
						    DCOPReleaseString, 1, DCOPVersions,
						    DCOPAuthCount, DCOPAuthNames,
						    DCOPClientAuthProcs, 0L)) < 0) {
    qDebug("Could not register DCOP protocol with ICE");
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
    qDebug("DCOP open connection: %s",errBuf);
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
    qDebug("DCOP protocol setup: %s", errBuf);
    return false;
  } else if (setupstat == IceProtocolAlreadyActive) {
    /* should not happen because 3rd arg to IceOpenConnection was 0. */
    qDebug("DCOP: internal error in IceOpenConnection");
    return false;
  }


  // check if we have a qApp instantiated.  If we do,
  // we can create a QSocketNotifier and use it for receiving data.
  if (qApp) {
    if (IceConnectionStatus(d->iceConn) != IceConnectAccepted)
      return false;

    if ( d->notifier )
	delete d->notifier;
    d->notifier = new QSocketNotifier(socket(),
				      QSocketNotifier::Read, 0, 0);
    connect(d->notifier, SIGNAL(activated(int)),
	    SLOT(processSocketData(int)));
  }

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
	if ( !attach() ) {
	    return result;
	}
    }
	
    // register the application identifier with the server
    QByteArray data, replyData;
    QDataStream arg( data, IO_WriteOnly );
    arg <<appId;
    if ( call( "DCOPServer", "", "QCString registerAs(QCString)", data, replyData ) ) {
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

bool DCOPClient::send(const QCString &remApp, const QCString &remObjId,
		      const QCString &remFun, const QByteArray &data,
		      bool fast)
{
  if ( !isAttached() )
      return false;

  DCOPMsg *pMsg;

  QByteArray ba;
  QDataStream ds(ba, IO_WriteOnly);
  ds << remApp << remObjId << remFun << data;

  IceGetHeader(d->iceConn, d->majorOpcode, DCOPSend,
	       sizeof(DCOPMsg), DCOPMsg, pMsg);

  int datalen = ba.size();
  pMsg->length += datalen;

  IceWriteData(d->iceConn, datalen, (char *) ba.data());

  IceFlush(d->iceConn);

  if (IceConnectionStatus(d->iceConn) != IceConnectAccepted)
    return false;
  else
    return true;
}

bool DCOPClient::send(const QCString &remApp, const QCString &remObjId,
		      const QCString &remFun, const QString &data,
		      bool fast)
{
  QByteArray ba;
  QDataStream ds(ba, IO_WriteOnly);
  ds << data;
  return send(remApp, remObjId, remFun, ba);
}

bool DCOPClient::process(const QCString &fun, const QByteArray &data,
			 QByteArray &replyData)
{
  return false;
}

bool DCOPClient::isApplicationRegistered( const QCString& remApp)
{
  QByteArray data, replyData;
  QDataStream arg( data, IO_WriteOnly );
  arg << remApp;
  int result = false;
  if ( call( "DCOPServer", "", "bool isApplicationRegistered(QCString)", data, replyData ) ) {
    QDataStream reply( replyData, IO_ReadOnly );
    reply >> result;
  }
  return result;
}

QCStringList DCOPClient::registeredApplications()
{
  QByteArray data, replyData;
  QCStringList result;
  if ( call( "DCOPServer", "", "QCStringList registeredApplications()", data, replyData ) ) {
    QDataStream reply( replyData, IO_ReadOnly );
    reply >> result;
  }
  return result;
}

bool DCOPClient::receive(const QCString &app, const QCString &objId,
			 const QCString &fun, const QByteArray &data,
			 QByteArray &replyData)
{
  if ( !app.isEmpty() && app != d->appId) {
      qDebug("WEIRD! we somehow received a DCOP message w/a different appId");
    return false;
  }

  if ( objId.isEmpty() ) {
      if ( fun == "void applicationRegistered(QCString)" ) {
	  QDataStream ds( data, IO_ReadOnly );
	  QCString r;
	  ds >> r;
	  emit applicationRegistered( r );
	  return TRUE;
      } else if ( fun == "void applicationRemoved(QCString)" ) {
	  QDataStream ds( data, IO_ReadOnly );
	  QCString r;
	  ds >> r;
	  emit applicationRemoved( r );
	  return TRUE;
      }
      return process( fun, data, replyData );
  }
  if (!DCOPObject::hasObject(objId)) {
    qDebug("we received a DCOP message for an object we don't know about!");
    return false;
  } else {
    DCOPObject *objPtr = DCOPObject::find(objId);
    if (!objPtr->process(fun, data, replyData)) {
      qDebug("for some reason, the function didn't process the DCOP request.");
      // obj doesn't understand function or some other error.
      return false;
    }
  }

  return true;
}

bool DCOPClient::call(const QCString &remApp, const QCString &remObjId,
		      const QCString &remFun, const QByteArray &data,
		      QByteArray &replyData, bool fast)
{
  if ( !isAttached() )
      return false;

  DCOPMsg *pMsg;

  QByteArray ba;
  QDataStream ds(ba, IO_WriteOnly);
  ds << remApp << remObjId << remFun << data;

  IceGetHeader(d->iceConn, d->majorOpcode, DCOPCall,
	       sizeof(DCOPMsg), DCOPMsg, pMsg);

  int datalen = ba.size();
  pMsg->length += datalen;

  IceWriteData(d->iceConn, datalen, (char *) ba.data());

  IceFlush(d->iceConn);

  if (IceConnectionStatus(d->iceConn) != IceConnectAccepted)
    return false;

  IceReplyWaitInfo waitInfo;
  waitInfo.sequence_of_request = IceLastSentSequenceNumber(d->iceConn);
  waitInfo.major_opcode_of_request = d->majorOpcode;
  waitInfo.minor_opcode_of_request = DCOPCall;
  ReplyStruct tmp;
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

void DCOPClient::processSocketData(int socknum)
{
  IceProcessMessagesStatus s =  IceProcessMessages(d->iceConn, 0, 0);

  if (s == IceProcessMessagesIOError) {
    IceCloseConnection(d->iceConn);
    qDebug("received an error processing data from the DCOP server!");
    return;
  }
}
