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

#include <qsocketnotifier.h>
#include <qmutex.h>

#include "qsocketaddress.h"
#include "qresolver.h"
#include "qsocketbase.h"
#include "qsocketimpl.h"
#include "qstreamsocket.h"
#include "qserversocket.h"

ADDON_USE

class ADDON_NAMESPACE::QServerSocketPrivate
{
public:
  QResolver resolver;
  QResolverResults resolverResults;

  enum { None, LookupDone, Bound, Listening } state;
  int backlog;

  bool bindWhenFound : 1, listenWhenBound : 1;

  QServerSocketPrivate()
    : state(None), bindWhenFound(false), listenWhenBound(false)
  { 
    resolver.setFlags(QResolver::Passive);
    resolver.setFamily(QResolver::KnownFamily);
  }
};

QServerSocket::QServerSocket(QObject* parent, const char *name)
  : QObject(parent, name), d(new QServerSocketPrivate)
{
  QObject::connect(&d->resolver, SIGNAL(finished(QResolverResults)), 
		   this, SLOT(lookupFinishedSlot()));
}

QServerSocket::QServerSocket(const QString& service, QObject* parent, const char *name)
  : QObject(parent, name), d(new QServerSocketPrivate)
{
  QObject::connect(&d->resolver, SIGNAL(finished(QResolverResults)), 
		   this, SLOT(lookupFinishedSlot()));
  d->resolver.setServiceName(service);
}

QServerSocket::QServerSocket(const QString& node, const QString& service,
			     QObject* parent, const char* name)
  : QObject(parent, name), d(new QServerSocketPrivate)
{
  QObject::connect(&d->resolver, SIGNAL(finished(QResolverResults)), 
		   this, SLOT(lookupFinishedSlot()));
  setAddress(node, service);
}

QServerSocket::~QServerSocket()
{
  close();
  delete d;
}

bool QServerSocket::setSocketOptions(int opts)
{
  QMutexLocker locker(mutex());
  QSocketBase::setSocketOptions(opts); // call parent
  bool result = socketDevice()->setSocketOptions(opts); // and set the implementation
  copyError();
  return result;
}

QResolver& QServerSocket::resolver() const
{
  return d->resolver;
}

const QResolverResults& QServerSocket::resolverResults() const
{
  return d->resolverResults;
}

void QServerSocket::setResolutionEnabled(bool enable)
{
  if (enable)
    d->resolver.setFlags(d->resolver.flags() & ~QResolver::NoResolve);
  else
    d->resolver.setFlags(d->resolver.flags() | QResolver::NoResolve);
}

void QServerSocket::setFamily(int families)
{
  d->resolver.setFamily(families);
}

void QServerSocket::setAddress(const QString& service)
{
  d->resolver.setNodeName(QString::null);
  d->resolver.setServiceName(service);
  d->resolverResults.empty();
}

void QServerSocket::setAddress(const QString& node, const QString& service)
{
  d->resolver.setNodeName(node);
  d->resolver.setServiceName(service);
  d->resolverResults.empty();
}

bool QServerSocket::lookup()
{
  setError(NoError);
  if (d->resolver.isRunning() && !blocking())
    return true;		// already doing lookup

  if (d->state >= QServerSocketPrivate::LookupDone)
    return true;		// results are already available

  // make sure we have at least one parameter for lookup
  if (d->resolver.serviceName().isNull() &&
      !d->resolver.nodeName().isNull())
    d->resolver.setServiceName(QString::fromLatin1(""));

  // don't restart the lookups if they had succeeded and
  // the input values weren't changed

  // reset results
  d->resolverResults = QResolverResults();

  if (d->resolver.status() <= 0)
    // if it's already running, there's no harm in calling again
    d->resolver.start();

  if (blocking())
    {
      // we're in blocking mode operation
      // wait for the results

      d->resolver.wait();
      // lookupFinishedSlot has been called
    }

  return true;
}

bool QServerSocket::bind(const QResolverEntry& address)
{
  if (socketDevice()->bind(address))
    {
      setError(NoError);

      d->state = QServerSocketPrivate::Bound;
      emit bound(address);
      return true;
    }
  copyError();
  return false;
}

bool QServerSocket::bind(const QString& node, const QString& service)
{
  setAddress(node, service);
  return bind();
}

bool QServerSocket::bind(const QString& service)
{
  setAddress(service);
  return bind();
}

bool QServerSocket::bind()
{
  if (d->state >= QServerSocketPrivate::Bound)
    return true;

  if (d->state < QServerSocketPrivate::LookupDone)
    {
      d->bindWhenFound = true;
      bool ok = lookup();	// will call bind again
      if (d->state >= QServerSocketPrivate::Bound)
	d->bindWhenFound = false;
      return ok;
    }

  if (!doBind())
    {
      setError(NotSupported);
      emit gotError(NotSupported);
      return false;
    }
  
  return true;;
}

bool QServerSocket::listen(int backlog)
{
  // WARNING
  // this function has to be reentrant
  // due to the mechanisms used for binding, this function might
  // end up calling itself

  if (d->state == QServerSocketPrivate::Listening)
    return true;		// already listening

  if (d->state < QServerSocketPrivate::Bound)
    {
      // we must bind
      // note that we can end up calling ourselves here
      d->listenWhenBound = true;
      d->backlog = backlog;
      if (!bind())
	{
	  d->listenWhenBound = false;
	  return false;
	}

      if (d->state < QServerSocketPrivate::Bound)
	// asynchronous lookup in progress...
	// we can't be blocking here anyways
	return true;

      d->listenWhenBound = false;
    }

  if (d->state < QServerSocketPrivate::Listening)
    {
      if (!socketDevice()->listen(backlog))
	{
	  copyError();
	  emit gotError(error());
	  return false;		// failed to listen
	}

      // set up ready accept signal
      QObject::connect(socketDevice()->readNotifier(), SIGNAL(activated(int)),
		       this, SIGNAL(readyAccept()));
      d->state = QServerSocketPrivate::Listening;
      return true;
    }

  return true;
}

void QServerSocket::close()
{
  socketDevice()->close();
  if (d->resolver.isRunning())
    d->resolver.cancel(false);
  d->state = QServerSocketPrivate::None;
  emit closed();
}

QActiveSocketBase* QServerSocket::accept()
{
  if (d->state < QServerSocketPrivate::Listening)
    {
      if (!blocking())
	{
	  listen();
	  setError(WouldBlock);
	  return NULL;
	}
      else if (!listen())
	// error happened during listen
	return false;
    }

  // we're listening here
  QSocketImpl* accepted = socketDevice()->accept();
  if (!accepted)
    {
      // error happened during accept
      copyError();
      return NULL;
    }

  QStreamSocket* streamsocket = new QStreamSocket();
  streamsocket->setSocketDevice(accepted);

  // FIXME!
  // when QStreamSocket can find out the state of the socket passed through
  // setSocketDevice, this will probably be unnecessary:
  streamsocket->setState(QStreamSocket::Connected);
  streamsocket->setFlags(IO_Sequential | IO_Raw | IO_ReadWrite | IO_Open | IO_Async);

  return streamsocket;
}

QSocketAddress QServerSocket::localAddress() const
{
  return socketDevice()->localAddress();
}

QSocketAddress QServerSocket::externalAddress() const
{
  return socketDevice()->externalAddress();
}

void QServerSocket::lookupFinishedSlot()
{
  if (d->resolver.isRunning())
    return;

  if (d->resolver.status() < 0)
    {
      setError(LookupFailure);
      emit gotError(LookupFailure);
      d->bindWhenFound = d->listenWhenBound = false;
      d->state = QServerSocketPrivate::None;
      return;
    }

  // lookup succeeded
  d->resolverResults = d->resolver.results();
  d->state = QServerSocketPrivate::LookupDone;
  emit hostFound();

  if (d->bindWhenFound)
    doBind();
}

void QServerSocket::copyError()
{
  setError(socketDevice()->error());
}

bool QServerSocket::doBind()
{
  d->bindWhenFound = false;
  // loop through the results and bind to the first that works

  QResolverResults::ConstIterator it = d->resolverResults.begin();
  for ( ; it != d->resolverResults.end(); ++it)
    if (bind(*it))
      {
	if (d->listenWhenBound)
	  listen(d->backlog);
	return true;
      }

  // failed to bind
  emit gotError(error());
  return false;
}

#include "qserversocket.moc"
