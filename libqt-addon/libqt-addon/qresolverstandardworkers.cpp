/*  -*- C++ -*-
 *  Copyright (C) 2003 Thiago Macieira <thiago.macieira@kdemail.net>
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

#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif

#include <qthread.h>
#include <qmutex.h>
#include <qstrlist.h>

#include "qresolver.h"
#include "qsocketaddress.h"
#include "qresolver_p.h"
#include "qresolverstandardworkers_p.h"

struct hostent;
struct addrinfo;

ADDON_USE
using namespace ADDON_NAMESPACE::Internal;

namespace
{
#ifndef HAVE_GETADDRINFO
  class GetHostByNameThread: public QResolverWorkerBase
  {
  public:
    QCString m_hostname;	// might be different!
    Q_UINT16 m_port;
    int m_scopeid;
    int m_af;
    QResolverResults& results;

    GetHostByNameThread(const char * hostname, Q_UINT16 port,
			int scopeid, int af, QResolverResults* res) :
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
     * we will choose gethostbyname2 if AF_INET is defined.
     *
     * Lastly, gethostbyname will be used if nothing else is present.
     */

    /*
     * Note on the use of mutexes:
     * if gethostbyname_r and gethostbyname2_r aren't available, we must assume
     * that all function calls are non-reentrant. Therefore, we will lock
     * a global mutex for protection.
     */

    hostent *resultptr;
    hostent my_results;
    unsigned buflen = 1024;
    int res;
    int my_h_errno;
    char *buf = 0L;
  
    qDebug("ResolveThread::run(): started threaded gethostbyname for %s (af = %d)", 
	   m_hostname.data(), m_af);
    do
      {
	res = 0;
	my_h_errno = HOST_NOT_FOUND;

# ifdef HAVE_GETHOSTBYNAME2_R
	buf = new char[buflen];
	res = gethostbyname2_r(m_hostname, m_af, &my_results, buf, buflen,
			       &resultptr, &my_h_errno);

# elif defined(HAVE_GETHOSTBYNAME_R) && (!defined(AF_INET6) || !defined(HAVE_GETHOSTBYNAME2))
	if (m_af == AF_INET)
	  {
	    buf = new char[buflen];
	    res = gethostbyname_r(m_hostname, &my_results, buf, buflen,
				  &resultptr, &my_h_errno);
	  }
	else
	  resultptr = 0;		// signal error

# elif defined(HAVE_GETHOSTBYNAME2)
	// must lock mutex
	QMutexLocker locker(&getXXbyYYmutex);
	resultptr = gethostbyname2(m_hostname, m_af);
	my_h_errno = h_errno;

# else
	if (m_af == AF_INET)
	  {
	    // must lock mutex
	    QMutexLocker locker(&getXXbyYYmutex);
	    resultptr = gethostbyname(m_hostname);
	    my_h_errno = h_errno;
	  }
	else
	  resultptr = 0;
# endif

	if (resultptr != 0L)
	  my_h_errno = 0;
	qDebug("GetHostByNameThread::run(): gethostbyname for %s (af = %d) returned: %d",
	       m_hostname.data(), m_af, my_h_errno);

	if (res == ERANGE)
	  {
	    // Enlarge the buffer
	    buflen += 1024;
	    delete [] buf;
	    buf = new char[buflen];
	  }
      }
    while (res == ERANGE);
    processResults(resultptr, my_h_errno);

    delete [] buf;

    finished();
    return results.errorCode() == QResolver::NoError;
  }

  void GetHostByNameThread::processResults(hostent *he, int herrno)
  {
    if (herrno)
      {
	qDebug("QStandardWorker::processResults: got error %d", herrno);
	switch (herrno)
	  {
	  case HOST_NOT_FOUND:
	    results.setError(QResolver::NoName);
	    return;

	  case TRY_AGAIN:
	    results.setError(QResolver::TryAgain);
	    return;

	  case NO_RECOVERY:
	    results.results.setError(QResolver::NonRecoverable);
	    return;

	  case NO_ADDRESS:
	    results.setError(QResolver::NoName);
	    return;

	  default:
	    results.setError(QResolver::UnknownError);
	    return;
	  }
      }
    else if (he == 0L)
      {
	results.setError(QResolver::NoName);
	return;			// this was an error
      }

    // clear any errors
    setError(QResolver::NoError);
    results.setError(QResolver::NoError);

    // we process results in the reverse order
    // that is, we prepend each result to the list of results
    int proto = protocol();
    int socktype = socketType();
    if (socktype == 0)
      socktype = SOCK_STREAM;	// default

    QString canon = QResolver::domainToUnicode(QString::fromLatin1(he->h_name));
    QInetSocketAddress sa;
    sa.setPort(m_port);
    if (he->h_addrtype != AF_INET)
      sa.setScopeId(m_scopeid);	// this will also change the socket into IPv6

    for (int i = 0; he->h_addr_list[i]; i++)
      {
	sa.setHost(QIpAddress(he->h_addr_list[i], he->h_addrtype == AF_INET ? 4 : 6));
	results.prepend(QResolverEntry(sa, socktype, proto, canon, m_hostname));
	qDebug("QStandardWorker::processResults: adding %s", sa.toString().latin1());
      }
    //  qDebug("QStandardWorker::processResults: added %d entries", i);
  }

#else  // HAVE_GETADDRINFO

  class GetAddrInfoThread: public QResolverWorkerBase
  {
  public:
    QCString m_node;
    QCString m_serv;
    int m_af;
    QResolverResults& results;

    GetAddrInfoThread(const char* node, const char* serv, int af,
		      QResolverResults* res) :
      m_node(node), m_serv(serv), m_af(af), results(*res)
    { }

    ~GetAddrInfoThread()
    { }

    virtual bool preprocess()
    { return true; }

    virtual bool run();

    void processResults(addrinfo* ai, int ret_code, QResolverResults& rr);
  };

  bool GetAddrInfoThread::run()
  {
# ifdef NEED_MUTEX
    // in some platforms, getaddrinfo(3) is not thread-safe nor reentrant
    // therefore, we will lock a global mutex (defined in qresolver.cpp)
    // relating to all the lookup functions

    QMutexLocker locker(&getXXbyYYmutex);
# endif

    // process hints
    addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = m_af;
    hint.ai_socktype = socketType();
    hint.ai_protocol = protocol();

    if (hint.ai_socktype == 0)
      hint.ai_socktype = SOCK_STREAM; // default

    if (flags() & QResolver::Passive)
      hint.ai_flags |= AI_PASSIVE;
    if (flags() & QResolver::CanonName)
      hint.ai_flags |= AI_CANONNAME;
# ifdef AI_NUMERICHOST
    if (flags() & QResolver::NoResolve)
      hint.ai_flags |= AI_NUMERICHOST;
# endif

    // now we do the blocking processing
    if (m_node.isEmpty())
      m_node = "*";

    addrinfo *result;
    int res = getaddrinfo(m_node, m_serv, &hint, &result);
    qDebug("QGetAddrInfoThread::run: getaddrinfo for [%s]:%s (af = %d); result = %d", 
	   m_node.data(), m_serv.data(), m_af, res);

    if (res != 0)
      {
	switch (res)
	  {
	  case EAI_BADFLAGS:
	    results.setError(QResolver::BadFlags);
	    break;

#ifdef EAI_NODATA
          // In some systems, EAI_NODATA was #define'd to EAI_NONAME which would break this case.
#if EAI_NODATA != EAI_NONAME
	  case EAI_NODATA:	// it was removed in RFC 3493
#endif
#endif
	  case EAI_NONAME:
	    results.setError(QResolver::NoName);
	    break;

	  case EAI_AGAIN:
	    results.setError(QResolver::TryAgain);
	    break;

	  case EAI_FAIL:
	    results.setError(QResolver::NonRecoverable);
	    break;

	  case EAI_FAMILY:
	    results.setError(QResolver::UnsupportedFamily);
	    break;

	  case EAI_SOCKTYPE:
	    results.setError(QResolver::UnsupportedSocketType);
	    break;

	  case EAI_SERVICE:
	    results.setError(QResolver::UnsupportedService);
	    break;

	  case EAI_MEMORY:
	    results.setError(QResolver::Memory);
	    break;

	  case EAI_SYSTEM:
	    results.setError(QResolver::SystemError, errno);
	    break;

	  default:
	    results.setError(QResolver::UnknownError, errno);
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
	    canon = QResolver::domainToUnicode(QString::fromAscii(p->ai_canonname));
	    previous_canon = p->ai_canonname;
	  }

	results.append(QResolverEntry(p->ai_addr, p->ai_addrlen, p->ai_socktype, 
				      p->ai_protocol, canon, m_node));
      }

    freeaddrinfo(result);
    results.setError(QResolver::NoError);
    finished();
    return results.errorCode() == QResolver::NoError;
  }

#endif // HAVE_GETADDRINFO
}; // namespace

bool QStandardWorker::sanityCheck()
{
  // check that the requested values are sensible

  if (!nodeName().isEmpty())
    {
      QString node = nodeName();
      if (node.find('%') != -1)
	node.truncate(node.find('%'));

      if (node.isEmpty() || node == QString::fromLatin1("*") ||
	  node == QString::fromLatin1("localhost"))
	m_encodedName.truncate(0);
      else
	{
	  m_encodedName = QResolver::domainToAscii(node);

	  if (m_encodedName.isNull())
	    {
	      qDebug("could not encode hostname '%s' (UTF-8)", node.utf8().data());
	      setError(QResolver::NoName);
	      return false;		// invalid hostname!
	    }

	  qDebug("Using encoded hostname '%s' for '%s' (UTF-8)", m_encodedName.data(),
		 node.utf8().data());
	}
    }
  else
    m_encodedName.truncate(0);	// just to be sure, but it should be clear already

  if (protocol() == -1)
    {
      setError(QResolver::NonRecoverable);
      return false;		// user passed invalid protocol name
    }

  return true;			// it's sane
}

bool QStandardWorker::resolveScopeId()
{
  // we must test the original name, not the encoded one
  scopeid = 0;
  int pos = nodeName().findRev('%');
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
      scopeid = if_nametoindex(scopename.latin1());
#else
      scopeid = 0;
#endif
    }

  return true;
}

bool QStandardWorker::resolveService()
{
  // find the service first
  bool ok;
  port = serviceName().toUInt(&ok);
  if (!ok)
    {
      // service name does not contain a port number
      // must be a name

      if (serviceName().isEmpty() || serviceName().compare(QString::fromLatin1("*")) == 0)
	port = 0;
      else
	{
	  // it's a name. We need the protocol name in order to lookup.
	  QCString protoname = protocolName();

	  if (protoname.isEmpty() && protocol())
	    {
	      protoname = QResolver::protocolName(protocol()).first();

	      // if it's still empty...
	      if (protoname.isEmpty())
		{
		  // lookup failed!
		  setError(QResolver::NoName);
		  return false;
		}
	    }
	  else
	    protoname = "tcp";

	  // it's not, so we can do a port lookup
	  int result = QResolver::servicePort(serviceName().latin1(), protoname);
	  if (result == -1)
	    {
	      // lookup failed!
	      setError(QResolver::NoName);
	      return false;
	    }

	  // it worked, we have a port number
	  port = (Q_UINT16)result;
	}
    }

  // we found a port
  return true;
}

QResolver::ErrorCodes QStandardWorker::addUnix()
{
  // before trying to add, see if the user wants Unix sockets
  if ((familyMask() & QResolver::UnixFamily) == 0)
    // no, Unix sockets are not wanted
    return QResolver::UnsupportedFamily;

  // now check if the requested data are good for a Unix socket
  if (!m_encodedName.isEmpty())
    return QResolver::AddrFamily; // non local hostname

  if (protocol() || protocolName())
    return QResolver::BadFlags;	// cannot have Unix sockets with protocols

  QString pathname = serviceName();
  if (pathname.isEmpty())
    return QResolver::NoName;;	// no path?

  if (pathname[0] != '/')
    // non absolute pathname
    // put it in /tmp
    pathname.prepend("/tmp/");

  //  qDebug("QNoResolveWorker::addUnix(): adding Unix socket for %s", pathname.local8Bit().data());
  QUnixSocketAddress sa(pathname);
  int socktype = socketType();
  if (socktype == 0)
    socktype = SOCK_STREAM;	// default

  results.append(QResolverEntry(sa, socktype, 0));
  setError(QResolver::NoError);
 
  return QResolver::NoError;
}

bool QStandardWorker::resolveNumerically()
{
  // if the NoResolve flag is active, our result from this point forward
  // will always be true, even if the resolution failed.
  // that indicates that our result is authoritative.

  bool wantV4 = familyMask() & QResolver::IPv4Family,
    wantV6 = familyMask() & QResolver::IPv6Family;

  if (!wantV6 && !wantV4)
    // no Internet address is wanted!
    return (flags() & QResolver::NoResolve);

  // now try to find results
  if (!resolveScopeId() || !resolveService())
    return (flags() & QResolver::NoResolve);

  // we have scope IDs and port numbers
  // now try to resolve the hostname numerically
  QInetSocketAddress sa;
  setError(QResolver::NoError);
  sa.setHost(QIpAddress(QString::fromLatin1(m_encodedName)));
  
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
	results.append(QResolverEntry(sa, socktype, proto));
      else
	{
	  // Note: the address *IS* a numeric IP
	  // but it's not of the kind the user asked for
	  //
	  // that means that it cannot be a Unix socket (because it's an IP)
	  // and that means that no resolution will tell us otherwise
	  //
	  // This is a failed resolution

	  setError(QResolver::AddrFamily);
	  return true;
	}
    }
  else if (m_encodedName.isEmpty())
    {
      // user wanted localhost
      if (flags() & QResolver::Passive)
	{
	  if (wantV6)
	    {
	      sa.setHost(QIpAddress::anyhostV6);
	      results.append(QResolverEntry(sa, socktype, proto));
	    }

	  if (wantV4)
	    {
	      sa.setHost(QIpAddress::anyhostV4);
	      results.append(QResolverEntry(sa, socktype, proto));
	    }
	}
      else
	{
	  if (wantV6)
	    {
	      sa.setHost(QIpAddress::localhostV6);
	      results.append(QResolverEntry(sa, socktype, proto));
	    }

	  if (wantV4)
	    {
	      sa.setHost(QIpAddress::localhostV4);
	      results.append(QResolverEntry(sa, socktype, proto));
	    }
	}

      ok = true;
    }
  else
    {
      // probably bad flags, since the address is not convertible without 
      // resolution

      setError(QResolver::BadFlags);
      ok = false;
    }

  return ok || (flags() & QResolver::NoResolve);
}

bool QStandardWorker::preprocess()
{
  // check sanity
  if (!sanityCheck())
    return false;

  // this worker class can only handle known families
  if (familyMask() & QResolver::UnknownFamily)
    {
      setError(QResolver::UnsupportedFamily);
      return false;		// we don't know about this
    }

  // check the socket types
  if (socketType() != SOCK_STREAM && socketType() != SOCK_DGRAM && socketType() != 0)
    {
      setError(QResolver::UnsupportedSocketType);
      return false;
    }

  // check if we can resolve all numerically
  // resolveNumerically always returns true if the NoResolve flag is set
  if (resolveNumerically() || m_encodedName.isEmpty())
    {
      // indeed, we have resolved numerically
      setError(addUnix());
      if (results.count())
	setError(QResolver::NoError);
      finished();
      return true;
    }

  // check if the user wants something we know about
#ifdef AF_INET6
# define mask	(QResolver::IPv6Family | QResolver::IPv4Family | QResolver::UnixFamily)
#else
# define mask	(QResolver::IPv4Family | QResolver::UnixFamily)
#endif

  if ((familyMask() & mask) == 0)
    // errr... nothing we know about
    return false;

#undef mask

  return true;			// it's ok
}

bool QStandardWorker::run()
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
  setError(QResolver::NoName);

  // these are the family types that we know of
  struct
  {
    QResolver::SocketFamilies mask;
    int af;
  } families[] = { { QResolver::IPv4Family, AF_INET }
#ifdef AF_INET6					  
		  , { QResolver::IPv6Family, AF_INET6 }
#endif
  };
  const int familyCount = sizeof(families)/sizeof(families[0]);
  resultList.setAutoDelete(true);

  for (int i = 0; i < familyCount; i++)
    if (familyMask() & families[i].mask)
      {
	QResolverWorkerBase *worker;
	QResolverResults *res = new QResolverResults;
	resultList.append(res);
#ifdef HAVE_GETADDRINFO
	worker = new GetAddrInfoThread(m_encodedName, 
				       serviceName().latin1(),
				       families[i].af, res);
#else
	worker = new GetHostByNameThread(m_encodedName, port, scopeid,
					 families[i].af, res);
#endif

	enqueue(worker);
      }

  // not finished
  return true;
}

bool QStandardWorker::postprocess()
{
  if (results.count())
    return true;		// no need
  // now copy over what we need from the underlying results

  // start backwards because IPv6 was launched later (if at all)
  if (resultList.isEmpty())
    {
      results.setError(QResolver::NoName);
      return true;
    }

  QResolverResults *rr = resultList.last();
  while (rr)
    {
      if (!rr->isEmpty())
	{
	  results.setError(QResolver::NoError);
	  QResolverResults::Iterator it = rr->begin();
	  for ( ; it != rr->end(); ++it)
	    results.append(*it);
	}
      else if (results.isEmpty())
	// this generated an error
	// copy the error code over
	setError(rr->errorCode(), rr->systemError());

      rr = resultList.prev();
    }

  resultList.clear();
  return true;
}

#ifdef HAVE_GETADDRINFO
QGetAddrinfoWorker::~QGetAddrinfoWorker()
{
}

bool QGetAddrinfoWorker::preprocess()
{
  // getaddrinfo(3) can always handle any kind of request that makes sense
  if (!sanityCheck())
    return false;

  if (flags() & QResolver::NoResolve)
    // oops, numeric resolution?
    return run();

  return true;
}

bool QGetAddrinfoWorker::run()
{
  // make an AF_UNSPEC getaddrinfo(3) call
  GetAddrInfoThread worker(m_encodedName, serviceName().latin1(), 
			   AF_UNSPEC, &results);

  if (!worker.run())
    {
      if (wantThis(AF_UNIX))
	{
	  if (addUnix() == QResolver::NoError)
	    setError(QResolver::NoError);
	}
      else
	setError(worker.results.errorCode(), worker.results.systemError());

      return false;
    }

  // The worker has finished working
  // now copy over only what we may want
  // keep track of any Unix-domain sockets

  bool seen_unix = false;
  QResolverResults::Iterator it = results.begin();
  for ( ; it != results.end(); )
    {
      if ((*it).family() == AF_UNIX)
	seen_unix = true;
      if (!wantThis((*it).family()))
	it = results.remove(it);
      else
	++it;
    }

  if (!seen_unix)
    addUnix();

  finished();
  return true;
}

bool QGetAddrinfoWorker::wantThis(int family)
{
  // tells us if the user wants a socket of this family

#ifdef AF_INET6
  if (family == AF_INET6 && familyMask() & QResolver::IPv6Family)
    return true;
#endif
  if (family == AF_INET && familyMask() & QResolver::IPv4Family)
    return true;
  if (family == AF_UNIX && familyMask() & QResolver::UnixFamily)
    return true;

  // it's not a family we know about...
  if (familyMask() & QResolver::UnknownFamily)
    return true;

  return false;
}

void ADDON_NAMESPACE::Internal::initStandardWorkers()
{
  // register the workers in the order we want them to be tried
  // note the no-resolving worker isn't registered. It's handled as a
  // special case in QResolverManager::findWorker

  QResolverWorkerFactoryBase::registerNewWorker(new QResolverWorkerFactory<QStandardWorker>);
  QResolverWorkerFactoryBase::registerNewWorker(new QResolverWorkerFactory<QGetAddrinfoWorker>);
}

#endif
