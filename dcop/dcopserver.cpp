#include <stdlib.h>
#include "dcopserver.moc"
#include <dcopglobal.h>
#include <qdatastream.h>
#include <qstack.h>

static DCOPServer* the_server = 0;
class DCOPListener : public QSocketNotifier
{
 public:
  DCOPListener( IceListenObj obj )
    : QSocketNotifier( IceGetListenConnectionNumber( obj ), 
		       QSocketNotifier::Read, 0, 0)
    {
      listenObj = obj;
    }

  IceListenObj listenObj;
};

class DCOPConnection : public QSocketNotifier
{
 public:
  DCOPConnection( IceConn conn )
    : QSocketNotifier( IceConnectionNumber( conn ), 
		       QSocketNotifier::Read, 0, 0 )
    {
      ice_conn = conn;
      status = IceConnectPending;
    }

  QString appId;
  IceConn ice_conn;
  IceConnectStatus status;
  QStack< _IceConn> waitingForReply;
};


IceAuthDataEntry*	authDataEntries;
static IceListenObj *listenObjs = 0;
int numTransports = 0;

#define MAGIC_COOKIE_LEN 16



static IceIOErrorHandler prev_handler;

void
MyIoErrorHandler ( IceConn ice_conn )

{
    if (prev_handler)
	(*prev_handler) (ice_conn);
}    

void
InstallIOErrorHandler ()

{
    IceIOErrorHandler default_handler;

    prev_handler = IceSetIOErrorHandler (NULL);
    default_handler = IceSetIOErrorHandler (MyIoErrorHandler);
    if (prev_handler == default_handler)
	prev_handler = NULL;
}


void DCOPWatchProc ( IceConn ice_conn, IcePointer client_data, Bool opening, IcePointer* watch_data)
{
  DCOPServer* ds = ( DCOPServer*) client_data;

  if (opening) {
    *watch_data = (IcePointer) ds->watchConnection( ice_conn );
  }
  else  {
    ds->removeConnection( (void*) *watch_data );
  }
}

void DCOPProcessMessage( IceConn iceConn, IcePointer clientData, int opcode, unsigned long length, Bool swap)
{
  the_server->processMessage( iceConn, opcode, length, swap );
}

void DCOPServer::processMessage( IceConn iceConn, int opcode, unsigned long length, Bool swap)
{
  switch( opcode ) {
  case DCOPRegisterClient:
    {
      DCOPMsg *pMsg = 0;
      IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
      QByteArray ba( length );
      IceReadData(iceConn, length, ba.data() );
      QDataStream ds( ba, IO_ReadOnly );
      QString app;
      ds >> app;
      DCOPConnection* con = clients.find( iceConn );
      if ( con ) {
	qDebug("registered app %s", app.latin1() );
	DCOPConnection* con = clients[iceConn];
	con->appId = app;
	appIds.insert( app, con );
      }
    }
    break;
  case DCOPSend:
    {
      DCOPMsg *pMsg = 0;
      IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
      QByteArray ba( length );
      IceReadData(iceConn, length, ba.data() );
      QDataStream ds( ba, IO_ReadOnly );
      QString app;
      ds >> app;
      DCOPConnection* target = appIds.find( app );
      if ( target ) {
	IceGetHeader( target->ice_conn, majorOpcode, DCOPRegisterClient, 
		     sizeof(DCOPMsg), DCOPMsg, pMsg );
	int datalen = ba.size();
	pMsg->length += datalen;
	IceWriteData( target->ice_conn, datalen, (char *) ba.data());
	IceFlush( target->ice_conn );
      } else if ( app == "DCOPServer" ) {
	QString obj, fun;
	QByteArray data;
	ds >> obj >> fun >> data;
	QByteArray replyData;
	receive( app, obj, fun, data, replyData );
      }
    }
    break;
  case DCOPCall:
    {
      DCOPMsg *pMsg = 0;
      IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
      QByteArray ba( length );
      IceReadData(iceConn, length, ba.data() );
      QDataStream ds( ba, IO_ReadOnly );
      QString app;
      ds >> app;
      DCOPConnection* target = appIds.find( app );
      int datalen = ba.size();
      if ( target ) {
	target->waitingForReply.push( iceConn );
	IceGetHeader( target->ice_conn, majorOpcode, DCOPCall, 
		     sizeof(DCOPMsg), DCOPMsg, pMsg );
	pMsg->length += datalen;
	IceWriteData( target->ice_conn, datalen, (char *) ba.data());
	IceFlush( target->ice_conn );
      } else { 
	QByteArray replyData;
	bool b = FALSE;
	if ( app = "DCOPserver" ) {
	  QString obj, fun;
	  QByteArray data;
	  ds >> obj >> fun >> data;
	  b = receive( app, obj, fun, data, replyData );
	}
	int datalen = replyData.size();
	IceGetHeader( iceConn, majorOpcode, b? DCOPReply : DCOPReplyFailed, 
		      sizeof(DCOPMsg), DCOPMsg, pMsg );
	pMsg->length += datalen;
	IceWriteData( iceConn, datalen, (char *) replyData.data());
	IceFlush( iceConn );
      }
    }
    break;
  case DCOPReply:
  case DCOPReplyFailed:
    {
      DCOPMsg *pMsg = 0;
      IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
      QByteArray ba( length );
      IceReadData(iceConn, length, ba.data() );
      DCOPConnection* con = clients.find( iceConn );
      if ( con ) {
	DCOPConnection* conreply = clients.find( con->waitingForReply.pop() );
	if ( conreply ) {
	  IceGetHeader( conreply->ice_conn, majorOpcode, opcode, 
			sizeof(DCOPMsg), DCOPMsg, pMsg );
	  int datalen = ba.size();
	  pMsg->length += datalen;
	  IceWriteData( conreply->ice_conn, datalen, (char *) ba.data());
	  IceFlush( conreply->ice_conn );
	}
      }
    }
    break;
  default:
    qDebug("DCOPServer::processMessage unknown message");
  }
}


IcePaVersionRec DCOPVersions[] = {
  { DCOPVersionMajor, DCOPVersionMinor,  DCOPProcessMessage }
};



static Bool HostBasedAuthProc ( char* hostname)
{

  qDebug("HostBasedAuthProc for %s", hostname);
  return TRUE; // do no security at all (for now)
  return (0);	      /* For now, we don't support host based authentication */
}

typedef struct DCOPServerConnStruct *DCOPServerConn;

struct DCOPServerConnStruct
{
    /*
     * We use ICE to esablish a connection with the client.
     */

    IceConn		iceConn;


    /*
     * Major and minor versions of the XSMP.
     */

    int			proto_major_version;
    int			proto_minor_version;


    QCString clientId;


};


static Status DCOPServerProtocolSetupProc ( IceConn iceConn,
    int majorVersion, int minorVersion, char* vendor, char* release,
    IcePointer *clientDataRet, char **failureReasonRet)
{
    DCOPServerConn serverConn;
//     unsigned long 	mask;
//     Status		status;

    /*
     * vendor/release are undefined for ProtocolSetup in DCOP
     */

    if (vendor)
	free (vendor);
    if (release)
	free (release);


    /*
     * Allocate new SmsConn.
     */

    serverConn = new DCOPServerConnStruct;

    serverConn->iceConn = iceConn;
    serverConn->proto_major_version = majorVersion;
    serverConn->proto_minor_version = minorVersion;
    //serverConn->clientId already initialized

    *clientDataRet = (IcePointer) serverConn;


    return 1;
}

static void
CloseListeners ()

{
    IceFreeListenObjs (numTransports, listenObjs);
}


DCOPServer::DCOPServer()
  : QObject(0,0), appIds(200), clients(200)
{
  the_server = this;
  if (( majorOpcode = IceRegisterForProtocolReply ("DCOP",
					      DCOPVendorString, DCOPReleaseString, 
					      1, DCOPVersions,
					      1, DCOPAuthNames, DCOPServerAuthProcs, HostBasedAuthProc,
						 DCOPServerProtocolSetupProc,
						 NULL,	/* IceProtocolActivateProc - we don't care about
							   when the Protocol Reply is sent, because the
							   session manager can not immediately send a
							   message - it must wait for RegisterClient. */
						 NULL	/* IceIOErrorProc */
						 )) < 0)
    {
      qDebug("Could not register DCOP protocol with ICE");
    }
  char errormsg[256];

//   if (!IceListenForConnections (&numTransports, &listenObjs,
// 				256, errormsg))
  if (!IceListenForWellKnownConnections ("5000", &numTransports, &listenObjs,
				256, errormsg))
    {
      fprintf (stderr, "%s\n", errormsg);
      exit (1);
    }

    atexit(CloseListeners);

 authDataEntries = new IceAuthDataEntry[ numTransports * 2  ];


 for ( int i = 0; i < numTransports * 2; i += 2)
    {
	authDataEntries[i].network_id =
	    IceGetListenConnectionString (listenObjs[i/2]);
	authDataEntries[i].protocol_name = "ICE";
	authDataEntries[i].auth_name = "MIT-MAGIC-COOKIE-1";

	authDataEntries[i].auth_data =
	    IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
	authDataEntries[i].auth_data_length = MAGIC_COOKIE_LEN;

	authDataEntries[i+1].network_id =
	    IceGetListenConnectionString (listenObjs[i/2]);
	authDataEntries[i+1].protocol_name = "DCOP";
	authDataEntries[i+1].auth_name = "MIT-MAGIC-COOKIE-1";

	authDataEntries[i+1].auth_data = 
	    IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
	authDataEntries[i+1].auth_data_length = MAGIC_COOKIE_LEN;

//#### 	write_iceauth (addfp, removefp, authDataEntries[i]);
// 	write_iceauth (addfp, removefp, authDataEntries[i+1]);

 	IceSetPaAuthData (2, &authDataEntries[i]);

	IceSetHostBasedAuthProc (listenObjs[i/2], HostBasedAuthProc);
    }


 IceAddConnectionWatch (DCOPWatchProc, (IcePointer) this);



 listener.setAutoDelete( TRUE );
 DCOPListener* con;
    for ( int i = 0; i < numTransports; i++) {
      con = new DCOPListener( listenObjs[i] );
      listener.append( con );
      connect( con, SIGNAL( activated(int) ), this, SLOT( newClient(int) ) );
    }

    char * networkIds = IceComposeNetworkIdList (numTransports, listenObjs);
    qDebug("networkids: %s", networkIds );
    
    free ( networkIds );
}

DCOPServer::~DCOPServer()
{
}


void DCOPServer::processData( int socket )
{
  DCOPConnection* con = (DCOPConnection*)sender();
  IceProcessMessagesStatus s =  IceProcessMessages( con->ice_conn, 0, 0 );


  if (s == IceProcessMessagesIOError) {
    IceCloseConnection( con->ice_conn );
    return;
  }
  con->status = IceConnectionStatus(  con->ice_conn );
}

void DCOPServer::newClient( int socket )
{
  IceAcceptStatus status;
  IceConn ice_conn = IceAcceptConnection( ((DCOPListener*)sender())->listenObj, &status);
  IceSetShutdownNegotiation( ice_conn, False );


  IceConnectStatus cstatus;
  while ((cstatus = IceConnectionStatus (ice_conn))==IceConnectPending) {
    qApp->processOneEvent();
  }
  if (cstatus == IceConnectAccepted) {
    char* connstr = IceConnectionString (ice_conn);
    free (connstr);
  } else {
    if (cstatus == IceConnectIOError)
      qDebug ("IO error opening ICE Connection!\n");
    else
      qDebug ("ICE Connection rejected!\n");
    IceCloseConnection (ice_conn);
  }
}

void* DCOPServer::watchConnection( IceConn ice_conn )
{
  DCOPConnection* con = new DCOPConnection( ice_conn );
  connect( con, SIGNAL( activated(int) ), this, SLOT( processData(int) ) );
  
  clients.insert(ice_conn, con );

  return (void*) con;
}

void DCOPServer::removeConnection( void* data )
{
  DCOPConnection* con = (DCOPConnection*)data;
  qDebug("remove appId %s", con->appId.latin1() );
  clients.remove(con->ice_conn );
  appIds.remove( con->appId );
  delete con;
}

bool DCOPServer::receive(const QString &app, const QString &obj, 
			 const QString &fun, const QByteArray& data,
			 QByteArray &replyData)
{
  if ( fun == "attachedApplications" ) {
    QDataStream reply( replyData, IO_WriteOnly );
    QStringList applications;
    QDictIterator<DCOPConnection> it( appIds );
    while ( it.current() ) {
      applications << it.currentKey();
      ++it;
    }
    reply << applications;
    return TRUE;
  } else if ( fun == "isApplicationAttached" ) {
    QDataStream args( data, IO_ReadOnly );
    if (!args.atEnd()) {
      QString s;
      args >> s;
      QDataStream reply( replyData, IO_WriteOnly );
      bool b = ( appIds.find( s ) != 0 );
      reply << b;
      return TRUE;
    }
  }

  return FALSE;
}

int main( int argc, char* argv[] )
{
  QApplication a( argc, argv );
  InstallIOErrorHandler();
  DCOPServer server;
  
  return a.exec();
}
