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
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <sys/types.h>
#include <sys/file.h>

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif
#include <qtextstream.h>
#include <qfile.h>
#include <qsocketnotifier.h>

#include <dcopserver.h>
#include <dcopglobal.h>
#include <dcopclient.h>
#include <dcopobject.h>
#include <dcopref.h>

#include <X11/Xmd.h>
extern "C" {
#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICEutil.h>
#include <X11/ICE/ICEmsg.h>
#include <X11/ICE/ICEproto.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
}

template class QList<DCOPObjectProxy>;
template class QList<DCOPClientTransaction>;
template class QList<_IceConn>;

class DCOPClientPrivate
{
public:
    DCOPClient *parent;
    QCString appId;
    IceConn iceConn;
    int majorOpcode; // major opcode negotiated w/server and used to tag all comms.

    int majorVersion, minorVersion; // protocol versions negotiated w/server
    char *vendor, *release; // information from server

    static const char* serverAddr; // location of server in ICE-friendly format.
    QSocketNotifier *notifier;
    bool notifier_enabled;
    bool non_blocking_call_lock;
    bool registered;

    QCString senderId;

    QCString defaultObject;
    QList<DCOPClientTransaction> *transactionList;
    bool transaction;
    Q_INT32 transactionId;
    

    CARD32 time;
};

class DCOPClientTransaction
{
public:
    Q_INT32 id;
    QCString senderId;
};

struct ReplyStruct
{

    enum ReplyStatus { Pending, Ok, Rejected, Failed };
    ReplyStruct() {
	status = Pending;
	replyType = 0;
	replyData = 0;
	replyId = 0;
    }
    ReplyStatus status;
    QCString* replyType;
    QByteArray* replyData;
    Q_INT32 replyId;
};

const char* DCOPClientPrivate::serverAddr = 0;

// SM DUMMY
#include <X11/SM/SMlib.h>
static Bool HostBasedAuthProc ( char* /*hostname*/)
{
    return false; // no host based authentication
}
static Status NewClientProc ( SmsConn, SmPointer, unsigned long*, SmsCallbacks*, char** )
{
    return 0;
};

static void registerXSM()
{
    char 	errormsg[256];
    if (!SmsInitialize (const_cast<char *>("SAMPLE-SM"), const_cast<char *>("1.0"),
			NewClientProc, NULL,
			HostBasedAuthProc, 256, errormsg))
	{
	    qFatal("register xsm failed");
	}
}



/**
 * Callback for ICE.
 */
void DCOPProcessMessage(IceConn iceConn, IcePointer clientObject,
			int opcode, unsigned long length, Bool /*swap*/,
			IceReplyWaitInfo *replyWait,
			Bool *replyWaitRet)
{
    DCOPMsg *pMsg = 0;
    DCOPClientPrivate *d = static_cast<DCOPClientPrivate *>(clientObject);
    DCOPClient *c = d->parent;

    IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);

    if ( pMsg->time > d->time )
	d->time = pMsg->time;

    switch (opcode ) {
    case DCOPCallRejected:
	if ( replyWait ) {
	    static_cast<ReplyStruct*>(replyWait->reply)->status = ReplyStruct::Rejected;
	    *replyWaitRet = True;
	    return;
	} else {
	    qWarning("Very strange! got a DCOPCallRejected opcode, but we were not waiting for a reply!");
	    return;
	}
    case DCOPReplyFailed:
	if ( replyWait ) {
	    QByteArray tmp( length );
	    IceReadData(iceConn, length, tmp.data() );
	    static_cast<ReplyStruct*>(replyWait->reply)->status = ReplyStruct::Failed;
	    *replyWaitRet = True;
	    return;
	} else {
	    qWarning("Very strange! got a DCOPReplyFailed opcode, but we were not waiting for a reply!");
	    return;
	}
    case DCOPReply:
	if ( replyWait ) {
	    QByteArray tmp( length );
	    IceReadData(iceConn, length, tmp.data() );

	    QByteArray* b = static_cast<ReplyStruct*>(replyWait->reply)->replyData;
	    QCString* t =  static_cast<ReplyStruct*>(replyWait->reply)->replyType;
	    static_cast<ReplyStruct*>(replyWait->reply)->status = ReplyStruct::Ok;

	    // TODO: avoid this data copying
	    QDataStream tmpStream( tmp, IO_ReadOnly );
	    tmpStream >> *t >> *b;

	    *replyWaitRet = True;
	    return;
	} else {
	    qWarning("Very strange! got a DCOPReply opcode, but we were not waiting for a reply!");
	    return;
	}
    case DCOPReplyWait:
	if ( replyWait ) {
	    QByteArray tmp( length );
	    IceReadData(iceConn, length, tmp.data() );

	    Q_INT32 id;
	    QDataStream tmpStream( tmp, IO_ReadOnly );
	    tmpStream >> id;
	    static_cast<ReplyStruct*>(replyWait->reply)->replyId = id;
	    return;
	} else {
	    qWarning("Very strange! got a DCOPReplyWait opcode, but we were not waiting for a reply!");
	    return;
	}
    case DCOPReplyDelayed:
	if ( replyWait ) {
	    QByteArray tmp( length );
	    IceReadData(iceConn, length, tmp.data() );

	    QByteArray* b = static_cast<ReplyStruct*>(replyWait->reply)->replyData;
	    static_cast<ReplyStruct*>(replyWait->reply)->status = ReplyStruct::Ok;
	    QCString* t =  static_cast<ReplyStruct*>(replyWait->reply)->replyType;

	    QDataStream ds( tmp, IO_ReadOnly );
	    QCString calledApp, app;
	    Q_INT32 id;

	    ds >> calledApp >> app >> id >> *t >> *b;
	    if (id != static_cast<ReplyStruct*>(replyWait->reply)->replyId) {
		static_cast<ReplyStruct*>(replyWait->reply)->status = ReplyStruct::Failed;
		qWarning("Very strange! DCOPReplyDelayed got wrong sequence id!");
	    }

	    *replyWaitRet = True;
	} else {
	    qWarning("Very strange! got a DCOPReplyDelayed opcode, but we were not waiting for a reply!");
	}
	return;
    case DCOPCall:
    case DCOPFind:
    case DCOPSend:
	QByteArray tmp( length );
	IceReadData(iceConn, length, tmp.data() );
	QDataStream ds( tmp, IO_ReadOnly );
	QCString app, objId, fun;
	QByteArray data;
	ds >> d->senderId >> app >> objId >> fun >> data;
	
	QCString replyType;
	QByteArray replyData;
	bool b;
	
	{
	    // block any new requests via the socket notifier while we are
	    // processing a call. This makes it possible for clients to
	    // re-enter the main event loop for user interaction in
	    // process() without confusing the DCOP call stack.
	    bool old_notifier_enabled = d->notifier_enabled;
	    d->notifier_enabled = false;
            if (opcode == DCOPFind)
                b = c->find(app, objId, fun, data, replyType, replyData );
            else
                b = c->receive( app, objId, fun, data, replyType, replyData );
	    // set notifier back to previous state
	    d->notifier_enabled = old_notifier_enabled; 
	}
	
	if (opcode == DCOPSend)
	    return;

	QByteArray reply;
	QDataStream replyStream( reply, IO_WriteOnly );

	Q_INT32 id = c->transactionId();
	if (id) {
	    // Call delayed. Send back the transaction ID.
	    replyStream << id;

	    IceGetHeader( iceConn, d->majorOpcode, DCOPReplyWait,
			  sizeof(DCOPMsg), DCOPMsg, pMsg );
	    pMsg->time = d->time;
	    pMsg->length += reply.size();
	    IceSendData( iceConn, reply.size(), const_cast<char *>(reply.data()));
	    return;
	}

	if ( !b )	{
	    qWarning("DCOP failure in app %s:\n   object '%s' has no function '%s'", app.data(), objId.data(), fun.data() );
	    // Call failed. No data send back.

	    IceGetHeader( iceConn, d->majorOpcode, DCOPReplyFailed,
			  sizeof(DCOPMsg), DCOPMsg, pMsg );
	    pMsg->time = d->time;
	    pMsg->length += reply.size();
	    IceSendData( iceConn, reply.size(), const_cast<char *>(reply.data()));
	    return;
	}

	// Call successfull. Send back replyType and replyData.
	replyStream << replyType << replyData.size();

	// we are calling, so we need to set up reply data
	IceGetHeader( iceConn, d->majorOpcode, DCOPReply,
		      sizeof(DCOPMsg), DCOPMsg, pMsg );
	int datalen = reply.size() + replyData.size();
	pMsg->time = d->time;
	pMsg->length += datalen;
	// use IceSendData not IceWriteData to avoid a copy.  Output buffer
	// shouldn't need to be flushed.
	IceSendData( iceConn, reply.size(), const_cast<char *>(reply.data()));
	IceSendData( iceConn, replyData.size(), const_cast<char *>(replyData.data()));
	return;
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
    d->time = 0;
    d->appId = 0;
    d->notifier = 0L;
    d->notifier_enabled = true;
    d->non_blocking_call_lock = false;
    d->registered = false;
    d->transactionList = 0L;
    d->transactionId = 0;
}

DCOPClient::~DCOPClient()
{
    if (d->iceConn)
	if (IceConnectionStatus(d->iceConn) == IceConnectAccepted)
	    detach();

    delete d->notifier;
    delete d->transactionList;
    delete d;
}

void DCOPClient::setServerAddress(const QCString &addr)
{
    delete DCOPClientPrivate::serverAddr;
    DCOPClientPrivate::serverAddr = qstrdup( addr.data() );
}

bool DCOPClient::attach()
{
    if (!attachInternal( true ))
       if (!attachInternal( true ))
          return false; // Try two times!
    return true;
}

void DCOPClient::bindToApp()
{
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
}

bool DCOPClient::attachInternal( bool registerAsAnonymous )
{
    char errBuf[1024];

    if ( isAttached() )
	detach();

    extern int _IceLastMajorOpcode; // from libICE
    if (_IceLastMajorOpcode < 1 )
	registerXSM();

    if ((d->majorOpcode = IceRegisterForProtocolSetup(const_cast<char *>("DCOP"),
						      const_cast<char *>(DCOPVendorString),
						      const_cast<char *>(DCOPReleaseString),
						      1, DCOPVersions,
						      DCOPAuthCount,
						      const_cast<char **>(DCOPAuthNames),
						      DCOPClientAuthProcs, 0L)) < 0) {
	emit attachFailed("Communications could not be established.");
	return false;
    }

    // first, check if serverAddr was ever set.
    if (!d->serverAddr) {
	// here, we obtain the list of possible DCOP connections,
	// and attach to them.
	QString dcopSrv;
	dcopSrv = ::getenv("DCOPSERVER");
	if (dcopSrv.isNull()) {
	    QString fName = ::getenv("HOME");
	    fName += "/.DCOPserver";
	    QFile f(fName);
	    if (!f.open(IO_ReadOnly)) {
		emit attachFailed("Could not read network connection list.");
		return false;
	    }
	    QTextStream t(&f);
	    dcopSrv = t.readLine();
	}
	d->serverAddr = qstrdup( const_cast<char *>(dcopSrv.latin1()) );
    }

    if ((d->iceConn = IceOpenConnection(const_cast<char*>(d->serverAddr),
					static_cast<IcePointer>(this), False, d->majorOpcode,
					sizeof(errBuf), errBuf)) == 0L) {
	emit attachFailed(errBuf);
	d->iceConn = 0;
	return false;
    }

    IceSetShutdownNegotiation(d->iceConn, False);

    int setupstat;
    setupstat = IceProtocolSetup(d->iceConn, d->majorOpcode,
				 static_cast<IcePointer>(d),
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

    bindToApp();

    if ( registerAsAnonymous )
	registerAs( "anonymous", true );

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
	else
	    d->iceConn = 0L;
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


QCString DCOPClient::registerAs( const QCString &appId, bool addPID )
{
    QCString result;

    QCString _appId = appId;

    // Detach before reregistering.
    if ( isRegistered() ) {
	detach();
    }

    if ( !isAttached() ) {
        if (!attachInternal( false ))
            if (!attachInternal( false ))
                return result; // Try two times
    }

    if (addPID) {
	QCString pid;
	pid.sprintf("-%d", getpid());
	_appId = _appId + pid;
    }

    // register the application identifier with the server
    QCString replyType;
    QByteArray data, replyData;
    QDataStream arg( data, IO_WriteOnly );
    arg << _appId;
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
    while ( true ) {
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
    pMsg->time = d->time;
    pMsg->length += datalen;

    IceSendData( d->iceConn, ba.size(), const_cast<char *>(ba.data()) );
    IceSendData( d->iceConn, data.size(), const_cast<char *>(data.data()) );

    //IceFlush(d->iceConn);

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

bool DCOPClient::findObject(const QCString &remApp, const QCString &remObj,
                            const QCString &remFun, const QByteArray &data,
                            QCString &foundApp, QCString &foundObj,
                            bool useEventLoop, bool fast)
{
    QCStringList appList;
    QCString app = remApp;
    if (app.isEmpty())
       app = "*";

    foundApp = 0;
    foundObj = 0;

    if (app[app.length()-1] == '*')
    {
       // Find all apps that match 'app'. 
       // NOTE: It would be more efficient to do the filtering in 
       // the dcopserver itself.
       int len = app.length()-1;
       QCStringList apps=registeredApplications();
       for( QCStringList::ConstIterator it = apps.begin();
            it != apps.end();
            ++it)
       {
          if ( strncmp( (*it).data(), app.data(), len) == 0)
             appList.append(*it);
       }
    }
    else
    {
       appList.append(app);
    }

    for( QCStringList::ConstIterator it = appList.begin();
         it != appList.end();
         ++it)
    {
        QCString replyType;
        QByteArray replyData;
        if (callInternal((*it), remObj, remFun, data, 
                     replyType, replyData, useEventLoop, fast, DCOPFind))
        {
           if (replyType == "DCOPRef")
           {
              DCOPRef ref;
              QDataStream reply( replyData, IO_ReadOnly );
              reply >> ref;
              
              if (ref.app() == (*it)) // Consistency check
              {
                 // replyType contains objId.
                 foundApp = ref.app();
                 foundObj = ref.object();           
                 return true; 
              }
           }
        }
    }
    return false;       
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

void DCOPClient::setNotifications(bool enabled)
{
    QByteArray data;
    QDataStream ds(data, IO_WriteOnly);
    ds << static_cast<Q_INT8>(enabled);

    QCString replyType;
    QByteArray reply;
    if (!call("DCOPServer", "", "setNotifications( bool )", data, replyType, reply))
	qWarning("I couldn't enable notifications at the dcopserver!");
}


bool DCOPClient::receive(const QCString &app, const QCString &objId,
			 const QCString &fun, const QByteArray &data,
			 QCString& replyType, QByteArray &replyData)
{
    if ( !app.isEmpty() && app != d->appId && app[app.length()-1] != '*') {
	qWarning("WEIRD! we somehow received a DCOP message w/a different appId");
	return false;
    }

    if ( objId.isEmpty() ) {
	if ( fun == "applicationRegistered(QCString)" ) {
	    QDataStream ds( data, IO_ReadOnly );
	    QCString r;
	    ds >> r;
	    emit applicationRegistered( r );
	    return true;
	} else if ( fun == "applicationRemoved(QCString)" ) {
	    QDataStream ds( data, IO_ReadOnly );
	    QCString r;
	    ds >> r;
	    emit applicationRemoved( r );
	    return true;
	}
	
	if ( process( fun, data, replyType, replyData ) )
	    return true;

	// fall through and send to defaultObject if available
	if ( !d->defaultObject.isEmpty() && DCOPObject::hasObject( d->defaultObject ) ) {
	    if (DCOPObject::find( d->defaultObject )->process(fun, data, replyType, replyData))
		return true;
	}

	// fall through and send to object proxies
    }
    d->transaction = false; // Assume no transaction.

    if (objId[objId.length()-1] == '*') {
	// handle a multicast to several objects.
	// doesn't handle proxies currently.  should it?
	QList<DCOPObject> matchList =
	    DCOPObject::match(objId.left(objId.length()-1));
	for (DCOPObject *objPtr = matchList.first();
	     objPtr != 0L; objPtr = matchList.next()) {
	    if (!objPtr->process(fun, data, replyType, replyData))
		return false;
	}
	return true;
    } else if (!DCOPObject::hasObject(objId)) {
	if ( DCOPObjectProxy::proxies ) {
	    for ( QListIterator<DCOPObjectProxy> it( *DCOPObjectProxy::proxies ); it.current();  ++it ) {
		if ( it.current()->process( objId, fun, data, replyType, replyData ) )
		    return true;
	    }
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

// Check if the function result is a bool with the value "true" 
// If so set the function result to DCOPRef pointing to (app,objId) and
// return true. Return false otherwise.
static bool findResultOk(QCString &replyType, QByteArray &replyData)
{
    Q_INT8 success; // Tsk.. why is there no operator>>(bool)?
    if (replyType != "bool") return false;

    QDataStream reply( replyData, IO_ReadOnly );
    reply >> success;

    if (!success) return false;
    return true;
}

// set the function result to DCOPRef pointing to (app,objId) and
// return true. 
static bool findSuccess(const QCString &app, const QCString objId, QCString &replyType, QByteArray &replyData)
{
    DCOPRef ref(app, objId);
    replyType = "DCOPRef";

    replyData = QByteArray();
    QDataStream final_reply( replyData, IO_WriteOnly );
    final_reply << ref;
    return true;
}
    

bool DCOPClient::find(const QCString &app, const QCString &objId,
		      const QCString &fun, const QByteArray &data,
                      QCString& replyType, QByteArray &replyData)
{
    if ( !app.isEmpty() && app != d->appId && app[app.length()-1] != '*') {
	qWarning("WEIRD! we somehow received a DCOP message w/a different appId");
	return false;
    }

    if (objId.isEmpty() || objId[objId.length()-1] != '*') 
    {
        if (fun.isEmpty())
            return findSuccess(app, objId, replyType, replyData);
        // Message to application or single object...
        if (receive(app, objId, fun, data, replyType, replyData))
        {
            if (findResultOk(replyType, replyData))
                return findSuccess(app, objId, replyType, replyData);
        }
        return false;
    }
    else {
	// handle a multicast to several objects.
	// doesn't handle proxies currently.  should it?
	QList<DCOPObject> matchList =
	    DCOPObject::match(objId.left(objId.length()-1));
	for (DCOPObject *objPtr = matchList.first();
	     objPtr != 0L; objPtr = matchList.next()) 
        {
            replyType = 0;
            replyData = QByteArray();
            if (fun.isEmpty())
                return findSuccess(app, objPtr->objId(), replyType, replyData);
	    if (objPtr->process(fun, data, replyType, replyData))
		if (findResultOk(replyType, replyData))
                    return findSuccess(app, objPtr->objId(), replyType, replyData);
	}
	return false;
    } 
    return false;
}


bool DCOPClient::call(const QCString &remApp, const QCString &remObjId,
		      const QCString &remFun, const QByteArray &data,
		      QCString& replyType, QByteArray &replyData, 
                      bool useEventLoop, bool fast)
{
    return callInternal(remApp, remObjId, remFun, data, 
                         replyType, replyData, useEventLoop, fast, DCOPCall);
}

bool DCOPClient::callInternal(const QCString &remApp, const QCString &remObjId,
		      const QCString &remFun, const QByteArray &data,
		      QCString& replyType, QByteArray &replyData, 
                      bool useEventLoop, bool, int minor_opcode)
{
    if ( !isAttached() )
	return false;

    bool success = FALSE;

    while ( 1 ) {
	DCOPMsg *pMsg;

	QByteArray ba;
	QDataStream ds(ba, IO_WriteOnly);
	ds << d->appId << remApp << remObjId << normalizeFunctionSignature(remFun) << data.size();

	IceGetHeader(d->iceConn, d->majorOpcode, minor_opcode,
		     sizeof(DCOPMsg), DCOPMsg, pMsg);

	pMsg->time = d->time;
	int datalen = ba.size() + data.size();
	pMsg->length += datalen;

	IceSendData(d->iceConn, ba.size(), const_cast<char *>(ba.data()));
	IceSendData(d->iceConn, data.size(), const_cast<char *>(data.data()));


	if (IceConnectionStatus(d->iceConn) != IceConnectAccepted)
	    return false;

	IceFlush (d->iceConn);

	IceReplyWaitInfo waitInfo;
	waitInfo.sequence_of_request = IceLastSentSequenceNumber(d->iceConn);
	waitInfo.major_opcode_of_request = d->majorOpcode;
	waitInfo.minor_opcode_of_request = minor_opcode;
	ReplyStruct tmp;
	tmp.replyType = &replyType;
	tmp.replyData = &replyData;
	waitInfo.reply = static_cast<IcePointer>(&tmp);

	Bool readyRet = False;
	IceProcessMessagesStatus s;

	do {
	    
	    if ( useEventLoop && d->notifier ) { // we have a socket notifier and a qApp
		
		int msecs = 100; // timeout for the GUI refresh
		fd_set fds;
		struct timeval tv;
		FD_ZERO( &fds );
		FD_SET( socket(), &fds );
		tv.tv_sec = msecs / 1000;
		tv.tv_usec = (msecs % 1000) * 1000;
		if ( select( socket() + 1, &fds, 0, 0, &tv ) <= 0 ) {
		    // nothing was available, we got a timeout. Reactivate
		    // the GUI in blocked state.
		    bool old_lock = d->non_blocking_call_lock;
		    if ( !old_lock ) {
			d->non_blocking_call_lock = true;
			emit blockUserInput( true );
		    }
		    qApp->enter_loop();
		    if ( !old_lock ) {
			d->non_blocking_call_lock = false;
			emit blockUserInput( false );
		    }
		}
	    }
	    
	    // something is available
	    s = IceProcessMessages(d->iceConn, &waitInfo,
				   &readyRet);
	    if (s == IceProcessMessagesIOError) {
		IceCloseConnection(d->iceConn);
		return false;
	    }
	} while (!readyRet);
	
	// if we were rejected by the server, we try again, otherwise we return
	if ( tmp.status == ReplyStruct::Rejected ) {
            // WABA: Looks like an attempt to keep your CPU usage at 100%
	    continue;
	}
	
	success = tmp.status == ReplyStruct::Ok;
	break;
    }

    return success;
}

void DCOPClient::processSocketData(int)
{
    
    if ( d->non_blocking_call_lock ) {
	qApp->exit_loop();
	return;
    }
    
    if ( !d->notifier_enabled )
	return;

    IceProcessMessagesStatus s =  IceProcessMessages(d->iceConn, 0, 0);

    if (s == IceProcessMessagesIOError) {
	IceCloseConnection(d->iceConn);
	qWarning("received an error processing data from the DCOP server!");
	return;
    }
}

void DCOPClient::setDefaultObject( const QCString& objId )
{
    d->defaultObject = objId;
}


QCString DCOPClient::defaultObject() const
{
    return d->defaultObject;
}

DCOPClientTransaction *
DCOPClient::beginTransaction()
{
    if (!d->transactionList)
	d->transactionList = new QList<DCOPClientTransaction>;

    d->transaction = true;
    DCOPClientTransaction *trans = new DCOPClientTransaction();
    trans->senderId = d->senderId;
    if (!d->transactionId)  // transactionId should not be 0!
	d->transactionId++;
    trans->id = ++(d->transactionId);

    d->transactionList->append( trans );
    return trans;
}

Q_INT32
DCOPClient::transactionId()
{
    if (d->transaction)
	return d->transactionId;
    else
	return 0;
}

void
DCOPClient::endTransaction( DCOPClientTransaction *trans, QCString& replyType,
			    QByteArray &replyData)
{
    if ( !isAttached() )
	return;

    if ( !d->transactionList) {
	qWarning("Transaction unknown: No pending transactions!");
	return; // No pending transactions!
    }

    if ( !d->transactionList->removeRef( trans ) ) {
	qWarning("Transaction unknown: Not on list of pending transactions!");
	return; // Transaction
    }

    DCOPMsg *pMsg;

    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << d->appId << trans->senderId << trans->id << replyType << replyData;

    IceGetHeader(d->iceConn, d->majorOpcode, DCOPReplyDelayed,
		 sizeof(DCOPMsg), DCOPMsg, pMsg);

    pMsg->time = d->time;
    pMsg->length += ba.size();

    IceSendData( d->iceConn, ba.size(), const_cast<char *>(ba.data()) );

    delete trans;
}

#include <dcopclient.moc>

