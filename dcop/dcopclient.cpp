#include <dcopclient.moc>
#include <dcopclient.h>

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
  QString appId;
  IceConn iceConn;
  int majorOpcode; // major opcode negotiated w/server and used to tag all comms.

  int majorVersion, minorVersion; // protocol versions negotiated w/server
  char *vendor, *release; // information from server

  static QString serverAddr; // location of server in ICE-friendly format.
  QSocketNotifier *notifier;
};

struct ReplyStruct
{
  bool result;
  QByteArray* replyData;
};

QString DCOPClientPrivate::serverAddr = QString::null;

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
    QString app, objId, fun;
    QByteArray data;
    ds >> app >> objId >> fun >> data;
    
    QByteArray replyData;
    bool b = c->receive( app, objId, fun, 
			 data, replyData );

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

DCOPClient::DCOPClient(const QString &appId)
{
  d = new DCOPClientPrivate;
  d->parent = this;
  d->iceConn = 0L;
  d->majorOpcode = 0;
  d->appId = appId;
  d->notifier = 0L;
}

DCOPClient::~DCOPClient()
{
  if (IceConnectionStatus(d->iceConn) == IceConnectAccepted) {
    detach();
  }

  delete d->notifier;
  delete d;
}

void DCOPClient::setServerAddress(const QString &addr)
{ 
  DCOPClientPrivate::serverAddr = addr; 
}

bool DCOPClient::attach()
{
  char errBuf[1024];

  if ((d->majorOpcode = IceRegisterForProtocolSetup("DCOP", DCOPVendorString, 
						    DCOPReleaseString, 1, DCOPVersions,
						    DCOPAuthCount, DCOPAuthNames,
						    DCOPClientAuthProcs, 0L)) < 0) {
    qDebug("Could not register DCOP protocol with ICE");
    return false;
  }
  
  // first, check if serverAddr was ever set.
  if (d->serverAddr.isEmpty())
    // here, we will check some environment variable and find the
    // DCOP server.  Now, we hardcode it.  CHANGE ME
    d->serverAddr = "local/localhost:/tmp/.ICE-unix/5432";
    //    serverAddr = "tcp/faui06e:5000";
  
  if ((d->iceConn = IceOpenConnection((char *) d->serverAddr.ascii(), 
				      0, 0, d->majorOpcode, sizeof(errBuf),
				      errBuf)) == 0L) {
    qDebug("Could not open connection to DCOP server, msg %s",errBuf);
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
    qDebug("Error setting up DCOP protocol, msg is %s", errBuf);
    return false;
  } else if (setupstat == IceProtocolAlreadyActive) {
    /* should not happen because 3rd arg to IceOpenConnection was 0. */
    qDebug("internal error in IceOpenConnection");
    return false;
  }

  DCOPMsg *pMsg;
  
  IceGetHeader(d->iceConn, d->majorOpcode, DCOPRegisterClient, 
	       sizeof(DCOPMsg), DCOPMsg, pMsg);

  QByteArray ba;
  QDataStream ds(ba, IO_WriteOnly);
  ds << d->appId;

  int datalen = ba.size();
  
  pMsg->length += datalen;
  IceWriteData(d->iceConn, datalen, (char *) ba.data());
  
  IceFlush(d->iceConn);

  // check if we have a qApp instantiated.  If we do,
  // we can create a QSocketNotifier and use it for receiving data.
  if (qApp) {
    if (IceConnectionStatus(d->iceConn) != IceConnectAccepted)
      return false;

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

  IceProtocolShutdown(d->iceConn, d->majorOpcode);
  status = IceCloseConnection(d->iceConn);
  if (status != IceClosedNow)
    return false;
  else
    return true;
}

int DCOPClient::socket() const
{
  return IceConnectionNumber(d->iceConn); 
}

bool DCOPClient::send(const QString &remApp, const QString &remObjId, 
		      const QString &remFun, const QByteArray &data, 
		      bool fast)
{
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

bool DCOPClient::send(const QString &remApp, const QString &remObjId,
		      const QString &remFun, const QString &data, 
		      bool fast)
{
  QByteArray ba;
  QDataStream ds(ba, IO_WriteOnly);
  ds << data;
  return send(remApp, remObjId, remFun, ba);
}

bool DCOPClient::process(const QString &fun, const QByteArray &data,
			 QByteArray &replyData)
{
  return false;
}

bool DCOPClient::isApplicationAttached( const QString& remApp)
{
  QByteArray data, replyData;
  QDataStream arg( data, IO_WriteOnly );
  arg << remApp;
  int result = false;
  if ( call( "DCOPServer", "", "isApplicationAttached", data, replyData ) ) {
    QDataStream reply( replyData, IO_ReadOnly );
    reply >> result;
  }
  return result;
}

QStringList DCOPClient::attachedApplications()
{
  QByteArray data, replyData;
  QStringList result;
  if ( call( "DCOPServer", "", "isApplicationAttached", data, replyData ) ) {
    QDataStream reply( replyData, IO_ReadOnly );
    reply >> result;
  }
  return result;
}

bool DCOPClient::receive(const QString &app, const QString &objId, 
			 const QString &fun, const QByteArray &data,
			 QByteArray &replyData)
{
  if (app != d->appId) {
    qDebug("WEIRD! we somehow received a DCOP message w/a different appId");
    return false;
  }

  if ( objId.isEmpty() ) {
    return process( fun, data, replyData );
  }
  if (!DCOPObject::hasObject(objId)) {
    qDebug("we received a message for an object we don't know about!");
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

bool DCOPClient::call(const QString &remApp, const QString &remObjId,
		      const QString &remFun, const QByteArray &data,
		      QByteArray &replyData, bool fast)
{
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
