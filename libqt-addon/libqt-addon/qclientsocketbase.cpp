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
#include <qtimer.h>
#include <qmutex.h>

#include "qsocketaddress.h"
#include "qresolver.h"
#include "qsocketbase.h"
#include "qsocketimpl.h"
#include "qclientsocketbase.h"

ADDON_USE

class ADDON_NAMESPACE::QClientSocketBasePrivate
{
public:
  int state;

  QResolver localResolver, peerResolver;
  QResolverResults localResults, peerResults;

  bool enableRead : 1, enableWrite : 1;
};

QClientSocketBase::QClientSocketBase(QObject *parent, const char *name)
  : QObject(parent, name), d(new QClientSocketBasePrivate)
{
  d->state = Idle;
  d->enableRead = true;
  d->enableWrite = false;
}

QClientSocketBase::~QClientSocketBase()
{
  close();
  delete d;
}

QClientSocketBase::SocketState QClientSocketBase::state() const
{
  return static_cast<SocketState>(d->state);
}

void QClientSocketBase::setState(SocketState state)
{
  d->state = state;
  stateChanging(state);
}

bool QClientSocketBase::setSocketOptions(int opts)
{
  QMutexLocker locker(mutex());
  QSocketBase::setSocketOptions(opts); // call parent

  // don't create the device unnecessarily
  if (hasDevice())
    {
      bool result = socketDevice()->setSocketOptions(opts); // and set the implementation
      copyError();
      return result;
    }

  return true;
}

QResolver& QClientSocketBase::peerResolver() const
{
  return d->peerResolver;
}

const QResolverResults& QClientSocketBase::peerResults() const
{
  return d->peerResults;
}

QResolver& QClientSocketBase::localResolver() const
{
  return d->localResolver;
}

const QResolverResults& QClientSocketBase::localResults() const
{
  return d->localResults;
}

void QClientSocketBase::setResolutionEnabled(bool enable)
{
  if (enable)
    {
      d->localResolver.setFlags(d->localResolver.flags() & ~QResolver::NoResolve);
      d->peerResolver.setFlags(d->peerResolver.flags() & ~QResolver::NoResolve);
    }
  else
    {
      d->localResolver.setFlags(d->localResolver.flags() | QResolver::NoResolve);
      d->peerResolver.setFlags(d->peerResolver.flags() | QResolver::NoResolve);
    }
}

void QClientSocketBase::setFamily(int families)
{
  d->localResolver.setFamily(families);
  d->peerResolver.setFamily(families);
}

bool QClientSocketBase::lookup()
{
  if (state() == HostLookup && !blocking())
    return true;		// already doing lookup

  if (state() > HostLookup)
    return true;		// results are already available

  if (state() < HostLookup)
    {
      if (d->localResolver.serviceName().isNull() &&
	  !d->localResolver.nodeName().isNull())
	d->localResolver.setServiceName(QString::fromLatin1(""));

      // don't restart the lookups if they had succeeded and
      // the input values weren't changed
      QObject::connect(&d->peerResolver, SIGNAL(finished(QResolverResults)), 
		       this, SLOT(lookupFinishedSlot()));
      QObject::connect(&d->localResolver, SIGNAL(finished(QResolverResults)), 
		       this, SLOT(lookupFinishedSlot()));

      if (d->localResolver.status() <= 0)
	d->localResolver.start();
      if (d->peerResolver.status() <= 0)
	d->peerResolver.start();

      setState(HostLookup);
      emit stateChanged(HostLookup);

      if (!d->localResolver.isRunning() && !d->peerResolver.isRunning())
	{
	  // if nothing is running, then the lookup results are still valid
	  // pretend we had done lookup
	  if (blocking())
	    lookupFinishedSlot();
	  else
	    QTimer::singleShot(0, this, SLOT(lookupFinishedSlot()));
	}
      else
	{
	  d->localResults = d->peerResults = QResolverResults();
	}
    }

  if (blocking())
    {
      // we're in blocking mode operation
      // wait for the results

      localResolver().wait();
      peerResolver().wait();

      // lookupFinishedSlot has been called
    }

  return true;
}

bool QClientSocketBase::bind(const QResolverEntry& address)
{
  if (state() == HostLookup || state() > Connecting)
    return false;

  if (socketDevice()->bind(address))
    {
      resetError();

      // don't set the state or emit signals if we are in a higher state
      if (state() < Bound)
	{
	  setState(Bound);
	  emit stateChanged(Bound);
	  emit bound(address);
	}
      return true;
    }
  return false;
}

bool QClientSocketBase::connect(const QResolverEntry& address)
{
  if (state() == Connected)
    return true;		// to be compliant with the other classes
  if (state() == HostLookup || state() > Connecting)
    return false;

  bool ok = socketDevice()->connect(address);
  copyError();

  if (ok)
    {
      SocketState newstate;
      if (error() == InProgress)
	newstate = Connecting;
      else
	newstate = Connected;

      if (state() < newstate)
	{
	  setState(newstate);
	  emit stateChanged(newstate);
	  if (error() == NoError)
	    {
	      setFlags(IO_Sequential | IO_Raw | IO_ReadWrite | IO_Open | IO_Async);
	      emit connected(address);
	    }
	}

      return true;
    }
  return false;
}

bool QClientSocketBase::disconnect()
{
  if (state() != Connected)
    return false;

  bool ok = socketDevice()->disconnect();
  copyError();

  if (ok)
    {
      setState(Unconnected);
      emit stateChanged(Unconnected);
      return true;
    }
  return false;
}

void QClientSocketBase::close()
{
  if (state() == Idle)
    return; 			// nothing to do

  if (state() == HostLookup)
    {
      d->peerResolver.cancel(false);
      d->localResolver.cancel(false);
    }

  d->localResults = d->peerResults = QResolverResults();

  socketDevice()->close();
  setState(Idle);
  emit stateChanged(Idle);
  emit closed();
}

Q_LONG QClientSocketBase::bytesAvailable() const
{
  return socketDevice()->bytesAvailable();
}

Q_LONG QClientSocketBase::waitForMore(int msecs, bool *timeout)
{
  return socketDevice()->waitForMore(msecs, timeout);
}

Q_LONG QClientSocketBase::readBlock(char *data, Q_ULONG maxlen)
{
  return socketDevice()->readBlock(data, maxlen);
}

Q_LONG QClientSocketBase::readBlock(char *data, Q_ULONG maxlen, QSocketAddress& from)
{
  return socketDevice()->readBlock(data, maxlen, from);
}

Q_LONG QClientSocketBase::peekBlock(char *data, Q_ULONG maxlen)
{
  return socketDevice()->peekBlock(data, maxlen);
}

Q_LONG QClientSocketBase::peekBlock(char *data, Q_ULONG maxlen, QSocketAddress& from)
{
  return socketDevice()->peekBlock(data, maxlen, from);
}

Q_LONG QClientSocketBase::writeBlock(const char *data, Q_ULONG len)
{
  return socketDevice()->writeBlock(data, len);
}

Q_LONG QClientSocketBase::writeBlock(const char *data, Q_ULONG len, const QSocketAddress& to)
{
  return socketDevice()->writeBlock(data, len, to);
}

QSocketAddress QClientSocketBase::localAddress() const
{
  return socketDevice()->localAddress();
}

QSocketAddress QClientSocketBase::peerAddress() const
{
  return socketDevice()->peerAddress();
}

bool QClientSocketBase::emitsReadyRead() const
{
  return d->enableRead;
}

void QClientSocketBase::enableRead(bool enable)
{
  QMutexLocker locker(mutex());

  d->enableRead = enable;
  QSocketNotifier *n = socketDevice()->readNotifier();
  if (n)
    n->setEnabled(enable);
}

bool QClientSocketBase::emitsReadyWrite() const
{
  return d->enableWrite;
}

void QClientSocketBase::enableWrite(bool enable)
{
  QMutexLocker locker(mutex());

  d->enableWrite = enable;
  QSocketNotifier *n = socketDevice()->writeNotifier();
  if (n)
    n->setEnabled(enable);
}

void QClientSocketBase::slotReadActivity()
{
  if (d->enableRead)
    emit readyRead();
}

void QClientSocketBase::slotWriteActivity()
{
  if (d->enableWrite)
    emit readyWrite();
}

void QClientSocketBase::lookupFinishedSlot()
{
  if (d->peerResolver.isRunning() || d->localResolver.isRunning() || state() != HostLookup)
    return;

  QObject::disconnect(&d->peerResolver, 0L, this, SLOT(lookupFinishedSlot()));
  QObject::disconnect(&d->localResolver, 0L, this, SLOT(lookupFinishedSlot()));
  if (d->peerResolver.status() < 0 || d->localResolver.status() < 0)
    {
      setState(Idle);		// backtrack
      setError(IO_LookupError, LookupFailure);
      emit stateChanged(Idle);
      emit gotError(LookupFailure);
      return;
    }

  d->localResults = d->localResolver.results();
  d->peerResults = d->peerResolver.results();
  setState(HostFound);
  emit stateChanged(HostFound);
  emit hostFound();
}

void QClientSocketBase::stateChanging(SocketState newState)
{
  if (newState == Connected && socketDevice())
    {
      QSocketNotifier *n = socketDevice()->readNotifier();
      if (n)
	{
	  n->setEnabled(d->enableRead);
	  QObject::connect(n, SIGNAL(activated(int)), this, SLOT(slotReadActivity()));
	}
      else
	return;

      n = socketDevice()->writeNotifier();
      if (n)
	{
	  n->setEnabled(d->enableWrite);
	  QObject::connect(n, SIGNAL(activated(int)), this, SLOT(slotWriteActivity()));
	}
      else
	return;
    }
}

void QClientSocketBase::copyError()
{
  setError(socketDevice()->status(), socketDevice()->error());
}

#include "qclientsocketbase.moc"
