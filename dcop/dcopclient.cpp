#include <X11/Xmd.h>

#define INT32 QINT32

#include <qdatastream.h>

#include <dcopobject.h>

#include <dcopclient.moc>
#include <dcopclient.h>

QString DCOPClient::serverAddr = QString::null;

class DCOPClientPrivate : public DCOPClient
{
public:
  DCOPClientPrivate(const QString &appId) : DCOPClient(appId) {}
  void processIceMessage(IceConn iceConn, int opcode,
			 unsigned long length, Bool swap,
			 IceReplyWaitInfo *replyWait, 
			 Bool *replyWaitRet)
    { DCOPClient::processIceMessage(iceConn, opcode,
				    length, swap, replyWait,
				    replyWaitRet);
    }
};


void DCOPProcessMessage(IceConn iceConn, IcePointer clientObject,
			int opcode, unsigned long length, Bool swap,
			IceReplyWaitInfo *replyWait,
			Bool *replyWaitRet)
{
  /* simply forward this message to the object to which it belongs. */
  ( (DCOPClientPrivate *) clientObject )->processIceMessage(iceConn, opcode, length,
							    swap, replyWait, 
							    replyWaitRet);
}


DCOPClient::DCOPClient(const QString &appId)
{
  iceConn = 0L;
  majorOpcode = 0;
  this->appId = appId;
  notifier = 0L;
}

DCOPClient::~DCOPClient()
{
  if (IceConnectionStatus(iceConn) == IceConnectAccepted) {
    detach();
  }

  delete notifier;
}

bool DCOPClient::attach()
{
  char errBuf[1024];

  if ((majorOpcode = IceRegisterForProtocolSetup("DCOP", DCOPVendorString, 
					    DCOPReleaseString, 1, DCOPVersions,
					    DCOPAuthCount, DCOPAuthNames,
					    DCOPClientAuthProcs, 0L)) < 0) {
    qDebug("Could not register DCOP protocol with ICE");
    return false;
  }
  
  // first, check if serverAddr was ever set.
  if (serverAddr.isEmpty())
    // here, we will check some environment variable and find the
    // DCOP server.  Now, we hardcode it.  CHANGE ME
    serverAddr = "local/faui06f:/tmp/.ICE-unix/5000";
    //    serverAddr = "tcp/faui06e:5000";

  if ((iceConn = IceOpenConnection((char *) serverAddr.ascii(), 
				   0, 0, majorOpcode, sizeof(errBuf),
				   errBuf)) == 0L) {
    qDebug("Could not open connection to DCOP server, msg %s",errBuf);
    return false;
  }

  IceSetShutdownNegotiation(iceConn, False);

  int setupstat;
  setupstat = IceProtocolSetup(iceConn, majorOpcode,  (IcePointer) this,
			       False, /* must authenticate */
			       &majorVersion, &minorVersion, 
			       &vendor, &release, 1024, errBuf);

  if (setupstat == IceProtocolSetupFailure ||
      setupstat == IceProtocolSetupIOError) {
    IceCloseConnection(iceConn);
    qDebug("Error setting up DCOP protocol, msg is %s", errBuf);
    return false;
  } else if (setupstat == IceProtocolAlreadyActive) {
    /* should not happen because 3rd arg to IceOpenConnection was 0. */
    qDebug("internal error in IceOpenConnection");
    return false;
  }

  DCOPMsg *pMsg;
  
  IceGetHeader(iceConn, majorOpcode, DCOPRegisterClient, 
	       sizeof(DCOPMsg), DCOPMsg, pMsg);

  QByteArray ba;
  QDataStream ds(ba, IO_WriteOnly);
  ds << appId;

  int datalen = ba.size();
  
  pMsg->length += datalen;
  IceWriteData(iceConn, datalen, (char *) ba.data());
  
  IceFlush(iceConn);

  // check if we have a qApp instantiated.  If we do,
  // we can create a QSocketNotifier and use it for receiving data.
  if (qApp) {
    if (IceConnectionStatus(iceConn) != IceConnectAccepted)
      return false;

    notifier = new QSocketNotifier(IceConnectionNumber(iceConn),
				   QSocketNotifier::Read, 0, 0);
    connect(notifier, SIGNAL(activated(int)),
	    SLOT(processSocketData(int)));
  }

  return true;
}


bool DCOPClient::detach()
{
  int status;

  IceProtocolShutdown(iceConn, majorOpcode);
  status = IceCloseConnection(iceConn);
  if (status != IceClosedNow)
    return false;
  else
    return true;
}

bool DCOPClient::send(const QString &remApp, const QString &remObjId, 
		      const QString &remFun, const QByteArray &data)
{
  DCOPMsg *pMsg;
  
  QByteArray ba;
  QDataStream ds(ba, IO_WriteOnly);
  ds << remApp << remObjId << remFun << data;
  
  IceGetHeader(iceConn, majorOpcode, DCOPSend, 
	       sizeof(DCOPMsg), DCOPMsg, pMsg);
  
  int datalen = ba.size();
  pMsg->length += datalen;

  IceWriteData(iceConn, datalen, (char *) ba.data());
  
  IceFlush(iceConn);

  if (IceConnectionStatus(iceConn) != IceConnectAccepted)
    return false;
  else
    return true;
}

bool DCOPClient::send(const QString &remApp, const QString &remObjId,
		      const QString &remFun, const QString &data)
{
  QByteArray d;
  QDataStream ds(d, IO_WriteOnly);
  ds << data;
  return send(remApp, remObjId, remFun, d);
}

bool DCOPClient::process(const QString &fun, const QByteArray &data,
			 QByteArray &replyData)
{
  return false;
}

bool DCOPClient::receive(const QString &app, const QString &objId, 
			 const QString &fun, const QByteArray &data,
			 QByteArray &replyData)
{
  if (app != appId) {
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
		      QByteArray &replyData)
{
  DCOPMsg *pMsg;
  
  QByteArray ba;
  QDataStream ds(ba, IO_WriteOnly);
  ds << remApp << remObjId << remFun << data;
  
  IceGetHeader(iceConn, majorOpcode, DCOPCall, 
	       sizeof(DCOPMsg), DCOPMsg, pMsg);
  
  int datalen = ba.size();
  pMsg->length += datalen;

  IceWriteData(iceConn, datalen, (char *) ba.data());
  
  IceFlush(iceConn);

  if (IceConnectionStatus(iceConn) != IceConnectAccepted)
    return false;

  IceReplyWaitInfo waitInfo;
  waitInfo.sequence_of_request = IceLastSentSequenceNumber(iceConn);
  waitInfo.major_opcode_of_request = majorOpcode;
  waitInfo.minor_opcode_of_request = DCOPCall;
  waitInfo.reply = (IcePointer) &replyData;
  
  Bool readyRet = False;
  IceProcessMessagesStatus s;

  do {
    s = IceProcessMessages(iceConn, &waitInfo,
			   &readyRet);
  } while (!readyRet);

  if (s == IceProcessMessagesIOError) {
    IceCloseConnection(iceConn);
    qDebug("received an error processing data from DCOP server!");
    return false;
  }

  return true;
}

void DCOPClient::processSocketData(int socknum)
{
  IceProcessMessagesStatus s =  IceProcessMessages( iceConn, 0, 0 );

  if (s == IceProcessMessagesIOError) {
    IceCloseConnection( iceConn );
    qDebug("received an error processing data from the DCOP server!");
    return;
  }
}


void DCOPClient::processIceMessage(IceConn iceConn, int opcode,
				   unsigned long length, Bool swap,
				   IceReplyWaitInfo *replyWait, 
				   Bool *replyWaitRet)
{
  DCOPMsg *pMsg = 0;

  if (opcode == DCOPReply) {
    if ( replyWait ) {
      QByteArray* b = (QByteArray*) replyWait->reply;
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
    receive( app, objId, fun, data, replyData );

    if (opcode != DCOPCall)
      return;

    // we are calling, so we need to set up reply data
    IceGetHeader( iceConn, 1, DCOPReply, 
		  sizeof(DCOPMsg), DCOPMsg, pMsg );
    int datalen = replyData.size();
    pMsg->length += datalen;
    IceWriteData( iceConn, datalen, (char *) replyData.data());
    IceFlush( iceConn );
  }
}
