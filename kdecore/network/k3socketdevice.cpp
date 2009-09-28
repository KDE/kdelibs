/*  -*- C++ -*-
 *  Copyright (C) 2003,2005 Thiago Macieira <thiago@kde.org>
 *
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

// syssocket.h needs to come before any header that includes k3socketbase.h
#include "syssocket.h"

#include "k3socketdevice.h" //krazy:exclude=includes (KDE3 compat: not worth fixing)

#include <config.h>
#include <config-network.h>

#include <QMap>

#ifdef HAVE_SYS_FILIO_H
# include <sys/filio.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>	// WARNING: verify if this is portable
#include <unistd.h>

#ifdef HAVE_POLL
# include <sys/poll.h>
#else
# ifdef HAVE_SYS_SELECT_H
#  include <sys/select.h>
# endif
#endif

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#include <QMutex>
#include <QSocketNotifier>

#include "k3resolver.h"
#include "k3socketaddress.h"
#include "k3socketbase.h"
#ifndef KDE_USE_FINAL
#include "k3socks.h"
#endif
using namespace KNetwork;

class KNetwork::KSocketDevicePrivate
{
public:
  mutable QSocketNotifier *input, *output, *exception;
  KSocketAddress local, peer;
  int af;
  int proto;

  inline KSocketDevicePrivate()
  {
    input = output = exception = 0L;
    af = proto = 0;
  }
};


KSocketDevice::KSocketDevice(const KSocketBase* parent, QObject* objparent)
  : KActiveSocketBase(objparent), m_sockfd(-1),
    d(new KSocketDevicePrivate)
{
  setSocketDevice(this);
  if (parent)
    setSocketOptions(parent->socketOptions());
}

KSocketDevice::KSocketDevice(int fd, OpenMode mode)
  : KActiveSocketBase(0L), m_sockfd(fd), d(new KSocketDevicePrivate)
{
  if (mode)
    mode |= Unbuffered;
  KActiveSocketBase::open(mode);
  setSocketDevice(this);
  d->af = localAddress().family();
}

KSocketDevice::KSocketDevice(QObject* parent)
  : KActiveSocketBase(parent), m_sockfd(-1), d(new KSocketDevicePrivate)
{
  setSocketDevice(this);
}

KSocketDevice::KSocketDevice(bool, const KSocketBase* parent)
  : KActiveSocketBase(0L), m_sockfd(-1), d(new KSocketDevicePrivate)
{
  // do not set parent
  if (parent)
    setSocketOptions(parent->socketOptions());
}

KSocketDevice::~KSocketDevice()
{
  close();			// deletes the notifiers
  unsetSocketDevice(); 		// prevent double deletion
  delete d;
}

int KSocketDevice::socket() const
{
    return m_sockfd;
}

int KSocketDevice::capabilities() const
{
    return 0;
}

bool KSocketDevice::setSocketOptions(int opts)
{
  // must call parent
  QMutexLocker locker(mutex());
  KSocketBase::setSocketOptions(opts);

  if (m_sockfd == -1)
    return true;		// flags are stored

#ifdef Q_WS_WIN
  u_long iMode = ((opts & Blocking) == Blocking) ? 0 : 1;
  // disable non blocking
  if (ioctlsocket(m_sockfd, FIONBIO, &iMode) == SOCKET_ERROR)
  {
    // socket can't made blocking because WSAAsyncSelect/WSAEventSelect (==QSocketNotifier)
    // is activated for them
    if(WSAGetLastError() == WSAEINVAL)
      return true;
    qDebug("socket set %s failed %d", iMode ? "nonblocking" : "blocking", GetLastError());
    setError(UnknownError);
    return false;  // error
  }

#else
    {
      int fdflags = fcntl(m_sockfd, F_GETFL, 0);
      if (fdflags == -1)
	{
	  setError(UnknownError);
	  return false;		// error
	}

      if (opts & Blocking)
	fdflags &= ~O_NONBLOCK;
      else
	fdflags |= O_NONBLOCK;

      if (fcntl(m_sockfd, F_SETFL, fdflags) == -1)
	{
	  setError(UnknownError);
	  return false;		// error
	}
    }
#endif

    {
      int on = opts & AddressReuseable ? 1 : 0;
      if (setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) == -1)
	{
	  setError(UnknownError);
	  return false;		// error
	}
    }

#if defined(IPV6_V6ONLY) && defined(AF_INET6)
  if (d->af == AF_INET6)
    {
      // don't try this on non-IPv6 sockets, or we'll get an error

      int on = opts & IPv6Only ? 1 : 0;
      if (setsockopt(m_sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&on, sizeof(on)) == -1)
	{
	  setError(UnknownError);
	  return false;		// error
	}
    }
#endif

   {
     int on = opts & Broadcast ? 1 : 0;
     if (setsockopt(m_sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&on, sizeof(on)) == -1)
       {
	 setError(UnknownError);
	 return false;		// error
       }
   }

  if ((d->proto == IPPROTO_TCP || d->proto == 0) &&
      (d->af == AF_INET
#if defined(AF_INET6)
       || d->af == AF_INET6
#endif
       ))
   {
     int on = opts & NoDelay ? 1 : 0;
     if (setsockopt(m_sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&on, sizeof(on)) == -1)
       {
	 setError(UnknownError);
	 return false;		// error
       }
   }

  return true;			// all went well
}

void KSocketDevice::close()
{
  resetError();
  if (m_sockfd != -1)
    {
      delete d->input;
      delete d->output;
      delete d->exception;

      d->input = d->output = d->exception = 0L;
#ifdef Q_WS_WIN
      ::closesocket(m_sockfd);
#else
      d->local.setFamily(AF_UNSPEC);
      d->peer.setFamily(AF_UNSPEC);

      ::close(m_sockfd);
#endif
    }
  setOpenMode(0);		// closed

  m_sockfd = -1;
}

bool KSocketDevice::flush()
{
    return false;
}

bool KSocketDevice::create(int family, int type, int protocol)
{
  resetError();

  if (m_sockfd != -1)
    {
      // it's already created!
      setError(AlreadyCreated);
      return false;
    }

  // no socket yet; we have to create it
  m_sockfd = kde_socket(family, type, protocol);

  if (m_sockfd == -1)
    {
      setError(NotSupported);
      return false;
    }

  d->af = family;
  d->proto = protocol;
  setSocketOptions(socketOptions());
  setOpenMode(Unbuffered);	// there's no "Open" flag
  return true;		// successfully created
}

bool KSocketDevice::create(const KResolverEntry& address)
{
  return create(address.family(), address.socketType(), address.protocol());
}

bool KSocketDevice::bind(const KResolverEntry& address)
{
  resetError();

  if (m_sockfd == -1 && !create(address))
    return false;		// failed creating

  // we have a socket, so try and bind
  if (kde_bind(m_sockfd, address.address(), address.length()) == -1)
    {
      if (errno == EADDRINUSE)
      {
	setError(AddressInUse);
	return false;
      }
      else if (errno == EINVAL)
	setError(AlreadyBound);
      else 
       {
#ifdef Q_WS_WIN
         qDebug(" bind failed: %s ",address.address().toString().toLatin1().constData());
#endif
         // assume the address is the cause
         setError(NotSupported);
         return false;
       } 
    }

  return true;
}

bool KSocketDevice::listen(int backlog)
{
  if (m_sockfd != -1)
    {
      if (kde_listen(m_sockfd, backlog) == -1)
	{
	  setError(NotSupported);
	  return false;
	}

      resetError();
      setOpenMode(QIODevice::Unbuffered | QIODevice::ReadWrite);
      return true;
    }

  // we don't have a socket
  // can't listen
  setError(NotCreated);
  return false;
}

bool KSocketDevice::connect(const KResolverEntry& address, OpenMode mode)
{
  resetError();

  if (m_sockfd == -1 && !create(address))
    return false;		// failed creating!

  if (kde_connect(m_sockfd, address.address(), address.length()) == -1)
    {
      if (errno == EISCONN)
	{
	  KActiveSocketBase::open(Unbuffered | mode);
	  return true;		// we're already connected
	}
      else if (errno == EALREADY || errno == EINPROGRESS)
	{
	  KActiveSocketBase::open(Unbuffered | mode);
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

  KActiveSocketBase::open(Unbuffered | mode);
  return true;			// all is well
}

KSocketDevice* KSocketDevice::accept()
{
  if (m_sockfd == -1)
    {
      // can't accept without a socket
      setError(NotCreated);
      return 0L;
    }

  struct sockaddr sa;
  socklen_t len = sizeof(sa);
  int newfd = kde_accept(m_sockfd, &sa, &len);
  if (newfd == -1)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
	setError(WouldBlock);
      else
	setError(UnknownError);
      return NULL;
    }

  return new KSocketDevice(newfd);
}

bool KSocketDevice::disconnect()
{
  resetError();

  if (m_sockfd == -1)
    return false;		// can't create

  KSocketAddress address;
  address.setFamily(AF_UNSPEC);
  if (kde_connect(m_sockfd, address.address(), address.length()) == -1)
    {
      if (errno == EALREADY || errno == EINPROGRESS)
	{
	  setError(InProgress);
	  return false;
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

  setOpenMode(QIODevice::Unbuffered | QIODevice::ReadWrite);
  return true;			// all is well
}

qint64 KSocketDevice::bytesAvailable() const
{
  if (m_sockfd == -1)
    return -1;			// there's nothing to read in a closed socket

  int nchars;
  if (kde_ioctl(m_sockfd, FIONREAD, &nchars) == -1)
    return -1;			// error!

  return nchars;
}

qint64 KSocketDevice::waitForMore(int msecs, bool *timeout)
{
  if (m_sockfd == -1)
    return -1;			// there won't ever be anything to read...

  bool input;
  if (!poll(&input, 0, 0, msecs, timeout))
    return -1;			// failed polling

  return bytesAvailable();
}

static int do_read_common(int sockfd, char *data, qint64 maxlen, KSocketAddress* from, ssize_t &retval, bool peek = false)
{
  socklen_t len;
  if (from)
    {
      from->setLength(len = 128); // arbitrary length
      retval = ::recvfrom(sockfd, data, maxlen, peek ? MSG_PEEK : 0, from->address(), &len);
    }
  else
    retval = ::recvfrom(sockfd, data, maxlen, peek ? MSG_PEEK : 0, NULL, NULL);

  if (retval == -1)
    {
#ifdef Q_WS_WIN
     if (WSAGetLastError() == WSAEWOULDBLOCK )
	return KSocketDevice::WouldBlock;
	   else 
#endif
      if (errno == EAGAIN || errno == EWOULDBLOCK )
	return KSocketDevice::WouldBlock;
      else
	return KSocketDevice::UnknownError;
    }
  if (retval == 0)
    return KSocketDevice::RemotelyDisconnected;

  if (from)
    from->setLength(len);
  return 0;
}

qint64 KSocketDevice::readData(char *data, qint64 maxlen, KSocketAddress *from)
{
  resetError();
  if (m_sockfd == -1)
    return -1;			// nothing to do here

  if (data == 0L || maxlen == 0)
    return 0;			// user doesn't want to read

  ssize_t retval;
  int err = do_read_common(m_sockfd, data, maxlen, from, retval);

  if (err)
    {
      setError(static_cast<SocketError>(err));
      return -1;
    }

  return retval;
}

qint64 KSocketDevice::peekData(char *data, qint64 maxlen, KSocketAddress* from)
{
  resetError();
  if (m_sockfd == -1)
    return -1;			// nothing to do here

  if (data == 0L || maxlen == 0)
    return 0;			// user doesn't want to read

  ssize_t retval;
  int err = do_read_common(m_sockfd, data, maxlen, from, retval, true);

  if (err)
    {
      setError(static_cast<SocketError>(err));
      return -1;
    }

  return retval;
}

qint64 KSocketDevice::writeData(const char *data, qint64 len, const KSocketAddress* to)
{
  resetError();
  if (m_sockfd == -1)
    return -1;			// can't write to unopen socket

  if (data == 0L || len == 0)
    return 0;			// nothing to be written

  ssize_t retval;
  if (to != 0L)
    retval = ::sendto(m_sockfd, data, len, 0, to->address(), to->length());
  else
#ifdef Q_WS_WIN
    retval = ::send(m_sockfd, data, len, 0);
#else
    retval = ::write(m_sockfd, data, len);
#endif
  if (retval == -1)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
	setError(WouldBlock);
      else
	setError(UnknownError);
      return -1;		// nothing written
    }
  else if (retval == 0)
    setError(RemotelyDisconnected);

  return retval;
}

KSocketAddress KSocketDevice::localAddress() const
{
  if (m_sockfd == -1)
    return KSocketAddress();	// not open, empty value

  if (d->local.family() != AF_UNSPEC)
    return d->local;

  socklen_t len;
  KSocketAddress localAddress;
  localAddress.setLength(len = 32);	// arbitrary value
  if (kde_getsockname(m_sockfd, localAddress.address(), &len) == -1)
    // error!
    return d->local = KSocketAddress();

#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
  len = localAddress.address()->sa_len;
#endif

  if (len <= localAddress.length())
    {
      // it has fit already
      localAddress.setLength(len);
      return d->local = localAddress;
    }

  // no, the socket address is actually larger than we had anticipated
  // call again
  localAddress.setLength(len);
  if (kde_getsockname(m_sockfd, localAddress.address(), &len) == -1)
    // error!
    return d->local = KSocketAddress();

  return d->local = localAddress;
}

KSocketAddress KSocketDevice::peerAddress() const
{
  if (m_sockfd == -1)
    return KSocketAddress();	// not open, empty value

  if (d->peer.family() != AF_UNSPEC)
    return d->peer;

  socklen_t len;
  KSocketAddress peerAddress;
  peerAddress.setLength(len = 32);	// arbitrary value
  if (kde_getpeername(m_sockfd, peerAddress.address(), &len) == -1)
    // error!
    return d->peer = KSocketAddress();

#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
  len = peerAddress.address()->sa_len;
#endif

  if (len <= peerAddress.length())
    {
      // it has fit already
      peerAddress.setLength(len);
      return d->peer = peerAddress;
    }

  // no, the socket address is actually larger than we had anticipated
  // call again
  peerAddress.setLength(len);
  if (kde_getpeername(m_sockfd, peerAddress.address(), &len) == -1)
    // error!
    return d->peer = KSocketAddress();

  return d->peer = peerAddress;
}

KSocketAddress KSocketDevice::externalAddress() const
{
  // for normal sockets, the externally visible address is the same
  // as the local address
  return localAddress();
}

QSocketNotifier* KSocketDevice::readNotifier() const
{
  if (d->input)
    return d->input;

  QMutexLocker locker(mutex());
  if (d->input)
    return d->input;

  if (m_sockfd == -1)
    {
      // socket doesn't exist; can't create notifier
      return 0L;
    }

  return d->input = createNotifier(QSocketNotifier::Read);
}

QSocketNotifier* KSocketDevice::writeNotifier() const
{
  if (d->output)
    return d->output;

  QMutexLocker locker(mutex());
  if (d->output)
    return d->output;

  if (m_sockfd == -1)
    {
      // socket doesn't exist; can't create notifier
      return 0L;
    }

  return d->output = createNotifier(QSocketNotifier::Write);
}

QSocketNotifier* KSocketDevice::exceptionNotifier() const
{
  if (d->exception)
    return d->exception;

  QMutexLocker locker(mutex());
  if (d->exception)
    return d->exception;

  if (m_sockfd == -1)
    {
      // socket doesn't exist; can't create notifier
      return 0L;
    }

  return d->exception = createNotifier(QSocketNotifier::Exception);
}

bool KSocketDevice::poll(bool *input, bool *output, bool *exception,
			 int timeout, bool* timedout)
{
  if (m_sockfd == -1)
    {
      setError(NotCreated);
      return false;
    }

  resetError();
#ifdef HAVE_POLL
  struct pollfd fds;
  fds.fd = m_sockfd;
  fds.events = 0;

  if (input)
    {
      fds.events |= POLLIN;
      *input = false;
    }
  if (output)
    {
      fds.events |= POLLOUT;
      *output = false;
    }
  if (exception)
    {
      fds.events |= POLLPRI;
      *exception = false;
    }

  int retval = ::poll(&fds, 1, timeout);
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

  if (input && fds.revents & POLLIN)
    *input = true;
  if (output && fds.revents & POLLOUT)
    *output = true;
  if (exception && fds.revents & POLLPRI)
    *exception = true;
  if (timedout)
    *timedout = false;

  return true;
#else
  /*
   * We don't have poll(2). We'll have to make do with select(2).
   */

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
    retval = select(m_sockfd + 1, preadfds, pwritefds, pexceptfds, 0L);
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
#endif
}

bool KSocketDevice::poll(int timeout, bool *timedout)
{
  bool input, output, exception;
  return poll(&input, &output, &exception, timeout, timedout);
}

QSocketNotifier* KSocketDevice::createNotifier(QSocketNotifier::Type type) const
{
  if (m_sockfd == -1)
    return 0L;

  return new QSocketNotifier(m_sockfd, type);
}

namespace
{
  // simple class to avoid pointer stuff
  template<class T> class ptr
  {
    typedef T type;
    type* obj;
  public:
    ptr() : obj(0)
    { }

    ptr(const ptr<T>& other) : obj(other.obj)
    { }

    ptr(type* _obj) : obj(_obj)
    { }

    ~ptr()
    { }

    ptr<T>& operator=(const ptr<T>& other)
    { obj = other.obj; return *this; }

    ptr<T>& operator=(T* _obj)
    { obj = _obj; return  *this; }

    type* operator->() const { return obj; }

    operator T*() const { return obj; }

    bool isNull() const
    { return obj == 0; }
  };
}

static KSocketDeviceFactoryBase* defaultImplFactory;
static QMutex defaultImplFactoryMutex;
typedef QMap<int, KSocketDeviceFactoryBase* > factoryMap;
static factoryMap factories;

KSocketDevice* KSocketDevice::createDefault(KSocketBase* parent)
{
  KSocketDevice* device = dynamic_cast<KSocketDevice*>(parent);
  if (device != 0L)
    return device;

  if (defaultImplFactory)
    return defaultImplFactory->create(parent);

  // the really default
  return new KSocketDevice(parent);
}

KSocketDevice* KSocketDevice::createDefault(KSocketBase* parent, int capabilities)
{
  KSocketDevice* device = dynamic_cast<KSocketDevice*>(parent);
  if (device != 0L)
    return device;

  QMutexLocker locker(&defaultImplFactoryMutex);
  factoryMap::ConstIterator it = factories.constBegin();
  for ( ; it != factories.constEnd(); ++it)
    if ((it.key() & capabilities) == capabilities)
      // found a match
      return it.value()->create(parent);

  return 0L;			// no default
}

KSocketDeviceFactoryBase*
KSocketDevice::setDefaultImpl(KSocketDeviceFactoryBase* factory)
{
  QMutexLocker locker(&defaultImplFactoryMutex);
  KSocketDeviceFactoryBase* old = defaultImplFactory;
  defaultImplFactory = factory;
  return old;
}

void KSocketDevice::addNewImpl(KSocketDeviceFactoryBase* factory, int capabilities)
{
  QMutexLocker locker(&defaultImplFactoryMutex);
  if (factories.contains(capabilities))
    delete factories[capabilities];
  factories.insert(capabilities, factory);
}

