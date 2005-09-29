/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000-2004 Thiago Macieira <thiago.macieira@kdemail.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <netdb.h>

#include <stdlib.h>
#include <unistd.h>

#include <qglobal.h>
#include <qstring.h>
#include <qiodevice.h>
#include <qsocketnotifier.h>
#include <qguardedptr.h>

#include "kresolver.h"

#include "kdebug.h"
#include "kextsock.h"
#include "ksockaddr.h"
#include "ksocks.h"

#ifdef __CYGWIN__
#include "netsupp.h"
#endif 

using namespace KNetwork;

//
// Internal class definitions
//

class KExtendedSocketPrivate
{
public:
  int flags;			// socket flags
  int status;			// status
  int syserror;			// the system error value

  timeval timeout;		// connection/acception timeout

  KResolver resRemote;		// the resolved addresses
  KResolver resLocal;		// binding resolution
  unsigned current;		// used by the asynchronous connection

  ::KSocketAddress *local;	// local socket address
  ::KSocketAddress *peer;	// peer socket address

  QSocketNotifier *qsnIn, *qsnOut;
  int inMaxSize, outMaxSize;
  bool emitRead : 1, emitWrite : 1;
  mutable bool addressReusable : 1, ipv6only : 1;

  KExtendedSocketPrivate() :
    flags(0), status(0), syserror(0),
    current(0), local(0), peer(0),
    qsnIn(0), qsnOut(0), inMaxSize(-1), outMaxSize(-1), emitRead(false), emitWrite(false),
    addressReusable(false), ipv6only(false)
  {
    timeout.tv_sec = timeout.tv_usec = 0;
  }
};

// translate KExtendedSocket flags into KResolver ones
static bool process_flags(int flags, int& socktype, int& familyMask, int& outflags)
{
  switch (flags & (KExtendedSocket::streamSocket | KExtendedSocket::datagramSocket | KExtendedSocket::rawSocket))
    {
    case 0:
      /* No flags given, use default */

    case KExtendedSocket::streamSocket:
      /* streaming socket requested */
      socktype = SOCK_STREAM;
      break;

    case KExtendedSocket::datagramSocket:
      /* datagram packet socket requested */
      socktype = SOCK_DGRAM;
      break;

    case KExtendedSocket::rawSocket:
      /* raw socket requested. I wouldn't do this if I were you... */
      socktype = SOCK_RAW;
      break;

    default:
      /* the flags were used in an invalid manner */
      return false;
    }

  if (flags & KExtendedSocket::knownSocket)
    {
      familyMask = 0;
      if ((flags & KExtendedSocket::unixSocket) == KExtendedSocket::unixSocket)
	familyMask |= KResolver::UnixFamily;

      switch ((flags & (KExtendedSocket::ipv6Socket|KExtendedSocket::ipv4Socket)))
	{
	case KExtendedSocket::ipv4Socket:
	  familyMask |= KResolver::IPv4Family;
	  break;
	case KExtendedSocket::ipv6Socket:
	  familyMask |= KResolver::IPv6Family;
	  break;
	case KExtendedSocket::inetSocket:
	  familyMask |= KResolver::InternetFamily;
	  break;
	}

      // those are all the families we know about
    }
  else
    familyMask = KResolver::KnownFamily;

  /* check other flags */
  outflags = (flags & KExtendedSocket::passiveSocket ? KResolver::Passive : 0) |
    (flags & KExtendedSocket::canonName ? KResolver::CanonName : 0) |
    (flags & KExtendedSocket::noResolve ? KResolver::NoResolve : 0);

  if (getenv("KDE_NO_IPV6"))
    familyMask &= ~KResolver::IPv6Family;

  return true;
}

// "skips" at most len bytes from file descriptor fd
// that is, we will try and read that much data and discard
// it. We will stop when we have read those or when the read
// function returns error
static int skipData(int fd, unsigned len)
{
  char buf[1024];
  unsigned skipped = 0;
  while (len)
    {
      int count = sizeof(buf);
      if ((unsigned)count > len)
	count = len;
      count = KSocks::self()->read(fd, buf, count);
      if (count == -1)
	return -1;
      else
	{
	  len -= count;
	  skipped += count;
	}
    }
  return skipped;
}

/*
 * class KExtendedSocket
 */

// default constructor
KExtendedSocket::KExtendedSocket() :
  sockfd(-1), d(new KExtendedSocketPrivate)
{
}

// constructor with hostname
KExtendedSocket::KExtendedSocket(const QString& host, int port, int flags) :
  sockfd(-1), d(new KExtendedSocketPrivate)
{
  setAddress(host, port);
  setSocketFlags(flags);
}

// same
KExtendedSocket::KExtendedSocket(const QString& host, const QString& service, int flags) :
  sockfd(-1), d(new KExtendedSocketPrivate)
{
  setAddress(host, service);
  setSocketFlags(flags);
}

// destroy the class
KExtendedSocket::~KExtendedSocket()
{
  closeNow();

  if (d->local != NULL)
    delete d->local;
  if (d->peer != NULL)
    delete d->peer;

  if (d->qsnIn != NULL)
    delete d->qsnIn;
  if (d->qsnOut != NULL)
    delete d->qsnOut;

  delete d;
}

void KExtendedSocket::reset()
{
  closeNow();
  release();
  d->current = 0;
  d->status = nothing;
  d->syserror = 0;
}

int KExtendedSocket::socketStatus() const
{
  return d->status;
}

void KExtendedSocket::setSocketStatus(int newstatus)
{
  d->status = newstatus;
}

void KExtendedSocket::setError(int errorcode, int syserror)
{
  setStatus(errorcode);
  d->syserror = syserror;
}

int KExtendedSocket::systemError() const
{
  return d->syserror;
}

/*
 * Sets socket flags
 * This is only allowed if we are in nothing state
 */
int KExtendedSocket::setSocketFlags(int flags)
{
  if (d->status > nothing)
    return -1;			// error!

  return d->flags = flags;
}

int KExtendedSocket::socketFlags() const
{
  return d->flags;
}

/*
 * Sets socket target hostname
 * This is only allowed if we are in nothing state
 */
bool KExtendedSocket::setHost(const QString& host)
{
  if (d->status > nothing)
    return false;		// error!

  d->resRemote.setNodeName(host);
  return true;
}

/*
 * returns the hostname
 */
QString KExtendedSocket::host() const
{
  return d->resRemote.nodeName();
}

/*
 * Sets the socket target port/service
 * Same thing: only state 'nothing'
 */
bool KExtendedSocket::setPort(int port)
{
  return setPort(QString::number(port));
}

bool KExtendedSocket::setPort(const QString& service)
{
  if (d->status > nothing)
    return false;		// error

  d->resRemote.setServiceName(service);
  return true;
}

/*
 * returns the service port number
 */
QString KExtendedSocket::port() const
{
  return d->resRemote.serviceName();
}

/*
 * sets the address
 */
bool KExtendedSocket::setAddress(const QString& host, int port)
{
  return setHost(host) && setPort(port);
}

/*
 * the same
 */
bool KExtendedSocket::setAddress(const QString& host, const QString& serv)
{
  return setHost(host) && setPort(serv);
}

/*
 * Sets the bind hostname
 * This is only valid in the 'nothing' state and if this is not a
 * passiveSocket socket
 */
bool KExtendedSocket::setBindHost(const QString& host)
{
  if (d->status > nothing || d->flags & passiveSocket)
    return false;		// error

  d->resLocal.setServiceName(host);
  return true;
}

/*
 * Unsets the bind hostname
 * same thing
 */
bool KExtendedSocket::unsetBindHost()
{
  return setBindHost(QString::null);
}

/*
 * returns the binding host
 */
QString KExtendedSocket::bindHost() const
{
  return d->resLocal.serviceName();
}

/*
 * Sets the bind port
 * Same condition as setBindHost
 */
bool KExtendedSocket::setBindPort(int port)
{
  return setBindPort(QString::number(port));
}

bool KExtendedSocket::setBindPort(const QString& service)
{
  if (d->status > nothing || d->flags & passiveSocket)
    return false;		// error

  d->resLocal.setServiceName(service);
  return true;
}

/*
 * unsets the bind port
 */
bool KExtendedSocket::unsetBindPort()
{
  return setBindPort(QString::null);
}

/*
 * returns the binding port
 */
QString KExtendedSocket::bindPort() const
{
  return d->resLocal.serviceName();
}

/*
 * sets the binding address
 */
bool KExtendedSocket::setBindAddress(const QString& host, int port)
{
  return setBindHost(host) && setBindPort(port);
}

/*
 * same
 */
bool KExtendedSocket::setBindAddress(const QString& host, const QString& service)
{
  return setBindHost(host) && setBindPort(service);
}

/*
 * unsets binding address
 */
bool KExtendedSocket::unsetBindAddress()
{
  return unsetBindHost() && unsetBindPort();
}

/*
 * sets the timeout for the connection
 */
bool KExtendedSocket::setTimeout(int secs, int usecs)
{
  if (d->status >= connected)	// closed?
    return false;

  d->timeout.tv_sec = secs;
  d->timeout.tv_usec = usecs;
  return true;
}

/*
 * returns the timeout
 */
timeval KExtendedSocket::timeout() const
{
  return d->timeout;
}

/*
 * Sets the blocking mode on this socket
 */
bool KExtendedSocket::setBlockingMode(bool enable)
{
  cleanError();
  if (d->status < created)
    return false;

  if (sockfd == -1)
    return false;		// error!

  int fdflags = fcntl(sockfd, F_GETFL, 0);
  if (fdflags == -1)
    return false;		// error!

  if (!enable)
    fdflags |= O_NONBLOCK;
  else
    fdflags &= ~O_NONBLOCK;

  if (fcntl(sockfd, F_SETFL, fdflags) == -1)
    {
      setError(IO_UnspecifiedError, errno);
      return false;
    }
  return true;
}

/*
 * Returns the blocking mode on the socket
 */
bool KExtendedSocket::blockingMode()
{
  cleanError();
  if (d->status < created)
    return false;		// sockets not created are in blocking mode

  if (sockfd == -1)
    return false;		// error

  int fdflags = fcntl(sockfd, F_GETFL, 0);
  if (fdflags == -1)
    {
      setError(IO_UnspecifiedError, errno);
      return false;
    }
  return (fdflags & O_NONBLOCK) == 0; // non-blocking == false
}

/*
 * Sets the reusability flag for this socket in the OS
 */
bool KExtendedSocket::setAddressReusable(bool enable)
{
  cleanError();
  d->addressReusable = enable;
  if (d->status < created)
    return true;

  if (sockfd == -1)
    return true;

  if (!setAddressReusable(sockfd, enable))
    {
      setError(IO_UnspecifiedError, errno);
      return false;
    }
  return true;
}

bool KExtendedSocket::setAddressReusable(int fd, bool enable)
{
  if (fd == -1)
    return false;

  int on = enable;		// just to be on the safe side

  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) == -1)
    return false;
  return true;
}

/*
 * Retrieves the reusability flag for this socket
 */
bool KExtendedSocket::addressReusable()
{
  cleanError();
  if (d->status < created)
    return d->addressReusable;

  if (sockfd == -1)
    return d->addressReusable;

  int on;
  socklen_t onsiz = sizeof(on);
  if (getsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, &onsiz) == -1)
    {
      setError(IO_UnspecifiedError, errno);
      return false;
    }

  return on != 0;
}

/*
 * Set the IPV6_V6ONLY flag
 */
bool KExtendedSocket::setIPv6Only(bool enable)
{
#ifdef IPV6_V6ONLY
  cleanError();

  d->ipv6only = enable;
  if (sockfd == -1)
    return true;		// can't set on a non-existing socket

  int on = enable;

  if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY,
		 (char *)&on, sizeof(on)) == -1)
    {
      setError(IO_UnspecifiedError, errno);
      return false;
    }
  else
    return true;

#else
  // we don't have the IPV6_V6ONLY constant in this system
  d->ipv6only = enable;

  setError(IO_UnspecifiedError, ENOSYS);
  return false;			// can't set if we don't know about this flag
#endif
}

/*
 * retrieve the IPV6_V6ONLY flag
 */
bool KExtendedSocket::isIPv6Only()
{
#ifdef IPV6_V6ONLY
  cleanError();

  if (d->status < created || sockfd == -1)
    return d->ipv6only;

  int on;
  socklen_t onsiz = sizeof(on);
  if (getsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY,
		 (char *)&on, &onsiz) == -1)
    {
      setError(IO_UnspecifiedError, errno);
      return false;
    }

  return d->ipv6only = on;

#else
  // we don't have the constant
  setError(IO_UnspecifiedError, ENOSYS);
  return false;
#endif
}

/*
 * Sets the buffer sizes in this socket
 * Also, we create or delete the socket notifiers
 */
bool KExtendedSocket::setBufferSize(int rsize, int wsize)
{
  cleanError();
  if (d->status < created)
    return false;

  if (sockfd == -1)
    return false;

  if (d->flags & passiveSocket)
    return false;		// no I/O on passive sockets

  if (rsize < -2)
    return false;

  if (wsize < -2)
    return false;

  // LOCK BUFFER MUTEX

  // The input socket notifier is always enabled
  // That happens because we want to be notified of when the socket gets
  // closed
  if (d->qsnIn == NULL)
    {
      d->qsnIn = new QSocketNotifier(sockfd, QSocketNotifier::Read);
      QObject::connect(d->qsnIn, SIGNAL(activated(int)), this, SLOT(socketActivityRead()));
      d->qsnIn->setEnabled(true);
    }

  if (rsize == 0 && d->flags & inputBufferedSocket)
    {
      // user wants to disable input buffering
      d->flags &= ~inputBufferedSocket;

      consumeReadBuffer(readBufferSize(), NULL, true);
      d->inMaxSize = 0;
    }
  else if (rsize != -2)
    {
      // enabling input buffering
      if (rsize)
	d->flags |= inputBufferedSocket;
      d->inMaxSize = rsize;

      if (rsize > 0 && (unsigned)rsize < readBufferSize())
	// input buffer has more data than the new size; discard
	consumeReadBuffer(readBufferSize() - rsize, NULL, true);

    }

  if (wsize == 0 && d->flags & outputBufferedSocket)
    {
      // disabling output buffering
      d->flags &= ~outputBufferedSocket;
      if (d->qsnOut && !d->emitWrite)
	d->qsnOut->setEnabled(false);
      consumeWriteBuffer(writeBufferSize());
      d->outMaxSize = 0;
    }
  else if (wsize != -2)
    {
      // enabling input buffering
      if (wsize)
	d->flags |= outputBufferedSocket;
      d->outMaxSize = wsize;

      if (wsize > 0 && (unsigned)wsize < writeBufferSize())
	// output buffer is bigger than it is to become; shrink
	consumeWriteBuffer(writeBufferSize() - wsize);

      if (d->qsnOut == NULL)
	{
	  d->qsnOut = new QSocketNotifier(sockfd, QSocketNotifier::Write);
	  QObject::connect(d->qsnOut, SIGNAL(activated(int)), this, SLOT(socketActivityWrite()));
	  // if the class is being created now, there's nothing to write yet
	  // so socketActivityWrite() will get called once and disable
	  // the notifier
	}
    }

  // UNLOCK BUFFER MUTEX

  setFlags((mode() & ~IO_Raw) | ((d->flags & bufferedSocket) ? 0 : IO_Raw));

  // check we didn't turn something off we shouldn't
  if (d->emitWrite && d->qsnOut == NULL)
    {
      d->qsnOut = new QSocketNotifier(sockfd, QSocketNotifier::Write);
      QObject::connect(d->qsnOut, SIGNAL(activated(int)), this, SLOT(socketActivityWrite()));
    }

  return true;
}

/*
 * Finds the local address for this socket
 * if we have done this already, we return it. Otherwise, we'll have
 * to find the socket name
 */
const ::KSocketAddress *KExtendedSocket::localAddress()
{
  if (d->local != NULL)
    return d->local;
  if (d->status < bound)
    return NULL;

  return d->local = localAddress(sockfd);
}

/*
 * Same thing, but for peer address. Which means this does not work on
 * passiveSocket and that we require to be connected already. Also note that
 * the behavior on connectionless sockets is not defined here.
 */
const ::KSocketAddress* KExtendedSocket::peerAddress()
{
  if (d->peer != NULL)
    return d->peer;
  if (d->flags & passiveSocket || d->status < connected)
    return NULL;

  return d->peer = peerAddress(sockfd);
}

/*
 * Perform the lookup on the addresses given
 */
int KExtendedSocket::lookup()
{
  if (startAsyncLookup() != 0)
    return -1;

  if (!d->resRemote.wait() || !d->resLocal.wait())
    {
      d->status = nothing;
      return -1;
    }

  d->status = lookupDone;
  if (d->resRemote.error() != KResolver::NoError)
    return d->resRemote.error();
  if (d->resLocal.error() != KResolver::NoError)
    return d->resLocal.error();
  return 0;
}

/*
 * Performs an asynchronous lookup on the given address(es)
 */
int KExtendedSocket::startAsyncLookup()
{
  cleanError();
  if (d->status > lookupInProgress)
    return -1;
  if (d->status == lookupInProgress)
    // already in progress
    return 0;

  /* check socket type flags */
  int socktype, familyMask, flags;
  if (!process_flags(d->flags, socktype, familyMask, flags))
    return -2;

  // perform the global lookup before
  if (!d->resRemote.isRunning())
    {
      d->resRemote.setFlags(flags);
      d->resRemote.setFamily(familyMask);
      d->resRemote.setSocketType(socktype);
      QObject::connect(&d->resRemote, SIGNAL(finished(KResolverResults)), 
		       this, SLOT(dnsResultsReady()));

      if (!d->resRemote.start())
	{
	  setError(IO_LookupError, d->resRemote.error());
	  return d->resRemote.error();
	}
    }

  if ((d->flags & passiveSocket) == 0 && !d->resLocal.isRunning())
    {
      /* keep flags, but make this passive */
      flags |= KResolver::Passive;
      d->resLocal.setFlags(flags);
      d->resLocal.setFamily(familyMask);
      d->resLocal.setSocketType(socktype);
      QObject::connect(&d->resLocal, SIGNAL(finished(KResolverResults)), 
		       this, SLOT(dnsResultsReady()));

      if (!d->resLocal.start())
	{
	  setError(IO_LookupError, d->resLocal.error());
	  return d->resLocal.error();
	}
    }

  // if we are here, there were no errors
  if (d->resRemote.isRunning() || d->resLocal.isRunning())
    d->status = lookupInProgress; // only if there actually is a running lookup
  else
    {
      d->status = lookupDone;
      emit lookupFinished(d->resRemote.results().count() + 
			  d->resLocal.results().count());
    }
  return 0;
}

void KExtendedSocket::cancelAsyncLookup()
{
  cleanError();
  if (d->status != lookupInProgress)
    return;			// what's to cancel?

  d->status = nothing;
  d->resLocal.cancel(false);
  d->resRemote.cancel(false);
}

int KExtendedSocket::listen(int N)
{
  cleanError();
  if ((d->flags & passiveSocket) == 0 || d->status >= listening)
    return -2;
  if (d->status < lookupDone)
    if (lookup() != 0)
      return -2;		// error!
  if (d->resRemote.error())
    return -2;
  
  // doing the loop:
  KResolverResults::const_iterator it;
  KResolverResults res = d->resRemote.results();
  for (it = res.begin(); it != res.end(); ++it)
    {
      //kdDebug(170) << "Trying to listen on " << (*it).address().toString() << endl;
      sockfd = ::socket((*it).family(), (*it).socketType(), (*it).protocol());
      if (sockfd == -1)
	{
	  // socket failed creating
	  //kdDebug(170) << "Failed to create: " << perror << endl;
	  continue;
	}
	
      fcntl(sockfd, F_SETFD, FD_CLOEXEC);

      if (d->addressReusable)
	setAddressReusable(sockfd, true);
      setIPv6Only(d->ipv6only);
      cleanError();
      if (KSocks::self()->bind(sockfd, (*it).address().address(), (*it).length()) == -1)
	{
	  //kdDebug(170) << "Failed to bind: " << perror << endl;
	  ::close(sockfd);
	  sockfd = -1;
	  continue;
	}

      // ok, socket has bound
      // kdDebug(170) << "Socket bound: " << sockfd << endl;

      d->status = bound;
      break;
    }

  if (sockfd == -1)
    {
      setError(IO_ListenError, errno);
      //kdDebug(170) << "Listen error - sockfd is -1 " << endl;
      return -1;
    }

  d->status = bound;
  setFlags(IO_Sequential | IO_Raw | IO_ReadWrite);

  int retval = KSocks::self()->listen(sockfd, N);
  if (retval == -1)
    setError(IO_ListenError, errno);
  else
    {
      d->status = listening;
      d->qsnIn = new QSocketNotifier(sockfd, QSocketNotifier::Read);
      QObject::connect(d->qsnIn, SIGNAL(activated(int)), this, SLOT(socketActivityRead()));
    }
  return retval == -1 ? -1 : 0;
}

int KExtendedSocket::accept(KExtendedSocket *&sock)
{
  cleanError();
  sock = NULL;
  if ((d->flags & passiveSocket) == 0 || d->status >= accepting)
    return -2;
  if (d->status < listening)
    if (listen() < 0)
      return -2;		// error!

  // let's see
  // if we have a timeout in place, we have to place this socket in non-blocking
  // mode
  bool block = blockingMode();
  struct sockaddr sa;
  ksocklen_t len = sizeof(sa);
  sock = NULL;

  if (d->timeout.tv_sec > 0 || d->timeout.tv_usec > 0)
    {
      fd_set set;

      setBlockingMode(false);	// turn on non-blocking
      FD_ZERO(&set);
      FD_SET(sockfd, &set);

      //kdDebug(170).form("Accepting on %d with %d.%06d second timeout\n",
      //	     sockfd, d->timeout.tv_sec, d->timeout.tv_usec);
      // check if there is anything to accept now
      int retval = KSocks::self()->select(sockfd + 1, &set, NULL, NULL, &d->timeout);
      if (retval == -1)
	{
	  setError(IO_UnspecifiedError, errno);
	  return -1;		// system error
	}
      else if (retval == 0 || !FD_ISSET(sockfd, &set))
	{
	  setError(IO_TimeOutError, 0);
	  return -3;		// timeout
	}
    }

  // it's common stuff here
  int newfd = KSocks::self()->accept(sockfd, &sa, &len);

  if (newfd == -1)
    {
      setError(IO_AcceptError, errno);
      kdWarning(170) << "Error accepting on socket " << sockfd << ":"
		     << perror << endl;
      return -1;
    }

  fcntl(newfd, F_SETFD, FD_CLOEXEC);

  //kdDebug(170).form("Socket %d accepted socket %d\n", sockfd, newfd);

  setBlockingMode(block);	// restore blocking mode

  sock = new KExtendedSocket;
  sock->d->status = connected;
  sock->sockfd = newfd;
  sock->setFlags(IO_Sequential | IO_Raw | IO_ReadWrite | IO_Open | IO_Async);
  sock->setBufferSize(0, 0);	// always unbuffered here. User can change that later

  return 0;
}

/*
 * tries to connect
 *
 * FIXME!
 * This function is critical path. It has to be cleaned up and made faster
 */
int KExtendedSocket::connect()
{
  cleanError();
  if (d->flags & passiveSocket || d->status >= connected)
    return -2;
  if (d->status < lookupDone)
    if (lookup() != 0)
      return -2;

  timeval end, now;
  // Ok, things are a little tricky here
  // Let me explain
  // getaddrinfo() will return several different families of sockets
  // When we have to bind before we connect, we have to make sure we're binding
  // and connecting to the same family, or things won't work

  bool doingtimeout = d->timeout.tv_sec > 0 || d->timeout.tv_usec > 0;
  if (doingtimeout)
    {
      gettimeofday(&end, NULL);
      end.tv_usec += d->timeout.tv_usec;
      end.tv_sec += d->timeout.tv_sec;
      if (end.tv_usec > 1000*1000)
	{
	  end.tv_usec -= 1000*1000;
	  end.tv_sec++;
	}
//	kdDebug(170).form("Connection with timeout of %d.%06d seconds (ends in %d.%06d)\n",
//		     d->timeout.tv_sec, d->timeout.tv_usec, end.tv_sec, end.tv_usec);
    }

  KResolverResults remote = d->resRemote.results(),
    local = d->resLocal.results();
  KResolverResults::const_iterator it, it2;
  //kdDebug(170) << "Starting connect to " << host() << '|' << port() 
  //             << ": have " << local.count() << " local entries and "
  //             << remote.count() << " remote" << endl;
  for (it = remote.begin(), it2 = local.begin(); it != remote.end(); ++it)
    {
      //kdDebug(170) << "Trying to connect to " << (*it).address().toString() << endl;
      if (it2 != local.end())
	{
//	  //kdDebug(170) << "Searching bind socket for family " << p->ai_family << endl;
	  if ((*it).family() != (*it2).family())
	    // differing families, scan local for a matching family
	    for (it2 = local.begin(); it2 != local.end(); ++it2)
	      if ((*it).family() == (*it2).family())
		break;

	  if ((*it).family() != (*it2).family())
	    {
	      // no matching families for this
	      //kdDebug(170) << "No matching family for bind socket\n";
	      it2 = local.begin();
	      continue;
	    }

	  //kdDebug(170) << "Binding on " << (*it2).address().toString() << " before connect" << endl;
	  errno = 0;
	  sockfd = ::socket((*it).family(), (*it).socketType(), (*it).protocol());
	  setError(IO_ConnectError, errno);
	  if (sockfd == -1)
	    continue;		// cannot create this socket
          fcntl(sockfd, F_SETFD, FD_CLOEXEC);
	  if (d->addressReusable)
	    setAddressReusable(sockfd, true);
	  setIPv6Only(d->ipv6only);
	  cleanError();
	  if (KSocks::self()->bind(sockfd, (*it2).address(), (*it2).length()))
	    {
	      //kdDebug(170) << "Bind failed: " << perror << endl;
	      ::close(sockfd);
	      sockfd = -1;
	      continue;
	    }
	}
      else
	{
	  // no need to bind, just create
	  sockfd = ::socket((*it).family(), (*it).socketType(), (*it).protocol());
	  if (sockfd == -1)
	    {
	      setError(IO_ConnectError, errno);
	      continue;
	    }
          fcntl(sockfd, F_SETFD, FD_CLOEXEC);
	  if (d->addressReusable)
	    setAddressReusable(sockfd, true);
	  setIPv6Only(d->ipv6only);
	  cleanError();
	}

//      kdDebug(170) << "Socket " << sockfd << " created" << endl;
      d->status = created;

      // check if we have to do timeout
      if (doingtimeout && KSocks::self()->hasWorkingAsyncConnect())
	{
	  fd_set rd, wr;

	  setBlockingMode(false);

	  // now try and connect
	  if (KSocks::self()->connect(sockfd, (*it).address(), (*it).length()) == -1)
	    {
	      // this could be EWOULDBLOCK
	      if (errno != EWOULDBLOCK && errno != EINPROGRESS)
		{
		  //kdDebug(170) << "Socket " << sockfd << " did not connect: " << perror << endl;
		  setError(IO_ConnectError, errno);
		  ::close(sockfd);
		  sockfd = -1;
		  continue;	// nope, another error
		}

	      FD_ZERO(&rd);
	      FD_ZERO(&wr);
	      FD_SET(sockfd, &rd);
	      FD_SET(sockfd, &wr);

	      int retval = KSocks::self()->select(sockfd + 1, &rd, &wr, NULL, &d->timeout);
	      if (retval == -1)
		{
		  setError(IO_FatalError, errno);
		  continue;	// system error
		}
	      else if (retval == 0)
		{
		  ::close(sockfd);
		  sockfd = -1;
//		  kdDebug(170) << "Time out while trying to connect to " <<
//		    (*it).address().toString() << endl;
		  d->status = lookupDone;
		  setError(IO_TimeOutError, 0);
		  return -3;	// time out
		}

	      // adjust remaining time
	      gettimeofday(&now, NULL);
	      d->timeout.tv_sec = end.tv_sec - now.tv_sec;
	      d->timeout.tv_usec = end.tv_usec - now.tv_usec;
	      if (d->timeout.tv_usec < 0)
		{
		  d->timeout.tv_usec += 1000*1000;
		  d->timeout.tv_sec--;
		}
//	      kdDebug(170).form("Socket %d activity; %d.%06d seconds remaining\n",
//			     sockfd, d->timeout.tv_sec, d->timeout.tv_usec);

	      // this means that an event occurred in the socket
	      int errcode;
	      socklen_t len = sizeof(errcode);
	      retval = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&errcode,
				  &len);
	      if (retval == -1 || errcode != 0)
		{
		  // socket did not connect
		  //kdDebug(170) << "Socket " << sockfd << " did not connect: "
		  //	    << strerror(errcode) << endl;
		  ::close(sockfd);
		  sockfd = -1;

		  // this is HIGHLY UNLIKELY
		  if (d->timeout.tv_sec == 0 && d->timeout.tv_usec == 0)
		    {
		      d->status = lookupDone;
		      setError(IO_TimeOutError, 0);
		      return -3; // time out
		    }

		  setError(IO_ConnectError, errcode);
		  continue;
		}
	    }

	  // getting here means it connected
	  // setBufferSize() takes care of creating the socket notifiers
	  setBlockingMode(true);
	  d->status = connected;
	  setFlags(IO_Sequential | IO_Raw | IO_ReadWrite | IO_Open | IO_Async);
	  setBufferSize(d->flags & inputBufferedSocket ? -1 : 0,
			d->flags & outputBufferedSocket ? -1 : 0);
	  emit connectionSuccess();
//	  kdDebug(170) << "Socket " << sockfd << " connected\n";
	  return 0;
	}
      else
	{
	  // without timeouts
	  if (KSocks::self()->connect(sockfd, (*it).address(), (*it).length()) == -1)
	    {
	      //kdDebug(170) << "Socket " << sockfd << " to " << (*it).address().toString() 
	      //	   << " did not connect: " << perror << endl;
	      setError(IO_ConnectError, errno);
	      ::close(sockfd);
	      sockfd = -1;
	      continue;
	    }

	  d->status = connected;
	  setFlags(IO_Sequential | IO_Raw | IO_ReadWrite | IO_Open | IO_Async);
	  setBufferSize(d->flags & inputBufferedSocket ? -1 : 0,
			d->flags & outputBufferedSocket ? -1 : 0);
	  emit connectionSuccess();
//	  kdDebug(170) << "Socket " << sockfd << " connected\n";
	  return 0;		// it connected
	}
    }

  // getting here means no socket connected or stuff like that
  emit connectionFailed(d->syserror);
  //kdDebug(170) << "Failed to connect\n";
  return -1;
}

int KExtendedSocket::startAsyncConnect()
{
  cleanError();
  // check status
  if (d->status >= connected || d->flags & passiveSocket)
    return -2;

  if (d->status == connecting)
    // already on async connect
    return 0;

  // check if we have to do lookup
  // if we do, then we'll use asynchronous lookup and use
  // signal lookupFinished to do connection
  if (d->status < lookupDone)
    {
      QObject::connect(this, SIGNAL(lookupFinished(int)), this, SLOT(startAsyncConnectSlot()));
      if (d->status < lookupInProgress)
	return startAsyncLookup();
      else
	return 0;		// we still have to wait
    }

  // here we have d->status >= lookupDone and <= connecting
  // we can do our connection
  d->status = connecting;
  QGuardedPtr<QObject> p = this;
  connectionEvent();
  if (!p) 
    return -1; // We have been deleted.
  if (d->status < connecting)
    return -1;
  return 0;
}

void KExtendedSocket::cancelAsyncConnect()
{
  if (d->status != connecting)
    return;

  if (sockfd != -1)
    {
      // we have a waiting connection
      if (d->qsnIn)
	delete d->qsnIn;
      if (d->qsnOut)
	delete d->qsnOut;
      d->qsnIn = d->qsnOut = NULL;

      ::close(sockfd);
      sockfd = -1;
    }
  d->status = lookupDone;
}

bool KExtendedSocket::open(int mode)
{
  if (mode != IO_Raw | IO_ReadWrite)
    return false;		// invalid open mode

  if (d->flags & passiveSocket)
    return listen() == 0;
  else if (d->status < connecting)
    return connect() == 0;
  else
    return false;
}

void KExtendedSocket::close()
{
  if (sockfd == -1 || d->status >= closing)
    return;			// nothing to close

  // LOCK BUFFER MUTEX
  if (d->flags & outputBufferedSocket && writeBufferSize() > 0)
    {
      // write buffer not empty, go into closing state
      d->status = closing;
      if (d->qsnIn)
	delete d->qsnIn;
      d->qsnIn = NULL;
      // we keep the outgoing socket notifier because we want
      // to send data, but not receive
    }
  else
    {
      // nope, write buffer is empty
      // we can close now
      if (d->qsnIn)
	delete d->qsnIn;
      if (d->qsnOut)
	delete d->qsnOut;
      d->qsnIn = d->qsnOut = NULL;

      ::close(sockfd);
      d->status = done;
      emit closed(readBufferSize() != 0 ? availRead : 0);
    }
  // UNLOCK BUFFER MUTEX
}


void KExtendedSocket::closeNow()
{
  if (d->status >= done)
    return;			// nothing to close

  // close the socket
  delete d->qsnIn;
  delete d->qsnOut;
  d->qsnIn = d->qsnOut = NULL;

  if (d->status > connecting && sockfd != -1)
    {
      ::close(sockfd);
      sockfd = -1;
    }
  else if (d->status == connecting)
    cancelAsyncConnect();
  else if (d->status == lookupInProgress)
    cancelAsyncLookup();

  d->status = done;

  emit closed(closedNow |
	      (readBufferSize() != 0 ? availRead : 0) |
	      (writeBufferSize() != 0 ? dirtyWrite : 0));
}

void KExtendedSocket::release()
{
  // release our hold on the socket
  sockfd = -1;
  d->status = done;

  d->resRemote.cancel(false);
  d->resLocal.cancel(false);

  if (d->local != NULL)
    delete d->local;
  if (d->peer != NULL)
    delete d->peer;

  d->peer = d->local = NULL;

  if (d->qsnIn != NULL)
    delete d->qsnIn;
  if (d->qsnOut != NULL)
    delete d->qsnOut;

  d->qsnIn = d->qsnOut = NULL;

  // now that the socket notificators are done with, we can flush out the buffers
  consumeReadBuffer(readBufferSize(), NULL, true);
  consumeWriteBuffer(writeBufferSize());

  // don't delete d
  // leave that for the destructor
}

void KExtendedSocket::flush()
{
  cleanError();
  if (d->status < connected || d->status >= done || d->flags & passiveSocket)
    return;

  if (sockfd == -1)
    return;

  if ((d->flags & outputBufferedSocket) == 0)
    return;			// nothing to do

  // LOCK MUTEX

  unsigned written = 0;
  unsigned offset = outBufIndex; // this happens only for the first
  while (writeBufferSize() - written > 0)
    {
      // we have to write each output buffer in outBuf
      // but since we can have several very small buffers, we can make things
      // better by concatenating a few of them into a big buffer
      // question is: how big should that buffer be? 16 kB should be enough

      QByteArray buf(16384);
      QByteArray *a = outBuf.first();
      unsigned count = 0;

      while (a && count + (a->size() - offset) <= buf.size())
	{
	  memcpy(buf.data() + count, a->data() + offset, a->size() - offset);
	  count += a->size() - offset;
	  offset = 0;
	  a = outBuf.next();
	}

      // see if we can still fit more
      if (a && count < buf.size())
	{
	  // getting here means this buffer (a) is larger than
	  // (buf.size() - count) (even for count == 0).
	  memcpy(buf.data() + count, a->data() + offset, buf.size() - count);
	  offset += buf.size() - count;
	  count = buf.size();
	}

      // now try to write those bytes
      int wrote = KSocks::self()->write(sockfd, buf, count);

      if (wrote == -1)
	{
	  // could be EAGAIN (EWOULDBLOCK)
	  setError(IO_WriteError, errno);
	  break;
	}
      written += wrote;

      if ((unsigned)wrote != count)
	break;
    }
  if (written)
    {
      consumeWriteBuffer(written);
      emit bytesWritten(written);
    }

  // UNLOCK MUTEX
}


Q_LONG KExtendedSocket::readBlock(char *data, Q_ULONG maxlen)
{
  cleanError();
  if (d->status < connected || d->flags & passiveSocket)
    return -2;

  int retval;

  if ((d->flags & inputBufferedSocket) == 0)
    {
      // we aren't buffering this socket, so just pass along
      // the call to the real read method

      if (sockfd == -1)
	return -2;
      if (data)
	retval = KSocks::self()->read(sockfd, data, maxlen);
      else
	retval = skipData(sockfd, maxlen);
      if (retval == -1)
	setError(IO_ReadError, errno);
    }
  else
    {
      // this socket is being buffered. So read from the buffer

      // LOCK BUFFER MUTEX

      retval = consumeReadBuffer(maxlen, data);
      if (retval == 0)
	{
	  // consumeReadBuffer returns 0 only if the buffer is
	  // empty
	  if (sockfd == -1)
	    return 0;		// buffer is clear now, indicate EOF
	  setError(IO_ReadError, EWOULDBLOCK);
	  retval = -1;
	}

      // UNLOCK BUFFER MUTEX

    }
  return retval;
}

Q_LONG KExtendedSocket::writeBlock(const char *data, Q_ULONG len)
{
  cleanError();
  if (d->status < connected || d->status >= closing || d->flags & passiveSocket)
    return -2;
  if (sockfd == -1)
    return -2;

  if (len == 0)
    return 0;			// what's to write?

  int retval;

  if ((d->flags & outputBufferedSocket) == 0)
    {
      // socket not buffered. Just call write
      retval = KSocks::self()->write(sockfd, data, len);
      if (retval == -1)
	setError(IO_WriteError, errno);
      else
	emit bytesWritten(retval);
    }
  else
    {
      // socket is buffered. Feed the write buffer

      // LOCK BUFFER MUTEX

      register unsigned wsize = writeBufferSize();
      if (d->outMaxSize == (int)wsize) // (int) to get rid of annoying warning
	{
	  // buffer is full!
	  setError(IO_WriteError, EWOULDBLOCK);
	  retval = -1;
	}
      else
	{
	  if (d->outMaxSize != -1 && wsize + len > (unsigned)d->outMaxSize)
	    // we cannot write all data. Write just as much as to fill the buffer
	    len = d->outMaxSize - wsize;

	  // len > 0 here
	  retval = feedWriteBuffer(len, data);
	  if (wsize == 0 || d->emitWrite)
	    // buffer was empty, which means that the notifier is probably disabled
	    d->qsnOut->setEnabled(true);
	}

      // UNLOCK BUFFER MUTEX
    }

  return retval;
}

int KExtendedSocket::peekBlock(char *data, uint maxlen)
{
  if (d->status < connected || d->flags & passiveSocket)
    return -2;
  if (sockfd == -1)
    return -2;

  // need to LOCK MUTEX around this call...

  if (d->flags & inputBufferedSocket)
    return consumeReadBuffer(maxlen, data, false);

  return 0;
}

int KExtendedSocket::unreadBlock(const char *, uint)
{
  // Always return -1, indicating this is not supported
  setError(IO_ReadError, ENOSYS);
  return -1;
}

int KExtendedSocket::bytesAvailable() const
{
  if (d->status < connected || d->flags & passiveSocket)
    return -2;

  // as of now, we don't do any extra processing
  // we only work in input-buffered sockets
  if (d->flags & inputBufferedSocket)
    return KBufferedIO::bytesAvailable();

  return 0;			// TODO: FIONREAD ioctl
}

int KExtendedSocket::waitForMore(int msecs)
{
  cleanError();
  if (d->flags & passiveSocket || d->status < connected || d->status >= closing)
    return -2;
  if (sockfd == -1)
    return -2;

  fd_set rd;
  FD_ZERO(&rd);
  FD_SET(sockfd, &rd);
  timeval tv;
  tv.tv_sec = msecs / 1000;
  tv.tv_usec = (msecs % 1000) * 1000;

  int retval = KSocks::self()->select(sockfd + 1, &rd, NULL, NULL, &tv);
  if (retval == -1)
    {
      setError(IO_FatalError, errno);
      return -1;
    }
  else if (retval != 0)
    socketActivityRead();	// do read processing

  return bytesAvailable();
}

int KExtendedSocket::getch()
{
  unsigned char c;
  int retval;
  retval = readBlock((char*)&c, sizeof(c));

  if (retval < 0)
    return retval;
  return c;
}

int KExtendedSocket::putch(int ch)
{
  unsigned char c = (char)ch;
  return writeBlock((char*)&c, sizeof(c));
}

// sets the emission of the readyRead signal
void KExtendedSocket::enableRead(bool enable)
{
  // check if we can disable the socket notifier
  // saves us a few cycles
  // this is so because in buffering mode, we rely on these signals
  // being emitted to do our I/O. We couldn't disable them here
  if (!enable && (d->flags & inputBufferedSocket) == 0 && d->qsnIn)
    d->qsnIn->setEnabled(false);
  else if (enable && d->qsnIn)
    // we can enable it always
    d->qsnIn->setEnabled(true);
  d->emitRead = enable;
}

// sets the emission of the readyWrite signal
void KExtendedSocket::enableWrite(bool enable)
{
  // same thing as above
  if (!enable && (d->flags & outputBufferedSocket) == 0 && d->qsnOut)
    d->qsnOut->setEnabled(false);
  else if (enable && d->qsnOut)
    // we can enable it always
    d->qsnOut->setEnabled(true);
  d->emitWrite = enable;
}

// protected slot
// this is connected to d->qsnIn::activated(int)
void KExtendedSocket::socketActivityRead()
{
  if (d->flags & passiveSocket)
    {
      emit readyAccept();
      return;
    }
  if (d->status == connecting)
    {
      connectionEvent();
      return;
    }
  if (d->status != connected)
    return;

  // do we need to do I/O here?
  if (d->flags & inputBufferedSocket)
    {
      // aye. Do read from the socket and feed our buffer
      QByteArray a;
      char buf[1024];
      int len, totalread = 0;

      // LOCK MUTEX

      unsigned cursize = readBufferSize();

      if (d->inMaxSize == -1 || cursize < (unsigned)d->inMaxSize)
	{
	  do
	    {
	      // check that we can read that many bytes
	      if (d->inMaxSize != -1 && d->inMaxSize - (cursize + totalread) < sizeof(buf))
		// no, that would overrun the buffer
		// note that this will also make us exit the loop
		len = d->inMaxSize - (cursize + totalread);
	      else
		len = sizeof(buf);

	      len = KSocks::self()->read(sockfd, buf, len);
	      if (len > 0)
		{
		  // normal read operation
		  a.resize(a.size() + len);
		  memcpy(a.data() + totalread, buf, len);
		  totalread += len;	// totalread == a.size() now
		}
	      else if (len == 0)
		{
		  // EOF condition here
		  ::close(sockfd);
		  sockfd = -1;	// we're closed
		  d->qsnIn->deleteLater();
		  delete d->qsnOut;
		  d->qsnIn = d->qsnOut = NULL;
		  d->status = done;
		  emit closed(involuntary |
			      (readBufferSize() ? availRead : 0) |
			      (writeBufferSize() ? dirtyWrite : 0));
		  return;
		}
	      else
		{
		  // error!
		  setError(IO_ReadError, errno);
		  return;
		}
	      // will loop only for normal read operations
	    }
	  while (len == sizeof(buf));

	  feedReadBuffer(a.size(), a.data());
	}

      // UNLOCK MUTEX
    }
  else
    {
      // No input buffering, but the notifier fired
      // That means that either there is data to be read or that the 
      // socket closed.

      // try to read one byte. If we can't, then the socket got closed

      char c;
      int len = KSocks::self()->recv(sockfd, &c, sizeof(c), MSG_PEEK);
      if (len == 0)
	{
	  // yes, it's an EOF condition
	  d->qsnIn->setEnabled(false);
	  ::close(sockfd);
	  sockfd = -1;
	  d->status = done;
	  emit closed(involuntary);
	  return;
	}
    }

  if (d->emitRead)
    emit readyRead();
}

void KExtendedSocket::socketActivityWrite()
{
  if (d->flags & passiveSocket)
    return;
  if (d->status == connecting)
    {
      connectionEvent();
      return;
    }
  if (d->status != connected && d->status != closing)
    return;

  flush();

  bool empty = writeBufferSize() == 0;

  if (d->emitWrite && empty)
    emit readyWrite();
  else if (!d->emitWrite)
    {
      // check if we can disable the notifier
      d->qsnOut->setEnabled(!empty); // leave it enabled only if we have more data to send
    }
  if (d->status == closing && empty)
    {
      // done sending the missing data!
      d->status = done;

      delete d->qsnOut;
      ::close(sockfd);

      d->qsnOut = NULL;
      sockfd = -1;
      emit closed(delayed | (readBufferSize() ? availRead : 0));
    }
}

// this function is called whenever we have a "connection event"
// that is, whenever our asynchronously connecting socket throws
// an event
void KExtendedSocket::connectionEvent()
{
  if (d->status != connecting)
    return;			// move along. There's nothing to see here

  KResolverResults remote = d->resRemote.results();
  if (remote.count() == 0)
    {
      // We have a problem! Abort?
      kdError(170) << "KExtendedSocket::connectionEvent() called but no data available!\n";
      return;
    }

  int errcode = 0;

  if (sockfd != -1)
    {
      // our socket has activity
      // find out what it was
      int retval;
      socklen_t len = sizeof(errcode);
      retval = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&errcode, &len);

      if (retval == -1 || errcode != 0)
	{
	  // socket activity and there was error?
	  // that means the socket probably did not connect
	  if (d->qsnIn)
	    delete d->qsnIn;
	  if (d->qsnOut)
	    delete d->qsnOut;
	  ::close(sockfd);

	  sockfd = -1;
	  d->qsnIn = d->qsnOut = NULL;
	  d->current++;
	  setError(IO_ConnectError, errcode);
	}
      else
	{
	  // hmm, socket activity and there was no error?
	  // that means it connected
	  // YAY!
	  cleanError();
	  d->status = connected;
	  setBlockingMode(true);
	  setFlags(IO_Sequential | IO_Raw | IO_ReadWrite | IO_Open | IO_Async);
	  setBufferSize(d->flags & inputBufferedSocket ? -1 : 0,
			d->flags & outputBufferedSocket ? -1 : 0);
	  emit connectionSuccess();
	  return;
	}
    }

  // ok, we have to try something here
  // and sockfd == -1
  KResolverResults local = d->resLocal.results();
  unsigned localidx = 0;
  for ( ; d->current < remote.count(); d->current++)
    {
      // same code as in connect()
      if (local.count() != 0)
	{
	  // scan bindres for a local resuls family
	  for (localidx = 0; localidx < local.count(); localidx++)
	    if (remote[d->current].family() == local[localidx].family())
	      break;

	  if (remote[d->current].family() != local[localidx].family())
	    {
	      // no matching families for this
	      continue;
	    }

	  errno = 0;
	  sockfd = ::socket(remote[d->current].family(), remote[d->current].socketType(),
			    remote[d->current].protocol());
	  setError(IO_ConnectError, errno);
	  errcode = errno;
	  if (sockfd == -1)
	    continue;		// cannot create this socket
          fcntl(sockfd, F_SETFD, FD_CLOEXEC);
	  if (d->addressReusable)
	    setAddressReusable(sockfd, true);
	  setIPv6Only(d->ipv6only);
	  cleanError();
	  if (KSocks::self()->bind(sockfd, local[localidx].address(), 
				   local[localidx].length()) == -1)
	    {
	      ::close(sockfd);
	      sockfd = -1;
	      continue;
	    }
	}
      else
	{
	  // no need to bind, just create
	  sockfd = ::socket(remote[d->current].family(), remote[d->current].socketType(),
			    remote[d->current].protocol());
	  if (sockfd == -1)
	    {
	      setError(IO_ConnectError, errno);
	      errcode = errno;
	      continue;
	    }
          fcntl(sockfd, F_SETFD, FD_CLOEXEC);
	  if (d->addressReusable)
	    setAddressReusable(sockfd, true);
	  setIPv6Only(d->ipv6only);
	  cleanError();
	}

      if (KSocks::self()->hasWorkingAsyncConnect())
        setBlockingMode(false);
      if (KSocks::self()->connect(sockfd, remote[d->current].address(), 
				  remote[d->current].length()) == -1)
	{
	  if (errno != EWOULDBLOCK && errno != EINPROGRESS)
	    {
	      setError(IO_ConnectError, errno);
	      ::close(sockfd);
	      sockfd = -1;
	      errcode = errno;
	      continue;
	    }

	  // error here is either EWOULDBLOCK or EINPROGRESS
	  // so, it is a good condition
	  d->qsnIn = new QSocketNotifier(sockfd, QSocketNotifier::Read);
	  QObject::connect(d->qsnIn, SIGNAL(activated(int)), this, SLOT(socketActivityRead()));
	  d->qsnOut = new QSocketNotifier(sockfd, QSocketNotifier::Write);
	  QObject::connect(d->qsnOut, SIGNAL(activated(int)), this, SLOT(socketActivityWrite()));

	  // ok, let the Qt event loop do the selecting for us
	  return;
	}

      // eh, what?
      // the non-blocking socket returned valid connection?
      // already?
      // I suppose that could happen...
      cleanError();
      d->status = connected;
      setBlockingMode(true);
      setFlags(IO_Sequential | IO_Raw | IO_ReadWrite | IO_Open | IO_Async);
      setBufferSize(d->flags & inputBufferedSocket ? -1 : 0,
		    d->flags & outputBufferedSocket ? -1 : 0);
      emit connectionSuccess();
      return;
    }

  // if we got here, it means that there are no more options to connect
  d->status = lookupDone;	// go back
  emit connectionFailed(errcode);
}

void KExtendedSocket::dnsResultsReady()
{
  // check that this function was called in a valid state
  if (d->status != lookupInProgress)
    return;

  // valid state. Are results fully ready?
  if (d->resRemote.isRunning() || d->resLocal.isRunning())
    // no, still waiting for answer in one of the lookups
    return;

  // ok, we have all results
  // count how many results we have
  int n = d->resRemote.results().count() + d->resLocal.results().count();

  if (n)
    {
      d->status = lookupDone;
      cleanError();
    }
  else
    {
      d->status = nothing;
      setError(IO_LookupError, KResolver::NoName);
    }

  emit lookupFinished(n);

  return;
}

void KExtendedSocket::startAsyncConnectSlot()
{
  QObject::disconnect(this, SIGNAL(lookupFinished(int)), this, SLOT(startAsyncConnectSlot()));

  if (d->status == lookupDone)
    startAsyncConnect();
}

int KExtendedSocket::resolve(sockaddr *sock, ksocklen_t len, QString &host,
			     QString &port, int flags)
{
  kdDebug(170) << "Deprecated function called:" << k_funcinfo << endl;

  int err;
  char h[NI_MAXHOST], s[NI_MAXSERV];

  h[0] = s[0] = '\0';

  err = getnameinfo(sock, len, h, sizeof(h) - 1, s, sizeof(s) - 1, flags);
  host = QString::fromUtf8(h);
  port = QString::fromUtf8(s);

  return err;
}

int KExtendedSocket::resolve(::KSocketAddress *sock, QString &host, QString &port,
			     int flags)
{
  return resolve(sock->data, sock->datasize, host, port, flags);
}

QPtrList<KAddressInfo> KExtendedSocket::lookup(const QString& host, const QString& port,
					    int userflags, int *error)
{
  kdDebug(170) << "Deprecated function called:" << k_funcinfo << endl;

  int socktype, familyMask, flags;
  unsigned i;
  QPtrList<KAddressInfo> l;

  /* check socket type flags */
  if (!process_flags(userflags, socktype, familyMask, flags))
    return l;

//  kdDebug(170) << "Performing lookup on " << host << "|" << port << endl;
  KResolverResults res = KResolver::resolve(host, port, flags, familyMask);
  if (res.error())
    {
      if (error)
	*error = res.error();
      return l;
    }

  for (i = 0; i < res.count(); i++)
    {
      KAddressInfo *ai = new KAddressInfo();

      // I should have known that using addrinfo was going to come
      // and bite me back some day...
      ai->ai = (addrinfo *) malloc(sizeof(addrinfo));
      memset(ai->ai, 0, sizeof(addrinfo));

      ai->ai->ai_family = res[i].family();
      ai->ai->ai_socktype = res[i].socketType();
      ai->ai->ai_protocol = res[i].protocol();
      QString canon = res[i].canonicalName();
      if (!canon.isEmpty())
	{
	  ai->ai->ai_canonname = (char *) malloc(canon.length()+1);
	  strcpy(ai->ai->ai_canonname, canon.ascii()); // ASCII here is intentional
	}
      if ((ai->ai->ai_addrlen = res[i].length()))
	{
	  ai->ai->ai_addr = (struct sockaddr *) malloc(res[i].length());
	  memcpy(ai->ai->ai_addr, res[i].address().address(), res[i].length());
	}
      else
	{
	  ai->ai->ai_addr = 0;
	}

      ai->addr = ::KSocketAddress::newAddress(ai->ai->ai_addr, ai->ai->ai_addrlen);

      l.append(ai);
    }

  if ( error )
      *error = 0;               // all is fine!

  return l;
}

::KSocketAddress *KExtendedSocket::localAddress(int fd)
{
  ::KSocketAddress *local;
  struct sockaddr static_sa, *sa = &static_sa;
  ksocklen_t len = sizeof(static_sa);

  /* find out the socket length, in advance
   * we use a sockaddr allocated on the heap just not to pass down
   * a NULL pointer to the first call. Some systems are reported to
   * set len to 0 if we pass NULL as the sockaddr */
  if (KSocks::self()->getsockname(fd, sa, &len) == -1)
    return NULL;		// error!

  /* was it enough? */
  if (len > sizeof(static_sa)
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
      || sa->sa_len > sizeof(static_sa)
#endif
      )
    {
      /* nope, malloc a new socket with the proper size */

#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
      if (sa->sa_len != len)
        len = sa->sa_len;
#endif

      sa = (sockaddr*)malloc(len);
      if (sa == NULL)
	return NULL;		// out of memory

      if (KSocks::self()->getsockname(fd, sa, &len) == -1)
	{
	  free(sa);
	  return NULL;
	}

      local = ::KSocketAddress::newAddress(sa, len);
      free(sa);
    }
  else
    local = ::KSocketAddress::newAddress(sa, len);

  return local;
}

/* This is exactly the same code as localAddress, except
 * we call getpeername here */
::KSocketAddress *KExtendedSocket::peerAddress(int fd)
{
  ::KSocketAddress *peer;
  struct sockaddr static_sa, *sa = &static_sa;
  ksocklen_t len = sizeof(static_sa);

  /* find out the socket length, in advance
   * we use a sockaddr allocated on the heap just not to pass down
   * a NULL pointer to the first call. Some systems are reported to
   * set len to 0 if we pass NULL as the sockaddr */
  if (KSocks::self()->getpeername(fd, sa, &len) == -1)
    return NULL;		// error!

  /* was it enough? */
  if (len > sizeof(static_sa)
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
      || sa->sa_len > sizeof(static_sa)
#endif
      )
    {
      /* nope, malloc a new socket with the proper size */

#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
      if (sa->sa_len != len)
        len = sa->sa_len;
#endif

      sa = (sockaddr*)malloc(len);
      if (sa == NULL)
	return NULL;		// out of memory

      if (KSocks::self()->getpeername(fd, sa, &len) == -1)
	{
	  free(sa);
	  return NULL;
	}

      peer = ::KSocketAddress::newAddress(sa, len);
      free(sa);
    }
  else
    peer = ::KSocketAddress::newAddress(sa, len);

  return peer;
}

QString KExtendedSocket::strError(int code, int syserr)
{
  const char * msg;
  if (code == IO_LookupError)
    msg = gai_strerror(syserr);
  else
    msg = strerror(syserr);

  return QString::fromLocal8Bit(msg);
}


QSocketNotifier *KExtendedSocket::readNotifier() { return d->qsnIn; }
QSocketNotifier *KExtendedSocket::writeNotifier() { return d->qsnOut; }

/*
 * class KAddressInfo
 */

#if 0
KAddressInfo::KAddressInfo(addrinfo *p)
{
   ai = (addrinfo *) malloc(sizeof(addrinfo));
   memcpy(ai, p, sizeof(addrinfo));
   ai->ai_next = NULL;
   if (p->ai_canonname)
   {
      ai->ai_canonname = (char *) malloc(strlen(p->ai_canonname)+1);
      strcpy(ai->ai_canonname, p->ai_canonname);
   }
   if (p->ai_addr && p->ai_addrlen)
   {
      ai->ai_addr = (struct sockaddr *) malloc(p->ai_addrlen);
      memcpy(ai->ai_addr, p->ai_addr, p->ai_addrlen);
   }
   else
   {
      ai->ai_addr = 0;
      ai->ai_addrlen = 0;
   }

   addr = ::KSocketAddress::newAddress(ai->ai_addr, ai->ai_addrlen);
}
#endif
KAddressInfo::~KAddressInfo()
{
  if (ai && ai->ai_canonname)
    free(ai->ai_canonname);

  if (ai && ai->ai_addr)
    free(ai->ai_addr);  

  if (ai)
    free(ai);
  delete addr;
}

int KAddressInfo::flags() const
{
  return ai->ai_flags;
}

int KAddressInfo::family() const
{
  return ai->ai_family;
}

int KAddressInfo::socktype() const
{
  return ai->ai_socktype;
}

int KAddressInfo::protocol() const
{
  return ai->ai_protocol;
}

const char* KAddressInfo::canonname() const
{
  return ai->ai_canonname;
}

void KExtendedSocket::virtual_hook( int id, void* data )
{ KBufferedIO::virtual_hook( id, data ); }

#include "kextsock.moc"
