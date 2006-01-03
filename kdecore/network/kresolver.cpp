/*  -*- C++ -*-
 *  Copyright (C) 2003-2005 Thiago Macieira <thiago@kde.org>
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

#include "config.h"

// System includes
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>

// Qt includes
#include <QCoreApplication>
#include <QPointer>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QSharedData>
#include <QTime>
#include <QTimer>

// IDN
#ifdef HAVE_IDNA_H
# include <idna.h>
#endif

// KDE
#include <klocale.h>

// Us
#include "kresolver.h"
#include "kresolver_p.h"
#include "ksocketaddress.h"

#ifdef NEED_MUTEX
#ifdef __GNUC__
#warning "mutex"
#endif
QMutex getXXbyYYmutex;
#endif

using namespace KNetwork;
using namespace KNetwork::Internal;

/////////////////////////////////////////////
// class KResolverEntry

class KNetwork::KResolverEntryPrivate: public QSharedData
{
public:
  KSocketAddress addr;
  int socktype;
  int protocol;
  QString canonName;
  QByteArray encodedName;

  inline KResolverEntryPrivate() :
    socktype(0), protocol(0)
  { }
};

// default constructor
KResolverEntry::KResolverEntry() :
  d(0L)
{
}

// constructor with stuff
KResolverEntry::KResolverEntry(const KSocketAddress& addr, int socktype, int protocol,
			       const QString& canonName, const QByteArray& encodedName) :
  d(new KResolverEntryPrivate)
{
  d->addr = addr;
  d->socktype = socktype;
  d->protocol = protocol;
  d->canonName = canonName;
  d->encodedName = encodedName;
}

// constructor with even more stuff
KResolverEntry::KResolverEntry(const struct sockaddr* sa, quint16 salen, int socktype,
			       int protocol, const QString& canonName,
			       const QByteArray& encodedName) :
  d(new KResolverEntryPrivate)
{
  d->addr = KSocketAddress(sa, salen);
  d->socktype = socktype;
  d->protocol = protocol;
  d->canonName = canonName;
  d->encodedName = encodedName;
}

// copy constructor
KResolverEntry::KResolverEntry(const KResolverEntry& that) :
  d(0L)
{
  *this = that;
}

// destructor
KResolverEntry::~KResolverEntry()
{
}

// returns the socket address
KSocketAddress KResolverEntry::address() const
{
  return d->addr;
}

// returns the length
quint16 KResolverEntry::length() const
{
  return d->addr.length();
}

// returns the family
int KResolverEntry::family() const
{
  return d->addr.family();
}

// returns the canonical name
QString KResolverEntry::canonicalName() const
{
  return d->canonName;
}

// returns the encoded name
QByteArray KResolverEntry::encodedName() const
{
  return d->encodedName;
}

// returns the socket type
int KResolverEntry::socketType() const
{
  return d->socktype;
}

// returns the protocol
int KResolverEntry::protocol() const
{
  return d->protocol;
}

// assignment operator
KResolverEntry& KResolverEntry::operator= (const KResolverEntry& that)
{
  d = that.d;
  return *this;
}

/////////////////////////////////////////////
// class KResolverResults

class KNetwork::KResolverResultsPrivate: public QSharedData
{
public:
  QString node, service;
  int errorcode, syserror;

  KResolverResultsPrivate() :
    errorcode(0), syserror(0)
  { }
};

// default constructor
KResolverResults::KResolverResults()
  : d(new KResolverResultsPrivate)
{
}

// copy constructor
KResolverResults::KResolverResults(const KResolverResults& other)
  : QList<KResolverEntry>(other), d(new KResolverResultsPrivate)
{
  d = other.d;
}

// destructor
KResolverResults::~KResolverResults()
{
}

// assignment operator
KResolverResults&
KResolverResults::operator= (const KResolverResults& other)
{
  // copy over the other data
  d = other.d;

  // now let QList do the rest of the work
  QList<KResolverEntry>::operator =(other);

  return *this;
}

// gets the error code
int KResolverResults::error() const
{
  return d->errorcode;
}

// gets the system errno
int KResolverResults::systemError() const
{
  return d->syserror;
}

// sets the error codes
void KResolverResults::setError(int errorcode, int systemerror)
{
  d->errorcode = errorcode;
  d->syserror = systemerror;
}

// gets the hostname
QString KResolverResults::nodeName() const
{
  return d->node;
}

// gets the service name
QString KResolverResults::serviceName() const
{
  return d->service;
}

// sets the address
void KResolverResults::setAddress(const QString& node,
				  const QString& service)
{
  d->node = node;
  d->service = service;
}

void KResolverResults::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


///////////////////////
// class KResolver

typedef QSet<QString> QStringSet;
QStringSet *KResolver::idnDomains;


// default constructor
KResolver::KResolver(QObject *parent)
  : QObject(parent), d(new KResolverPrivate(this))
{
}

// constructor with host and service
KResolver::KResolver(const QString& nodename, const QString& servicename,
		   QObject *parent)
  : QObject(parent), d(new KResolverPrivate(this, nodename, servicename))
{
}

// destructor
KResolver::~KResolver()
{
  cancel(false);
  delete d;
}

// get the status
int KResolver::status() const
{
  return d->status;
}

// get the error code
int KResolver::error() const
{
  return d->errorcode;
}

// get the errno
int KResolver::systemError() const
{
  return d->syserror;
}

// are we running?
bool KResolver::isRunning() const
{
  return d->status > 0 && d->status < Success;
}

// get the hostname
QString KResolver::nodeName() const
{
  return d->input.node;
}

// get the service
QString KResolver::serviceName() const
{
  return d->input.service;
}

// sets the hostname
void KResolver::setNodeName(const QString& nodename)
{
  // don't touch those values if we're working!
  if (!isRunning())
    {
      d->input.node = nodename;
      d->status = Idle;
      d->results.setAddress(nodename, d->input.service);
    }
}

// sets the service
void KResolver::setServiceName(const QString& service)
{
  // don't change if running
  if (!isRunning())
    {
      d->input.service = service;
      d->status = Idle;
      d->results.setAddress(d->input.node, service);
    }
}

// sets the address
void KResolver::setAddress(const QString& nodename, const QString& service)
{
  setNodeName(nodename);
  setServiceName(service);
}

// get the flags
int KResolver::flags() const
{
  return d->input.flags;
}

// sets the flags
int KResolver::setFlags(int flags)
{
  int oldflags = d->input.flags;
  if (!isRunning())
    {
      d->input.flags = flags;
      d->status = Idle;
    }
  return oldflags;
}

// sets the family mask
void KResolver::setFamily(int families)
{
  if (!isRunning())
    {
      d->input.familyMask = families;
      d->status = Idle;
    }
}

// sets the socket type
void KResolver::setSocketType(int type)
{
  if (!isRunning())
    {
      d->input.socktype = type;
      d->status = Idle;
    }
}

// sets the protocol
void KResolver::setProtocol(int protonum, const char *name)
{
  if (isRunning())
    return;			// can't change now

  // we copy the given protocol name. If it isn't an empty string
  // and the protocol number was 0, we will look it up in /etc/protocols
  // we also leave the error reporting to the actual lookup routines, in
  // case the given protocol name doesn't exist

  d->input.protocolName = name;
  if (protonum == 0 && name != 0L && *name != '\0')
    {
      // must look up the protocol number
      d->input.protocol = KResolver::protocolNumber(name);
    }
  else
    d->input.protocol = protonum;
  d->status = Idle;
}

bool KResolver::start()
{
  if (!isRunning())
    {
      d->results.empty();

      // is there anything to be queued?
      if (d->input.node.isEmpty() && d->input.service.isEmpty())
	{
	  d->status = KResolver::Success;
	  emitFinished();
	}
      else
	KResolverManager::manager()->enqueue(this, 0L);
    }

  return true;
}

bool KResolver::wait(int msec)
{
  if (!isRunning())
    {
      emitFinished();
      return true;
    }

  QMutexLocker locker(&d->mutex);

  if (!isRunning())
    {
      // it was running and no longer is?
      // That means the manager has finished its processing and has posted
      // an event for the signal to be emitted already. This means the signal
      // will be emitted twice!

      emitFinished();
      return true;
    }
  else
    {
      QTime t;
      t.start();

      while (!msec || t.elapsed() < msec)
	{
	  // wait on the manager to broadcast completion
	  d->waiting = true;
	  if (msec)
	    KResolverManager::manager()->notifyWaiters.wait(&d->mutex, msec - t.elapsed());
	  else
	    KResolverManager::manager()->notifyWaiters.wait(&d->mutex);

	  // the manager has processed
	  // see if this object is done
	  if (!isRunning())
	    {
	      // it's done
	      d->waiting = false;
	      emitFinished();
	      return true;
	    }
	}

      // if we've got here, we've timed out
      d->waiting = false;
      return false;
    }
}

void KResolver::cancel(bool emitSignal)
{
  KResolverManager::manager()->dequeue(this);
  if (emitSignal)
    emitFinished();
}

KResolverResults
KResolver::results() const
{
  if (!isRunning())
    return d->results;

  // return a dummy, empty result
  KResolverResults r;
  r.setAddress(d->input.node, d->input.service);
  r.setError(d->errorcode, d->syserror);
  return r;
}

bool KResolver::event(QEvent* e)
{
  if (static_cast<int>(e->type()) == KResolverManager::ResolutionCompleted)
    {
      emitFinished();
      return true;
    }

  return false;
}

void KResolver::emitFinished()
{
  if (isRunning())
    d->status = KResolver::Success;

  QPointer<QObject> p = this; // guard against deletion

  emit finished(d->results);

  if (p && d->deleteWhenDone)
    deleteLater();		// in QObject
}

QString KResolver::errorString(int errorcode, int syserror)
{
  // no i18n now...
  static const char * const messages[] =
  {
    I18N_NOOP("no error"),	// NoError
    I18N_NOOP("requested family not supported for this host name"), // AddrFamily
    I18N_NOOP("temporary failure in name resolution"),	// TryAgain
    I18N_NOOP("non-recoverable failure in name resolution"), // NonRecoverable
    I18N_NOOP("invalid flags"),			// BadFlags
    I18N_NOOP("memory allocation failure"),	// Memory
    I18N_NOOP("name or service not known"),	// NoName
    I18N_NOOP("requested family not supported"),	// UnsupportedFamily
    I18N_NOOP("requested service not supported for this socket type"), // UnsupportedService
    I18N_NOOP("requested socket type not supported"),	// UnsupportedSocketType
    I18N_NOOP("unknown error"),			// UnknownError
    I18N_NOOP2("1: the i18n'ed system error code, from errno",
	      "system error: %1")		// SystemError
  };

  // handle the special value
  if (errorcode == Canceled)
    return i18n("request was canceled");

  if (errorcode > 0 || errorcode < SystemError)
    return QString();

  QString msg = i18n(messages[-errorcode]);
  if (errorcode == SystemError)
    msg.arg(QString::fromLocal8Bit(strerror(syserror)));

  return msg;
}

KResolverResults
KResolver::resolve(const QString& host, const QString& service, int flags,
		  int families)
{
  KResolver qres(host, service, QCoreApplication::instance());
  qres.setObjectName("synchronous KResolver");
  qres.setFlags(flags);
  qres.setFamily(families);
  qres.start();
  qres.wait();
  return qres.results();
}

bool KResolver::resolveAsync(QObject* userObj, const char *userSlot,
			     const QString& host, const QString& service,
			     int flags, int families)
{
  KResolver* qres = new KResolver(host, service, QCoreApplication::instance());
  QObject::connect(qres, SIGNAL(finished(const KNetwork::KResolverResults&)), 
		   userObj, userSlot);
  qres->setObjectName("asynchronous KResolver");
  qres->setFlags(flags);
  qres->setFamily(families);
  qres->d->deleteWhenDone = true; // this is the only difference from the example code
  return qres->start();
}

QList<QByteArray> KResolver::protocolName(int protonum)
{
  struct protoent *pe;
#ifndef HAVE_GETPROTOBYNAME_R
  QMutexLocker locker(&getXXbyYYmutex);

  pe = getprotobynumber(protonum);

#else
  size_t buflen = 1024;
  struct protoent protobuf;
  char *buf;
  do
    {
      buf = new char[buflen];
# ifdef USE_SOLARIS // Solaris uses a 4 argument getprotobynumber_r which returns struct *protoent or NULL
      if ((pe = getprotobynumber_r(protonum, &protobuf, buf, buflen)) && (errno == ERANGE))
# else
      if (getprotobynumber_r(protonum, &protobuf, buf, buflen, &pe) == ERANGE)
# endif
	{
	  buflen += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (pe == 0L);
#endif

  // Do common processing
  QList<QByteArray> lst;
  if (pe != NULL)
    {
      lst.append(pe->p_name);
      for (char **p = pe->p_aliases; *p; p++)
	lst.append(*p);
    }

#ifdef HAVE_GETPROTOBYNAME_R
  delete [] buf;
#endif

  return lst;
}

QList<QByteArray> KResolver::protocolName(const char *protoname)
{
  struct protoent *pe;
#ifndef HAVE_GETPROTOBYNAME_R
  QMutexLocker locker(&getXXbyYYmutex);

  pe = getprotobyname(protoname);

#else
  size_t buflen = 1024;
  struct protoent protobuf;
  char *buf;
  do
    {
      buf = new char[buflen];
# ifdef USE_SOLARIS // Solaris uses a 4 argument getprotobyname_r which returns struct *protoent or NULL
      if ((pe = getprotobyname_r(protoname, &protobuf, buf, buflen)) && (errno == ERANGE))
# else
      if (getprotobyname_r(protoname, &protobuf, buf, buflen, &pe) == ERANGE)
# endif
	{
	  buflen += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (pe == 0L);
#endif

  // Do common processing
  QList<QByteArray> lst;
  if (pe != NULL)
    {
      lst.append(pe->p_name);
      for (char **p = pe->p_aliases; *p; p++)
	lst.append(*p);
    }

#ifdef HAVE_GETPROTOBYNAME_R
  delete [] buf;
#endif

  return lst;
}

int KResolver::protocolNumber(const char *protoname)
{
  struct protoent *pe;
#ifndef HAVE_GETPROTOBYNAME_R
  QMutexLocker locker(&getXXbyYYmutex);

  pe = getprotobyname(protoname);

#else
  size_t buflen = 1024;
  struct protoent protobuf;
  char *buf;
  do
    {
      buf = new char[buflen];
# ifdef USE_SOLARIS // Solaris uses a 4 argument getprotobyname_r which returns struct *protoent or NULL
      if ((pe = getprotobyname_r(protoname, &protobuf, buf, buflen)) && (errno == ERANGE))
# else
      if (getprotobyname_r(protoname, &protobuf, buf, buflen, &pe) == ERANGE)
# endif
	{
	  buflen += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (pe == 0L);
#endif

  // Do common processing
  int protonum = -1;
  if (pe != NULL)
    protonum = pe->p_proto;

#ifdef HAVE_GETPROTOBYNAME_R
  delete [] buf;
#endif

  return protonum;
}

int KResolver::servicePort(const char *servname, const char *protoname)
{
  struct servent *se;
#ifndef HAVE_GETSERVBYNAME_R
  QMutexLocker locker(&getXXbyYYmutex);

  se = getservbyname(servname, protoname);

#else
  size_t buflen = 1024;
  struct servent servbuf;
  char *buf;
  do
    {
      buf = new char[buflen];
# ifdef USE_SOLARIS // Solaris uses a 5 argument getservbyname_r which returns struct *servent or NULL
      if ((se = getservbyname_r(servname, protoname, &servbuf, buf, buflen)) && (errno == ERANGE))
# else
      if (getservbyname_r(servname, protoname, &servbuf, buf, buflen, &se) == ERANGE)
# endif
	{
	  buflen += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (se == 0L);
#endif

  // Do common processing
  int servport = -1;
  if (se != NULL)
    servport = ntohs(se->s_port);

#ifdef HAVE_GETSERVBYNAME_R
  delete [] buf;
#endif

  return servport;
}

QList<QByteArray> KResolver::serviceName(const char* servname, const char *protoname)
{
  struct servent *se;
#ifndef HAVE_GETSERVBYNAME_R
  QMutexLocker locker(&getXXbyYYmutex);

  se = getservbyname(servname, protoname);

#else
  size_t buflen = 1024;
  struct servent servbuf;
  char *buf;
  do
    {
      buf = new char[buflen];
# ifdef USE_SOLARIS // Solaris uses a 5 argument getservbyname_r which returns struct *servent or NULL
      if ((se = getservbyname_r(servname, protoname, &servbuf, buf, buflen)) && (errno == ERANGE))
# else
      if (getservbyname_r(servname, protoname, &servbuf, buf, buflen, &se) == ERANGE)
# endif
	{
	  buflen += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (se == 0L);
#endif

  // Do common processing
  QList<QByteArray> lst;
  if (se != NULL)
    {
      lst.append(se->s_name);
      for (char **p = se->s_aliases; *p; p++)
	lst.append(*p);
    }

#ifdef HAVE_GETSERVBYNAME_R
  delete [] buf;
#endif

  return lst;
}

QList<QByteArray> KResolver::serviceName(int port, const char *protoname)
{
  struct servent *se;
#ifndef HAVE_GETSERVBYPORT_R
  QMutexLocker locker(&getXXbyYYmutex);

  se = getservbyport(port, protoname);

#else
  size_t buflen = 1024;
  struct servent servbuf;
  char *buf;
  do
    {
      buf = new char[buflen];
# ifdef USE_SOLARIS // Solaris uses a 5 argument getservbyport_r which returns struct *servent or NULL
      if ((se = getservbyport_r(port, protoname, &servbuf, buf, buflen)) && (errno == ERANGE))
# else
      if (getservbyport_r(port, protoname, &servbuf, buf, buflen, &se) == ERANGE)
# endif
	{
	  buflen += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (se == 0L);
#endif

  // Do common processing
  QList<QByteArray> lst;
  if (se != NULL)
    {
      lst.append(se->s_name);
      for (char **p = se->s_aliases; *p; p++)
	lst.append(*p);
    }

#ifdef HAVE_GETSERVBYPORT_R
  delete [] buf;
#endif

  return lst;
}

QString KResolver::localHostName()
{
  QByteArray name;
  int len;

#ifdef MAXHOSTNAMELEN
  len = MAXHOSTNAMELEN;
#else
  len = 256;
#endif

  while (true)
    {
      name.resize(len);

      if (gethostname(name.data(), len) == 0)
	{
	  // Call succeeded, but it's not guaranteed to be NUL-terminated
	  // Fortunately, QByteArray is always NUL-terminated

	  // Note that some systems return success even if they did truncation
	  break;
	}

      // Call failed
      if (errno == ENAMETOOLONG || errno == EINVAL)
	len += 256;
      else
	{
	  // Oops! Unknown error!
	  name.clear();
	}
    }

  if (name.isEmpty())
    return QLatin1String("localhost");

  if (name.indexOf('.') == -1)
    {
      // not fully qualified
      // must resolve
      KResolverResults results = resolve(name, "0", CanonName);
      if (results.isEmpty())
	// cannot find a valid hostname!
	return QLatin1String("localhost");
      else
	return results.first().canonicalName();
    }

  return domainToUnicode(name);
}

// forward declaration
static QStringList splitLabels(const QString& unicodeDomain);
static QByteArray ToASCII(const QString& label);
static QString ToUnicode(const QString& label);

static QStringSet *KResolver_initIdnDomains()
{
  const char *kde_use_idn = getenv("KDE_USE_IDN");
  if (!kde_use_idn)
     kde_use_idn = "ac:at:br:ch:cl:cn:de:dk:fi:hu:info:io:jp:kr:li:lt:museum:no:se:sh:th:tm:tw:vn";
  return new QStringSet(QString::fromLatin1(kde_use_idn).toLower().split(':').toSet());
}

// implement the ToAscii function, as described by IDN documents
QByteArray KResolver::domainToAscii(const QString& unicodeDomain)
{
  if (!idnDomains)
    idnDomains = KResolver_initIdnDomains();

  QByteArray retval;
  // RFC 3490, section 4 describes the operation:
  // 1) this is a query, so don't allow unassigned

  // 2) split the domain into individual labels, without
  // separators.
  QStringList input = splitLabels(unicodeDomain);

  // Do we allow IDN names for this TLD?
  const QString& tld = input.at(input.count() - 1);
  if (input.count() && !idnDomains->contains(tld.toLower()))
    return input.join(".").toLower().toLatin1(); // No IDN allowed for this TLD

  // 3) decide whether to enforce the STD3 rules for chars < 0x7F
  // we don't enforce

  // 4) for each label, apply ToASCII
  QStringList::Iterator it = input.begin();
  const QStringList::Iterator end = input.end();
  for ( ; it != end; ++it)
    {
      QByteArray cs = ToASCII(*it);
      if (cs.isNull())
	return QByteArray();	// error!

      // no, all is Ok.
      if (!retval.isEmpty())
	retval += '.';
      retval += cs;
    }

  return retval;
}

QString KResolver::domainToUnicode(const QByteArray& asciiDomain)
{
  return domainToUnicode(QString::fromLatin1(asciiDomain));
}

// implement the ToUnicode function, as described by IDN documents
QString KResolver::domainToUnicode(const QString& asciiDomain)
{
  if (asciiDomain.isEmpty())
    return asciiDomain;
  if (!idnDomains)
    idnDomains = KResolver_initIdnDomains();

  QString retval;

  // draft-idn-idna-14.txt, section 4 describes the operation:
  // 1) this is a query, so don't allow unassigned
  //   besides, input is ASCII

  // 2) split the domain into individual labels, without
  // separators.
  QStringList input = splitLabels(asciiDomain);

  // Do we allow IDN names for this TLD?
  const QString& tld = input.at(input.count() - 1);
  if (input.count() && !idnDomains->contains(tld.toLower()))
    return asciiDomain.toLower(); // No TLDs allowed

  // 3) decide whether to enforce the STD3 rules for chars < 0x7F
  // we don't enforce

  // 4) for each label, apply ToUnicode
  QStringList::Iterator it;
  const QStringList::Iterator end = input.end();
  for (it = input.begin(); it != end; ++it)
    {
      QString label = ToUnicode(*it).toLower();

      // ToUnicode can't fail
      if (!retval.isEmpty())
	retval += '.';
      retval += label;
    }

  return retval;
}

QString KResolver::normalizeDomain(const QString& domain)
{
  return domainToUnicode(domainToAscii(domain));
}

void KResolver::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

// here follows IDN functions
// all IDN functions conform to the following documents:
//  RFC 3454 - Preparation of Internationalized Strings
//  RFC 3490 - Internationalizing Domain Names in Applications (IDNA)
//  RFC 3491 - Nameprep: A Stringprep Profile for
//                Internationalized Domain Names (IDN
//  RFC 3492 - Punycode: A Bootstring encoding of Unicode
//          for Internationalized Domain Names in Applications (IDNA)

static QStringList splitLabels(const QString& unicodeDomain)
{
  // From RFC 3490 section 3.1:
  // "Whenever dots are used as label separators, the following characters
  // MUST be recognized as dots: U+002E (full stop), U+3002 (ideographic full
  // stop), U+FF0E (fullwidth full stop), U+FF61 (halfwidth ideographic full
  // stop)."
  static const unsigned int separators[] = { 0x002E, 0x3002, 0xFF0E, 0xFF61 };

  QStringList lst;
  int start = 0;
  int i;
  for (i = 0; i < unicodeDomain.length(); i++)
    {
      unsigned int c = unicodeDomain[i].unicode();

      if (c == separators[0] ||
	  c == separators[1] ||
	  c == separators[2] ||
	  c == separators[3])
	{
	  // found a separator!
	  lst << unicodeDomain.mid(start, i - start);
	  start = i + 1;
	}
    }
  if ((long)i >= start)
    // there is still one left
    lst << unicodeDomain.mid(start, i - start);

  return lst;
}

static QByteArray ToASCII(const QString& label)
{
#ifdef HAVE_IDNA_H
  // We have idna.h, so we can use the idna_to_ascii
  // function :)

  if (label.length() > 64)
    return (char*)0L;		// invalid label

  if (label.length() == 0)
    // this is allowed
    return QByteArray("");	// empty, not null

  QByteArray retval;
  char buf[65];

  quint32* ucs4 = new quint32[label.length() + 1];

  int i;
  for (i = 0; i < label.length(); i++)
    ucs4[i] = (unsigned long)label[i].unicode();
  ucs4[i] = 0;			// terminate with NUL, just to be on the safe side

  if (idna_to_ascii_4i(ucs4, label.length(), buf, 0) == IDNA_SUCCESS)
    // success!
    retval = buf;

  delete [] ucs4;
  return retval;
#else
  return label.toLatin1();
#endif
}

static QString ToUnicode(const QString& label)
{
#ifdef HAVE_IDNA_H
  // We have idna.h, so we can use the idna_to_unicode
  // function :)

  quint32 *ucs4_input, *ucs4_output;
  size_t outlen;

  ucs4_input = new quint32[label.length() + 1];
  for (int i = 0; i < label.length(); i++)
    ucs4_input[i] = (unsigned long)label[i].unicode();

  // try the same length for output
  ucs4_output = new quint32[outlen = label.length()];

  idna_to_unicode_44i(ucs4_input, label.length(),
		      ucs4_output, &outlen,
		      0);

  if (outlen > (size_t)label.length())
    {
      // it must have failed
      delete [] ucs4_output;
      ucs4_output = new quint32[outlen];

      idna_to_unicode_44i(ucs4_input, label.length(),
			  ucs4_output, &outlen,
			  0);
    }

  // now set the answer
  QString result;
  result.resize(outlen);
  for (uint i = 0; i < outlen; i++)
    result[i] = (unsigned int)ucs4_output[i];

  delete [] ucs4_input;
  delete [] ucs4_output;

  return result;
#else
  return label;
#endif
}

#include "kresolver.moc"
