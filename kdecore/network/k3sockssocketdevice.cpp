/*  -*- C++ -*-
 *  Copyright (C) 2004 Thiago Macieira <thiago@kde.org>
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
 */

#include "k3sockssocketdevice.h"

#include <config.h>
#include <config-network.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif


#include <QCoreApplication>
#define I_KNOW_KSOCKS_ISNT_PUBLIC
#include "k3socks.h"
#undef I_KNOW_KSOCKS_ISNT_PUBLIC
#include "k3socketaddress.h"
#include "k3resolver.h"

using namespace KNetwork;

// constructor
// nothing to do
KSocksSocketDevice::KSocksSocketDevice(const KSocketBase* obj)
  : KSocketDevice(obj), d(0)
{
}

// constructor with argument
// nothing to do
KSocksSocketDevice::KSocksSocketDevice(int fd)
  : KSocketDevice(fd), d(0)
{
}

// destructor
// also nothing to do
KSocksSocketDevice::~KSocksSocketDevice()
{
}

// returns the capabilities
int KSocksSocketDevice::capabilities() const
{
  return 0;			// can do everything!
}

// From here on, the code is almost exactly a copy of KSocketDevice
// the differences are the use of KSocks where appropriate

bool KSocksSocketDevice::bind(const KResolverEntry& address)
{
  resetError();

  if (m_sockfd == -1 && !create(address))
    return false;		// failed creating

  // we have a socket, so try and bind
  if (KSocks::self()->bind(m_sockfd, address.address(), address.length()) == -1)
    {
      if (errno == EADDRINUSE)
	setError(AddressInUse);
      else if (errno == EINVAL)
	setError(AlreadyBound);
      else
	// assume the address is the cause
	setError(NotSupported);
      return false;
    }

  return true;
}


bool KSocksSocketDevice::listen(int backlog)
{
  if (m_sockfd != -1)
    {
      if (KSocks::self()->listen(m_sockfd, backlog) == -1)
	{
	  setError(NotSupported);
	  return false;
	}

      resetError();
      setOpenMode(ReadWrite | Unbuffered);
      return true;
    }

  // we don't have a socket
  // can't listen
  setError(NotCreated);
  return false;
}

bool KSocksSocketDevice::connect(const KResolverEntry& address)
{
  resetError();

  if (m_sockfd == -1 && !create(address))
    return false;		// failed creating!

  int retval;
  if (KSocks::self()->hasWorkingAsyncConnect())
    retval = KSocks::self()->connect(m_sockfd, address.address(),
				     address.length());
  else
    {
      // work around some SOCKS implementation bugs
      // we will do a *synchronous* connection here!
      // FIXME: KDE4, write a proper SOCKS implementation
      bool isBlocking = blocking();
      setBlocking(true);
      retval = KSocks::self()->connect(m_sockfd, address.address(),
				       address.length());
      setBlocking(isBlocking);
    }

  if (retval == -1)
    {
      if (errno == EISCONN)
	return true;		// we're already connected
      else if (errno == EALREADY || errno == EINPROGRESS)
	{
	  setError(InProgress);
	  return true;
	}
      else if (errno == ECONNREFUSED)
	setError(ConnectionRefused);
      else if (errno == ENETDOWN || errno == ENETUNREACH ||
	       errno == ENETRESET || errno == ECONNABORTED ||
	       errno == ECONNRESET || errno == EHOSTDOWN ||
	       errno == EHOSTUNREACH)
	setError(NetFailure);
      else
	setError(NotSupported);

      return false;
    }

  setOpenMode(ReadWrite | Unbuffered);
  return true;			// all is well
}

KSocksSocketDevice* KSocksSocketDevice::accept()
{
  if (m_sockfd == -1)
    {
      // can't accept without a socket
      setError(NotCreated);
      return 0L;
    }

  struct sockaddr sa;
  kde_socklen_t len = sizeof(sa);
  int newfd = KSocks::self()->accept(m_sockfd, &sa, &len);
  if (newfd == -1)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
	setError(WouldBlock);
      else
	setError(UnknownError);
      return NULL;
    }

  return new KSocksSocketDevice(newfd);
}

static int socks_read_common(int sockfd, char *data, quint64 maxlen, KSocketAddress* from, ssize_t &retval, bool peek = false)
{
  kde_socklen_t len;
  if (from)
    {
      from->setLength(len = 128); // arbitrary length
      retval = KSocks::self()->recvfrom(sockfd, data, maxlen, peek ? MSG_PEEK : 0, from->address(), &len);
    }
  else
    retval = KSocks::self()->recvfrom(sockfd, data, maxlen, peek ? MSG_PEEK : 0, NULL, NULL);

  if (retval == -1)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
	return KSocketDevice::WouldBlock;
      else
	return KSocketDevice::UnknownError;
    }

  if (from)
    from->setLength(len);
  return 0;
}

qint64 KSocksSocketDevice::readBlock(char *data, quint64 maxlen)
{
  resetError();
  if (m_sockfd == -1)
    return -1;

  if (maxlen == 0 || data == 0L)
    return 0;			// can't read

  ssize_t retval;
  int err = socks_read_common(m_sockfd, data, maxlen, 0L, retval);

  if (err)
    {
      setError(static_cast<SocketError>(err));
      return -1;
    }

  return retval;
}

qint64 KSocksSocketDevice::readBlock(char *data, quint64 maxlen, KSocketAddress &from)
{
  resetError();
  if (m_sockfd == -1)
    return -1;			// nothing to do here

  if (data == 0L || maxlen == 0)
    return 0;			// user doesn't want to read

  ssize_t retval;
  int err = socks_read_common(m_sockfd, data, maxlen, &from, retval);

  if (err)
    {
      setError(static_cast<SocketError>(err));
      return -1;
    }

  return retval;
}

qint64 KSocksSocketDevice::peekBlock(char *data, quint64 maxlen)
{
  resetError();
  if (m_sockfd == -1)
    return -1;

  if (maxlen == 0 || data == 0L)
    return 0;			// can't read

  ssize_t retval;
  int err = socks_read_common(m_sockfd, data, maxlen, 0L, retval, true);

  if (err)
    {
      setError(static_cast<SocketError>(err));
      return -1;
    }

  return retval;
}

qint64 KSocksSocketDevice::peekBlock(char *data, quint64 maxlen, KSocketAddress& from)
{
  resetError();
  if (m_sockfd == -1)
    return -1;			// nothing to do here

  if (data == 0L || maxlen == 0)
    return 0;			// user doesn't want to read

  ssize_t retval;
  int err = socks_read_common(m_sockfd, data, maxlen, &from, retval, true);

  if (err)
    {
      setError(static_cast<SocketError>(err));
      return -1;
    }

  return retval;
}

qint64 KSocksSocketDevice::writeBlock(const char *data, quint64 len)
{
  return writeBlock(data, len, KSocketAddress());
}

qint64 KSocksSocketDevice::writeBlock(const char *data, quint64 len, const KSocketAddress& to)
{
  resetError();
  if (m_sockfd == -1)
    return -1;			// can't write to unopen socket

  if (data == 0L || len == 0)
    return 0;			// nothing to be written

  ssize_t retval = KSocks::self()->sendto(m_sockfd, data, len, 0, to.address(), to.length());
  if (retval == -1)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
	setError(WouldBlock);
      else
	setError(UnknownError);
      return -1;		// nothing written
    }

  return retval;
}

KSocketAddress KSocksSocketDevice::localAddress() const
{
  if (m_sockfd == -1)
    return KSocketAddress();	// not open, empty value

  kde_socklen_t len;
  KSocketAddress localAddress;
  localAddress.setLength(len = 32);	// arbitrary value
  if (KSocks::self()->getsockname(m_sockfd, localAddress.address(), &len) == -1)
    // error!
    return KSocketAddress();

  if (len <= localAddress.length())
    {
      // it has fit already
      localAddress.setLength(len);
      return localAddress;
    }

  // no, the socket address is actually larger than we had anticipated
  // call again
  localAddress.setLength(len);
  if (KSocks::self()->getsockname(m_sockfd, localAddress.address(), &len) == -1)
    // error!
    return KSocketAddress();

  return localAddress;
}

KSocketAddress KSocksSocketDevice::peerAddress() const
{
  if (m_sockfd == -1)
    return KSocketAddress();	// not open, empty value

  kde_socklen_t len;
  KSocketAddress peerAddress;
  peerAddress.setLength(len = 32);	// arbitrary value
  if (KSocks::self()->getpeername(m_sockfd, peerAddress.address(), &len) == -1)
    // error!
    return KSocketAddress();

  if (len <= peerAddress.length())
    {
      // it has fit already
      peerAddress.setLength(len);
      return peerAddress;
    }

  // no, the socket address is actually larger than we had anticipated
  // call again
  peerAddress.setLength(len);
  if (KSocks::self()->getpeername(m_sockfd, peerAddress.address(), &len) == -1)
    // error!
    return KSocketAddress();

  return peerAddress;
}

KSocketAddress KSocksSocketDevice::externalAddress() const
{
  // return empty, indicating unknown external address
  return KSocketAddress();
}

bool KSocksSocketDevice::poll(bool *input, bool *output, bool *exception,
			      int timeout, bool *timedout)
{
  if (m_sockfd == -1)
    {
      setError(NotCreated);
      return false;
    }

  resetError();
  fd_set readfds, writefds, exceptfds;
  fd_set *preadfds = 0L, *pwritefds = 0L, *pexceptfds = 0L;

  if (input)
    {
      preadfds = &readfds;
      FD_ZERO(preadfds);
      FD_SET(m_sockfd, preadfds);
      *input = false;
    }
  if (output)
    {
      pwritefds = &writefds;
      FD_ZERO(pwritefds);
      FD_SET(m_sockfd, pwritefds);
      *output = false;
    }
  if (exception)
    {
      pexceptfds = &exceptfds;
      FD_ZERO(pexceptfds);
      FD_SET(m_sockfd, pexceptfds);
      *exception = false;
    }

  int retval;
  if (timeout < 0)
    retval = KSocks::self()->select(m_sockfd + 1, preadfds, pwritefds, pexceptfds, 0L);
  else
    {
      // convert the milliseconds to timeval
      struct timeval tv;
      tv.tv_sec = timeout / 1000;
      tv.tv_usec = timeout % 1000 * 1000;

      retval = select(m_sockfd + 1, preadfds, pwritefds, pexceptfds, &tv);
    }

  if (retval == -1)
    {
      setError(UnknownError);
      return false;
    }
  if (retval == 0)
    {
      // timeout
      if (timedout)
	*timedout = true;
      return true;
    }

  if (input && FD_ISSET(m_sockfd, preadfds))
    *input = true;
  if (output && FD_ISSET(m_sockfd, pwritefds))
    *output = true;
  if (exception && FD_ISSET(m_sockfd, pexceptfds))
    *exception = true;

  return true;
}

void KSocksSocketDevice::initSocks()
{
  static bool init = false;

  if (init)
    return;

  if (QCoreApplication::instance() == 0L)
    return;			// no KApplication, so don't initialize
                                // this should, however, test for KComponentData

  init = true;

  if (KSocks::self()->hasSocks())
    delete KSocketDevice::setDefaultImpl(new KSocketDeviceFactory<KSocksSocketDevice>);
}

#if 0
static bool register()
{
  KSocketDevice::addNewImpl(new KSocketDeviceFactory<KSocksSocketDevice>, 0);
}

static bool register = registered();
#endif
