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

#include "config.h"

// System includes
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// Qt includes
#include <qapplication.h>
#include <qstring.h>
#include <qcstring.h>
#include <qstrlist.h>
#include <qstringlist.h>
#include <qshared.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qmutex.h>
#include <qguardedptr.h>

// IDN
#ifdef HAVE_IDNA_H
# include <idna.h>
#endif

// Us
#include "qresolver.h"
#include "qresolver_p.h"
#include "qsocketaddress.h"

ADDON_USE
using namespace ADDON_NAMESPACE::Internal;

/////////////////////////////////////////////
// class QResolverEntry

class ADDON_NAMESPACE::QResolverEntryPrivate: public QShared
{
public:
  QSocketAddress addr;
  int socktype;
  int protocol;
  QString canonName;
  QCString encodedName;

  inline QResolverEntryPrivate() :
    socktype(0), protocol(0)
  { }
};

// default constructor
QResolverEntry::QResolverEntry() :
  d(0L)
{
}

// constructor with stuff
QResolverEntry::QResolverEntry(const QSocketAddress& addr, int socktype, int protocol,
			       const QString& canonName, const QCString& encodedName) :
  d(new QResolverEntryPrivate)
{
  d->addr = addr;
  d->socktype = socktype;
  d->protocol = protocol;
  d->canonName = canonName;
  d->encodedName = encodedName;
}

// constructor with even more stuff
QResolverEntry::QResolverEntry(const struct sockaddr* sa, Q_UINT16 salen, int socktype,
			       int protocol, const QString& canonName,
			       const QCString& encodedName) :
  d(new QResolverEntryPrivate)
{
  d->addr = QSocketAddress(sa, salen);
  d->socktype = socktype;
  d->protocol = protocol;
  d->canonName = canonName;
  d->encodedName = encodedName;
}

// copy constructor
QResolverEntry::QResolverEntry(const QResolverEntry& that) :
  d(0L)
{
  *this = that;
}

// destructor
QResolverEntry::~QResolverEntry()
{
  if (d == 0L)
    return;

  if (d->deref())
    delete d;
}

// returns the socket address
QSocketAddress QResolverEntry::address() const
{
  return d ? d->addr : QSocketAddress();
}

// returns the length
Q_UINT16 QResolverEntry::length() const
{
  return d ? d->addr.length() : 0;
}

// returns the family
int QResolverEntry::family() const
{
  return d ? d->addr.family() : AF_UNSPEC;
}

// returns the canonical name
QString QResolverEntry::canonicalName() const
{
  return d ? d->canonName : QString::null;
}

// returns the encoded name
QCString QResolverEntry::encodedName() const
{
  return d ? d->encodedName : QCString();
}

// returns the socket type
int QResolverEntry::socketType() const
{
  return d ? d->socktype : 0;
}

// returns the protocol
int QResolverEntry::protocol() const
{
  return d ? d->protocol : 0;
}

// assignment operator
QResolverEntry& QResolverEntry::operator= (const QResolverEntry& that)
{
  // copy the data
  if (that.d)
    that.d->ref();

  if (d && d->deref())
    delete d;

  d = that.d;
  return *this;
}

/////////////////////////////////////////////
// class QResolverResults

class ADDON_NAMESPACE::QResolverResultsPrivate: public QShared
{
public:
  QString node, service;
  int errorcode, syserror;

  QResolverResultsPrivate() :
    errorcode(0), syserror(0)
  { }

  // duplicate the data if necessary, while decreasing the reference count
  // on the original data
  inline void dup(QResolverResultsPrivate*& d)
  {
    if (!d->count > 1)
      {
	d->deref();
	QResolverResultsPrivate *e = new QResolverResultsPrivate(*d);
	e->count = 1;
	d = e;			// set the pointer
      }
  }
};

// default constructor
QResolverResults::QResolverResults()
  : d(new QResolverResultsPrivate)
{
}

// copy constructor
QResolverResults::QResolverResults(const QResolverResults& other)
  : QValueList<QResolverEntry>(other), d(other.d)
{
  d->ref();
}

// destructor
QResolverResults::~QResolverResults()
{
  if (d->deref())
    delete d;
}

// assignment operator
QResolverResults&
QResolverResults::operator= (const QResolverResults& other)
{
  other.d->ref();

  // release our data
  if (d->deref())
    delete d;

  // copy over the other data
  d = other.d;

  // now let QValueList do the rest of the work
  QValueList<QResolverEntry>::operator =(other);

  return *this;
}

// gets the error code
int QResolverResults::errorCode() const
{
  return d->errorcode;
}

// gets the system errno
int QResolverResults::systemError() const
{
  return d->syserror;
}

// sets the error codes
void QResolverResults::setError(int errorcode, int systemerror)
{
  d->dup(d);

  d->errorcode = errorcode;
  d->syserror = systemerror;
}

// gets the hostname
QString QResolverResults::nodeName() const
{
  return d->node;
}

// gets the service name
QString QResolverResults::serviceName() const
{
  return d->service;
}

// sets the address
void QResolverResults::setAddress(const QString& node,
				  const QString& service)
{
  d->dup(d);

  d->node = node;
  d->service = service;
}
  
void QResolverResults::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


///////////////////////
// class QResolver

// default constructor
QResolver::QResolver(QObject *parent, const char *name)
  : QObject(parent, name), d(new QResolverPrivate(this))
{
}

// constructor with host and service
QResolver::QResolver(const QString& nodename, const QString& servicename,
		   QObject *parent, const char *name)
  : QObject(parent, name), d(new QResolverPrivate(this, nodename, servicename))
{
}

// destructor
QResolver::~QResolver()
{
  // this deletes our d pointer (if necessary)
  // and cancels the lookup as well
  QResolverManager::manager()->aboutToBeDeleted(this);
  d = 0L;
}

// get the status
int QResolver::status() const
{
  return d->status;
}

// get the error code
int QResolver::errorCode() const
{
  return d->errorcode;
}

// get the errno
int QResolver::systemError() const
{
  return d->syserror;
}

// are we running?
bool QResolver::isRunning() const
{
  return d->status > 0 && d->status < Success;
}

// get the hostname
QString QResolver::nodeName() const
{
  return d->input.node;
}

// get the service
QString QResolver::serviceName() const
{
  return d->input.service;
}

// sets the hostname
void QResolver::setNodeName(const QString& nodename)
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
void QResolver::setServiceName(const QString& service)
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
void QResolver::setAddress(const QString& nodename, const QString& service)
{
  setNodeName(nodename);
  setServiceName(service);
}

// get the flags
int QResolver::flags() const
{
  return d->input.flags;
}

// sets the flags
int QResolver::setFlags(int flags)
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
void QResolver::setFamily(int families)
{
  if (!isRunning())
    {
      d->input.familyMask = families;
      d->status = Idle;
    }
}

// sets the socket type
void QResolver::setSocketType(int type)
{
  if (!isRunning())
    {
      d->input.socktype = type;
      d->status = Idle;
    }
}

// sets the protocol
void QResolver::setProtocol(int protonum, const char *name)
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
      d->input.protocol = QResolver::protocolNumber(name);
    }
  else
    d->input.protocol = protonum;
  d->status = Idle;
}

bool QResolver::start()
{
  if (!isRunning())
    {
      d->results.empty();
      d->emitSignal = true;	// reset the variable

      // is there anything to be queued?
      if (d->input.node.isEmpty() && d->input.service.isEmpty())
	{
	  d->status = QResolver::Success;
	  emitFinished();
	}
      else
	QResolverManager::manager()->enqueue(this, 0L);
    }

  return true;
}

bool QResolver::wait(int msec)
{
  if (!isRunning())
    {
      emitFinished();
      return true;
    }

  QMutexLocker locker(&d->mutex);

  if (!isRunning())
    return true;
  else
    {
      QTime t;
      t.start();

      while (!msec || t.elapsed() < msec)
	{
	  // wait on the manager to broadcast completion
	  d->waiting = true;
	  if (msec)
	    QResolverManager::manager()->notifyWaiters.wait(&d->mutex, msec - t.elapsed());
	  else
	    QResolverManager::manager()->notifyWaiters.wait(&d->mutex);

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

void QResolver::cancel(bool emitSignal)
{
  d->emitSignal = emitSignal;
  QResolverManager::manager()->dequeue(this);
}

QResolverResults
QResolver::results() const
{
  if (!isRunning())
    return d->results;

  // return a dummy, empty result
  QResolverResults r;
  r.setAddress(d->input.node, d->input.service);
  r.setError(d->errorcode, d->syserror);
  return r;
}

bool QResolver::event(QEvent* e)
{
  if (static_cast<int>(e->type()) == QResolverManager::ResolutionCompleted)
    {
      emitFinished();
      return true;
    }

  return false;
}

void QResolver::emitFinished()
{
  if (isRunning())
    d->status = QResolver::Success;

  QGuardedPtr<QObject> p = this; // guard against deletion

  if (d->emitSignal)
    emit finished(d->results);

  if (p && d->deleteWhenDone)
    deleteLater();		// in QObject
}

QString QResolver::strError(int errorcode, int syserror)
{
  // no i18n now...
#define i18n
#define I18N_NOOP
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

#undef i18n
#undef I18N_NOOP
}

QResolverResults
QResolver::resolve(const QString& host, const QString& service, int flags,
		  int families)
{
  QResolver qres(host, service, qApp, "synchronous QResolver");
  qres.setFlags(flags);
  qres.setFamily(families);
  qres.start();
  qres.wait();
  return qres.results();
}

bool QResolver::resolveAsync(QObject* userObj, const char *userSlot,
			     const QString& host, const QString& service,
			     int flags, int families)
{
  QResolver* qres = new QResolver(host, service, qApp, "asynchronous QResolver");
  QObject::connect(qres, SIGNAL(finished(QResolverResults)), userObj, userSlot);
  qres->setFlags(flags);
  qres->setFamily(families);
  qres->d->deleteWhenDone = true; // this is the only difference from the example code
  return qres->start();
}

#ifdef NEED_MUTEX
QMutex getXXbyYYmutex;
#endif

QStrList QResolver::protocolName(int protonum)
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
      if (getprotobynumber_r(protonum, &protobuf, buf, buflen, &pe) == ERANGE)
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
  QStrList lst(true);	// use deep copies
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

QStrList QResolver::protocolName(const char *protoname)
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
      if (getprotobyname_r(protoname, &protobuf, buf, buflen, &pe) == ERANGE)
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
  QStrList lst(true);	// use deep copies
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

int QResolver::protocolNumber(const char *protoname)
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
      if (getprotobyname_r(protoname, &protobuf, buf, buflen, &pe) == ERANGE)
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

int QResolver::servicePort(const char *servname, const char *protoname)
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
      if (getservbyname_r(servname, protoname, &servbuf, buf, buflen, &se) == ERANGE)
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

QStrList QResolver::serviceName(const char* servname, const char *protoname)
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
      if (getservbyname_r(servname, protoname, &servbuf, buf, buflen, &se) == ERANGE)
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
  QStrList lst(true);	// use deep copies
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

QStrList QResolver::serviceName(int port, const char *protoname)
{
  struct servent *se;
#ifndef HAVE_GETSERVBYNAME_R
  QMutexLocker locker(&getXXbyYYmutex);

  se = getservbyport(port, protoname);

#else
  size_t buflen = 1024;
  struct servent servbuf;
  char *buf;
  do
    {
      buf = new char[buflen];
      if (getservbyport_r(port, protoname, &servbuf, buf, buflen, &se) == ERANGE)
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
  QStrList lst(true);	// use deep copies
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

// forward declaration
static QStringList splitLabels(const QString& unicodeDomain);
static QCString ToASCII(const QString& label);
static QString ToUnicode(const QString& label);
  
// implement the ToAscii function, as described by IDN documents
QCString QResolver::domainToAscii(const QString& unicodeDomain)
{
  QCString retval;
  // RFC 3490, section 4 describes the operation:
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

QString QResolver::domainToUnicode(const QCString& asciiDomain)
{
  return domainToUnicode(QString::fromLatin1(asciiDomain));
}

// implement the ToUnicode function, as described by IDN documents
QString QResolver::domainToUnicode(const QString& asciiDomain)
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
      QString label = ToUnicode(*it).lower();

      // ToUnicode can't fail
      if (!retval.isEmpty())
	retval += '.';
      retval += label;
    }

  return retval;
}

QString QResolver::normalizeDomain(const QString& domain)
{
  return domainToUnicode(domainToAscii(domain));
}

void QResolver::virtual_hook( int, void* )
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

static QCString ToASCII(const QString& label)
{
#ifdef HAVE_IDNA_H
  // We have idna.h, so we can use the idna_to_ascii
  // function :)

  if (label.length() > 64)
    return (char*)0L;		// invalid label

  QCString retval;
  char buf[65];

  Q_UINT32* ucs4 = new Q_UINT32[label.length() + 1];

  uint i;
  for (i = 0; i < label.length(); i++)
    ucs4[i] = (unsigned long)label[i].unicode();
  ucs4[i] = 0;			// terminate with NUL, just to be on the safe side

  if (idna_to_ascii_4i(ucs4, label.length(), buf, 0) == IDNA_SUCCESS)
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

  Q_UINT32 *ucs4_input, *ucs4_output;
  size_t outlen;

  ucs4_input = new Q_UINT32[label.length() + 1];
  for (uint i = 0; i < label.length(); i++)
    ucs4_input[i] = (unsigned long)label[i].unicode();

  // try the same length for output
  ucs4_output = new Q_UINT32[outlen = label.length()];

  idna_to_unicode_44i(ucs4_input, label.length(),
		      ucs4_output, &outlen,
		      0);

  if (outlen > label.length())
    {
      // it must have failed
      delete [] ucs4_output;
      ucs4_output = new Q_UINT32[outlen];

      idna_to_unicode_44i(ucs4_input, label.length(),
			  ucs4_output, &outlen,
			  0);
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

#include "qresolver.moc"
