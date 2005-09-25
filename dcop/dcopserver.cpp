/*****************************************************************

#include "dcopserver.h"

Copyright (c) 1999,2000 Preston Brown <pbrown@kde.org>
Copyright (c) 1999,2000 Matthias Ettrich <ettrich@kde.org>
Copyright (c) 1999,2001 Waldo Bastian <bastian@kde.org>

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

#include <config.h>

#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/resource.h>

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#define QT_CLEAN_NAMESPACE 1
#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qptrstack.h>
#include <qtimer.h>

#include "dcopserver.h"

#include <dcopsignals.h>
#include <dcopclient.h>
#include <dcopglobal.h>
#include "dcop-path.h"

#ifdef DCOP_LOG
#undef Unsorted
#include <qdir.h>
#include <string.h>
#endif

// #define DCOP_DEBUG

DCOPServer* the_server;

template class QDict<DCOPConnection>;
template class QPtrDict<DCOPConnection>;
template class QPtrList<DCOPListener>;

#define _DCOPIceSendBegin(x)	\
   int fd = IceConnectionNumber( x );		\
   long fd_fl = fcntl(fd, F_GETFL, 0);		\
   fcntl(fd, F_SETFL, fd_fl | O_NDELAY);
#define _DCOPIceSendEnd()	\
   fcntl(fd, F_SETFL, fd_fl);

static QCString findDcopserverShutdown()
{
#ifdef Q_OS_WIN32
	char szPath[512];
	char *pszFilePart;
	int ret;
	ret = SearchPathA(NULL,"dcopserver_shutdown","exe",sizeof(szPath)/sizeof(szPath[0]),szPath,&pszFilePart);
	if(ret != 0)
		return QCString(szPath);
#else
   QCString path = getenv("PATH");
   char *dir = strtok(path.data(), ":");
   while (dir)
   {
      QCString file = dir;
      file += "/dcopserver_shutdown";
      if (access(file.data(), X_OK) == 0)
         return file;
      dir = strtok(NULL, ":");
   }
   QCString file = DCOP_PATH;
   file += "/dcopserver_shutdown";
   if (access(file.data(), X_OK) == 0)
      return file;
#endif
   return QCString("dcopserver_shutdown");
}

static Bool HostBasedAuthProc ( char* /*hostname*/)
{
    return false; // no host based authentication
}

extern "C" {
extern IceWriteHandler _kde_IceWriteHandler;
extern IceIOErrorHandler _kde_IceIOErrorHandler;
void DCOPIceWriteChar(register IceConn iceConn, unsigned long nbytes, char *ptr);
}

static QCString readQCString(QDataStream &ds)
{
   QCString result;
   Q_UINT32 len;
   ds >> len;
   QIODevice *device = ds.device();
   int bytesLeft = device->size()-device->at();
   if ((bytesLeft < 0 ) || (len > (uint) bytesLeft))
   {
      qWarning("Corrupt data!\n");
      return result;
   }
   result.QByteArray::resize( (uint)len );
   if (len > 0)
      ds.readRawBytes( result.data(), (uint)len);
   return result;
}

static QByteArray readQByteArray(QDataStream &ds)
{
   QByteArray result;
   Q_UINT32 len;
   ds >> len;
   QIODevice *device = ds.device();
   int bytesLeft = device->size()-device->at();
   if ((bytesLeft < 0 ) || (len > (uint) bytesLeft))
   {
      qWarning("Corrupt data!\n");
      return result;
   }
   result.resize( (uint)len );
   if (len > 0)
      ds.readRawBytes( result.data(), (uint)len);
   return result;
}


extern "C" {
extern int _kde_IceTransWrite (void * ciptr, char *buf, int size);
}

static unsigned long writeIceData(IceConn iceConn, unsigned long nbytes, char *ptr)
{
    int fd = IceConnectionNumber(iceConn);
    unsigned long nleft = nbytes;
    while (nleft > 0)
    {
	int nwritten;

	if (iceConn->io_ok)
	{
		
//		nwritten = _kde_IceTransWrite (iceConn->trans_conn, ptr, (int) nleft);

		/*
		Use special write handling on windows platform. The write function from
		the runtime library (on MSVC) does not allow to write on sockets.
		*/
#ifdef Q_OS_WIN
		nwritten = send(fd, ptr, (int) nleft, 0);
#else
		nwritten = write(fd, ptr, (int) nleft);
#endif

	}
	else
	    return 0;

	if (nwritten <= 0)
	{
            if (errno == EINTR)
               continue;

            if (errno == EAGAIN)
               return nleft;

	    /*
	     * Fatal IO error.  First notify each protocol's IceIOErrorProc
	     * callback, then invoke the application IO error handler.
	     */

	    iceConn->io_ok = False;

	    if (iceConn->connection_status == IceConnectPending)
	    {
		/*
		 * Don't invoke IO error handler if we are in the
		 * middle of a connection setup.
		 */

		return 0;
	    }

	    if (iceConn->process_msg_info)
	    {
		int i;

		for (i = iceConn->his_min_opcode;
		     i <= iceConn->his_max_opcode; i++)
		{
		    _IceProcessMsgInfo *process;

		    process = &iceConn->process_msg_info[
			i - iceConn->his_min_opcode];

		    if (process->in_use)
		    {
			IceIOErrorProc IOErrProc = process->accept_flag ?
			    process->protocol->accept_client->io_error_proc :
			    process->protocol->orig_client->io_error_proc;

			if (IOErrProc)
			    (*IOErrProc) (iceConn);
		    }
		}
	    }

	    (*_kde_IceIOErrorHandler) (iceConn);
	    return 0;
	}

	nleft -= nwritten;
	ptr   += nwritten;
    }
    return 0;
}

void DCOPIceWriteChar(register IceConn iceConn, unsigned long nbytes, char *ptr)
{
    DCOPConnection* conn = the_server->findConn( iceConn );
#ifdef DCOP_DEBUG
qWarning("DCOPServer: DCOPIceWriteChar() Writing %d bytes to %d [%s]", nbytes, fd, conn ? conn->appId.data() : "<unknown>");
#endif

    if (conn)
    {
       if (conn->outputBlocked)
       {
          QByteArray _data(nbytes);
          memcpy(_data.data(), ptr, nbytes);
#ifdef DCOP_DEBUG
qWarning("DCOPServer: _IceWrite() outputBlocked. Queuing %d bytes.", _data.size());
#endif
          conn->outputBuffer.append(_data);
          return;
       }
       // assert(conn->outputBuffer.isEmpty());
    }

    unsigned long nleft = writeIceData(iceConn, nbytes, ptr);
    if ((nleft > 0) && conn)
    {
        QByteArray _data(nleft);
        memcpy(_data.data(), ptr, nleft);
        conn->waitForOutputReady(_data, 0);
        return;
    }
}

static void DCOPIceWrite(IceConn iceConn, const QByteArray &_data)
{
    DCOPConnection* conn = the_server->findConn( iceConn );
#ifdef DCOP_DEBUG
qWarning("DCOPServer: DCOPIceWrite() Writing %d bytes to %d [%s]", _data.size(), fd, conn ? conn->appId.data() : "<unknown>");
#endif
    if (conn)
    {
       if (conn->outputBlocked)
       {
#ifdef DCOP_DEBUG
qWarning("DCOPServer: DCOPIceWrite() outputBlocked. Queuing %d bytes.", _data.size());
#endif
          conn->outputBuffer.append(_data);
          return;
       }
       // assert(conn->outputBuffer.isEmpty());
    }

    unsigned long nleft = writeIceData(iceConn, _data.size(), _data.data());
    if ((nleft > 0) && conn)
    {
        conn->waitForOutputReady(_data, _data.size() - nleft);
        return;
    }
}

void DCOPConnection::waitForOutputReady(const QByteArray &_data, int start)
{
#ifdef DCOP_DEBUG
qWarning("DCOPServer: waitForOutputReady fd = %d datasize = %d start = %d", socket(), _data.size(), start);
#endif
   outputBlocked = true;
   outputBuffer.append(_data);
   outputBufferStart = start;
   if (!outputBufferNotifier)
   {
      outputBufferNotifier = new QSocketNotifier(socket(), Write);
      connect(outputBufferNotifier, SIGNAL(activated(int)),
              the_server, SLOT(slotOutputReady(int)));
   }
   outputBufferNotifier->setEnabled(true);
   return;
}

void DCOPServer::slotOutputReady(int socket)
{
#ifdef DCOP_DEBUG
qWarning("DCOPServer: slotOutputReady fd = %d", socket);
#endif
   // Find out connection.
   DCOPConnection *conn = fd_clients.find(socket);
   //assert(conn);
   //assert(conn->outputBlocked);
   //assert(conn->socket() == socket);
   // Forward
   conn->slotOutputReady();
}


void DCOPConnection::slotOutputReady()
{
   //assert(outputBlocked);
   //assert(!outputBuffer.isEmpty());

   QByteArray data = outputBuffer.first();

   int fd = socket();

   long fd_fl = fcntl(fd, F_GETFL, 0);
   fcntl(fd, F_SETFL, fd_fl | O_NDELAY);
   /*
	Use special write handling on windows platform. The write function from
	the runtime library (on MSVC) does not allow to write on sockets.
   */
   int nwritten;
#ifdef Q_OS_WIN
   nwritten = ::send(fd,data.data()+outputBufferStart,data.size()-outputBufferStart,0);
#else
   nwritten = write(fd, data.data()+outputBufferStart, data.size()-outputBufferStart);
#endif
   
   int e = errno;
   fcntl(fd, F_SETFL, fd_fl);

#ifdef DCOP_DEBUG
qWarning("DCOPServer: slotOutputReady() %d bytes written", nwritten);
#endif

   if (nwritten < 0)
   {
      if ((e == EINTR) || (e == EAGAIN))
         return;
      (*_kde_IceIOErrorHandler) (iceConn);
      return;
   }
   outputBufferStart += nwritten;

   if (outputBufferStart == data.size())
   {
      outputBufferStart = 0;
      outputBuffer.remove(outputBuffer.begin());
      if (outputBuffer.isEmpty())
      {
#ifdef DCOP_DEBUG
qWarning("DCOPServer: slotOutputRead() all data transmitted.");
#endif
         outputBlocked = false;
         outputBufferNotifier->setEnabled(false);
      }
#ifdef DCOP_DEBUG
else
{
qWarning("DCOPServer: slotOutputRead() more data to send.");
}
#endif
   }
}

static void DCOPIceSendData(register IceConn _iceConn,
                            const QByteArray &_data)
{
   if (_iceConn->outbufptr > _iceConn->outbuf)
   {
#ifdef DCOP_DEBUG
qWarning("DCOPServer: Flushing data, fd = %d", IceConnectionNumber(_iceConn));
#endif
      IceFlush( _iceConn );
   }
   DCOPIceWrite(_iceConn, _data);
}

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

DCOPConnection::DCOPConnection( IceConn conn )
	: QSocketNotifier( IceConnectionNumber( conn ),
			   QSocketNotifier::Read, 0, 0 )
{
    iceConn = conn;
    notifyRegister = 0;
    _signalConnectionList = 0;
    daemon = false;
    outputBlocked = false;
    outputBufferNotifier = 0;
    outputBufferStart = 0;
}

DCOPConnection::~DCOPConnection()
{
    delete _signalConnectionList;
    delete outputBufferNotifier;
}

DCOPSignalConnectionList *
DCOPConnection::signalConnectionList()
{
    if (!_signalConnectionList)
       _signalConnectionList = new DCOPSignalConnectionList;
    return _signalConnectionList;
}

static IceAuthDataEntry *authDataEntries;
static char *addAuthFile;

static IceListenObj *listenObjs;
static int numTransports;
static int ready[2];


/* for printing hex digits */
static void fprintfhex (FILE *fp, unsigned int len, char *cp)
{
    static char hexchars[] = "0123456789abcdef";

    for (; len > 0; len--, cp++) {
	unsigned char s = *cp;
	putc(hexchars[s >> 4], fp);
	putc(hexchars[s & 0x0f], fp);
    }
}

/*
 * We use temporary files which contain commands to add entries to
 * the .ICEauthority file.
 */
static void
write_iceauth (FILE *addfp, IceAuthDataEntry *entry)
{
    fprintf (addfp,
	     "add %s \"\" %s %s ",
	     entry->protocol_name,
	     entry->network_id,
	     entry->auth_name);
    fprintfhex (addfp, entry->auth_data_length, entry->auth_data);
    fprintf (addfp, "\n");
}

#ifndef HAVE_MKSTEMPS
#include <string.h>
#include <strings.h>

/* this is based on code taken from the GNU libc, distributed under the LGPL license */

/* Generate a unique temporary file name from TEMPLATE.

   TEMPLATE has the form:

   <path>/ccXXXXXX<suffix>

   SUFFIX_LEN tells us how long <suffix> is (it can be zero length).

   The last six characters of TEMPLATE before <suffix> must be "XXXXXX";
   they are replaced with a string that makes the filename unique.

   Returns a file descriptor open on the file for reading and writing.  */

int mkstemps (char* _template, int suffix_len)
{
  static const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  char *XXXXXX;
  int len;
  int count;
  int value;

  len = strlen (_template);

  if ((int) len < 6 + suffix_len || strncmp (&_template[len - 6 - suffix_len], "XXXXXX", 6))
      return -1;

  XXXXXX = &_template[len - 6 - suffix_len];

  value = rand();
  for (count = 0; count < 256; ++count)
  {
      int v = value;
      int fd;

      /* Fill in the random bits.  */
      XXXXXX[0] = letters[v % 62];
      v /= 62;
      XXXXXX[1] = letters[v % 62];
      v /= 62;
      XXXXXX[2] = letters[v % 62];
      v /= 62;
      XXXXXX[3] = letters[v % 62];
      v /= 62;
      XXXXXX[4] = letters[v % 62];
      v /= 62;
      XXXXXX[5] = letters[v % 62];

      fd = open (_template, O_RDWR|O_CREAT|O_EXCL, 0600);
      if (fd >= 0)
	/* The file does not exist.  */
	return fd;

      /* This is a random value.  It is only necessary that the next
	 TMP_MAX values generated by adding 7777 to VALUE are different
	 with (module 2^32).  */
      value += 7777;
    }
  /* We return the null string if we can't find a unique file name.  */
  _template[0] = '\0';
  return -1;
}

#endif

static char *unique_filename (const char *path, const char *prefix, int *pFd)
{
    char tempFile[PATH_MAX];
    char *ptr;

#ifdef Q_OS_WIN
    snprintf (tempFile, PATH_MAX, "%s\\%sXXXXXX", path, prefix);
#else
    snprintf (tempFile, PATH_MAX, "%s/%sXXXXXX", path, prefix);
#endif
    ptr = static_cast<char *>(malloc(strlen(tempFile) + 1));
    if (ptr != NULL)
	{
		int fd = mkstemps(tempFile, 0);
		if(fd >= 0)
		{
			*pFd = fd;
	    strcpy(ptr, tempFile);
		}
		else
		{
			free(ptr);
			ptr = NULL;
		}
	}
    return ptr;
}

#define MAGIC_COOKIE_LEN 16

Status
SetAuthentication (int count, IceListenObj *_listenObjs,
		   IceAuthDataEntry **_authDataEntries)
{
    FILE        *addfp = NULL;
    const char  *path;
    int         original_umask;
    int         i;
    QCString command;
    int         fd;

    original_umask = umask (0077);      /* disallow non-owner access */

#ifdef Q_OS_WIN
	char temppath[512];
	DWORD dw = GetTempPathA(sizeof(temppath),temppath);
	if(dw != 0)
	{
		temppath[dw - 1] = 0;
		path = temppath;
	}
	else
		path = ".";
#else
    path = getenv ("DCOP_SAVE_DIR");
    if (!path)
	path = "/tmp";
#endif
    if ((addAuthFile = unique_filename (path, "dcop", &fd)) == NULL)
	goto bad;

    if (!(addfp = fdopen(fd, "wb")))
	goto bad;

    if ((*_authDataEntries = static_cast<IceAuthDataEntry *>(malloc (count * 2 * sizeof (IceAuthDataEntry)))) == NULL)
	goto bad;

    for (i = 0; i < numTransports * 2; i += 2) {
	(*_authDataEntries)[i].network_id =
	    IceGetListenConnectionString (_listenObjs[i/2]);
	(*_authDataEntries)[i].protocol_name = const_cast<char *>("ICE");
	(*_authDataEntries)[i].auth_name = const_cast<char *>("MIT-MAGIC-COOKIE-1");

	(*_authDataEntries)[i].auth_data =
	    IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
	(*_authDataEntries)[i].auth_data_length = MAGIC_COOKIE_LEN;

	(*_authDataEntries)[i+1].network_id =
	    IceGetListenConnectionString (_listenObjs[i/2]);
	(*_authDataEntries)[i+1].protocol_name = const_cast<char *>("DCOP");
	(*_authDataEntries)[i+1].auth_name = const_cast<char *>("MIT-MAGIC-COOKIE-1");

	(*_authDataEntries)[i+1].auth_data =
	    IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
	(*_authDataEntries)[i+1].auth_data_length = MAGIC_COOKIE_LEN;

	write_iceauth (addfp, &(*_authDataEntries)[i]);
	write_iceauth (addfp, &(*_authDataEntries)[i+1]);

	IceSetPaAuthData (2, &(*_authDataEntries)[i]);

	IceSetHostBasedAuthProc (_listenObjs[i/2], HostBasedAuthProc);
    }

    fclose (addfp);

    umask (original_umask);

    command = DCOPClient::iceauthPath();

    if (command.isEmpty())
    {
       fprintf( stderr, "dcopserver: 'iceauth' not found in path, aborting.\n" );
       exit(1);
    }

    command += " source ";
    command += addAuthFile;
    system (command);

    unlink(addAuthFile);

    return (1);

 bad:

    if (addfp)
	fclose (addfp);

    if (addAuthFile) {
	unlink(addAuthFile);
	free(addAuthFile);
    }

    umask (original_umask);

    return (0);
}

/*
 * Free up authentication data.
 */
void
FreeAuthenticationData(int count, IceAuthDataEntry *_authDataEntries)
{
    /* Each transport has entries for ICE and XSMP */
    int i;

    for (i = 0; i < count * 2; i++) {
	free (_authDataEntries[i].network_id);
	free (_authDataEntries[i].auth_data);
    }

    free(_authDataEntries);
    free(addAuthFile);
}

void DCOPWatchProc ( IceConn iceConn, IcePointer client_data, Bool opening, IcePointer* watch_data)
{
    DCOPServer* ds = static_cast<DCOPServer*>(client_data);

    if (opening) {
	*watch_data = static_cast<IcePointer>(ds->watchConnection( iceConn ));
    }
    else  {
	ds->removeConnection( static_cast<void*>(*watch_data) );
    }
}

void DCOPProcessMessage( IceConn iceConn, IcePointer /*clientData*/,
			 int opcode, unsigned long length, Bool swap)
{
    the_server->processMessage( iceConn, opcode, length, swap );
}

void DCOPServer::processMessage( IceConn iceConn, int opcode,
				 unsigned long length, Bool /*swap*/)
{
    DCOPConnection* conn = clients.find( iceConn );
    if ( !conn ) {
	qWarning("DCOPServer::processMessage message from unknown connection. [opcode = %d]", opcode);
	return;
    }
    switch( opcode ) {
    case DCOPSend:
    case DCOPReplyDelayed:
	{
	    DCOPMsg *pMsg = 0;
	    IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
	    CARD32 key = pMsg->key;
	    QByteArray ba( length );
	    IceReadData(iceConn, length, ba.data() );
	    QDataStream ds( ba, IO_ReadOnly );
	    QCString fromApp = readQCString(ds);
            QCString toApp = readQCString(ds);

	    DCOPConnection* target = findApp( toApp );
	    int datalen = ba.size();
	    if ( opcode == DCOPReplyDelayed ) {
		if ( !target )
		    qWarning("DCOPServer::DCOPReplyDelayed for unknown connection.");
		else if ( !conn )
		    qWarning("DCOPServer::DCOPReplyDelayed from unknown connection.");
		else if (!conn->waitingForDelayedReply.removeRef( target->iceConn ))
		    qWarning("DCOPServer::DCOPReplyDelayed from/to does not match. (#2)");
                else if (!target->waitingOnReply.removeRef(iceConn))
                       qWarning("DCOPServer::DCOPReplyDelayed for client who wasn't waiting on one!");
	    }
	    if ( target ) {
#ifdef DCOP_DEBUG
if (opcode == DCOPSend)
{
   QCString obj = readQCString(ds);
   QCString fun = readQCString(ds);
   qWarning("Sending %d bytes from %s to %s. DCOPSend %s", length, fromApp.data(), toApp.data(), fun.data());
}
#endif
		IceGetHeader( target->iceConn, majorOpcode, opcode,
			      sizeof(DCOPMsg), DCOPMsg, pMsg );
		pMsg->key = key;
		pMsg->length += datalen;
		_DCOPIceSendBegin( target->iceConn );
		DCOPIceSendData(target->iceConn, ba);
                _DCOPIceSendEnd();
	    } else if ( toApp == "DCOPServer" ) {
		QCString obj = readQCString(ds);
		QCString fun = readQCString(ds);
		QByteArray data = readQByteArray(ds);

		QCString replyType;
		QByteArray replyData;
		if ( !receive( toApp, obj, fun, data, replyType, replyData, iceConn ) ) {
		    qWarning("%s failure: object '%s' has no function '%s'", toApp.data(), obj.data(), fun.data() );
		}
	    } else if ( toApp[toApp.length()-1] == '*') {
#ifdef DCOP_DEBUG
if (opcode == DCOPSend)
{
   QCString obj = readQCString(ds);
   QCString fun = readQCString(ds);
   qWarning("Sending %d bytes from %s to %s. DCOPSend %s", length, fromApp.data(), toApp.data(), fun.data());
}
#endif
		// handle a multicast.
		QAsciiDictIterator<DCOPConnection> aIt(appIds);
		int l = toApp.length()-1;
		for ( ; aIt.current(); ++aIt) {
		    DCOPConnection *client = aIt.current();
		    if (!l || (strncmp(client->appId.data(), toApp.data(), l) == 0))
			{
			    IceGetHeader(client->iceConn, majorOpcode, DCOPSend,
					 sizeof(DCOPMsg), DCOPMsg, pMsg);
			    pMsg->key = key;
			    pMsg->length += datalen;
			    _DCOPIceSendBegin( client->iceConn );
			    DCOPIceSendData(client->iceConn, ba);
                            _DCOPIceSendEnd();
			}
		}
	    }
	}
	break;
    case DCOPCall:
    case DCOPFind:
	{
	    DCOPMsg *pMsg = 0;
	    IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
	    CARD32 key = pMsg->key;
	    QByteArray ba( length );
	    IceReadData(iceConn, length, ba.data() );
	    QDataStream ds( ba, IO_ReadOnly );
	    QCString fromApp = readQCString(ds);
	    QCString toApp = readQCString(ds);
	    DCOPConnection* target = findApp( toApp );
	    int datalen = ba.size();

	    if ( target ) {
#ifdef DCOP_DEBUG
if (opcode == DCOPCall)
{
   QCString obj = readQCString(ds);
   QCString fun = readQCString(ds);
   qWarning("Sending %d bytes from %s to %s. DCOPCall %s", length, fromApp.data(), toApp.data(), fun.data());
}
#endif
		target->waitingForReply.append( iceConn );
                conn->waitingOnReply.append( target->iceConn);

		IceGetHeader( target->iceConn, majorOpcode, opcode,
			      sizeof(DCOPMsg), DCOPMsg, pMsg );
		pMsg->key = key;
		pMsg->length += datalen;
		_DCOPIceSendBegin( target->iceConn );
		DCOPIceSendData(target->iceConn, ba);
                _DCOPIceSendEnd();
	    } else {
		QCString replyType;
		QByteArray replyData;
		bool b = false;
		// DCOPServer itself does not do DCOPFind.
		if ( (opcode == DCOPCall) && (toApp == "DCOPServer") ) {
   		    QCString obj = readQCString(ds);
		    QCString fun = readQCString(ds);
		    QByteArray data = readQByteArray(ds);
		    b = receive( toApp, obj, fun, data, replyType, replyData, iceConn );
		    if ( !b )
			qWarning("%s failure: object '%s' has no function '%s'", toApp.data(), obj.data(), fun.data() );
		}

		if (b) {
		    QByteArray reply;
		    QDataStream replyStream( reply, IO_WriteOnly );
		    replyStream << toApp << fromApp << replyType << replyData.size();
		    int replylen = reply.size() + replyData.size();
		    IceGetHeader( iceConn, majorOpcode, DCOPReply,
				  sizeof(DCOPMsg), DCOPMsg, pMsg );
		    if ( key != 0 )
			pMsg->key = key;
		    else
			pMsg->key = serverKey++;
		    pMsg->length += replylen;
                    _DCOPIceSendBegin( iceConn );
		    DCOPIceSendData( iceConn, reply);
		    DCOPIceSendData( iceConn, replyData);
                    _DCOPIceSendEnd();
		} else {
		    QByteArray reply;
		    QDataStream replyStream( reply, IO_WriteOnly );
		    replyStream << toApp << fromApp;
		    IceGetHeader( iceConn, majorOpcode, DCOPReplyFailed,
				  sizeof(DCOPMsg), DCOPMsg, pMsg );
		    if ( key != 0 )
			pMsg->key = key;
		    else
			pMsg->key = serverKey++;
		    pMsg->length += reply.size();
                    _DCOPIceSendBegin( iceConn );
		    DCOPIceSendData( iceConn, reply );
                    _DCOPIceSendEnd();
		}
	    }
	}
	break;
    case DCOPReply:
    case DCOPReplyFailed:
    case DCOPReplyWait:
	{
	    DCOPMsg *pMsg = 0;
	    IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
	    CARD32 key = pMsg->key;
	    QByteArray ba( length );
	    IceReadData(iceConn, length, ba.data() );
	    QDataStream ds( ba, IO_ReadOnly );
            QCString fromApp = readQCString(ds);
            QCString toApp = readQCString(ds);

	    DCOPConnection* connreply = findApp( toApp );
	    int datalen = ba.size();

	    if ( !connreply )
		qWarning("DCOPServer::DCOPReply for unknown connection.");
	    else {
		conn->waitingForReply.removeRef( connreply->iceConn );
		if ( opcode == DCOPReplyWait )
                {
		    conn->waitingForDelayedReply.append( connreply->iceConn );
                }
                else
                { // DCOPReply or DCOPReplyFailed
                    if (!connreply->waitingOnReply.removeRef(iceConn))
                       qWarning("DCOPServer::DCOPReply for client who wasn't waiting on one!");
                }
		IceGetHeader( connreply->iceConn, majorOpcode, opcode,
			      sizeof(DCOPMsg), DCOPMsg, pMsg );
		pMsg->key = key;
		pMsg->length += datalen;
                _DCOPIceSendBegin( connreply->iceConn );
		DCOPIceSendData(connreply->iceConn, ba);
                _DCOPIceSendEnd();
	    }
	}
	break;
    default:
	qWarning("DCOPServer::processMessage unknown message");
    }
}

static const IcePaVersionRec DCOPServerVersions[] = {
    { DCOPVersionMajor, DCOPVersionMinor,  DCOPProcessMessage }
};

static const IcePoVersionRec DUMMYVersions[] = {
    { DCOPVersionMajor, DCOPVersionMinor, 0 }
};

static Status DCOPServerProtocolSetupProc ( IceConn /*iceConn*/,
					    int majorVersion, int minorVersion,
					    char* vendor, char* release,
					    IcePointer *clientDataRet,
					    char ** /*failureReasonRet*/)
{
    /*
     * vendor/release are undefined for ProtocolSetup in DCOP
     */

    if (vendor)
	free (vendor);
    if (release)
	free (release);

    *clientDataRet = 0;

    return (majorVersion == DCOPVersionMajor && minorVersion == DCOPVersionMinor);
}

#ifndef Q_OS_WIN
static int pipeOfDeath[2];

static void sighandler(int sig)
{
    if (sig == SIGHUP) {
	signal(SIGHUP, sighandler);
	return;
    }

    write(pipeOfDeath[1], "x", 1);
}
#endif

extern "C"
{
	extern int _kde_IceLastMajorOpcode; // from libICE
}

DCOPServer::DCOPServer(bool _suicide)
    : QObject(0,0), currentClientNumber(0), appIds(263), clients(263)
{
    serverKey = 42;

    suicide = _suicide;
    shutdown = false;

    dcopSignals = new DCOPSignals;

    if (_kde_IceLastMajorOpcode < 1 )
        IceRegisterForProtocolSetup(const_cast<char *>("DUMMY"),
				    const_cast<char *>("DUMMY"),
				    const_cast<char *>("DUMMY"),
				    1, const_cast<IcePoVersionRec *>(DUMMYVersions),
				    DCOPAuthCount, const_cast<char **>(DCOPAuthNames),
				    DCOPClientAuthProcs, 0);
    if (_kde_IceLastMajorOpcode < 1 )
	qWarning("DCOPServer Error: incorrect major opcode!");

    the_server = this;
    if (( majorOpcode = IceRegisterForProtocolReply (const_cast<char *>("DCOP"),
						     const_cast<char *>(DCOPVendorString),
						     const_cast<char *>(DCOPReleaseString),
						     1, const_cast<IcePaVersionRec *>(DCOPServerVersions),
						     1, const_cast<char **>(DCOPAuthNames),
						     DCOPServerAuthProcs,
						     HostBasedAuthProc,
						     DCOPServerProtocolSetupProc,
						     NULL,	/* IceProtocolActivateProc - we don't care about
								   when the Protocol Reply is sent, because the
								   session manager can not immediately send a
								   message - it must wait for RegisterClient. */
						     NULL	/* IceIOErrorProc */
						     )) < 0)
	{
	    qWarning("Could not register DCOP protocol with ICE");
	}

    char errormsg[256];
    int orig_umask = umask(077); /*old libICE's don't reset the umask() they set */
    if (!IceListenForConnections (&numTransports, &listenObjs,
				  256, errormsg))
	{
	    fprintf (stderr, "%s\n", errormsg);
	    exit (1);
	} else {
	    (void) umask(orig_umask);
	    // publish available transports.
	    QCString fName = DCOPClient::dcopServerFile();
	    FILE *f;
	    if(!(f = ::fopen(fName.data(), "w+"))) {
	        fprintf (stderr, "Can not create file %s: %s\n",
			 fName.data(), ::strerror(errno));
		exit(1);
	    }
	    char *idlist = IceComposeNetworkIdList(numTransports, listenObjs);
	    if (idlist != 0) {
	        fprintf(f, "%s", idlist);
		free(idlist);
	    }
	    fprintf(f, "\n%i\n", getpid());
	    fclose(f);
#ifndef Q_OS_WIN32
	    if (QCString(getenv("DCOPAUTHORITY")).isEmpty())
	    {
                // Create a link named like the old-style (KDE 2.x) naming
                QCString compatName = DCOPClient::dcopServerFileOld();
                ::symlink(fName,compatName);
            }
#endif // Q_OS_WIN32
	}

#if 0
	if (!SetAuthentication_local(numTransports, listenObjs))
	    qFatal("DCOPSERVER: authentication setup failed.");
#endif
    if (!SetAuthentication(numTransports, listenObjs, &authDataEntries))
        qFatal("DCOPSERVER: authentication setup failed.");

    IceAddConnectionWatch (DCOPWatchProc, static_cast<IcePointer>(this));
    _IceWriteHandler = DCOPIceWriteChar;

    listener.setAutoDelete( true );
    DCOPListener* con;
    for ( int i = 0; i < numTransports; i++) {
	con = new DCOPListener( listenObjs[i] );
	listener.append( con );
	connect( con, SIGNAL( activated(int) ), this, SLOT( newClient(int) ) );
    }
    char c = 0;
    write(ready[1], &c, 1); // dcopserver is started
    close(ready[1]);

    m_timer =  new QTimer(this);
    connect( m_timer, SIGNAL(timeout()), this, SLOT(slotTerminate()) );
    m_deadConnectionTimer = new QTimer(this);
    connect( m_deadConnectionTimer, SIGNAL(timeout()), this, SLOT(slotCleanDeadConnections()) );

#ifdef Q_OS_WIN
	char szEventName[256];
	sprintf(szEventName,"dcopserver%i",GetCurrentProcessId());
	m_evTerminate = CreateEventA(NULL,TRUE,FALSE,(LPCSTR)szEventName);
	ResetEvent(m_evTerminate);
	m_hTerminateThread = CreateThread(NULL,0,TerminatorThread,this,0,&m_dwTerminateThreadId);
	if(m_hTerminateThread)
		CloseHandle(m_hTerminateThread);
#endif

#ifdef DCOP_LOG
    char hostname_buffer[256];
    memset( hostname_buffer, 0, sizeof( hostname_buffer ) );
    if ( gethostname( hostname_buffer, 255 ) < 0 )
      hostname_buffer[0] = '\0';
    m_logger = new QFile( QString( "%1/.dcop-%2.log" ).arg( QDir::homeDirPath() ).arg( hostname_buffer ) );
    if ( m_logger->open( IO_WriteOnly ) ) {
        m_stream = new QTextStream( m_logger );
    }
#endif
}

DCOPServer::~DCOPServer()
{
    system(findDcopserverShutdown()+" --nokill");
    IceFreeListenObjs(numTransports, listenObjs);
    FreeAuthenticationData(numTransports, authDataEntries);
    delete dcopSignals;
#ifdef DCOP_LOG
    delete m_stream;
    m_logger->close();
    delete m_logger;
#endif
#ifdef Q_OS_WIN
	SetEvent(m_evTerminate);
	CloseHandle(m_evTerminate);
#endif
}

DCOPConnection* DCOPServer::findApp( const QCString& appId )
{
    if ( appId.isNull() )
	return 0;
    DCOPConnection* conn = appIds.find( appId );
    return conn;
}

/*!
  Called by timer after write errors.
 */
void DCOPServer::slotCleanDeadConnections()
{
qWarning("DCOP Cleaning up dead connections.");
    while(!deadConnections.isEmpty())
    {
       IceConn iceConn = deadConnections.take(0);
       IceSetShutdownNegotiation (iceConn, False);
       (void) IceCloseConnection( iceConn );
    }
}

/*!
  Called from our IceIoErrorHandler
 */
void DCOPServer::ioError( IceConn iceConn  )
{
    deadConnections.removeRef(iceConn);
    deadConnections.prepend(iceConn);
    m_deadConnectionTimer->start(0, true);
}


void DCOPServer::processData( int /*socket*/ )
{
    IceConn iceConn = static_cast<const DCOPConnection*>(sender())->iceConn;
    IceProcessMessagesStatus status = IceProcessMessages( iceConn, 0, 0 );
    if ( status == IceProcessMessagesIOError ) {
        deadConnections.removeRef(iceConn);
        if (deadConnections.isEmpty())
           m_deadConnectionTimer->stop();
	IceSetShutdownNegotiation (iceConn, False);
	(void) IceCloseConnection( iceConn );
    }
}

void DCOPServer::newClient( int /*socket*/ )
{
    IceAcceptStatus status;
    IceConn iceConn = IceAcceptConnection( static_cast<const  DCOPListener*>(sender())->listenObj, &status);
    if (!iceConn) {
      if (status == IceAcceptBadMalloc)
	 qWarning("Failed to alloc connection object!\n");
      else // IceAcceptFailure
         qWarning("Failed to accept ICE connection!\n");
      return;
    }

    IceSetShutdownNegotiation( iceConn, False );

    IceConnectStatus cstatus;
    while ((cstatus = IceConnectionStatus (iceConn))==IceConnectPending) {
	(void) IceProcessMessages( iceConn, 0, 0 );
    }

    if (cstatus != IceConnectAccepted) {
	if (cstatus == IceConnectIOError)
	    qWarning ("IO error opening ICE Connection!\n");
	else
	    qWarning ("ICE Connection rejected!\n");
        deadConnections.removeRef(iceConn);
	(void) IceCloseConnection (iceConn);
    }
}

void* DCOPServer::watchConnection( IceConn iceConn )
{
    DCOPConnection* con = new DCOPConnection( iceConn );
    connect( con, SIGNAL( activated(int) ), this, SLOT( processData(int) ) );

    clients.insert(iceConn, con );
    fd_clients.insert( IceConnectionNumber(iceConn), con);

    return static_cast<void*>(con);
}

void DCOPServer::removeConnection( void* data )
{
    DCOPConnection* conn = static_cast<DCOPConnection*>(data);

    dcopSignals->removeConnections(conn);

    clients.remove(conn->iceConn );
    fd_clients.remove( IceConnectionNumber(conn->iceConn) );

    // Send DCOPReplyFailed to all in conn->waitingForReply
    while (!conn->waitingForReply.isEmpty()) {
	IceConn iceConn = conn->waitingForReply.take(0);
	if (iceConn) {
	    DCOPConnection* target = clients.find( iceConn );
	    qWarning("DCOP aborting call from '%s' to '%s'", target ? target->appId.data() : "<unknown>" , conn->appId.data() );
	    QByteArray reply;
	    DCOPMsg *pMsg;
	    IceGetHeader( iceConn, majorOpcode, DCOPReplyFailed,
			  sizeof(DCOPMsg), DCOPMsg, pMsg );
	    pMsg->key = 1;
	    pMsg->length += reply.size();
            _DCOPIceSendBegin( iceConn );
	    DCOPIceSendData(iceConn, reply);
            _DCOPIceSendEnd();
            if (!target)
               qWarning("DCOP Error: unknown target in waitingForReply");
            else if (!target->waitingOnReply.removeRef(conn->iceConn))
               qWarning("DCOP Error: client in waitingForReply wasn't waiting on reply");
	}
    }

    // Send DCOPReplyFailed to all in conn->waitingForDelayedReply
    while (!conn->waitingForDelayedReply.isEmpty()) {
	IceConn iceConn = conn->waitingForDelayedReply.take(0);
	if (iceConn) {
	    DCOPConnection* target = clients.find( iceConn );
	    qWarning("DCOP aborting (delayed) call from '%s' to '%s'", target ? target->appId.data() : "<unknown>", conn->appId.data() );
	    QByteArray reply;
	    DCOPMsg *pMsg;
	    IceGetHeader( iceConn, majorOpcode, DCOPReplyFailed,
			  sizeof(DCOPMsg), DCOPMsg, pMsg );
	    pMsg->key = 1;
	    pMsg->length += reply.size();
            _DCOPIceSendBegin( iceConn );
	    DCOPIceSendData( iceConn, reply );
            _DCOPIceSendEnd();
            if (!target)
               qWarning("DCOP Error: unknown target in waitingForDelayedReply");
            else if (!target->waitingOnReply.removeRef(conn->iceConn))
               qWarning("DCOP Error: client in waitingForDelayedReply wasn't waiting on reply");
	}
    }
    while (!conn->waitingOnReply.isEmpty())
    {
	IceConn iceConn = conn->waitingOnReply.take(0);
        if (iceConn) {
           DCOPConnection* target = clients.find( iceConn );
           if (!target)
           {
               qWarning("DCOP Error: still waiting for answer from non-existing client.");
               continue;
           }
           qWarning("DCOP aborting while waiting for answer from '%s'", target->appId.data());
           if (!target->waitingForReply.removeRef(conn->iceConn) &&
               !target->waitingForDelayedReply.removeRef(conn->iceConn))
              qWarning("DCOP Error: called client has forgotten about caller");
        }
    }

    if ( !conn->appId.isNull() ) {
#ifndef NDEBUG
	qDebug("DCOP: unregister '%s'", conn->appId.data() );
#endif
        if ( !conn->daemon )
        {
            currentClientNumber--;
        }

	appIds.remove( conn->appId );

        broadcastApplicationRegistration( conn, "applicationRemoved(QCString)", conn->appId );
    }

    delete conn;

    if ( suicide && (currentClientNumber == 0) )
    {
        m_timer->start( 10000 ); // if within 10 seconds nothing happens, we'll terminate
    }
    if ( shutdown && appIds.isEmpty())
    {
        m_timer->start( 10 ); // Exit now
    }
}

void DCOPServer::slotTerminate()
{
#ifndef NDEBUG
    fprintf( stderr, "DCOPServer : slotTerminate() -> sending terminateKDE signal.\n" );
#endif
    QByteArray data;
    dcopSignals->emitSignal(0L /* dcopserver */, "terminateKDE()", data, false);
    disconnect( m_timer, SIGNAL(timeout()), this, SLOT(slotTerminate()) );
    connect( m_timer, SIGNAL(timeout()), this, SLOT(slotSuicide()) );
    system(findDcopserverShutdown()+" --nokill");
}

void DCOPServer::slotSuicide()
{
#ifndef NDEBUG
    fprintf( stderr, "DCOPServer : slotSuicide() -> exit.\n" );
#endif
    exit(0);
}

void DCOPServer::slotShutdown()
{
#ifndef NDEBUG
    fprintf( stderr, "DCOPServer : slotShutdown() -> waiting for clients to disconnect.\n" );
#endif
    char c;
#ifndef Q_OS_WIN
    read(pipeOfDeath[0], &c, 1);
#endif
    if (!shutdown)
    {
       shutdown = true;
       m_timer->start( 10000 ); // if within 10 seconds nothing happens, we'll terminate
       disconnect( m_timer, SIGNAL(timeout()), this, SLOT(slotTerminate()) );
       connect( m_timer, SIGNAL(timeout()), this, SLOT(slotExit()) );
       if (appIds.isEmpty())
         slotExit(); // Exit now
    }
}

void DCOPServer::slotExit()
{
#ifndef NDEBUG
	fprintf( stderr, "DCOPServer : slotExit() -> exit.\n" );
#endif
#ifdef Q_OS_WIN
	SetEvent(m_evTerminate);
	if(m_dwTerminateThreadId != GetCurrentThreadId())
		WaitForSingleObject(m_hTerminateThread,INFINITE);
	CloseHandle(m_hTerminateThread);
#endif
	exit(0);
}

bool DCOPServer::receive(const QCString &/*app*/, const QCString &obj,
			 const QCString &fun, const QByteArray& data,
			 QCString& replyType, QByteArray &replyData,
			 IceConn iceConn)
{
#ifdef DCOP_LOG
    (*m_stream) << "Received a message: obj =\""
                << obj << "\", fun =\""
                << fun << "\", replyType =\""
                << replyType << "\", data.size() =\""
                << data.size() << "\", replyData.size() ="
                << replyData.size() << "\n";
    m_logger->flush();
#endif

    if ( obj == "emit")
    {
        DCOPConnection* conn = clients.find( iceConn );
        if (conn) {
	    //qDebug("DCOPServer: %s emits %s", conn->appId.data(), fun.data());
	    dcopSignals->emitSignal(conn, fun, data, false);
        }
        replyType = "void";
        return true;
    }
    if ( fun == "setDaemonMode(bool)" ) {
        QDataStream args( data, IO_ReadOnly );
        if ( !args.atEnd() ) {
            Q_INT8 iDaemon;
            bool daemon;
            args >> iDaemon;

            daemon = static_cast<bool>( iDaemon );

	    DCOPConnection* conn = clients.find( iceConn );
            if ( conn && !conn->appId.isNull() ) {
                if ( daemon ) {
                    if ( !conn->daemon )
                    {
                        conn->daemon = true;

#ifndef NDEBUG
                        qDebug( "DCOP: new daemon %s", conn->appId.data() );
#endif

                        currentClientNumber--;

// David says it's safer not to do this :-)
//                        if ( currentClientNumber == 0 )
//                            m_timer->start( 10000 );
                    }
                } else
                {
                    if ( conn->daemon ) {
                        conn->daemon = false;

                        currentClientNumber++;

                        m_timer->stop();
                    }
                }
            }

            replyType = "void";
            return true;
        }
    }
    if ( fun == "registerAs(QCString)" ) {
	QDataStream args( data, IO_ReadOnly );
	if (!args.atEnd()) {
	    QCString app2 = readQCString(args);
	    QDataStream reply( replyData, IO_WriteOnly );
	    DCOPConnection* conn = clients.find( iceConn );
	    if ( conn && !app2.isEmpty() ) {
		if ( !conn->appId.isNull() &&
		     appIds.find( conn->appId ) == conn ) {
		    appIds.remove( conn->appId );

		}

                QCString oldAppId;
		if ( conn->appId.isNull() )
                {
                    currentClientNumber++;
                    m_timer->stop(); // abort termination if we were planning one
#ifndef NDEBUG
                    qDebug("DCOP: register '%s' -> number of clients is now %d", app2.data(), currentClientNumber );
#endif
                }
#ifndef NDEBUG
		else
                {
                    oldAppId = conn->appId;
		    qDebug("DCOP:  '%s' now known as '%s'", conn->appId.data(), app2.data() );
                }
#endif

		conn->appId = app2;
		if ( appIds.find( app2 ) != 0 ) {
		    // we already have this application, unify
		    int n = 1;
		    QCString tmp;
		    do {
			n++;
			tmp.setNum( n );
			tmp.prepend("-");
			tmp.prepend( app2 );
		    } while ( appIds.find( tmp ) != 0 );
		    conn->appId = tmp;
		}
		appIds.insert( conn->appId, conn );

		int c = conn->appId.find( '-' );
		if ( c > 0 )
		    conn->plainAppId = conn->appId.left( c );
		else
		    conn->plainAppId = conn->appId;

                if( !oldAppId.isEmpty())
                    broadcastApplicationRegistration( conn,
                        "applicationRemoved(QCString)", oldAppId );
                broadcastApplicationRegistration( conn, "applicationRegistered(QCString)", conn->appId );
	    }
	    replyType = "QCString";
	    reply << conn->appId;
	    return true;
	}
    }
    else if ( fun == "registeredApplications()" ) {
	QDataStream reply( replyData, IO_WriteOnly );
	QCStringList applications;
	QAsciiDictIterator<DCOPConnection> it( appIds );
	while ( it.current() ) {
	    applications << it.currentKey();
	    ++it;
	}
	replyType = "QCStringList";
	reply << applications;
	return true;
    } else if ( fun == "isApplicationRegistered(QCString)" ) {
	QDataStream args( data, IO_ReadOnly );
	if (!args.atEnd()) {
	    QCString s = readQCString(args);
	    QDataStream reply( replyData, IO_WriteOnly );
	    int b = ( findApp( s ) != 0 );
	    replyType = "bool";
	    reply << b;
	    return true;
	}
    } else if ( fun == "setNotifications(bool)" ) {
	QDataStream args( data, IO_ReadOnly );
	if (!args.atEnd()) {
	    Q_INT8 notifyActive;
	    args >> notifyActive;
	    DCOPConnection* conn = clients.find( iceConn );
	    if ( conn ) {
		if ( notifyActive )
		    conn->notifyRegister++;
		else if ( conn->notifyRegister > 0 )
		    conn->notifyRegister--;
	    }
	    replyType = "void";
	    return true;
	}
    } else if ( fun == "connectSignal(QCString,QCString,QCString,QCString,QCString,bool)") {
        DCOPConnection* conn = clients.find( iceConn );
        if (!conn) return false;
        QDataStream args(data, IO_ReadOnly );
        if (args.atEnd()) return false;
        QCString sender = readQCString(args);
        QCString senderObj = readQCString(args);
        QCString signal = readQCString(args);
        QCString receiverObj = readQCString(args);
        QCString slot = readQCString(args);
        Q_INT8 Volatile;
        args >> Volatile;
        //qDebug("DCOPServer: connectSignal(sender = %s senderObj = %s signal = %s recvObj = %s slot = %s)", sender.data(), senderObj.data(), signal.data(), receiverObj.data(), slot.data());
        bool b = dcopSignals->connectSignal(sender, senderObj, signal, conn, receiverObj, slot, (Volatile != 0));
        replyType = "bool";
        QDataStream reply( replyData, IO_WriteOnly );
        reply << (Q_INT8) (b?1:0);
        return true;
    } else if ( fun == "disconnectSignal(QCString,QCString,QCString,QCString,QCString)") {
        DCOPConnection* conn = clients.find( iceConn );
        if (!conn) return false;
        QDataStream args(data, IO_ReadOnly );
        if (args.atEnd()) return false;
        QCString sender = readQCString(args);
        QCString senderObj = readQCString(args);
        QCString signal = readQCString(args);
        QCString receiverObj = readQCString(args);
        QCString slot = readQCString(args);
        //qDebug("DCOPServer: disconnectSignal(sender = %s senderObj = %s signal = %s recvObj = %s slot = %s)", sender.data(), senderObj.data(), signal.data(), receiverObj.data(), slot.data());
        bool b = dcopSignals->disconnectSignal(sender, senderObj, signal, conn, receiverObj, slot);
        replyType = "bool";
        QDataStream reply( replyData, IO_WriteOnly );
        reply << (Q_INT8) (b?1:0);
        return true;
    }

    return false;
}

void DCOPServer::broadcastApplicationRegistration( DCOPConnection* conn, const QCString type,
    const QCString& appId )
{
    QByteArray data;
    QDataStream datas( data, IO_WriteOnly );
    datas << appId;
    QPtrDictIterator<DCOPConnection> it( clients );
    QByteArray ba;
    QDataStream ds( ba, IO_WriteOnly );
    ds <<QCString("DCOPServer") <<  QCString("") << QCString("")
       << type << data;
    int datalen = ba.size();
    DCOPMsg *pMsg = 0;
    while ( it.current() ) {
        DCOPConnection* c = it.current();
        ++it;
        if ( c->notifyRegister && (c != conn) ) {
            IceGetHeader( c->iceConn, majorOpcode, DCOPSend,
                          sizeof(DCOPMsg), DCOPMsg, pMsg );
            pMsg->key = 1;
	    pMsg->length += datalen;
            _DCOPIceSendBegin(c->iceConn);
	    DCOPIceSendData( c->iceConn, ba );
            _DCOPIceSendEnd();
        }
    }
}

void
DCOPServer::sendMessage(DCOPConnection *conn, const QCString &sApp,
                        const QCString &rApp, const QCString &rObj,
                        const QCString &rFun,  const QByteArray &data)
{
   QByteArray ba;
   QDataStream ds( ba, IO_WriteOnly );
   ds << sApp << rApp << rObj << rFun << data;
   int datalen = ba.size();
   DCOPMsg *pMsg = 0;

   IceGetHeader( conn->iceConn, majorOpcode, DCOPSend,
                 sizeof(DCOPMsg), DCOPMsg, pMsg );
   pMsg->length += datalen;
   pMsg->key = 1; // important!

#ifdef DCOP_LOG
   (*m_stream) << "Sending a message: sApp =\""
               << sApp << "\", rApp =\""
               << rApp << "\", rObj =\""
               << rObj << "\", rFun =\""
               << rFun << "\", datalen ="
               << datalen << "\n";
   m_logger->flush();
#endif

   _DCOPIceSendBegin( conn->iceConn );
   DCOPIceSendData(conn->iceConn, ba);
   _DCOPIceSendEnd();
}

void IoErrorHandler ( IceConn iceConn)
{
    the_server->ioError( iceConn );
}

static bool isRunning(const QCString &fName, bool printNetworkId = false)
{
    if (::access(fName.data(), R_OK) == 0) {
	QFile f(fName);
	f.open(IO_ReadOnly);
	int size = QMIN( 1024, f.size() ); // protection against a huge file
	QCString contents( size+1 );
	bool ok = f.readBlock( contents.data(), size ) == size;
	contents[size] = '\0';
	int pos = contents.find('\n');
	ok = ok && ( pos != -1 );
	pid_t pid = ok ? contents.mid(pos+1).toUInt(&ok) : 0;
	f.close();
	if (ok && pid && (kill(pid, SIGHUP) == 0)) {
	    if (printNetworkId)
	        qWarning("%s", contents.left(pos).data());
	    else
		qWarning( "---------------------------------\n"
		      "It looks like dcopserver is already running. If you are sure\n"
		      "that it is not already running, remove %s\n"
		      "and start dcopserver again.\n"
		      "---------------------------------\n",
		      fName.data() );

	    // lock file present, die silently.
	    return true;
	} else {
	    // either we couldn't read the PID or kill returned an error.
	    // remove lockfile and continue
	    unlink(fName.data());
	}
    } else if (errno != ENOENT) {
        // remove lockfile and continue
        unlink(fName.data());
    }
    return false;
}

const char* const ABOUT =
"Usage: dcopserver [--nofork] [--nosid] [--help]\n"
"       dcopserver --serverid\n"
"\n"
"DCOP is KDE's Desktop Communications Protocol. It is a lightweight IPC/RPC\n"
"mechanism built on top of the X Consortium's Inter Client Exchange protocol.\n"
"It enables desktop applications to communicate reliably with low overhead.\n"
"\n"
"Copyright (C) 1999-2001, The KDE Developers <http://www.kde.org>\n"
;

extern "C" DCOP_EXPORT int kdemain( int argc, char* argv[] )
{
    bool serverid = false;
    bool nofork = false;
    bool nosid = false;
    bool suicide = false;
    for(int i = 1; i < argc; i++) {
	if (strcmp(argv[i], "--nofork") == 0)
	    nofork = true;
	else if (strcmp(argv[i], "--nosid") == 0)
	    nosid = true;
	else if (strcmp(argv[i], "--nolocal") == 0)
	    ; // Ignore
	else if (strcmp(argv[i], "--suicide") == 0)
	    suicide = true;
	else if (strcmp(argv[i], "--serverid") == 0)
	    serverid = true;
	else {
	    fprintf(stdout, "%s", ABOUT );
	    return 0;
	}
    }

    if (serverid)
    {
       if (isRunning(DCOPClient::dcopServerFile(), true))
          return 0;
       return 1;
    }

    // check if we are already running
    if (isRunning(DCOPClient::dcopServerFile()))
       return 0;
#ifndef Q_OS_WIN32
    if (QCString(getenv("DCOPAUTHORITY")).isEmpty() &&
        isRunning(DCOPClient::dcopServerFileOld()))
    {
       // Make symlink for compatibility
       QCString oldFile = DCOPClient::dcopServerFileOld();
       QCString newFile = DCOPClient::dcopServerFile();
       symlink(oldFile.data(), newFile.data());
       return 0;
    }

    struct rlimit limits;

    int retcode = getrlimit(RLIMIT_NOFILE, &limits);
    if (!retcode) {
       if (limits.rlim_max > 512 && limits.rlim_cur < 512)
       {
          int cur_limit = limits.rlim_cur;
          limits.rlim_cur = 512;
          retcode = setrlimit(RLIMIT_NOFILE, &limits);

          if (retcode != 0)
          {
             qWarning("dcopserver: Could not raise limit on number of open files.");
             qWarning("dcopserver: Current limit = %d", cur_limit);
          }
       }
    }
#endif
    pipe(ready);

#ifndef Q_OS_WIN32
    if (!nofork) {
        pid_t pid = fork();
	if (pid > 0) {
	    char c = 1;
	    close(ready[1]);
	    read(ready[0], &c, 1); // Wait till dcopserver is started
	    close(ready[0]);
	    // I am the parent
	    if (c == 0)
            {
               // Test whether we are functional.
               DCOPClient client;
               if (client.attach())
                  return 0;
            }
            qWarning("DCOPServer self-test failed.");
            system(findDcopserverShutdown()+" --kill");
            return 1;
	}
	close(ready[0]);

	if (!nosid)
	    setsid();

	if (fork() > 0)
	    return 0; // get rid of controlling terminal
    }

    pipe(pipeOfDeath);

    signal(SIGHUP, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGPIPE, SIG_IGN);
#else
	{
		char c = 1;
		close(ready[1]);
		read(ready[0], &c, 1); // Wait till dcopserver is started
		close(ready[0]);
	}
#endif
    putenv(strdup("SESSION_MANAGER="));

    QApplication a( argc, argv, false );

    IceSetIOErrorHandler (IoErrorHandler );
    DCOPServer *server = new DCOPServer(suicide); // this sets the_server

#ifdef Q_OS_WIN
	SetConsoleCtrlHandler(DCOPServer::dcopServerConsoleProc,TRUE);
#else
	QSocketNotifier DEATH(pipeOfDeath[0], QSocketNotifier::Read, 0, 0);
		server->connect(&DEATH, SIGNAL(activated(int)), SLOT(slotShutdown()));
#endif

    int ret = a.exec();
    delete server;
    return ret;
}

#ifdef Q_OS_WIN
#include "dcopserver_win.cpp"
#endif

#include "dcopserver.moc"
