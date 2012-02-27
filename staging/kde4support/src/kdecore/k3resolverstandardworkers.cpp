/*  -*- C++ -*-
 *  Copyright (C) 2003,2004 Thiago Macieira <thiago@kde.org>
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

#include "k3resolverstandardworkers_p.h"

#include <config.h>
#include <config-network.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif

#include <QFile>
#include <QList>
#include <QMutex>
#include <QTextStream>
#include <QThread>
#ifdef Q_WS_WIN
#include <winsock2.h>
#endif

#include "kdebug.h"
#include "kglobal.h"
#include "kstandarddirs.h"

#include "k3resolver.h"
#include "k3socketaddress.h"

struct hostent;
struct addrinfo;

using namespace KNetwork;
using namespace KNetwork::Internal;

static bool hasIPv6()
{
#ifdef Q_WS_WIN
  extern void KNetwork_initSocket();
  KNetwork_initSocket();
#endif
#ifdef AF_INET6
  if (!qgetenv("KDE_NO_IPV6").isEmpty())
    return false;

# ifdef Q_WS_WIN
  SOCKET s = ::socket(AF_INET6, SOCK_STREAM, 0);
  if (s == INVALID_SOCKET)
    return false;
  ::closesocket(s);
# else
  int fd = ::socket(AF_INET6, SOCK_STREAM, 0);
  if (fd == -1)
    return false;
  ::close(fd);
# endif
  return true;
#else
  return false;
#endif
}

// blacklist management
static QMutex blacklistMutex;	// KDE4: change to a QReadWriteLock
QStringList KBlacklistWorker::blacklist;

void KBlacklistWorker::init()
{
  if (!KGlobal::hasMainComponent())
    return;

  static bool beenhere = false;

  if (beenhere)
    return;

  beenhere = true;
  loadBlacklist();
}

void KBlacklistWorker::loadBlacklist()
{
  QMutexLocker locker(&blacklistMutex);
  QStringList filelist = KGlobal::dirs()->findAllResources("config", QLatin1String("ipv6blacklist"));

  QStringList::ConstIterator it = filelist.constBegin(),
    end = filelist.constEnd();
  for ( ; it != end; ++it)
    {
      // for each file, each line is a domainname to be blacklisted
      QFile f(*it);
      if (!f.open(QIODevice::ReadOnly))
	continue;

      QTextStream stream(&f);
      stream.setCodec("latin1");
      for (QString line = stream.readLine(); !line.isNull();
	   line = stream.readLine())
	{
	  if (line.isEmpty())
	    continue;

	  // make sure there are no surrounding whitespaces
	  // and that it starts with .
	  line = line.trimmed();
	  if (line[0] != QLatin1Char('.'))
	    line.prepend(QLatin1Char('.'));

	  blacklist.append(line.toLower());
	}
    }
}

// checks the blacklist to see if the domain is listed
// it matches the domain ending part
bool KBlacklistWorker::isBlacklisted(const QString& host)
{
  KBlacklistWorker::init();

  // empty hostnames cannot be blacklisted
  if (host.isEmpty())
    return false;

  QString ascii = QLatin1String(KResolver::domainToAscii(host));

  QMutexLocker locker(&blacklistMutex);

  // now find out if this hostname is present
  QStringList::ConstIterator it = blacklist.constBegin(),
    end = blacklist.constEnd();
  for ( ; it != end; ++it)
    if (ascii.endsWith(*it))
      return true;

  // no match:
  return false;
}

bool KBlacklistWorker::preprocess()
{
  if (isBlacklisted(nodeName()))
    {
      results.setError(KResolver::NoName);
      finished();
      return true;
    }
  return false;
}

bool KBlacklistWorker::run()
{
  results.setError(KResolver::NoName);
  finished();
  return false;			// resolution failure
}

namespace
{
  /*
   * Note on the use of the system resolver functions:
   *
   * In all cases, we prefer to use the new getaddrinfo(3) call. That means
   * it will always be used if it is found.
   *
   * If it's not found, we have the option to use gethostbyname2_r,
   * gethostbyname_r, gethostbyname2 and gethostbyname. If gethostbyname2_r
   * is defined, we will use it.
   *
   * If it's not defined, we have to choose between the non-reentrant
   * gethostbyname2 and the reentrant but IPv4-only gethostbyname_r:
   * we will choose gethostbyname2 if AF_INET6 is defined.
   *
   * Lastly, gethostbyname will be used if nothing else is present.
   */

#ifndef HAVE_GETADDRINFO

# if defined(HAVE_GETHOSTBYNAME2_R)
#  define USE_GETHOSTBYNAME2_R
# elif defined(HAVE_GETHOSTBYNAME_R) && (!defined(AF_INET6) || !defined(HAVE_GETHOSTBYNAME2))
#  define USE_GETHOSTBYNAME_R
# elif defined(HAVE_GETHOSTBYNAME2)
#  define USE_GETHOSTBYNAME2)
# else
#  define USE_GETHOSTBYNAME
# endif

  class GetHostByNameThread: public KResolverWorkerBase
  {
  public:
    QByteArray m_hostname;	// might be different!
    quint16 m_port;
    int m_scopeid;
    int m_af;
    KResolverResults& results;

    GetHostByNameThread(const char * hostname, quint16 port,
			int scopeid, int af, KResolverResults* res) :
      m_hostname(hostname), m_port(port), m_scopeid(scopeid), m_af(af),
      results(*res)
    { }

    ~GetHostByNameThread()
    { }

    virtual bool preprocess()
    { return true; }

    virtual bool run();

    void processResults(hostent* he, int my_h_errno);
  };

  bool GetHostByNameThread::run()
  {

    hostent *resultptr;
    hostent my_results;
    unsigned buflen = 1024;
    int res;
    int my_h_errno;
    char *buf = 0L;

    // qDebug("ResolveThread::run(): started threaded gethostbyname for %s (af = %d)",
    //	   m_hostname.data(), m_af);

    ResolverLocker resLock( this );
    do
      {
	res = 0;
	my_h_errno = HOST_NOT_FOUND;

	// check blacklist
	if (m_af != AF_INET &&
	    KBlacklistWorker::isBlacklisted(QLatin1String(m_hostname)))
	  break;

# ifdef USE_GETHOSTBYNAME2_R
	buf = new char[buflen];
	res = gethostbyname2_r(m_hostname, m_af, &my_results, buf, buflen,
			       &resultptr, &my_h_errno);

# elif defined(USE_GETHOSTBYNAME_R)
	if (m_af == AF_INET)
	  {
	    buf = new char[buflen];
	    res = gethostbyname_r(m_hostname, &my_results, buf, buflen,
				  &resultptr, &my_h_errno);
	  }
	else
	  resultptr = 0;		// signal error

# elif defined(USE_GETHOSTBYNAME2)
	// must lock mutex
	resultptr = gethostbyname2(m_hostname, m_af);
	my_h_errno = h_errno;

# else
	if (m_af == AF_INET)
	  {
	    // must lock mutex
	    resultptr = gethostbyname(m_hostname);
	    my_h_errno = h_errno;
	  }
	else
	  resultptr = 0;
# endif

	if (resultptr != 0L)
	  my_h_errno = 0;
	// qDebug("GetHostByNameThread::run(): gethostbyname for %s (af = %d) returned: %d",
	//       m_hostname.data(), m_af, my_h_errno);

	if (res == ERANGE)
	  {
	    // Enlarge the buffer
	    buflen += 1024;
	    delete [] buf;
	    buf = new char[buflen];
	  }

	if ((res == ERANGE || my_h_errno != 0) && checkResolver())
	  {
	    // resolver needs updating, so we might as well do it now
	    resLock.openClose();
	  }
      }
    while (res == ERANGE);
    processResults(resultptr, my_h_errno);

    delete [] buf;

    finished();
    return results.error() == KResolver::NoError;
  }

  void GetHostByNameThread::processResults(hostent *he, int herrno)
  {
    if (herrno)
      {
	qDebug("KStandardWorker::processResults: got error %d", herrno);
	switch (herrno)
	  {
	  case HOST_NOT_FOUND:
	    results.setError(KResolver::NoName);
	    return;

	  case TRY_AGAIN:
	    results.setError(KResolver::TryAgain);
	    return;

	  case NO_RECOVERY:
	    results.setError(KResolver::NonRecoverable);
	    return;

	  case NO_ADDRESS:
	    results.setError(KResolver::NoName);
	    return;

	  default:
	    results.setError(KResolver::UnknownError);
	    return;
	  }
      }
    else if (he == 0L)
      {
	results.setError(KResolver::NoName);
	return;			// this was an error
      }

    // clear any errors
    setError(KResolver::NoError);
    results.setError(KResolver::NoError);

    // we process results in the reverse order
    // that is, we prepend each result to the list of results
    int proto = protocol();
    int socktype = socketType();
    if (socktype == 0)
      socktype = SOCK_STREAM;	// default

    QString canon = KResolver::domainToUnicode(QLatin1String(he->h_name));
    KInetSocketAddress sa;
    sa.setPort(m_port);
    if (he->h_addrtype != AF_INET)
      sa.setScopeId(m_scopeid);	// this will also change the socket into IPv6

    for (int i = 0; he->h_addr_list[i]; i++)
      {
	sa.setHost(KIpAddress(he->h_addr_list[i], he->h_addrtype == AF_INET ? 4 : 6));
	results.prepend(KResolverEntry(sa, socktype, proto, canon, m_hostname));
	// qDebug("KStandardWorker::processResults: adding %s", sa.toString().toLatin1().constData());
      }
    //  qDebug("KStandardWorker::processResults: added %d entries", i);
  }

#else  // HAVE_GETADDRINFO

  class GetAddrInfoThread: public KResolverWorkerBase
  {
  public:
    QByteArray m_node;
    QByteArray m_serv;
    int m_af;
    int m_flags;
    KResolverResults& results;

    GetAddrInfoThread(const char* node, const char* serv, int af, int flags,
		      KResolverResults* res) :
      m_node(node), m_serv(serv), m_af(af), m_flags(flags), results(*res)
    { }

    ~GetAddrInfoThread()
    { }

    virtual bool preprocess()
    { return true; }

    virtual bool run();

    void processResults(addrinfo* ai, int ret_code, KResolverResults& rr);
  };

  bool GetAddrInfoThread::run()
  {
    // check blacklist
    if ((m_af != AF_INET && m_af != AF_UNSPEC) &&
	KBlacklistWorker::isBlacklisted(QLatin1String(m_node)))
      {
	results.setError(KResolver::NoName);
	finished();
	return false;		// failed
      }

    do
      {
	ResolverLocker resLock( this );

	// process hints
	addrinfo hint;
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = m_af;
	hint.ai_socktype = socketType();
	hint.ai_protocol = protocol();

	if (hint.ai_socktype == 0)
	  hint.ai_socktype = SOCK_STREAM; // default

	if (m_flags & KResolver::Passive)
	  hint.ai_flags |= AI_PASSIVE;
	if (m_flags & KResolver::CanonName)
	  hint.ai_flags |= AI_CANONNAME;
# ifdef AI_NUMERICHOST
	if (m_flags & KResolver::NoResolve)
	  hint.ai_flags |= AI_NUMERICHOST;
# endif
# ifdef AI_ADDRCONFIG
	hint.ai_flags |= AI_ADDRCONFIG;
# endif

	// now we do the blocking processing
	if (m_node.isEmpty())
	  m_node = "*"; // krazy:exclude=doublequote_chars

	addrinfo *result;
	int res = getaddrinfo(m_node, m_serv, &hint, &result);
	//    kDebug(179) << "getaddrinfo(\""
	//		 << m_node << "\", \"" << m_serv << "\", af="
	//		 << m_af << ") returned " << res << endl;

	if (res != 0)
	  {
	    if (checkResolver())
	      {
		// resolver requires reinitialisation
		resLock.openClose();
		continue;
	      }

	    switch (res)
	      {
	      case EAI_BADFLAGS:
		results.setError(KResolver::BadFlags);
		break;

#ifdef EAI_NODATA
		// In some systems, EAI_NODATA was #define'd to EAI_NONAME which would break this case.
#if EAI_NODATA != EAI_NONAME
	      case EAI_NODATA:	// it was removed in RFC 3493
#endif
#endif
	      case EAI_NONAME:
		results.setError(KResolver::NoName);
		break;

	      case EAI_AGAIN:
		results.setError(KResolver::TryAgain);
		break;

	      case EAI_FAIL:
		results.setError(KResolver::NonRecoverable);
		break;

	      case EAI_FAMILY:
		results.setError(KResolver::UnsupportedFamily);
		break;

	      case EAI_SOCKTYPE:
		results.setError(KResolver::UnsupportedSocketType);
		break;

	      case EAI_SERVICE:
		results.setError(KResolver::UnsupportedService);
		break;

	      case EAI_MEMORY:
		results.setError(KResolver::Memory);
		break;

#ifdef EAI_SYSTEM // not available on windows
	      case EAI_SYSTEM:
		results.setError(KResolver::SystemError, errno);
		break;
#endif
	      default:
		results.setError(KResolver::UnknownError, errno);
		break;
	      }

	    finished();
	    return false;		// failed
	  }

	// if we are here, lookup succeeded
	QString canon;
	const char *previous_canon = 0L;

	for (addrinfo* p = result; p; p = p->ai_next)
	  {
	    // cache the last canon name to avoid doing the ToUnicode processing unnecessarily
	    if ((previous_canon && !p->ai_canonname) ||
		(!previous_canon && p->ai_canonname) ||
		(p->ai_canonname != previous_canon &&
		 strcmp(p->ai_canonname, previous_canon) != 0))
	      {
		canon = KResolver::domainToUnicode(QString::fromAscii(p->ai_canonname));
		previous_canon = p->ai_canonname;
	      }

	    results.append(KResolverEntry(p->ai_addr, p->ai_addrlen, p->ai_socktype,
					  p->ai_protocol, canon, m_node));
	  }

	freeaddrinfo(result);
	results.setError(KResolver::NoError);
	finished();
	return results.error() == KResolver::NoError;
      }
    while (true);
  }

#endif // HAVE_GETADDRINFO
} // namespace

KStandardWorker::~KStandardWorker()
{
  qDeleteAll(resultList);
}

bool KStandardWorker::sanityCheck()
{
  // check that the requested values are sensible

  if (!nodeName().isEmpty())
    {
      QString node = nodeName();
      if (node.indexOf(QLatin1Char('%')) != -1)
	node.truncate(node.indexOf(QLatin1Char('%')));

      if (node.isEmpty() || node == QLatin1String("*") ||
	  node == QLatin1String("localhost"))
	m_encodedName.truncate(0);
      else
	{
	  m_encodedName = KResolver::domainToAscii(node);

	  if (m_encodedName.isNull())
	    {
	      qDebug("could not encode hostname '%s' (UTF-8)", node.toUtf8().data());
	      setError(KResolver::NoName);
	      return false;		// invalid hostname!
	    }

	  // qDebug("Using encoded hostname '%s' for '%s' (UTF-8)", m_encodedName.data(),
	  //	 node.toUtf8().data());
	}
    }
  else
    m_encodedName.truncate(0);	// just to be sure, but it should be clear already

  if (protocol() == -1)
    {
      setError(KResolver::NonRecoverable);
      return false;		// user passed invalid protocol name
    }

  return true;			// it's sane
}

bool KStandardWorker::resolveScopeId()
{
  // we must test the original name, not the encoded one
  scopeid = 0;
  int pos = nodeName().lastIndexOf(QLatin1Char('%'));
  if (pos == -1)
    return true;

  QString scopename = nodeName().mid(pos + 1);

  bool ok;
  scopeid = scopename.toInt(&ok);
  if (!ok)
    {
      // it's not a number
      // therefore, it's an interface name
#ifdef HAVE_IF_NAMETOINDEX
      scopeid = if_nametoindex(scopename.toLatin1());
#else
      scopeid = 0;
#endif
    }

  return true;
}

bool KStandardWorker::resolveService()
{
  // find the service first
  bool ok;
  port = serviceName().toUInt(&ok);
  if (!ok)
    {
      // service name does not contain a port number
      // must be a name

      if (serviceName().isEmpty() || serviceName().compare(QLatin1String("*")) == 0)
	port = 0;
      else
	{
	  // it's a name. We need the protocol name in order to lookup.
	  QByteArray protoname = protocolName();

	  if (protoname.isEmpty() && protocol())
	    {
	      protoname = KResolver::protocolName(protocol()).first();

	      // if it's still empty...
	      if (protoname.isEmpty())
		{
		  // lookup failed!
		  setError(KResolver::NoName);
		  return false;
		}
	    }
	  else
	    protoname = "tcp";

	  // it's not, so we can do a port lookup
	  int result = KResolver::servicePort(serviceName().toLatin1(), protoname);
	  if (result == -1)
	    {
	      // lookup failed!
	      setError(KResolver::NoName);
	      return false;
	    }

	  // it worked, we have a port number
	  port = (quint16)result;
	}
    }

  // we found a port
  return true;
}

KResolver::ErrorCodes KStandardWorker::addUnix()
{
  // before trying to add, see if the user wants Unix sockets
  if ((familyMask() & KResolver::UnixFamily) == 0)
    // no, Unix sockets are not wanted
    return KResolver::UnsupportedFamily;

  // now check if the requested data are good for a Unix socket
  if (!m_encodedName.isEmpty())
    return KResolver::AddrFamily; // non local hostname

  if (protocol() || !protocolName().isNull())
    return KResolver::BadFlags;	// cannot have Unix sockets with protocols

  QString pathname = serviceName();
  if (pathname.isEmpty())
    return KResolver::NoName;;	// no path?

  if (pathname[0] != QLatin1Char('/'))
    // non absolute pathname
    // put it in /tmp
    pathname.prepend(QLatin1String("/tmp/"));

  //  qDebug("QNoResolveWorker::addUnix(): adding Unix socket for %s", pathname.toLocal8Bit().data());
  KUnixSocketAddress sa(pathname);
  int socktype = socketType();
  if (socktype == 0)
    socktype = SOCK_STREAM;	// default

  results.append(KResolverEntry(sa, socktype, 0));
  setError(KResolver::NoError);

  return KResolver::NoError;
}

bool KStandardWorker::resolveNumerically()
{
  // if the NoResolve flag is active, our result from this point forward
  // will always be true, even if the resolution failed.
  // that indicates that our result is authoritative.

  bool wantV4 = familyMask() & KResolver::IPv4Family,
    wantV6 = familyMask() & KResolver::IPv6Family;

  if (!wantV6 && !wantV4)
    // no Internet address is wanted!
    return (flags() & KResolver::NoResolve);

  // now try to find results
  if (!resolveScopeId() || !resolveService())
    return (flags() & KResolver::NoResolve);

  // we have scope IDs and port numbers
  // now try to resolve the hostname numerically
  KInetSocketAddress sa;
  setError(KResolver::NoError);
  sa.setHost(KIpAddress(QLatin1String(m_encodedName)));

  // if it failed, the length was reset to 0
  bool ok = sa.length() != 0;

  sa.setPort(port);
  if (sa.ipVersion() == 6)
    sa.setScopeId(scopeid);
  int proto = protocol();
  int socktype = socketType();
  if (socktype == 0)
    socktype = SOCK_STREAM;

  if (ok)
    {
      // the given hostname was successfully converted to an IP address
      // check if the user wanted this kind of address

      if ((sa.ipVersion() == 4 && wantV4) ||
	  (sa.ipVersion() == 6 && wantV6))
	results.append(KResolverEntry(sa, socktype, proto));
      else
	{
	  // Note: the address *IS* a numeric IP
	  // but it's not of the kind the user asked for
	  //
	  // that means that it cannot be a Unix socket (because it's an IP)
	  // and that means that no resolution will tell us otherwise
	  //
	  // This is a failed resolution

	  setError(KResolver::AddrFamily);
	  return true;
	}
    }
  else if (m_encodedName.isEmpty())
    {
      // user wanted localhost
      if (flags() & KResolver::Passive)
	{
	  if (wantV6)
	    {
	      sa.setHost(KIpAddress::anyhostV6);
	      results.append(KResolverEntry(sa, socktype, proto));
	    }

	  if (wantV4)
	    {
	      sa.setHost(KIpAddress::anyhostV4);
	      results.append(KResolverEntry(sa, socktype, proto));
	    }
	}
      else
	{
	  if (wantV6)
	    {
	      sa.setHost(KIpAddress::localhostV6);
	      results.append(KResolverEntry(sa, socktype, proto));
	    }

	  if (wantV4)
	    {
	      sa.setHost(KIpAddress::localhostV4);
	      results.append(KResolverEntry(sa, socktype, proto));
	    }
	}

      ok = true;
    }
  else
    {
      // probably bad flags, since the address is not convertible without
      // resolution

      setError(KResolver::BadFlags);
      ok = false;
    }

  return ok || (flags() & KResolver::NoResolve);
}

bool KStandardWorker::preprocess()
{
  // check sanity
  if (!sanityCheck())
    return false;

  // this worker class can only handle known families
  if (familyMask() & KResolver::UnknownFamily)
    {
      setError(KResolver::UnsupportedFamily);
      return false;		// we don't know about this
    }

  // check the socket types
  if (socketType() != SOCK_STREAM && socketType() != SOCK_DGRAM && socketType() != 0)
    {
      setError(KResolver::UnsupportedSocketType);
      return false;
    }

  // check if we can resolve all numerically
  // resolveNumerically always returns true if the NoResolve flag is set
  if (resolveNumerically() || m_encodedName.isEmpty())
    {
      // indeed, we have resolved numerically
      setError(addUnix());
      if (results.count())
	setError(KResolver::NoError);
      finished();
      return true;
    }

  // check if the user wants something we know about
#ifdef AF_INET6
# define mask	(KResolver::IPv6Family | KResolver::IPv4Family | KResolver::UnixFamily)
#else
# define mask	(KResolver::IPv4Family | KResolver::UnixFamily)
#endif

  if ((familyMask() & mask) == 0)
    // errr... nothing we know about
    return false;

#undef mask

  return true;			// it's ok
}

bool KStandardWorker::run()
{
#ifndef HAVE_GETADDRINFO
  // check the scope id first
  // since most of the resolutions won't have a scope id, this should be fast
  // and we won't have wasted time on services if this fails
  if (!resolveScopeId())
    return false;

  // resolve the service now, before entering the blocking operation
  if (!resolveService())
    return false;
#endif

  // good
  // now we need the hostname
  setError(KResolver::NoName);

  // these are the family types that we know of
  struct
  {
    KResolver::SocketFamilies mask;
    int af;
  } families[] = { { KResolver::IPv4Family, AF_INET }
#ifdef AF_INET6
		  , { KResolver::IPv6Family, AF_INET6 }
#endif
  };
  int familyCount = sizeof(families)/sizeof(families[0]);
  bool skipIPv6 = !hasIPv6();

  for (int i = 0; i < familyCount; i++)
    if (familyMask() & families[i].mask)
      {
#ifdef AF_INET6
	if (skipIPv6 && families[i].af == AF_INET6)
	  continue;
#endif

	KResolverWorkerBase *worker;
	KResolverResults *res = new KResolverResults;
	resultList.append(res);
#ifdef HAVE_GETADDRINFO
	worker = new GetAddrInfoThread(m_encodedName,
				       serviceName().toLatin1(),
				       families[i].af, flags(), res);
#else
	worker = new GetHostByNameThread(m_encodedName, port, scopeid,
					 families[i].af, res);
#endif

	enqueue(worker);
      }

  // not finished
  return true;
}

bool KStandardWorker::postprocess()
{
  if (results.count())
    return true;		// no need
  // now copy over what we need from the underlying results

  // start backwards because IPv6 was launched later (if at all)
  if (resultList.isEmpty())
    {
      results.setError(KResolver::NoName);
      return true;
    }

  for (int i = resultList.size(); i > 0; --i)
    {
      KResolverResults* rr = resultList.at(i - 1);
      if (!rr->isEmpty())
	{
	  results.setError(KResolver::NoError);
	  KResolverResults::Iterator it = rr->begin();
	  for ( ; it != rr->end(); ++it)
	    results.append(*it);
	}
      else if (results.isEmpty())
	// this generated an error
	// copy the error code over
	setError(rr->error(), rr->systemError());

      delete rr;
      resultList[i - 1] = 0L;
    }

  resultList.clear();
  return true;
}

#ifdef HAVE_GETADDRINFO
KGetAddrinfoWorker::~KGetAddrinfoWorker()
{
}

bool KGetAddrinfoWorker::preprocess()
{
  // getaddrinfo(3) can always handle any kind of request that makes sense
  if (!sanityCheck())
    return false;

  if (flags() & KResolver::NoResolve)
    // oops, numeric resolution?
    return run();

  return true;
}

bool KGetAddrinfoWorker::run()
{
  // make an AF_UNSPEC getaddrinfo(3) call
  GetAddrInfoThread worker(m_encodedName, serviceName().toLatin1(),
			   AF_UNSPEC, flags(), &results);

  if (!worker.run())
    {
      if (wantThis(AF_UNIX))
	{
	  if (addUnix() == KResolver::NoError)
	    setError(KResolver::NoError);
	}
      else
	setError(worker.results.error(), worker.results.systemError());

      return false;
    }

  // The worker has finished working
  // now copy over only what we may want
  // keep track of any Unix-domain sockets

  bool seen_unix = false;
  int i = 0;
  while ( i < results.count() )
    {
      const KResolverEntry& res = results[i];
      if (res.family() == AF_UNIX)
	seen_unix = true;
      if (!wantThis(res.family()))
	results.removeAt(i);
      else
	++i;
    }

  if (!seen_unix)
    addUnix();

  finished();
  return true;
}

bool KGetAddrinfoWorker::wantThis(int family)
{
  // tells us if the user wants a socket of this family

#ifdef AF_INET6
  if (family == AF_INET6 && familyMask() & KResolver::IPv6Family)
    return true;
#endif
  if (family == AF_INET && familyMask() & KResolver::IPv4Family)
    return true;
  if (family == AF_UNIX && familyMask() & KResolver::UnixFamily)
    return true;

  // it's not a family we know about...
  if (familyMask() & KResolver::UnknownFamily)
    return true;

  return false;
}

#endif

void KNetwork::Internal::initStandardWorkers()
{
  //KResolverWorkerFactoryBase::registerNewWorker(new KResolverWorkerFactory<KBlacklistWorker>);
  KResolverWorkerFactoryBase::registerNewWorker(new KResolverWorkerFactory<KStandardWorker>);

#ifdef HAVE_GETADDRINFO
  KResolverWorkerFactoryBase::registerNewWorker(new KResolverWorkerFactory<KGetAddrinfoWorker>);
#endif
}
