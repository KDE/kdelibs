/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000,2001 Thiago Macieira <thiagom@mail.com>
 *
 *  $Id$
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <config.h>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <sys/types.h>

#include <qglobal.h>
#include <qstring.h>
#include <qiodevice.h>

#include "kdebug.h"
#include "kextsock.h"
#include "ksockaddr.h"

#include "netsupp.h"

/*
 * getaddrinfo is defined in IEEE POSIX 1003.1g (Protocol Independent Interfaces)
 * and RFC 2553 extends that specification
 */

#ifndef AI_NUMERICHOST
/* Some systems have getaddrinfo according to POSIX, but not the RFC */
#define AI_NUMERICHOST		0
#endif

class KExtendedSocket::KExtendedSocketPrivate
{
public:
  timeval timeout;		// connection/acception timeout
  QString host;			// requested hostname
  QString service;		// requested service
  QString localhost;		// requested bind host or local hostname
  QString localservice;		// requested bind service or local port
  addrinfo *resolution;		// the resolved addresses
  addrinfo *bindres;		// binding resolution

  KSocketAddress *local;	// local socket address
  KSocketAddress *peer;		// peer socket address

  KExtendedSocketPrivate() :
    resolution(0), bindres(0), local(0), peer(0)
  {
    timeout.tv_sec = timeout.tv_usec = 0;
    host = QString::null;
    service = QString::null;
    localhost = QString::null;
    localservice = QString::null;
  }
};

static bool process_flags(int flags, addrinfo &hint)
{
  switch (flags & (KExtendedSocket::streamSocket | KExtendedSocket::datagramSocket | KExtendedSocket::rawSocket))
    {
    case 0:
      /* No flags given, use default */

    case KExtendedSocket::streamSocket:
      /* streaming socket requested */
      hint.ai_socktype = SOCK_STREAM;
      break;

    case KExtendedSocket::datagramSocket:
      /* datagram packet socket requested */
      hint.ai_socktype = SOCK_DGRAM;
      break;

    case KExtendedSocket::rawSocket:
      /* raw socket requested. I wouldn't do this if I were you... */
      hint.ai_socktype = SOCK_RAW;
      break;

    default:
      /* the flags were used in an invalid manner */
      return false;
    }

  /* check other flags */
  hint.ai_flags |= (flags & KExtendedSocket::passiveSocket ? AI_PASSIVE : 0) |
    (flags & KExtendedSocket::canonName ? AI_CANONNAME : 0) |
    (flags & KExtendedSocket::noResolve ? AI_NUMERICHOST : 0);
  return true;
}

static bool valid_family(addrinfo *p, int flags)
{
  if (flags & KExtendedSocket::knownSocket)
    {
#ifdef PF_INET6
      if (p->ai_family == PF_INET6)
	{
	  if (flags & 0x0e && (flags & 0x4) == 0)
	    return false;	// user hasn't asked for Internet sockets
	  if (flags & 0xf00 && (flags & 0x200) == 0)
	    return false;	// user hasn't asked for IPv6 sockets
	}
#endif
      else if (p->ai_family == PF_INET)
	{
	  if (flags & 0x0e && (flags & 0x4) == 0)
	    return false;	// user hasn't asked for Internet sockets
	  if (flags & 0xf00 && (flags & 0x100) == 0)
	    return false;	// user hasn't asked for IPv4 sockets
	}
      else if (p->ai_family == PF_UNIX)
	{
	  if (flags & 0x0e && (flags & 0x2) == 0)
	    return false;	// user hasn't asked for Unix Sockets
	}
      if (p->ai_family != PF_INET && p->ai_family != PF_UNIX
#ifdef PF_INET6
	  && p->ai_family != PF_INET6
#endif
	  )
	return false;		// not a known socket

      // if we got here, the family is acceptable
    }
  return true;
}

static QString pretty_sock(addrinfo *p)
{
  KSocketAddress *sa;
  QString ret;

  sa = KSocketAddress::newAddress(p->ai_addr, p->ai_addrlen);
  if (sa == NULL)
    return QString::fromLocal8Bit("<invalid>");

  switch (p->ai_family)
    {
    case AF_UNIX:
      ret = QString::fromLocal8Bit("Unix ");
      break;

    case AF_INET:
      ret = QString::fromLocal8Bit("Inet ");
      break;

#ifdef AF_INET6
    case AF_INET6:
      ret = QString::fromLocal8Bit("Inet6 ");
      break;
#endif

    default:
      ret = QString::fromLocal8Bit("<unknown> ");
      break;
    }

  ret += sa->pretty();
  return ret;
}
      

/*
 * class KExtendedSocket
 */

// default constructor
KExtendedSocket::KExtendedSocket() :
  m_flags(0), m_status(0), m_syserror(0), sockfd(-1),
  d(new KExtendedSocketPrivate)
{
}

// constructor with hostname
KExtendedSocket::KExtendedSocket(const QString& host, int port, int flags) :
  m_flags(0), m_status(0), m_syserror(0), sockfd(-1),
  d(new KExtendedSocketPrivate)
{
  setAddress(host, port);
  setSocketFlags(flags);
}

// same
KExtendedSocket::KExtendedSocket(const QString& host, const QString& service, int flags) :
  m_flags(0), m_status(0), m_syserror(0), sockfd(-1),
  d(new KExtendedSocketPrivate)
{
  setAddress(host, service);
  setSocketFlags(flags);
}

// destroy the class
KExtendedSocket::~KExtendedSocket()
{
  close();

  if (d->resolution != NULL)
    freeaddrinfo(d->resolution);
  if (d->bindres != NULL)
    freeaddrinfo(d->bindres);
  d->resolution = d->bindres = NULL;

  if (d->local != NULL)
    delete d->local;
  if (d->peer != NULL)
    delete d->peer;

  delete d;
}

/*
 * Sets socket flags
 * This is only allowed if we are in nothing state
 */
int KExtendedSocket::setSocketFlags(int flags)
{
  if (m_status > nothing)
    return -1;			// error!

  return m_flags = flags;
}

/*
 * Sets socket target hostname
 * This is only allowed if we are in nothing state
 */
bool KExtendedSocket::setHost(const QString& host)
{
  if (m_status > nothing)
    return false;		// error!

  d->host = host;
  return true;
}

/*
 * returns the hostname
 */
QString KExtendedSocket::host() const
{ 
  return d->host; 
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
  if (m_status > nothing)
    return false;		// error

  d->service = service;
  return true;
}

/*
 * returns the service port number
 */
QString KExtendedSocket::port() const
{ 
  return d->service; 
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
 * This is only valid in the 'nothing' state and if this is a 
 * passiveSocket socket
 */
bool KExtendedSocket::setBindHost(const QString& host)
{
  if (m_status > nothing || m_flags & passiveSocket)
    return false;		// error

  d->localhost = host;
  return true;
}

/*
 * Unsets the bind hostname
 * same thing
 */
bool KExtendedSocket::unsetBindHost()
{
  if (m_status > nothing || m_flags & passiveSocket)
    return false;		// error

  d->localhost.truncate(0);
  return true;
}

/*
 * returns the binding host
 */
QString KExtendedSocket::bindHost() const
{
  return d->localhost;
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
  if (m_status > nothing || m_flags & passiveSocket)
    return false;		// error

  d->localservice = service;
  return true;
}

/*
 * unsets the bind port
 */
bool KExtendedSocket::unsetBindPort()
{
  if (m_status > nothing || m_flags & passiveSocket)
    return false;

  d->localservice.truncate(0);
  return true;
}

/*
 * returns the binding port
 */
QString KExtendedSocket::bindPort() const
{ 
  return d->localservice; 
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
  if (m_status >= connected)	// closed?
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
  if (m_status < created)
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

  if (fcntl(sockfd, F_SETFL, fdflags) != -1)
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
  if (m_status < created)
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
 * Finds the local address for this socket
 * if we have done this already, we return it. Otherwise, we'll have
 * to find the socket name
 */
KSocketAddress *KExtendedSocket::localAddress()
{
  if (d->local != NULL)
    return d->local;
  if (m_status < bound)
    return NULL;

  return d->local = localAddress(sockfd);
}

/*
 * Same thing, but for peer address. Which means this does not work on
 * passiveSocket and that we require to be connected already. Also note that
 * the behaviour on connectionless sockets is not defined here.
 */
KSocketAddress* KExtendedSocket::peerAddress()
{
  if (d->peer != NULL)
    return d->peer;
  if (m_flags & passiveSocket || m_status < connected)
    return NULL;

  return d->peer = peerAddress(sockfd);
}

/*
 * Perform the lookup on the addresses given
 */
int KExtendedSocket::lookup()
{
  cleanError();
  if (m_status > lookupDone)
    return EAI_BADFLAGS;	// we needed an error...

  addrinfo hint;

  memset(&hint, 0, sizeof(hint));
  hint.ai_family = AF_UNSPEC;

  // perform the global lookup before
  if (d->resolution == NULL)
    {
      /* check socket type flags */
      if (!process_flags(m_flags, hint))
	return EAI_BADFLAGS;

      kdDebug(170) << "Performing lookup on " << d->host << "/" << d->service << endl;
      int err = doLookup(d->host, d->service, hint, &d->resolution);
      if (err != 0)
	{
	  setError(IO_LookupError, err);
	  return err;
	}
    }

  if (d->bindres == NULL && (d->localhost.length() > 0 || d->localservice.length() > 0))
    {
      /* leave hint.ai_socktype the same */
      hint.ai_flags |= AI_PASSIVE;  // this is passive, for bind()

      kdDebug(170) << "Performing lookup on " << d->localhost << "/" << d->localservice << endl;
      int err = doLookup(d->localhost, d->localservice, hint, &d->bindres);
      if (err != 0)
	{
	  setError(IO_LookupError, err);
	  return err;
	}
    }

  m_status = lookupDone;
  return 0;
}

int KExtendedSocket::listen(int N)
{
  cleanError();
  if ((m_flags & passiveSocket) == 0 || m_status >= listening)
    return -2;
  if (m_status < lookupDone)
    if (lookup() < 0)
      return -2;		// error!

  addrinfo *p;

  // doing the loop:
  for (p = d->resolution; p; p = p->ai_next)
    {
      // check for family restriction
      if (!valid_family(p, m_flags))
	continue;

      kdDebug(170) << "Trying to listen on " << pretty_sock(p) << endl;
      sockfd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
      if (sockfd == -1)
	{
	  // socket failed creating
	  kdDebug(170) << "Failed to create: " << perror << endl;
	  continue;
	}

      if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
	{
	  kdDebug(170) << "Failed to bind: " << perror << endl;
	  ::close(sockfd);
	  sockfd = -1;
	  continue;
	}

      // ok, socket has bound
      kdDebug(170) << "Socket bound: " << sockfd << endl;
      break;
    }

  if (sockfd == -1)
    {
      setError(IO_ListenError, errno);
      return -1;
    }

  m_status = listening;
  setFlags(IO_Sequential | IO_Raw | IO_ReadWrite);

  int retval = ::listen(sockfd, N);
  if (retval == -1)
    setError(IO_ListenError, errno);
  return retval == -1 ? -1 : 0;
}

int KExtendedSocket::accept(KExtendedSocket *&sock)
{
  cleanError();
  sock = NULL;
  if ((m_flags & passiveSocket) == 0 || m_status >= accepting)
    return -2;
  if (m_status < listening)
    if (listen() < 0)
      return -2;		// error!
  
  // let's see
  // if we have a timeout in place, we have to place this socket in non-blocking
  // mode
  bool block = blockingMode();
  ksize_t len = 0;		// we don't really need the sockaddr now
  sock = NULL;

  if (d->timeout.tv_sec > 0 || d->timeout.tv_usec > 0)
    {
      fd_set set;

      setBlockingMode(false);	// turn on non-blocking
      FD_ZERO(&set);
      FD_SET(sockfd, &set);

      kdDebug(170).form("Accepting on %d with %d.%06d second timeout\n",
		     sockfd, d->timeout.tv_sec, d->timeout.tv_usec);
      // check if there is anything to accept now
      int retval = select(sockfd + 1, &set, NULL, NULL, &d->timeout);
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
  int newfd = ::accept(sockfd, NULL, &len);

  if (newfd == -1)
    {
      setError(IO_AcceptError, errno);
      kdWarning() << "Error accepting on socket " << sockfd << ":" 
		  << perror << endl;
      return -1;
    }

  kdDebug(170).form("Socket %d accepted socket %d\n", sockfd, newfd);

  setBlockingMode(block);	// restore blocking mode

  sock = new KExtendedSocket;
  sock->m_status = connected;
  sock->sockfd = newfd;
  sock->setFlags(IO_Sequential | IO_Raw | IO_ReadWrite | IO_Open);

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
  if (m_flags & passiveSocket)
    return -2;
  if (m_status < lookupDone)
    if (lookup() < 0)
      return -2;

  addrinfo *p, *q;
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
      kdDebug(170).form("Connection with timeout of %d.%06d seconds (ends in %d.%06d)\n",
		     d->timeout.tv_sec, d->timeout.tv_usec, end.tv_sec, end.tv_usec);
    }

  for (p = d->resolution, q = d->bindres; p; p = p->ai_next)
    {
      // check for family restriction
      if (!valid_family(p, m_flags))
	continue;

      kdDebug(170) << "Trying to connect to " << pretty_sock(p) << endl;
      if (q != NULL)
	{
	  kdDebug(170) << "Searching bind socket for family " << p->ai_family << endl;
	  if (q->ai_family != p->ai_family)
	    // differing families, scan bindres for a matching family
	    for (q = d->bindres; q; q = q->ai_next)
	      if (q->ai_family == p->ai_family)
		break;

	  if (q == NULL || q->ai_family != p->ai_family)
	    {
	      // no matching families for this
	      kdDebug(170) << "No matching family for bind socket\n";
	      q = d->bindres;
	      continue;
	    }

	  kdDebug(170) << "Binding on " << pretty_sock(q) << " before connect" << endl;
	  sockfd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
	  if (sockfd == -1)
	    continue;		// cannot create this socket
	  if (::bind(sockfd, q->ai_addr, q->ai_addrlen) == -1)
	    {
	      kdDebug(170) << "Bind failed: " << perror << endl;
	      ::close(sockfd);
	      sockfd = -1;
	      continue;
	    }
	}
      else
	{
	  // no need to bind, just create
	  sockfd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
	  if (sockfd == -1)
	    continue;
	}

      kdDebug(170) << "Socket " << sockfd << " created" << endl;
      m_status = created;

      // check if we have to do timeout
      if (doingtimeout)
	{
	  fd_set rd, wr;

	  setBlockingMode(false);

	  // now try and connect
	  if (::connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
	    {
	      // this could be EWOULDBLOCK
	      if (errno != EWOULDBLOCK && errno != EINPROGRESS)
		{
		  kdDebug(170) << "Socket " << sockfd << " did not connect: " << perror << endl;
		  ::close(sockfd);
		  sockfd = -1;
		  continue;	// nope, another error
		}

	      FD_ZERO(&rd);
	      FD_ZERO(&wr);
	      FD_SET(sockfd, &rd);
	      FD_SET(sockfd, &wr);

	      int retval = select(sockfd + 1, &rd, &wr, NULL, &d->timeout);
	      if (retval == -1)
		continue;	// system error
	      else if (retval == 0)
		{
		  ::close(sockfd);
		  sockfd = -1;
		  kdDebug(170) << "Time out while trying to connect to " << 
		    pretty_sock(p) << endl;
		  m_status = lookupDone;
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
	      kdDebug(170).form("Socket %d activity; %d.%06d seconds remaining\n",
			     sockfd, d->timeout.tv_sec, d->timeout.tv_usec);

	      // this means that an event occurred in the socket
	      int errcode;
	      ksize_t len = sizeof(errcode);
	      retval = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&errcode,
				  &len);
	      if (retval == -1 || errcode != 0)
		{
		  // socket did not connect
		  kdDebug(170) << "Socket " << sockfd << " did not connect: " 
			    << strerror(errcode) << endl;
		  ::close(sockfd);
		  sockfd = -1;

		  // this is HIGHLY UNLIKELY
		  if (d->timeout.tv_sec == 0 && d->timeout.tv_usec == 0)
		    {
		      m_status = lookupDone;
		      setError(IO_TimeOutError, 0);
		      return -3; // time out
		    }

		  continue;
		}
	    }

	  // getting here means it connected
	  setBlockingMode(true);
	  m_status = connected;
	  setFlags(IO_Sequential | IO_Raw | IO_ReadWrite | IO_Open);
	  kdDebug(170) << "Socket " << sockfd << " connected\n";
	  return 0;
	}
      else
	{
	  // without timeouts
	  if (::connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
	    {
	      kdDebug(170) << "Socket " << sockfd << " did not connect: " << perror << endl;
	      ::close(sockfd);
	      sockfd = -1;
	      continue;
	    }

	  m_status = connected;
	  setFlags(IO_Sequential | IO_Raw | IO_ReadWrite | IO_Open);
	  kdDebug(170) << "Socket " << sockfd << " connected\n";
	  return 0;		// it connected
	}
    }

  // getting here means no socket connected or stuff like that
  kdDebug(170) << "Failed to connect\n";
  setError(IO_ConnectError, errno);
  return -1;
}

bool KExtendedSocket::open(int mode)
{
  if (mode != IO_Raw | IO_ReadWrite)
    return false;		// invalid open mode

  if (m_flags & passiveSocket)
    return listen() == 0;
  else
    return connect() == 0;
}

void KExtendedSocket::close()
{
  // close the socket
  if (sockfd != -1)
    ::close(sockfd);

  if (m_flags & passiveSocket)
    {
      localAddress();
      if (d->local != NULL && d->local->data != NULL && d->local->data->sa_family == PF_UNIX &&
	  (m_flags & unixSocketKeep) == 0)
	unlink( ((KUnixSocketAddress*)d->local)->pathname() );
    }

  m_status = closed;
}

void KExtendedSocket::release()
{
  // release our hold on the socket
  sockfd = -1;
  m_status = closed;

  // also do some garbage collecting
  if (d->resolution != NULL)
    freeaddrinfo(d->resolution);
  if (d->bindres != NULL)
    freeaddrinfo(d->bindres);
  d->resolution = d->bindres = NULL;

  d->host = d->service = d->localhost = d->localservice = (const char *)0;

  if (d->local != NULL)
    delete d->local;
  if (d->peer != NULL)
    delete d->peer;

  d->peer = d->local = NULL;

  // don't delete d
  // leave that for the destructor
}

int KExtendedSocket::readBlock(char *data, uint maxlen)
{
  if (m_status < connected || m_flags & passiveSocket)
    return -2;
  if (sockfd == -1)
    return -2;

  int retval = ::read(sockfd, data, maxlen);
  if (retval == -1)
    setError(IO_ReadError, errno);
  return retval;
}

int KExtendedSocket::writeBlock(const char *data, uint len)
{
  if (m_status < connected || m_flags & passiveSocket)
    return -2;
  if (sockfd == -1)
    return -2;

  int retval = ::write(sockfd, data, len);
  if (retval == -1)
    setError(IO_WriteError, errno);
  return retval;
}

int KExtendedSocket::getch()
{
  char c;
  int retval;
  retval = readBlock(&c, sizeof(c));

  if (retval < 0)
    return retval;
  return c;
}

int KExtendedSocket::putch(int ch)
{
  char c = (char)ch;
  return writeBlock(&c, sizeof(ch));
}

void KExtendedSocket::setError(int errorcode, int syserror)
{
  setStatus(errorcode);
  m_syserror = syserror;
}

int KExtendedSocket::doLookup(const QString &host, const QString &serv, addrinfo &hint,
			      addrinfo** res)
{
  int err;

  // FIXME! What is the encoding?
  err = getaddrinfo(host.isNull() ? NULL : host.utf8(), serv.isNull() ? NULL : serv.utf8(),
		    &hint, res);
  return err;
}

int KExtendedSocket::resolve(sockaddr *sock, ksocklen_t len, QString &host, 
			     QString &port, int flags)
{
  int err;
  char h[NI_MAXHOST], s[NI_MAXSERV];

  h[0] = s[0] = '\0';

  err = getnameinfo(sock, len, h, sizeof(h) - 1, s, sizeof(s) - 1, flags);
  host = QString::fromUtf8(h);
  port = QString::fromUtf8(s);

  return err;
}

int KExtendedSocket::resolve(KSocketAddress *sock, QString &host, QString &port,
			     int flags)
{
  return resolve(sock->data, sock->datasize, host, port, flags);
}

QList<KAddressInfo> KExtendedSocket::lookup(const QString& host, const QString& port,
					    int flags)
{
  int err;
  addrinfo hint, *res, *p;
  QList<KAddressInfo> l;

  memset(&hint, 0, sizeof(hint));
  if (!process_flags(flags, hint))
    return l;

  kdDebug(170) << "Performing lookup on " << host << "|" << port << endl;
  err = doLookup(host, port, hint, &res);
  if (err)
    return l;

  for (p = res; p; p = p->ai_next)
    if (valid_family(p, flags))
      {
	KAddressInfo *ai = new KAddressInfo;
	ai->ai = new addrinfo;
	memcpy(ai->ai, p, sizeof(*p));
	ai->ai->ai_next = NULL;
	ai->addr = KSocketAddress::newAddress(p->ai_addr, p->ai_addrlen);
	kdDebug(170) << "Using socket " << pretty_sock(p) << endl;
	l.append(ai);
      }

  freeaddrinfo(res);
  return l;
}

KSocketAddress *KExtendedSocket::localAddress(int fd)
{
  sockaddr *sa = NULL;
  socklen_t len = 0;

  /* find out the socket length, in advance */
  if (getsockname(fd, sa, &len) == -1)
    return NULL;

  /* now malloc the socket */
  sa = (sockaddr*)malloc(len);
  if (sa == NULL)
    return NULL;

  if (getsockname(fd, sa, &len) == -1)
    {
      free(sa);
      return NULL;
    }

  KSocketAddress *local = KSocketAddress::newAddress(sa, len);
  free(sa);

  return local;
}

KSocketAddress *KExtendedSocket::peerAddress(int fd)
{

  sockaddr *sa = NULL;
  socklen_t len = 0;

  /* find out the socket length, in advance */
  if (getpeername(fd, sa, &len) == -1)
    return NULL;

  /* now malloc the socket */
  sa = (sockaddr*)malloc(len);
  if (sa == NULL)
    return NULL;

  if (getpeername(fd, sa, &len) == -1)
    {
      free(sa);
      return NULL;
    }

  KSocketAddress *peer = KSocketAddress::newAddress(sa, len);
  free(sa);

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

/*
 * class KAddressInfo
 */

KAddressInfo::~KAddressInfo()
{
  if (ai)
    delete ai;
  if (addr)
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

#include "kextsock.moc"
