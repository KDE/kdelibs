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
 */
#ifndef KSOCKADDR_H
#define KSOCKADDR_H

#include <qobject.h>
#include <qcstring.h>
#include <qstring.h>

/*
 * This file defines a class that envelopes most, if not all, socket addresses
 */
typedef unsigned ksocklen_t;

struct sockaddr;

class KExtendedSocket;		// No need to define it fully

/**
 * A socket address.
 *
 * This class envelopes almost if not all socket addresses.
 *
 * @author Thiago Macieira <thiagom@mail.com>
 * @version $Id$
 * @short a socket address.
 */
class KSocketAddress: public QObject
{
  Q_OBJECT
protected:
  /**
   * Creates an empty class
   */
  KSocketAddress();

  /**
   * Creates with given data
   */
  KSocketAddress(const sockaddr* sa, ksocklen_t size);

public:
  /**
   * Destructor.
   */
  virtual ~KSocketAddress();

  /**
   * Returns a string representation of this socket
   */
  virtual QString pretty() const;

  /**
   * Returns a sockaddr structure, for passing down to library functions
   */
  const sockaddr* address() const
  { return data; }

  /**
   * Returns sockaddr structure size
   */
  virtual ksocklen_t size() const
  { return datasize; }

  /**
   * Returns a sockaddr structure, for passing down to library functions
   */
  operator const sockaddr*() const
  { return data; }

  /**
   * Returns the family of this address
   * AF_UNSPEC if it's undefined
   */
  int family() const;

  /**
   * Returns the IANA family number of this address
   */
  inline int ianaFamily() const
  { return ianaFamily(family()); }

  /**
   * Returns true if this equals the other socket
   * @param other	the other socket
   */
  virtual bool isEqual(const KSocketAddress& other) const;
  bool isEqual(const KSocketAddress* other) const
  { return isEqual(*other); }

  bool operator==(const KSocketAddress& other) const
  { return isEqual(other); }

  /**
   * Some sockets may differ in such things as services or port numbers,
   * like Internet sockets. This function compares only the core part
   * of that, if possible.
   *
   * If not possible, like the default implementation, this returns
   * the same as isEqual.
   * @param other	the other socket
   */
  virtual bool isCoreEqual(const KSocketAddress& other) const;
  bool isCoreEqual(const KSocketAddress* other) const
  { return isCoreEqual(*other); }

  /**
   * Returns the node name of this socket, as @ref KExtendedSocket::lookup expects
   * as the first argument.
   * In the case of Internet sockets, this is the hostname.
   * The default implementation returns QString::null.
   */
  virtual QString nodeName() const;

  /**
   * Returns the service name for this socket, as @ref KExtendedSocket::lookup expects
   * as the service argument.
   * In the case of Internet sockets, this is the port number.
   * The default implementation returns QString::null.
   */
  virtual QString serviceName() const;

protected:
  sockaddr*	data;
  ksocklen_t	datasize;
  bool		owndata;

private:
  /* No copy constructor */
  KSocketAddress(KSocketAddress&);
  KSocketAddress& operator=(KSocketAddress&);

public:
  /**
   * Creates a new KSocketAddress or descendant class from given
   * raw socket address
   * @param sa		new socket address
   * @param size	new socket address's length
   */
  static KSocketAddress* newAddress(const struct sockaddr*, ksocklen_t size);

  /**
   * Returns the IANA family number of the given address family
   * returns 0 if there is no corresponding IANA family number
   * @param af		the address family, in AF_* constants
   */
  static int ianaFamily(int af);

  /**
   * Returns the address family of the given IANA family number
   * returns AF_UNSPEC for unknown IANA family numbers
   */
  static int fromIanaFamily(int iana);

  friend class KExtendedSocket;
};

/*
 * External definitions
 * We need these for KInetSocketAddress
 */
struct sockaddr_in;
struct sockaddr_in6;
struct in_addr;
struct in6_addr;

/**
 * An Inet (IPv4 or IPv6) socket address
 *
 * This is an IPv4 or IPv6 address of the Internet
 *
 * This class inherits most of the functionality from @ref KSocketAddress, but
 * is targeted specifically to Internet addresses
 *
 * @author Thiago Macieira <thiagom@mail.com>
 * @version $Id$
 * @short an Internet socket address
 */
class KInetSocketAddress: public KSocketAddress
{
  Q_OBJECT
public:
  /**
   * Default constructor. Does nothing
   */
  KInetSocketAddress();

  /**
   * Copy constructor
   */
  KInetSocketAddress(const KInetSocketAddress&);

  /**
   * Creates an IPv4 socket from raw sockaddr_in
   * @param sin		a sockaddr_in structure to copy from
   */
  KInetSocketAddress(const sockaddr_in* sin, ksocklen_t len);

  /**
   * Creates an IPv6 socket from raw sockaddr_in6
   * @param sin6       	a sockaddr_in6 structure to copy from
   */
  KInetSocketAddress(const sockaddr_in6* sin6, ksocklen_t len);

  /**
   * Creates a socket from information
   * @param addr	a binary address
   * @param port	a port number
   */
  KInetSocketAddress(const in_addr& addr, unsigned short port);

  /**
   * Creates a socket from information
   * @param addr	a binary address
   * @param port	a port number
   */
  KInetSocketAddress(const in6_addr& addr, unsigned short port);

  /**
   * Creates a socket from text representation
   * @param addr	a text representation of the address
   * @param port	a port number
   */
  KInetSocketAddress(const QString& addr, unsigned short port, int family = -1);

  /**
   * Destructor
   */
  virtual ~KInetSocketAddress();

  /**
   * Sets this socket to given socket
   * @param ksa		the other socket
   */
  bool setAddress(const KInetSocketAddress& ksa);

  /**
   * Sets this socket to given raw socket
   * @param sin		the raw socket
   */
  bool setAddress(const sockaddr_in* sin, ksocklen_t len);

  /**
   * Sets this socket to given raw socket
   * @param sin6		the raw socket
   */
  bool setAddress(const sockaddr_in6* sin6, ksocklen_t len);

  /**
   * Sets this socket to raw address and port
   * @param addr	the address
   * @param port	the port number
   */
  bool setAddress(const in_addr& addr, unsigned short port);

  /**
   * Sets this socket to raw address and port
   * @param addr	the address
   * @param port	the port number
   */
  bool setAddress(const in6_addr& addr, unsigned short port);

  /**
   * Sets this socket to text address and port
   * @param addr	the address
   * @param port	the port number
   * @param family	the address family, -1 for any
   */
  bool setAddress(const QString& addr, unsigned short port, int family = -1);

  /**
   * Sets this socket's host address to given raw address
   * @param addr	the address
   */
  bool setHost(const in_addr& addr);

  /**
   * Sets this socket's host address to given raw address
   * @param addr	the address
   */
  bool setHost(const in6_addr& addr);

  /**
   * Sets this socket's host address to given text representation
   * @param addr	the address
   * @param family	the address family, -1 for any
   */
  bool setHost(const QString& addr, int family = -1);

  /**
   * Sets this socket's port number to given port number
   * @param port	the port number
   */
  bool setPort(unsigned short port);

  /**
   * Turns this into an IPv4 or IPv6 address
   * returns 0 if this is v6 and information was lost
   */
  bool setFamily(int family);

  /**
   * Sets flowinfo information for this socket address if this is IPv6
   * @param flowinfo	flowinfo
   */
  bool setFlowinfo(Q_UINT32 flowinfo);

  /**
   * Sets the scope id for this socket if this is IPv6
   * @param scopeid	the scope id
   */
  bool setScopeId(int scopeid);

  /**
   * Returns a pretty representation of this address
   */
  virtual QString pretty() const;

  /**
   * Returns the text representation of the host address
   */
  virtual QString nodeName() const;
  //  QString prettyHost() const;

  /**
   * Returns the text representation of the port number
   */
  virtual QString serviceName() const;

  /**
   * Returns the socket address
   * This will be NULL if this is a non-convertible v6
   */
  const sockaddr_in* addressV4() const;

  /**
   * Returns the socket address in IPv6
   */
  const sockaddr_in6* addressV6() const;

  /**
   * Returns the host address
   * Might be empty
   */
  in_addr hostV4() const;

  /**
   * Returns the host address
   * WARNING: this function is not defined if there is no IPv6 support
   */
  in6_addr hostV6() const;

  /**
   * Returns the port number
   */
  unsigned short port() const;

  /**
   * Returns flowinfo for IPv6 socket
   */
  Q_UINT32 flowinfo() const;

  /**
   * Returns the scope id for this IPv6 socket
   */
  int scopeId() const;

  /**
   * Returns the socket length
   * Will be either sizeof(sockaddr_in) or sizeof(sockaddr_in6)
   */
  virtual ksocklen_t size() const; // should be socklen_t

  /* comparation */
  static bool areEqualInet(const KSocketAddress &s1, const KSocketAddress &s2, bool coreOnly);
  static bool areEqualInet6(const KSocketAddress &s1, const KSocketAddress &s2, bool coreOnly);

  /**
   * @obsolete KDE 3.0: To be removed
   * This kind of socket supports core comparation. This will
   * compare the IP part of the sockets only.
   * @param other	the other socket
   */
  virtual bool isCoreEqual(const KSocketAddress& other) const;

  /* operators */

  /**
   * Returns the socket address
   * This will be NULL if this is a non-convertible v6
   */
  operator const sockaddr_in*() const
  { return addressV4(); }

  operator const sockaddr_in6*() const
  { return addressV6(); }

  /**
   * Sets this object to be the same as the other
   */
  KInetSocketAddress& operator=(const KInetSocketAddress &other)
  { setAddress(other); return *this; }

private:
  class Private;
  Private *d;

  void fromV4();
  void fromV6();

public:
  /**
   * Convert s the given raw address into text form.
   * This function returns QString::null if the address cannot be converted.
   * @param family	the family of the address
   * @param addr	the address, in raw form
   */
  static QString addrToString(int family, const void *addr);

  /**
   * Converts the address given in text form into raw form.
   * The size of the destination buffer @p dest is supposed to be
   * large enough to hold the address of the given family.
   * This function returns true if convertion was successful.
   * @param family	the family of the address
   * @param text	the text representation of the address
   * @param dest	the destination buffer of the address
   */
  static bool stringToAddr(int family, const char *text, void *dest);

  friend class KExtendedSocket;
};

extern const KInetSocketAddress addressAny, addressLoopback;

/*
 * External definition KUnixSocketAddress
 */
struct sockaddr_un;

/**
 * A Unix socket address
 *
 * This is a Unix socket address
 *
 * This class expects QCString instead of QString values, which means the
 * filenames should be encoded in whatever form locale/system deems necessary
 * before passing down to the function
 *
 * @author Thiago Macieira <thiagom@mail.com>
 * @version $Id$
 * @short a Unix socket address
 */
class KUnixSocketAddress: public KSocketAddress
{
  Q_OBJECT
public:
  /**
   * Default constructor
   */
  KUnixSocketAddress();

  /**
   * Constructor from raw data
   * @param raw_data	raw data
   * @param size	data length
   */
  KUnixSocketAddress(const sockaddr_un* raw_data, ksocklen_t size);

  /**
   * Constructor from pathname
   * @param path	pathname
   */
  KUnixSocketAddress(QCString pathname);

  /**
   * Destructor
   */
  virtual ~KUnixSocketAddress();

  /**
   * Sets this to given sockaddr_un
   * @param socket_address socket address
   * @param size	the socket length
   */
  bool setAddress(const sockaddr_un* socket_address, ksocklen_t size);

  /**
   * Sets this to given pathname
   * @param path	pathname
   */
  bool setAddress(QCString path);

  /**
   * Returns the pathname
   */
  QCString pathname() const;

  /**
   * Returns pretty representation of this socket
   */
  virtual QString pretty() const;

  /*
   * Returns the path in the form of a QString
   * This can be fed into KExtendedSocket
   */
  virtual QString serviceName() const;

  /**
   * Returns raw socket address
   */
  const sockaddr_un* address() const;

  /**
   * Returns raw socket address
   */
  operator const sockaddr_un*() const
  { return address(); }

  static bool areEqualUnix(const KSocketAddress &s1, const KSocketAddress &s2, bool coreOnly);

private:
  void init();

  class Private;
  Private *d;

  friend class KExtendedSocket;
};

#endif // KSOCKADDR_H
