/*  -*- C++ -*-
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Thiago Macieira <thiagom@mail.com>
 *
 *  $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License.
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

#include "config.h"

// System includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// Qt includes
#include <qstring.h>
#include <qcstring.h>
#include <qstrlist.h>
#include <qstringlist.h>

// KDE includes
#include <klocale.h>

// Us
#include "resolver.h"		// knetwork/resolver.h
#include "resolver_p.h"

// IDN
#ifdef HAVE_IDNA_H
# include <idna.h>
#endif

using KDE::Network::Resolver;
using KDE::Network::ResolverEntry;
using KDE::Network::ResolverResults;

/////////////////////////////////////////////
// class ResolverEntry

class KDE::Network::ResolverEntryPrivate
{
public:
  KSocketAddress *addr;
  int socktype;
  int protocol;
  QString canonName;
  QCString encodedName;

  volatile int refcount;

  ResolverEntryPrivate() :
    addr(0L), socktype(0), protocol(0), 
    refcount(1)
  { }
};

// default constructor
ResolverEntry::ResolverEntry() :
  d(0L)
{
}

// constructor with stuff
ResolverEntry::ResolverEntry(const KSocketAddress* addr, int socktype, int protocol,
			     const QString& canonName, const QCString& encodedName) :
  d(new KDE::Network::ResolverEntryPrivate)
{
  d->addr = KSocketAddress::newAddress(addr->address(), addr->size());
  d->socktype = socktype;
  d->protocol = protocol;
  d->canonName = canonName;
  d->encodedName = encodedName;
}

// constructor with even more stuff
ResolverEntry::ResolverEntry(const sockaddr* sa, ksocklen_t salen, int socktype,
			     int protocol, const QString& canonName,
			     const QCString& encodedName) :
  d(new KDE::Network::ResolverEntryPrivate)
{
  d->addr = KSocketAddress::newAddress(sa, salen);
  d->socktype = socktype;
  d->protocol = protocol;
  d->canonName = canonName;
  d->encodedName = encodedName;
}

// copy constructor
ResolverEntry::ResolverEntry(const ResolverEntry& that) :
  d(0L)
{
  *this = that;
}

// destructor
ResolverEntry::~ResolverEntry()
{
  if (d == 0L)
    return;

  d->refcount--;
  if (d->refcount == 0)
    delete d;
}

// returns the socket address
const KSocketAddress* ResolverEntry::address() const
{
  return d ? d->addr : 0L;
}

// returns the length
ksocklen_t ResolverEntry::length() const
{
  return d ? d->addr->size() : 0;
}

// returns the family
int ResolverEntry::family() const
{
  return d ? d->addr->family() : AF_UNSPEC;
}

// returns the canonical name
QString ResolverEntry::canonicalName() const
{
  return d ? d->canonName : QString::null;
}

// returns the socket type
int ResolverEntry::socketType() const
{
  return d ? d->socktype : 0;
}

// returns the protocol
int ResolverEntry::protocol() const
{
  return d ? d->protocol : 0;
}

// assignment operator
ResolverEntry& ResolverEntry::operator= (const ResolverEntry& that)
{
  if (d)
    {
      d->refcount--;
      if (d->refcount == 0)
	delete d;
    }

  // copy the data
  that.d->refcount++;
  d = that.d;
  return *this;
}

/////////////////////////////////////////////
// class ResolverResults

class KDE::Network::ResolverResultsPrivate
{
public:
  QString node, service;
  int errorcode, syserror;

  int refcount;

  ResolverResultsPrivate() :
    errorcode(0), syserror(0)
  { }

  // duplicate the data if necessary, while decreasing the reference count
  // on the original data
  static void dup(ResolverResultsPrivate*& d)
  {
    if (d->refcount == 1)
      return;			// no need for duplication

    ResolverResultsPrivate *e = new ResolverResultsPrivate;
    *e = *d;			// copy
    d->refcount--;		// decrease refcount
    e->refcount = 1;		// set ours
    d = e;			// set the pointer
  }
};

// default constructor
ResolverResults::ResolverResults()
  : d(new KDE::Network::ResolverResultsPrivate)
{
}

// copy constructor
ResolverResults::ResolverResults(const ResolverResults& other)
  : QValueList<ResolverEntry>(other), d(other.d)
{
  d->refcount++;
}

// destructor
ResolverResults::~ResolverResults()
{
  d->refcount--;
  if (!d->refcount)
    delete d;
}

// assignment operator
ResolverResults&
ResolverResults::operator= (const ResolverResults& other)
{
  // release our data
  d->refcount--;
  if (!d->refcount)
    delete d;

  // copy over the other data
  d = other.d;

  // now let QValueList do the rest of the work
  QValueList<ResolverEntry>::operator =(other);
  return *this;
}

// gets the error code
int ResolverResults::errorCode() const
{
  return d->errorcode;
}

// gets the system errno
int ResolverResults::systemError() const
{
  return d->syserror;
}

// sets the error codes
void ResolverResults::setError(int errorcode, int systemerror)
{
  KDE::Network::ResolverResultsPrivate::dup(d);

  d->errorcode = errorcode;
  d->syserror = systemerror;
}

// gets the hostname
QString ResolverResults::nodeName() const
{
  return d->node;
}

// gets the service name
QString ResolverResults::serviceName() const
{
  return d->service;
}

// sets the address
void ResolverResults::setAddress(const QString& node,
				 const QString& service)
{
  KDE::Network::ResolverResultsPrivate::dup(d);

  d->node = node;
  d->service = service;
}
  
void ResolverResults::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


/////////////////////////////////////////////
// class KDE::Net::Resolver

using KDE::Network::ResolverPrivate; // from knetwork/resolver_p.h

// default constructor
Resolver::Resolver(QObject *parent, const char *name)
  : QObject(parent, name), d(new ResolverPrivate(this))
{
}

// constructor with host and service
Resolver::Resolver(const QString& nodename, const QString& servicename,
		   QObject *parent, const char *name)
  : QObject(parent, name), d(new ResolverPrivate(this, nodename, servicename))
{
}

// destructor
Resolver::~Resolver()
{
  // cancel this lookup, if it's running
  cancel(false);

  delete d;
}

// get the status
int Resolver::status() const
{
  return d->status;
}

// get the error code
int Resolver::errorCode() const
{
  return d->errorcode;
}

// get the errno
int Resolver::systemError() const
{
  return d->syserror;
}

// are we running?
bool Resolver::isRunning() const
{
  return d->status == InProgress || d->status == Queued;
}

// get the hostname
QString Resolver::nodeName() const
{
  return d->node;
}

// get the service
QString Resolver::serviceName() const
{
  return d->service;
}

// sets the hostname
void Resolver::setNode(const QString& nodename)
{
  // don't touch those values if we're working!
  if (!isRunning())
    d->node = nodename;
}

// sets the service
void Resolver::setService(const QString& service)
{
  // don't change if running
  if (!isRunning())
    d->service = service;
}

// sets the address
void Resolver::setAddress(const QString& nodename, const QString& service)
{
  setNode(nodename);
  setService(service);
}

// get the flags
int Resolver::flags() const
{
  return d->flags;
}

// sets the flags
int Resolver::setFlags(int flags)
{
  int oldflags = d->flags;
  if (!isRunning())
    d->flags = flags;
  return oldflags;
}

// sets the family mask
void Resolver::setFamily(int families)
{
  if (!isRunning())
    d->familyMask = families;
}

// sets the socket type
void Resolver::setSocketType(int type)
{
  if (!isRunning() && d->status != Queued)
    d->socktype = type;
}

// sets the protocol
void Resolver::setProtocol(int protonum, const char *name)
{
  if (isRunning() || d->status == Queued)
    return;			// can't change now

  // we copy the given protocol name. If it isn't an empty string
  // and the protocol number was 0, we will look it up in /etc/protocols
  // we also leave the error reporting to the actual lookup routines, in
  // case the given protocol name doesn't exist

  d->protocolName = name;
  if (protonum == 0 && name != 0L && *name != '\0')
    {
      // must look up the protocol number
      d->protocol = Resolver::protocolNumber(name);
    }
  else
    d->protocol = protonum;
}

void Resolver::cancel(bool)
{
}

bool Resolver::start()
{
}

ResolverResults
Resolver::results() const
{
  if (!isRunning() && d->status != Queued)
    return *(ResolverResults*)&d->results;

  ResolverResults r;
  r.setAddress(d->node, d->service);
  r.setError(d->errorcode, d->syserror);
  return r;
}

QString Resolver::strError(int errorcode, int syserror)
{
  static const char * const messages[] =
  {
    I18N_NOOP("no error"),	// NoError
    I18N_NOOP("requested family not supported for this host name"), // AddrFamily
    I18N_NOOP("temporary failure in name resolution"),	// TryAgain
    I18N_NOOP("non-recoverable failure in name resolution"), // Failure
    I18N_NOOP("invalid flags"),			// BadFlags
    I18N_NOOP("memory allocation failure"),	// Memory
    I18N_NOOP("no address associated with nodename"),	// NoData
    I18N_NOOP("name or service not known"),	// NoName
    I18N_NOOP("requested family not supported"),	// UnsupportedFamily
    I18N_NOOP("requested service not supported for this socket type"), // UnsupportedService
    I18N_NOOP("requested socket type not supported"),	// UnsupportedSocketType
    I18N_NOOP("system error: %1")		// SystemError
  };

  // handle the special value
  if (errorcode == Canceled)
    return i18n("request was canceled");

  if (errorcode > 0 || errorcode < SystemError)
    return QString::null;

  QString msg = i18n(messages[-errorcode]);
  if (errorcode == SystemError)
    msg.arg(QString::fromLocal8Bit(strerror(syserror)));

  return msg;
}

ResolverResults
Resolver::resolve(const QString& host, const QString& service, int flags,
		  int families)
{
  Resolver kres(host, service);
  kres.setFlags(flags);
  kres.setFamily(families);
  kres.start();
  return kres.results();
}

bool Resolver::reverseResolve(const KSocketAddress* addr, QString& node,
			      QString& serv, int flags)
{
  return reverseResolve(addr->address(), addr->size(), node, serv, flags);
}

bool Resolver::reverseResolve(const sockaddr* sa, ksocklen_t salen,
			      QString& host, QString& serv, int flags)
{
  int err;
  char h[NI_MAXHOST], s[NI_MAXSERV];
  int niflags = 0;

  h[0] = s[0] = '\0';

  if (flags & NumericHost)
    niflags |= NI_NUMERICHOST;
  if (flags & NumericService)
    niflags |= NI_NUMERICSERV;
  if (flags & NodeNameOnly)
    niflags |= NI_NOFQDN;
  if (flags & Datagram)
    niflags |= NI_DGRAM;

  err = getnameinfo(sa, salen, h, sizeof(h) - 1, s, sizeof(s) - 1, niflags);
  host = domainToUnicode(QString::fromLatin1(h));
  serv = QString::fromLatin1(s);

  return err != 0;
}

#if !defined(HAVE_GETPROTOBYNAME_R) || !defined(HAVE_GETSERVBYNAME_R)
static QMutex getXXbyYYmutex;
#endif

QStrList Resolver::protocolName(int protonum)
{
  struct protoent *pe;
#ifndef HAVE_GETPROTOBYNAME_R
  QMutexLocker locker(&getXXbyYYmutex);

  pe = getprotobynumber(protonum);

#else
  size_t buflen = 1024;
  struct protoent protobuf;
  do
    {
      char *buf = new char[buflen];
      if (getprotobynumber_r(protonum, &protobuf, buf, buflen, &pe) != 0)
	{
	  buf += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (pe == 0L);
#endif

  // Do common processing
  QStrList lst(true);	// use deep copies
  if (pe != NULL)
    {
      lst.append(pe->p_name);
      for (char **p = pe->p_aliases; *p; p++)
	lst.append(*p);
    }

#ifndef HAVE_GETPROTOBYNAME_R
  delete [] buf;
#endif

  return lst;
}

QStrList protocolName(const char *protoname)
{
  struct protoent *pe;
#ifndef HAVE_GETPROTOBYNAME_R
  QMutexLocker locker(&getXXbyYYmutex);

  pe = getprotobyname(protoname);

#else
  size_t buflen = 1024;
  struct protoent protobuf;
  do
    {
      char *buf = new char[buflen];
      if (getprotobyname_r(protoname, &protobuf, buf, buflen, &pe) != 0)
	{
	  buf += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (pe == 0L);
#endif

  // Do common processing
  QStrList lst(true);	// use deep copies
  if (pe != NULL)
    {
      lst.append(pe->p_name);
      for (char **p = pe->p_aliases; *p; p++)
	lst.append(*p);
    }

#ifndef HAVE_GETPROTOBYNAME_R
  delete [] buf;
#endif

  return lst;
}

int Resolver::protocolNumber(const char *protoname)
{
  struct protoent *pe;
#ifndef HAVE_GETPROTOBYNAME_R
  QMutexLocker locker(&getXXbyYYmutex);

  pe = getprotobyname(protoname);

#else
  size_t buflen = 1024;
  struct protoent protobuf;
  do
    {
      char *buf = new char[buflen];
      if (getprotobyname_r(protoname, &protobuf, buf, buflen, &pe) != 0)
	{
	  buf += 1024;
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

#ifndef HAVE_GETPROTOBYNAME_R
  delete [] buf;
#endif

  return protonum;
}

int Resolver::servicePort(const char *servname, const char *protoname)
{
  struct servent *se;
#ifndef HAVE_GETSERVBYNAME_R
  QMutexLocker locker(&getXXbyYYmutex);

  se = getservbyname(servname, protoname);

#else
  size_t buflen = 1024;
  struct servent servbuf;
  do
    {
      char *buf = new char[buflen];
      if (getservbyname_r(servname, protoname, &servbuf, buf, buflen, &se) != 0)
	{
	  buf += 1024;
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

#ifndef HAVE_GETSERVBYNAME_R
  delete [] buf;
#endif

  return servport;
}

QStrList serviceName(const char* servname, const char *protoname)
{
  struct servent *se;
#ifndef HAVE_GETSERVBYNAME_R
  QMutexLocker locker(&getXXbyYYmutex);

  se = getservbyname(servname, protoname);

#else
  size_t buflen = 1024;
  struct servent servbuf;
  do
    {
      char *buf = new char[buflen];
      if (getservbyname_r(servname, protoname, &servbuf, buf, buflen, &se) != 0)
	{
	  buf += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (se == 0L);
#endif

  // Do common processing
  QStrList lst(true);	// use deep copies
  if (se != NULL)
    {
      lst.append(se->s_name);
      for (char **p = se->s_aliases; *p; p++)
	lst.append(*p);
    }

#ifndef HAVE_GETSERVBYNAME_R
  delete [] buf;
#endif

  return lst;
}

QStrList Resolver::serviceName(int port, const char *protoname)
{
  struct servent *se;
#ifndef HAVE_GETSERVBYNAME_R
  QMutexLocker locker(&getXXbyYYmutex);

  se = getservbyport(port, protoname);

#else
  size_t buflen = 1024;
  struct servent servbuf;
  do
    {
      char *buf = new char[buflen];
      if (getservbyport_r(port, protoname, &servbuf, buf, buflen, &se) != 0)
	{
	  buf += 1024;
	  delete [] buf;
	}
      else
	break;
    }
  while (se == 0L);
#endif

  // Do common processing
  QStrList lst(true);	// use deep copies
  if (se != NULL)
    {
      lst.append(se->s_name);
      for (char **p = se->s_aliases; *p; p++)
	lst.append(*p);
    }

#ifndef HAVE_GETSERVBYNAME_R
  delete [] buf;
#endif

  return lst;
}

// forward declaration
static QStringList splitLabels(const QString& unicodeDomain);
static QCString ToASCII(const QString& label);
static QString ToUnicode(const QString& label);
  
// implement the ToAscii function, as described by IDN documents
QCString Resolver::domainToAscii(const QString& unicodeDomain)
{
  QCString retval;
  // draft-idn-idna-14.txt, section 4 describes the operation:
  // 1) this is a query, so don't allow unassigned

  // 2) split the domain into individual labels, without
  // separators.
  QStringList input = splitLabels(unicodeDomain);

  // 3) decide whether to enforce the STD3 rules for chars < 0x7F
  // we don't enforce

  // 4) for each label, apply ToASCII
  QStringList::Iterator it = input.begin();
  for ( ; it != input.end(); it++)
    {
      QCString cs = ToASCII(*it);
      if (cs.isNull())
	return QCString();	// error!

      // no, all is Ok.
      if (!retval.isEmpty())
	retval += '.';
      retval += cs;
    }

  return retval;
}

QString Resolver::domainToUnicode(const QCString& asciiDomain)
{
  return domainToUnicode(QString::fromLatin1(asciiDomain));
}

// implement the ToUnicode function, as described by IDN documents
QString Resolver::domainToUnicode(const QString& asciiDomain)
{
  if (asciiDomain.isEmpty())
    return asciiDomain;

  QString retval;

  // draft-idn-idna-14.txt, section 4 describes the operation:
  // 1) this is a query, so don't allow unassigned
  //   besides, input is ASCII

  // 2) split the domain into individual labels, without
  // separators.
  QStringList input = splitLabels(asciiDomain);

  // 3) decide whether to enforce the STD3 rules for chars < 0x7F
  // we don't enforce

  // 4) for each label, apply ToUnicode
  QStringList::Iterator it;
  for (it = input.begin(); it != input.end(); it++)
    {
      QString label = ToUnicode(*it);

      // ToUnicode can't fail
      if (!retval.isEmpty())
	retval += '.';
      retval += label;
    }

  return retval;
}

QString Resolver::normalizeDomain(const QString& domain)
{
  return domainToUnicode(domainToAscii(domain));
}

void Resolver::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

// here follows IDN functions
// all IDN functions conform to the following documents:
//  RFC 3454 - Preparation of Internationalized Strings
//  http://www.ietf.org/internet-drafts/draft-ietf-idn-nameprep-11.txt -
//	A Stringprep profile for Internationalized Domain Names
//  http://www.ietf.org/internet-drafts/draft-ietf-idn-punycode-03.txt -
//	Punycode: A Bootstring encoding of Unicode for IDNA
//  http://www.ietf.org/internet-drafts/draft-ietf-idn-idna-14.txt -
//	Internationalizing Domain Names in Applications (IDNA)
// it is expected that the latter three documents become RFCs

static QStringList splitLabels(const QString& unicodeDomain)
{
  // From draft-idn-idna-14.txt section 3.1:
  // "Whenever dots are used as label separators, the following characters
  // MUST be recognized as dots: U+002E (full stop), U+3002 (ideographic full
  // stop), U+FF0E (fullwidth full stop), U+FF61 (halfwidth ideographic full
  // stop)."
  static const unsigned int separators[] = { 0x002E, 0x3002, 0xFF0E, 0xFF61 };

  QStringList lst;
  int start = 0;
  uint i;
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
  if ((long)i > start)
    // there is still one left
    lst << unicodeDomain.mid(start, i - start);

  return lst;
}

#if 1

static QCString ToASCII(const QString& label)
{
#ifdef HAVE_IDNA_H
  // We have idna.h, so we can use the idna_to_ascii
  // function :)

  QCString retval;
  char buf[65];
  unsigned long *ucs4 = new unsigned long[label.length()];
  for (uint i = 0; i < label.length(); i++)
    ucs4[i] = (unsigned long)label[i].unicode();

  if (idna_to_ascii(ucs4, label.length(), buf,
		    /* allowunassigned = */ false,
		    /* usestd3asciirules = */ false) == 0)
    // success!
    retval = buf;

  delete [] ucs4;
  return retval;
#else
  return label.latin1();
#endif
}

static QString ToUnicode(const QString& label)
{
#ifdef HAVE_IDNA_H
  // We have idna.h, so we can use the idna_to_unicode
  // function :)

  unsigned long *ucs4_input, *ucs4_output;
  size_t outlen;

  ucs4_input = new unsigned long[label.length()];
  for (uint i = 0; i < label.length(); i++)
    ucs4_input[i] = (unsigned long)label[i].unicode();

  // try the same length for output
  ucs4_output = new unsigned long[outlen = label.length()];

  idna_to_unicode(ucs4_input, label.length(),
		  ucs4_output, &outlen,
		  /* allowunassigned = */ false,
		  /* usestd3asciirules = */ false);

  if (outlen > label.length())
    {
      // it must have failed
      delete [] ucs4_output;
      ucs4_output = new unsigned long[outlen];

      idna_to_unicode(ucs4_input, label.length(),
		      ucs4_output, &outlen,
		      /* allowunassigned = */ false,
		      /* usestd3asciirules = */ false);
    }

  // now set the answer
  QString result;
  result.setLength(outlen);
  for (uint i = 0; i < outlen; i++)
    result[i] = (unsigned int)ucs4_output[i];

  delete [] ucs4_input;
  delete [] ucs4_output;
  
  return result;
#else
  return label;
#endif
}

#else  // 0
static QValueList<QCString> splitLabels(const QCString& asciiDomain)
{
  // here, all separators are dots (0x2E)
  QValueList<QCString> lst;

  if (asciiDomain.isEmpty())
    return lst;
  if (asciiDomain == ".")
    {
      lst <<  QCString("");	// empty, not null
      return lst;
    }

  // we're guaranteed now to have at least one char
  int start = 0, i;
  do
    {
      i = asciiDomain.find('.', start);
      if (i == -1)
	i = asciiDomain.length();
      lst.append(asciiDomain.mid(start, i - start));
      start = i + 1;
    }
  while (i < (int)asciiDomain.length());

  return lst;
}

// IDN functions forward declarations
static const char ACEPrefix[] = "zz--"; // FIXME! Change this to the right value 
				        // it's been published!

static QString nameprep(const QString& unprepped);
static QCString punyencode(const QString& uncoded);
static QCString ToASCII(const QString& label);
static QValueList<QCString> splitLabels(const QCString& asciiDomain);
static QString ToUnicode(const QString& label);
static QString punydecode(const QCString& encoded);



static QCString ToASCII(QString s)
{
  // Section 4.1: ToASCII
  // "The ToASCII operation takes a sequence of Unicode code points that make
  // up one label and transforms it into a sequence of code points in the
  // ASCII range (0..7F). If ToASCII succeeds, the original sequence and the
  // resulting sequence are equivalent labels.
  // It is important to note that the ToASCII operation can fail."

  //    1. If all code points in the sequence are in the ASCII range (0..7F)
  //       then skip to step 3.
  bool allascii = true;
  for (uint i = 0; allascii && i < s.length(); i++)
    if (s[i].unicode() > 0x007F)
      allascii = false;

  if (!allascii)
    {
      //    2. Perform the steps specified in [NAMEPREP] and fail if there is
      //       an error. The AllowUnassigned flag is used in [NAMEPREP].
      // so we do nameprep here
      s = nameprep(s);
      if (s.isNull())
	// error!
	return QCString();

      //    3. If the UseSTD3ASCIIRules flag is set, then perform these checks:
      // it isn't set :)

      //    4. If all code points in the sequence are in the ASCII range
      //       (0..7F), then skip to step 8.
      allascii = true;
      for (uint i = 0; allascii && i < s.length(); i++)
	if (s[i].unicode() > 0x007F)
	  allascii = false;
    }
  //else
    //    3. If the UseSTD3ASCIIRules flag is set, then perform these checks:
    // it isn't set :)


  QCString cs;
  if (!allascii)
    {
      //    5. Verify that the sequence does NOT begin with the ACE prefix.
      if (s.left(sizeof(ACEPrefix) - 1).lower() == ACEPrefix)
	// error
	// we can't have a string starting with the ACE prefix that is not
	// all ASCII already
	return QCString();

      //    6. Encode the sequence using the encoding algorithm in [PUNYCODE]
      //       and fail if there is an error.
      cs = punyencode(s);

      //    7. Prepend the ACE prefix.
      cs.prepend(ACEPrefix);
    }
  else
    // it is all ASCII
    cs = s.latin1();

  //    8. Verify that the number of code points is in the range 1 to 63
  //       inclusive.
  if (cs.isEmpty() || cs.length() > 63)
    // error again!
    return QCString();

  // all is ok!
  return cs;
}

static QString ToUnicode(const QCString& s)
{
  // Section 4.2: ToUnicode
  // "The ToUnicode operation takes a sequence of Unicode code points that
  // make up one label and returns a sequence of Unicode code points. If the
  // input sequence is a label in ACE form, then the result is an equivalent
  // internationalized label that is not in ACE form, otherwise the original
  // sequence is returned unaltered.
  //
  // ToUnicode never fails. If any step fails, then the original input
  // sequence is returned immediately in that step."

  // note that the documents refer to this as a Unicode string. This 
  // implementation works only with 8-bit character strings.

  //      1. If all code points in the sequence are in the ASCII range (0..7F)
  //       then skip to step 3.
  bool allascii = true;
  for (uint i = 0; allascii && i < s.length(); i++)
    if ((unsigned char)s[i] > 0x7F)
      allascii = false;

  if (!allascii)
    {
      //    2. Perform the steps specified in [NAMEPREP] and fail if there is an
      //       error.
      // FIXME! Why?

      //s = nameprep(s);
      // also note: this is not allowed! s is const!
    }

  //    3. Verify that the sequence begins with the ACE prefix, and save a
  //       copy of the sequence.
  if (qstrnicmp(s, ACEPrefix, sizeof(ACEPrefix) - 1) != 0)
    return s;

  //    4. Remove the ACE prefix.
  QCString unprefixed = s.mid(sizeof(ACEPrefix) - 1);
  
  //    5. Decode the sequence using the decoding algorithm in [PUNYCODE]
  //       and fail if there is an error. Save a copy of the result of
  //       this step.
  QString us = punydecode(unprefixed);

  //    6. Apply ToASCII.
  //    7. Verify that the result of step 6 matches the saved copy from
  //       step 3, using a case-insensitive ASCII comparison.
  if (qstricmp(s.data(), ToASCII(us).data()) != 0)
    return s;			// error, they don't match

  //    8. Return the saved copy from step 5.
  return us;
}

#endif // 0

#include "resolver.moc"
