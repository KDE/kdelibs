/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000,2001 Thiago Macieira <thiagom@mail.com>
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
 */
#ifndef KEXTSOCK_H
#define KEXTSOCK_H

#include <sys/time.h>

#include <qobject.h>
#include <qstring.h>
#include <qlist.h>
#include <qiodevice.h>

#include "ksockaddr.h"

/* External reference to netdb.h */
struct addrinfo;
class KAddressInfo;		/* our abstraction of it */

/*
 * This is extending QIODevice's error codes
 *
 * According to qiodevice.h, the last error is IO_UnspecifiedError
 */
#define IO_ListenError		(IO_UnspecifiedError+1)
#define IO_AcceptError		(IO_UnspecifiedError+2)
#define IO_LookupError		(IO_UnspecifiedError+3)

/**
 * The extended socket class.
 *
 * This class should be used instead of @ref KSocket whenever the user needs
 * fine-grained control over the socket being created. Unlike KSocket, which
 * does everything at once, without much intervention, KExtendedSocket allows
 * intervention at every step of the process and the setting of parameters.
 *
 * This class allows for the creation of both server and client sockets. The
 * only difference is that the passiveSocket flag must be passed either to
 * the constructor or to @ref setSocketFlags(). If passiveSocket is used, the class will
 * enable functions @ref listen() and @ref accept() and related signals, and will
 * also disable @ref readBlock() and @ref writeBlock().
 *
 * To create a Unix socket, one would pass flag unixSocket to the constructor
 * or @ref setSocketFlags(). The hostname and service/port can be set to whatever is 
 * necessary. If no hostname is given, but a service/port is, the socket created
 * will be implementation dependant (usually in /tmp). In any other case, the
 * fields will be concatenated.
 *
 * To create an Internet socket, inetSocket flag can be used. If, on the other
 * hand a specific IP protocol is desired, ipv4Socket and/or ipv6Socket can be
 * used.
 * 
 * Note that the socket type selection flags are cumulative. One could select
 * Unix and Internet sockets by using unixSocket | inetSocket. Or, for instance,
 * to make sure only IPv4 and IPv6 sockets are selected, even if future implementations
 * support newer IP protocols, ipv4Socket | ipv6Socket is your guy.
 *
 * @author Thiago Macieira <thiagom@mail.com>
 * @version $Id$
 * @short an extended socket
 */
class KExtendedSocket: public QObject, public QIODevice
{
  Q_OBJECT

public:
  /**
   * flags that can be passed down to the member functions
   */
  enum Flags
  {
    /* socket address families */
    /*
     * NOTE: if you change this, you have to change function valid_socket() as well
     * These values are hard coded!
     */
    anySocket = 0x00,
    knownSocket = 0x01,
    unixSocket = knownSocket | 0x02,
    inetSocket = knownSocket | 0x04,
    ipv4Socket = inetSocket | 0x100,
    ipv6Socket = inetSocket | 0x200,

    passiveSocket = 0x1000, /* passive socket (i.e., one that accepts connections) */
    canonName = 0x2000, /* request that the canon name be found */
    noResolve = 0x4000,	/* do not attempt to resolve, treat as numeric host */

    streamSocket = 0x8000, /* request a streaming socket (e.g., TCP) */
    datagramSocket = 0x10000, /* request a datagram socket (e.g., UDP) */
    rawSocket = 0x20000, /* request a raw socket. This probably requires privileges */

    unixSocketKeep = 0x100000 /* do not unlink() the unix socket, if one is created */
  };

  /**
   * status of the class
   * The status are sequential. If a change to one status is requested,
   * all the prior status will be passed and their actions, performed
   */
  enum SockStatus
  {
    error = -1,			// invalid status!

    nothing = 0,		// no status, the class has just been created

    lookupDone = 1,		// lookup has been done. Flags cannot be changed
				// from this point on

				// These two modes will never be seen on KExtendedSocket:
    created = 2,		// ::socket() has been called, a socket exists
    bound = 3,			// socket has been bound

    connecting = 4,		// socket is connecting (not passiveSocket)
    connected = 5,		// socket has connected (not passiveSocket)

    listening = 4,		// socket is listening (passiveSocket)
    accepting = 5,		// socket is accepting (passiveSocket)
    
    closed = 6			// socket has been closed
  };

public:
  /**
   * Creates an empty KExtendedSocket
   */
  KExtendedSocket();

  /**
   * Creates a socket with the given hostname and port
   * @param host	the hostname
   * @param port	the port number
   * @param flags	flags
   */
  KExtendedSocket(const QString& host, int port, int flags = 0);

  /**
   * Creates a socket with the given hostname and service
   * @param host	the hostname
   * @param serv	the service
   * @param flags	flags
   */
  KExtendedSocket(const QString& host, const QString& service, int flags = 0);

  /**
   * Destroys the socket, disconnecting if still connected and
   * freeing any related resources still being kept.
   */
  virtual ~KExtendedSocket();

  /*
   * --- status, flags and internal variables --- *
   */

  /**
   * Returns the class status
   */
  inline int socketStatus() const
  { return m_status; }

  /**
   * Returns the related system error code
   * Except for IO_LookupError errors, these are codes found in
   * errno
   */
  inline int systemError() const
  { return m_syserror; }

  /**
   * Sets the given flags. Will return the new flags status, or
   * -1 if flags can no longer be set.
   * @param flags	the flags to be set
   */
  int setSocketFlags(int flags);

  /**
   * Returns the current flags
   */
  inline int socketFlags() const
  { return m_flags; }

  /**
   * Sets the hostname to the given value
   * Returns true on success, false on error
   * @param host	the hostname
   */
  bool setHost(const QString& host);

  /**
   * Returns the hostname
   */
  QString host() const;

  /**
   * Sets the port/service
   * @param port	the port
   */
  bool setPort(int port);
  bool setPort(const QString& service);

  /**
   * Returns the port/service
   */
  QString port() const;

  /**
   * Sets the address where we will connect to
   * @param host	the hostname
   * @param port	port number
   */
  bool setAddress(const QString& host, int port);

  /**
   * Sets the address where we will connect to
   * @param host	the hostname
   * @param serv	the service
   */
  bool setAddress(const QString& host, const QString& serv);

  /**
   * Sets the hostname to which we will bind locally before connecting.
   * Returns false if this is a passiveSocket, otherwise true.
   * @param host	the hostname
   */
  bool setBindHost(const QString& host);

  /**
   * Unsets the bind hostname. That is, don't request a binding host.
   */
  bool unsetBindHost();

  /**
   * Returns the hostname to which the socket will be/is bound
   */
  inline QString bindHost() const;

  /**
   * Sets the port/service to which we will bind before connecting
   * @param port	the port number
   */
  bool setBindPort(int port);
  bool setBindPort(const QString& service);

  /**
   * Unsets the bind port/service.
   */
  bool unsetBindPort();

  /**
   * Returns the service to which the socket will be/is bound.
   */
  QString bindPort() const;

  /**
   * Sets both host and port to which we will bind the socket. Will return
   * -1 if this is a passiveSocket
   * @param host	the hostname
   * @param port	the port number
   */
  bool setBindAddress(const QString& host, int port);

  /**
   * Sets both host and service to which we will bind the socket. Will return
   * -1 if this is a passiveSocket
   * @param host	the hostname
   * @param serv	the service
   */
  bool setBindAddress(const QString& host, const QString& service);

  /**
   * Unsets the bind address for the socket. That means that we won't
   * attempt to bind to an address before connecting
   */
  bool unsetBindAddress();

  /**
   * Sets the timeout value for the connection, if this is not passiveSocket, or
   * acception, if this is a passiveSocket. In the event the given function
   * (connect or accept) returns due to time out, it's possible to call it again.
   * Setting the timeout to 0 disables the timeout feature.
   * Returns false if setting timeout makes no sense in the context.
   * @param secs	the timeout length, in seconds
   * @param usecs	the timeout complement, in microseconds
   */
  bool setTimeout(int secs, int usecs = 0);

  /**
   * Returns the timeout value for the connection
   */
  timeval timeout() const;

  /**
   * Sets/unsets blocking mode for the socket. When non-blocking mode is enabled,
   * I/O operations might return error and set errno to EWOULDBLOCK. Also,
   * it's not recommended to use this when using signals. Returns false on
   * error.
   * @param enable	if true, set blocking mode. False, non-blocking mode
   */
  bool setBlockingMode(bool enable);

  /**
   * Returns the current blocking mode for this socket
   */
  bool blockingMode();

  /**
   * Returns the local socket address
   */
  KSocketAddress *localAddress();

  /**
   * Returns the peer socket address. Use KExtendedSocket::resolve() to
   * resolve this to a human-readable hostname/service or port.
   */
  KSocketAddress *peerAddress();

  /**
   * Returns the file descriptor
   */
  inline int fd() const
  { return sockfd; }

  /*
   * -- socket creation -- *
   */

  /**
   * Performs lookup on the addresses we were given before
   * Returns 0 or an error. Codes are the same as for <em>getaddrinfo</em>
   * This will perform lookups on the bind addresses if they were given
   */
  virtual int lookup();

  /**
   * Place the socket in listen mode. The parameters are the same as for
   * the system listen() call. Returns 0 on success, -1 on system error (errno
   * available) and -2 if this is not a passiveSocket.
   * @param N		the queue length for pending connections
   */
  virtual int listen(int N = 5); // 5 is arbitrary

  /**
   * Accepts an incoming connection from the socket. If this socket is in
   * blocking mode, this function will block until a connection is received.
   * Otherwise, it might return with error. The sock parameter will be
   * initialised with the newly created socket.
   * Returns 0 on success, -1 on system error (errno set) and -2 if this is
   * not a passiveSocket and -3 if this took too long (time out)
   * @param sock	a pointer to an KExtendedSocket variable
   */
  virtual int accept(KExtendedSocket *&sock);

  /**
   * Attempts to connect to the remote host. The return values are:
   * 0: success
   * -1: system error, errno was set accordingly
   * -2: this socket cannot connect(); this is a passiveSocket. It can also
   *   mean that the function was unable to make a connection with the given
   *   bind address
   * -3: connection timed out
   */
  virtual int connect();

  /**
   * Implementation of QIODevice's open() pure virtual function.
   * This depends on the target host address already being there.
   * If this is a passiveSocket, this is identical to call listen(); else, if
   * this is not a passiveSocket, this is like connect().
   * @param mode	the open mode. Must be IO_Raw | IO_ReadWrite
   */
  virtual bool open(int mode = IO_Raw | IO_ReadWrite);

  /**
   * Closes the socket
   */
  virtual void close();

  /**
   * Releases the socket and anything we have holding on it. The class cannot 
   * be used anymore. In other words, this is just like close(), but it does 
   * not actually close the socket.
   * This is useful if you just want to connect and don't need the rest of the
   * class.
   */
  virtual void release();

  /*
   * -- I/O --
   */

  /**
   * Flushes the socket buffer. This socket is not buffered, so this does nothing
   */
  virtual inline void flush()
  { }

  /**
   * Returns length of this socket. This call is not supported on sockets
   */
  virtual inline uint size() const
  { return 0; }

  /**
   * Returns relative position from start. This call is not supported on sockets
   */
  virtual inline int at() const
  { return 0; }

  /**
   * Returns true if we are at position. This is not supported on sockets
   */
  virtual inline bool at(int)
  { return true; }

  /**
   * Returns true if we are at the end. This is not supported on sockets, but
   * we always are at the end in a socket...
   */
  virtual inline bool atEnd() const
  { return false; }

  /**
   * reads a block of data from the socket
   * @param data	where we will write the read data to
   * @param maxlen	maximum length of data to be read
   */
  virtual int readBlock(char *data, uint maxlen);

  /**
   * writes a block of data to the socket
   * @param data	the data to write
   * @param len		the length of data to write
   */
  virtual int writeBlock(const char *data, uint len);

  /**
   * gets a single character from the stream
   */
  virtual int getch();

  /**
   * writes a single character to the stream
   * @param ch		character to write, converted to char
   */
  virtual int putch(int ch);

  /**
   * unreads one character from the stream. This is not possible on sockets
   */
  virtual int ungetch(int)
  { return -1; }

protected:
  int m_flags;			// current flags
  int m_status;			// status
  int m_syserror;		// the system error code

  int sockfd;			// file descriptor of the socket

private:
  /* for the future */
  class KExtendedSocketPrivate;
  KExtendedSocketPrivate *d;

protected:

  /**
   * Sets the error code
   */
  void setError(int errorkind, int error);

  inline void cleanError()
  { setError(IO_Ok, 0); }

  /**
   * This is actually a wrapper around getaddrinfo()
   */
  static int doLookup(const QString& host, const QString& serv, addrinfo& hint,
		      addrinfo** result);

public:

  /**
   * Performs resolution on the given socket address
   * @param sockaddr	the socket address
   * @param host	where the hostname will be written
   * @param port	where the service-port will be written
   * @param flags	the same flags as getnameinfo()
   */
  static int resolve(sockaddr* sock, ksocklen_t len, QString& host, QString& port, int flags = 0);
  static int resolve(KSocketAddress* sock, QString& host, QString& port, int flags = 0);

  /**
   * Performs lookup on the given hostname/port combination and returns a list
   * of matching addresses.
   * The error code can be transformed into string by @ref KExtendedSocket::strError
   * with code of IO_LookupError
   * @param host	the hostname to look up
   * @param port	the port/service to look up
   * @param flags	flags to be used when looking up
   * @param error	pointer to a variable holding the error code
   */
  static QList<KAddressInfo> lookup(const QString& host, const QString& port, int flags = 0, int *error = 0);

  /**
   * Returns the local socket address
   * @param fd		the file descriptor
   */
  static KSocketAddress *localAddress(int fd);

  /**
   * Returns the peer socket address. Use KExtendedSocket::resolve() to
   * resolve this to a human-readable hostname/service or port.
   * @param fd		the file descriptor
   */
  static KSocketAddress *peerAddress(int fd);

  /**
   * Returns the representing text of this error code
   * @param code	the error code, as seen in status()
   * @param syserr	the system error, as from systemError()
   */
  static QString strError(int code, int syserr);
};

class KAddressInfo
{
private:
  addrinfo *ai;
  KSocketAddress *addr;

  inline KAddressInfo() : ai(0), addr(0)
  { }

public:
  ~KAddressInfo();

  inline KAddressInfo& operator=(KAddressInfo& kai)
  { ai = kai.ai; addr = kai.addr; return *this; }

  inline operator const KSocketAddress*() const
  { return addr; }

  inline operator const addrinfo&() const
  { return *ai; }

  inline const KSocketAddress* address() const
  { return addr; }

  int flags() const;
  int family() const;
  int socktype() const;
  int protocol() const;
  const char* canonname() const;

  inline int length() const
  { if (addr) return addr->size(); return 0; }

  friend class KExtendedSocket;
};

#endif // KEXTSOCK_H
