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

#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>

#include <qglobal.h>
#include <qfile.h>

#include "kdebug.h"
#include "klocale.h"
#include "ksockaddr.h"
//#include "kextsock.h"

#ifndef HAVE_SOCKADDR_IN6
// The system doesn't have sockaddr_in6
// But we can tell netsupp.h to define it for us, according to the RFC
#define CLOBBER_IN6
#endif

#ifndef IN6_IS_ADDR_V4MAPPED
#define NEED_IN6_TESTS
#endif

#include "netsupp.h"

#define V6_CAN_CONVERT_TO_V4(addr)	(IN6_IS_ADDR_V4MAPPED(addr) || IN6_IS_ADDR_V4COMPAT(addr))

#ifdef HAVE_SOCKADDR_SA_LEN
# define MY_MAX(a, b)  			((a) > (b) ? (a) : (b))
# define MIN_SOCKADDR_LEN		MY_MAX(offsetof(sockaddr, sa_family) + sizeof(((sockaddr*)0)->sa_family), \
					       offsetof(sockaddr, sa_len) + sizeof(((sockaddr*)0)->sa_len))
#else
# define MIN_SOCKADDR_LEN		(offsetof(sockaddr, sa_family) + sizeof(((sockaddr*)0)->sa_family))
#endif

#ifdef offsetof
#undef offsetof
#endif
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

// This is how it is
// 46 == strlen("1234:5678:9abc:def0:1234:5678:255.255.255.255")
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN		46
#endif


/**
 * Class KSocketAddress
 */

KSocketAddress::KSocketAddress()
{
  data = NULL;
  datasize = 0;
  owndata = false;
}

KSocketAddress::KSocketAddress(const sockaddr* sa, ksocklen_t size)
{
  if (sa != NULL)
    {
      data = (sockaddr*)malloc(size);
      if (data == NULL)
	return;
      memcpy(data, sa, size);
      datasize = size;
      owndata = true;
    }
  else
    {
      datasize = 0;
      owndata = false;
    }
}

KSocketAddress::~KSocketAddress()
{
  if (owndata && data != NULL)
    free(data);
}

QString KSocketAddress::pretty() const
{ 
  return i18n("<unknown socket>"); 
}

int KSocketAddress::family() const
{
  if (data != NULL)
    return data->sa_family;
  return AF_UNSPEC;
}

// This creates a new KSocketAddress with given sockaddr
KSocketAddress* KSocketAddress::newAddress(const struct sockaddr* sa, ksocklen_t size)
{
  if (size == 0)
    {
      kdWarning() << "KSocketAddress::newAddress called with size = 0!\n";
      return NULL;
    }

  // make sure we have the right stuff
  if (size < MIN_SOCKADDR_LEN)
    {
      kdWarning() << "KSocketAddress::newAddress called with invalid size\n";
      return NULL;
    }

  switch (sa->sa_family)
    {
    case AF_INET:
      if (size >= sizeof(sockaddr_in))
	return new KInetSocketAddress((const sockaddr_in*)sa, size);
      return NULL;
	
#ifdef AF_INET6
    case AF_INET6:
      if (size >= sizeof(sockaddr_in6))
	return new KInetSocketAddress((const sockaddr_in6*)sa, size);
      return NULL;
#endif

    case AF_UNIX:		// AF_LOCAL
      return new KUnixSocketAddress((const sockaddr_un*)sa, size);
    }

  return new KSocketAddress(sa, size);
}

bool KSocketAddress::isEqual(const KSocketAddress& other) const
{
  switch(family())
  {
     case AF_INET:
        return KInetSocketAddress::areEqualInet(*this, other, false);
#ifdef AF_INET6
     case AF_INET6:
        return KInetSocketAddress::areEqualInet6(*this, other, false);
#endif
     case AF_UNIX: // AF_LOCAL
        return KUnixSocketAddress::areEqualUnix(*this, other, false);
  }

  return false;
}

bool KSocketAddress::isCoreEqual(const KSocketAddress& other) const
{
  switch(family())
  {
     case AF_INET:
        return KInetSocketAddress::areEqualInet(*this, other, true);
#ifdef AF_INET6
     case AF_INET6:
        return KInetSocketAddress::areEqualInet6(*this, other, true);
#endif
     case AF_UNIX: // AF_LOCAL
        return KUnixSocketAddress::areEqualUnix(*this, other, true);
  }

  return false;
}

QString KSocketAddress::nodeName() const
{
  return QString::null;
}

QString KSocketAddress::serviceName() const
{
  return QString::null;
}

int KSocketAddress::ianaFamily(int af)
{
  switch (af)
    {
    case AF_INET:
      return 1;
#ifdef AF_INET6
    case AF_INET6:
      return 2;
#endif
    default:
      return 0;
    }
}

int KSocketAddress::fromIanaFamily(int iana)
{
  switch (iana)
    {
    case 1:
      return AF_INET;
#ifdef AF_INET6
    case 2:
      return AF_INET6;
#endif
    default:
      return AF_UNSPEC;
    }
}

/**
 * class KInetSocketAddress
 */
class KInetSocketAddress::Private
{
public:
  int sockfamily;
  sockaddr_in sin;
#ifdef AF_INET6
  sockaddr_in6 sin6;
#endif

  Private() :
    sockfamily(AF_UNSPEC)
  { 
    sin.sin_family = AF_INET;
    sin.sin_port = 0;
#ifdef HAVE_SOCKADDR_SA_LEN
    sin.sin_len = sizeof(sin);
#endif
#ifdef AF_INET6
    sin6.sin6_family = AF_INET6;
    sin6.sin6_port = 0;
    sin6.sin6_flowinfo = 0;
# ifdef HAVE_SOCKADDR_IN6_SCOPE_ID
    sin6.sin6_scope_id = 0;
# endif
# ifdef HAVE_SOCKADDR_SA_LEN
    sin6.sin6_len = sizeof(sin6);
# endif
#endif
  }
    
};

KInetSocketAddress::KInetSocketAddress() :
  d(new Private)
{ 
}

KInetSocketAddress::KInetSocketAddress(const KInetSocketAddress &other) :
  KSocketAddress(), d(new Private)
{
  setAddress(other);
}

KInetSocketAddress::KInetSocketAddress(const sockaddr_in* sin, ksocklen_t len) :
  d(new Private)
{ 
  setAddress(sin, len); 
}

KInetSocketAddress::KInetSocketAddress(const sockaddr_in6* sin6, ksocklen_t len) :
  d(new Private)
{
  setAddress(sin6, len); 
}

KInetSocketAddress::KInetSocketAddress(const in_addr& addr, unsigned short port) :
  d(new Private)
{
  setAddress(addr, port);
}

KInetSocketAddress::KInetSocketAddress(const in6_addr& addr, unsigned short port) :
  d(new Private)
{
  setAddress(addr, port);
}

KInetSocketAddress::KInetSocketAddress(const QString& addr, unsigned short port, int family) :
  d(new Private)
{
  setAddress(addr, port, family);
}

KInetSocketAddress::~KInetSocketAddress()
{
  delete d;

  //  KSocketAddress::~KSocketAddress();
}

bool KInetSocketAddress::setAddress(const KInetSocketAddress &other)
{
  if (other.family() == AF_INET)
    return setAddress(other.addressV4(), other.size());
#ifdef AF_INET6
  else if (other.family() == AF_INET6)
    return setAddress(other.addressV6(), other.size());
#endif
  return false;
}

bool KInetSocketAddress::setAddress(const sockaddr_in* sin, ksocklen_t len)
{
  // This is supposed to be a AF_INET socket
  if ((len < sizeof(sockaddr_in)) || (sin->sin_family != AF_INET))
    {
      kdWarning() << "KInetSocketAddress::setAddress(sockaddr_in*) called with invalid sockaddr_in\n";
      return false;
    }

  return setHost(sin->sin_addr) && setPort(ntohs(sin->sin_port));
}

bool KInetSocketAddress::setAddress(const sockaddr_in6* sin6, ksocklen_t len)
{
#ifdef AF_INET6
  // should be family AF_INET6
  if ((len < (offsetof(sockaddr_in6, sin6_addr) + sizeof(sin6->sin6_addr))) ||
      (sin6->sin6_family != AF_INET6))
    {
      kdWarning() << "KInetSocketAddress::setAddress(sockaddr_in6*) called with invalid sockaddr_in6\n";
      return 0;
    }

  memset(&d->sin6, 0, sizeof(d->sin6));
  if (len > sizeof(d->sin6))
    len = sizeof(d->sin6);
  memcpy(&d->sin6, sin6, len);

  /* Now make a sanity check */
  d->sockfamily = d->sin6.sin6_family = AF_INET6;
# ifdef HAVE_SOCKADDR_SA_LEN
  d->sin6.sin6_len = sizeof(d->sin6);
# endif

  fromV6();
  return true;
#else
  return false;
#endif
}

bool KInetSocketAddress::setAddress(const in_addr& addr, unsigned short port)
{
  return setHost(addr) && setPort(port);
}

bool KInetSocketAddress::setAddress(const in6_addr& addr, unsigned short port)
{
  return setHost(addr) && setPort(port);
}

bool KInetSocketAddress::setAddress(const QString& addr, unsigned short port, int family)
{ 
  return setHost(addr, family) && setPort(port); 
}

bool KInetSocketAddress::setHost(const in_addr& addr)
{
  d->sockfamily = AF_INET;	// set address to IPv4 type
  d->sin.sin_addr = addr;
  fromV4();
  return true;
}

bool KInetSocketAddress::setHost(const in6_addr& addr)
{
#ifdef AF_INET6
  d->sockfamily = AF_INET6;	// set address to IPv6 type
  d->sin6.sin6_addr = addr;
  fromV6();
  return true;
#else
  return false;
#endif
}

bool KInetSocketAddress::setHost(const QString& addr, int family)
{
  // if family == -1, we'll try to guess the host name
  if ((family != -1) && (family != AF_INET)
#ifdef AF_INET6
      && (family != AF_INET6)
#endif
      )
    {
      kdWarning() << "KInetSocketAddress::setHost(QString, int) called with unknown family address\n";
      return false;
    }

  if (family == -1)
    {
      // guess the family type

#ifdef AF_INET6
      // IPv6 addresses MUST contain colons (:) and IPv4 addresses must not
      if (addr.find(':') != -1)
	family = AF_INET6;
      else
	family = AF_INET;
#else

      // There's only one guess:
      family = AF_INET;
#endif
    }

  /*
   * FIXME! What is the decoding process for hostnames?
   */
  if (family == AF_INET)
    {
      inet_pton(family, addr.latin1(), (void*)&(d->sin.sin_addr));
      fromV4();
    }
#ifdef AF_INET6
  else
    {
      inet_pton(family, addr.latin1(), (void*)&(d->sin6.sin6_addr));
      fromV6();
    }
#endif
  d->sockfamily = family;
  return true;
}

bool KInetSocketAddress::setPort(unsigned short port)
{
  // set port on all socket types
  d->sin.sin_port = htons(port);
#ifdef AF_INET6
  d->sin6.sin6_port = htons(port);
#endif

  return true;
}

bool KInetSocketAddress::setFamily(int _family)
{
  if (_family != AF_INET 
#ifdef AF_INET6
      && _family != AF_INET6
#endif
      )
    {
      kdWarning() << "KInetSocketAddress::setFamily(int) called with unknown family\n";
      return false;
    }

  d->sockfamily = _family;
  if (_family == AF_INET)
    fromV4();
#ifdef AF_INET6
  else if (_family == AF_INET6)
    fromV6();
#endif

  return true;
}

bool KInetSocketAddress::setFlowinfo(Q_UINT32 flowinfo)
{
#ifdef AF_INET6
  if (d->sockfamily == AF_INET6)
    {
      d->sin6.sin6_flowinfo = flowinfo;
      return true;
    }
#endif
  return false;
}

bool KInetSocketAddress::setScopeId(int scopeid)
{
#if defined(AF_INET6) && defined(HAVE_SOCKADDR_IN6_SCOPE_ID)
  if (d->sockfamily == AF_INET6)
    {
      d->sin6.sin6_scope_id = scopeid;
      return true;
    }
#endif
  (void)scopeid;
  return false;
}

const sockaddr_in* KInetSocketAddress::addressV4() const
{
  if (d->sockfamily == AF_INET)
    return &d->sin;
#ifdef AF_INET6
  else if (d->sockfamily == AF_INET6)
    {
      // check if this IPv6 address was converted without loss
      if (V6_CAN_CONVERT_TO_V4(&d->sin6.sin6_addr))
	return &d->sin;
      else
	return NULL;		// there was loss, so return nothing
    }
#endif

  kdWarning() << "KInetSocketAddress::addressV4() called on uninitialized socket\n";
  return NULL;
}

const sockaddr_in6* KInetSocketAddress::addressV6() const
{
#ifdef AF_INET6
  return &d->sin6;
#else
  return NULL;
#endif
}

in_addr KInetSocketAddress::hostV4() const
{
  // this might be empty
  return d->sin.sin_addr;
}

/*
 * ATTENTION
 * This function is left undefined if no IPv6 support exists
 * This is intentional
 */
#ifdef AF_INET6
in6_addr KInetSocketAddress::hostV6() const
{
  return d->sin6.sin6_addr;
}
#endif

QString KInetSocketAddress::pretty() const
{
  if (d->sockfamily != AF_INET 
#ifdef AF_INET6
      && d->sockfamily != AF_INET6
#endif
      )
    {
      kdWarning() << "KInetSocketAddress::pretty() called on uninitialized class\n";
      return i18n("<empty>");
    }

  return i18n("1: hostname, 2: port number", "%1 port %2").arg(nodeName()).arg(serviceName());
}

QString KInetSocketAddress::nodeName() const
{
  char buf[INET6_ADDRSTRLEN];	// INET6_ADDRSTRLEN > INET_ADDRSTRLEN

  if (d->sockfamily == AF_INET)
    inet_ntop(d->sockfamily, (void*)&d->sin.sin_addr, buf, sizeof(buf));
#ifdef AF_INET6
  else if (d->sockfamily == AF_INET6)
    inet_ntop(d->sockfamily, (void*)&d->sin6.sin6_addr, buf, sizeof(buf));
#endif
  else
    {
      kdWarning() << "KInetSocketAddress::prettyHost() called on uninitialized class\n";
      return i18n("<empty>");
    }

  return QString::fromLocal8Bit(buf); // FIXME! What's the encoding?
}

QString KInetSocketAddress::serviceName() const
{
  return QString::number(port());
}

unsigned short KInetSocketAddress::port() const
{
#ifdef AF_INET6
  // we prefer sin6 here because fromV6() might make sin.sin_port be 0
  return ntohs(d->sin6.sin6_port);
#else
  return ntohs(d->sin.sin_port);
#endif
}

Q_UINT32 KInetSocketAddress::flowinfo() const
{
#ifdef AF_INET6
  if (d->sockfamily == AF_INET6)
    return (Q_UINT32)d->sin6.sin6_flowinfo;
#endif
  return 0;
}

ksocklen_t KInetSocketAddress::size() const
{
  if (d->sockfamily == AF_INET)
    return sizeof(d->sin);
#ifdef AF_INET6
  else if (d->sockfamily == AF_INET6)
    return sizeof(d->sin6);
#endif
  else
    return 0;
}

bool KInetSocketAddress::areEqualInet(const KSocketAddress &s1, const KSocketAddress &s2, bool coreOnly)
{
   if (s1.family() != s2.family())
      return false;
   if ((s1.size() < sizeof(sockaddr_in)) || (s2.size() < sizeof(sockaddr_in)))
      return false;

   struct sockaddr_in *sin1 = (sockaddr_in *) s1.address();
   struct sockaddr_in *sin2 = (sockaddr_in *) s2.address();

   if (coreOnly)
      return (memcmp(&sin1->sin_addr, &sin2->sin_addr, sizeof(struct in_addr))  == 0);
   else
      return (sin1->sin_port == sin2->sin_port) && 
             (memcmp(&sin1->sin_addr, &sin2->sin_addr, sizeof(struct in_addr))  == 0);
}

bool KInetSocketAddress::areEqualInet6(const KSocketAddress &s1, const KSocketAddress &s2, bool coreOnly)
{
#ifdef AF_INET6
   if (s1.family() != s2.family())
      return false;

   if ((s1.size() < sizeof(sockaddr_in6)) || (s2.size() < sizeof(sockaddr_in6)))
      return false;

   struct sockaddr_in6 *sin1 = (sockaddr_in6 *) s1.address();
   struct sockaddr_in6 *sin2 = (sockaddr_in6 *) s2.address();

   if (coreOnly)
     return (memcmp(&sin1->sin6_addr, &sin2->sin6_addr, sizeof(struct in6_addr))  == 0);
   else
     return (sin1->sin6_port == sin2->sin6_port) && 
            (sin1->sin6_flowinfo == sin2->sin6_flowinfo) && 
#ifdef HAVE_SOCKADDR_IN6_SCOPE_ID
            (sin1->sin6_scope_id == sin2->sin6_scope_id) && 
#endif
            (memcmp(&sin1->sin6_addr, &sin2->sin6_addr, sizeof(struct in6_addr))  == 0);
#else
   return false;
#endif
}

// KDE 3.0: Remove me!
bool KInetSocketAddress::isCoreEqual(const KSocketAddress& other) const
{
  return KSocketAddress::isCoreEqual(other);
}

void KInetSocketAddress::fromV4()
{
  // converts an address from v4

#ifdef AF_INET6
  d->sin6.sin6_port = d->sin.sin_port;

  // Make this a v4-mapped address
  ((Q_UINT32*)&d->sin6.sin6_addr)[0] = ((Q_UINT32*)&d->sin6.sin6_addr)[1] = 0;
  ((Q_UINT32*)&d->sin6.sin6_addr)[2] = htonl(0xffff);
  ((Q_UINT32*)&d->sin6.sin6_addr)[3] = *(Q_UINT32*)&d->sin.sin_addr;
#endif

  // data == KSocketAddress::data
  data = (sockaddr*)&d->sin;
  datasize = sizeof( sockaddr_in );
}

void KInetSocketAddress::fromV6()
{
#ifdef AF_INET6
  // convert to v4 only if this is a v4-mapped or v4-compat address
  if (V6_CAN_CONVERT_TO_V4(&d->sin6.sin6_addr))
    {
      d->sin.sin_port = d->sin6.sin6_port;
      *(Q_UINT32*)&d->sin.sin_addr = ((Q_UINT32*)&d->sin6.sin6_addr)[3];
    }
  else
    {
      d->sin.sin_port = 0;
      memset(&d->sin.sin_addr, 0, sizeof(d->sin.sin_addr));
    }

  data = (sockaddr*)&d->sin6;
  datasize = sizeof( sockaddr_in6 );
#endif
}

QString KInetSocketAddress::addrToString(int family, const void* addr)
{
  char buf[INET6_ADDRSTRLEN+1];

  return QString::fromUtf8(inet_ntop(family, addr, buf, INET6_ADDRSTRLEN));
}

bool KInetSocketAddress::stringToAddr(int family, const char *text, void *dest)
{
  return inet_pton(family, text, dest) != 0;
}

/**
 * class KUnixSocketAddress
 */

class KUnixSocketAddress::Private
{
public:
  sockaddr_un *m_sun;

  Private() : m_sun(NULL)
  { }
};

KUnixSocketAddress::KUnixSocketAddress() :
  d(new Private)
{
}

KUnixSocketAddress::KUnixSocketAddress(const sockaddr_un* _sun, ksocklen_t size) :
  d(new Private)
{
  setAddress(_sun, size);
}

KUnixSocketAddress::KUnixSocketAddress(QCString pathname) :
  d(new Private)
{
  setAddress(pathname); 
}

KUnixSocketAddress::~KUnixSocketAddress()
{
  delete d;
}

bool KUnixSocketAddress::setAddress(const sockaddr_un* _sun, ksocklen_t _size)
{
  if (_sun->sun_family != AF_UNIX)
    {
      kdWarning() << "KUnixSocketAddress::setAddress called with invalid socket\n";
      return false;
    }

  if (owndata && (d->m_sun != NULL) && (datasize >= _size))
    {
      // reuse this without reallocating
      memcpy(d->m_sun, _sun, _size);
    }
  else
    {
      if (owndata && (d->m_sun != NULL))
	free(d->m_sun);

      d->m_sun = (sockaddr_un*)malloc(_size);

      if (d->m_sun == NULL)
	{
	  // problems
	  owndata = false;
	  return false;
	}

      memcpy(d->m_sun, _sun, _size);
    }

  datasize = _size;
  data = (sockaddr*)d->m_sun;
  owndata = true;
#ifdef HAVE_SOCKADDR_SA_LEN
  data->sa_len = _size;
#endif
  return 1;
}

bool KUnixSocketAddress::setAddress(QCString path)
{
  ksocklen_t newsize = offsetof(sockaddr_un, sun_path) + path.length();

  if (owndata && (d->m_sun != NULL) && (datasize >= newsize))
    {
      // we can reuse this
      strcpy(d->m_sun->sun_path, path);
#ifdef HAVE_SOCKADDR_SA_LEN
      data->sa_len = newsize;
#endif
      return true;
    }

  // nah, we have to do better
  if (owndata && (d->m_sun != NULL))
    free(d->m_sun);

  d->m_sun = (sockaddr_un*) malloc(newsize);
  if (d->m_sun == NULL)
    {
      owndata = false;
      return false;
    }

  d->m_sun->sun_family = AF_UNIX;
  strcpy(d->m_sun->sun_path, path);
  data = (sockaddr*)d->m_sun;
  datasize = newsize;
#ifdef HAVE_SOCKADDR_SA_LEN
  data->sa_len = newsize;
#endif
  return 1;
}

QCString KUnixSocketAddress::pathname() const
{
  if (d->m_sun != NULL)
    {
      if (datasize > offsetof(sockaddr_un, sun_path))
	return d->m_sun->sun_path;
      return "";
    }
  return QCString(0);
}

QString KUnixSocketAddress::pretty() const
{
  QCString pname = pathname();
  if (pname.isEmpty())
    return i18n("<empty Unix socket>");
  return QFile::decodeName(pathname());
}

QString KUnixSocketAddress::serviceName() const
{
  return QString::fromUtf8(pathname());
}

const sockaddr_un* KUnixSocketAddress::address() const
{
  return d->m_sun;
}

bool KUnixSocketAddress::areEqualUnix(const KSocketAddress &s1, const KSocketAddress &s2, bool /* coreOnly */)
{
   if (s1.family() != s2.family())
      return false;   

   if ((s1.size() < sizeof(sockaddr_un)) || (s2.size() < sizeof(sockaddr_un)))
      return false;

   struct sockaddr_un *sun1 = (sockaddr_un *) s1.address();
   struct sockaddr_un *sun2 = (sockaddr_un *) s2.address();
    
   return (strcmp(sun1->sun_path, sun2->sun_path) == 0);
}


#include "ksockaddr.moc"
