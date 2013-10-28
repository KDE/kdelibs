/*  -*- C++ -*-
 *  Copyright (C) 2003,2005 Thiago Macieira <thiago@kde.org>
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

#include "k3clientsocketbase.h"

#include <config-network.h>

#include <QSocketNotifier>
#include <QTimer>
#include <QMutex>

#include "k3socketaddress.h"
#include "k3resolver.h"
#include "k3socketbase.h"
#include "k3socketdevice.h"

using namespace KNetwork;

class KNetwork::KClientSocketBasePrivate
{
public:
  int state;

  KResolver localResolver, peerResolver;
  KResolverResults localResults, peerResults;

  bool enableRead : 1, enableWrite : 1;
};

KClientSocketBase::KClientSocketBase(QObject *parent)
  : KActiveSocketBase(parent), d(new KClientSocketBasePrivate)
{
  d->state = Idle;
  d->enableRead = true;
  d->enableWrite = false;
}

KClientSocketBase::~KClientSocketBase()
{
  close();
  delete d;
}

KClientSocketBase::SocketState KClientSocketBase::state() const
{
  return static_cast<SocketState>(d->state);
}

void KClientSocketBase::setState(SocketState state)
{
  d->state = state;
  stateChanging(state);
}

bool KClientSocketBase::setSocketOptions(int opts)
{
  QMutexLocker locker(mutex());
  KSocketBase::setSocketOptions(opts); // call parent

  // don't create the device unnecessarily
  if (hasDevice())
    {
      bool result = socketDevice()->setSocketOptions(opts); // and set the implementation
      copyError();
      return result;
    }

  return true;
}

KResolver& KClientSocketBase::peerResolver() const
{
  return d->peerResolver;
}

const KResolverResults& KClientSocketBase::peerResults() const
{
  return d->peerResults;
}

KResolver& KClientSocketBase::localResolver() const
{
  return d->localResolver;
}

const KResolverResults& KClientSocketBase::localResults() const
{
  return d->localResults;
}

void KClientSocketBase::setResolutionEnabled(bool enable)
{
  if (enable)
    {
      d->localResolver.setFlags(d->localResolver.flags() & ~KResolver::NoResolve);
      d->peerResolver.setFlags(d->peerResolver.flags() & ~KResolver::NoResolve);
    }
  else
    {
      d->localResolver.setFlags(d->localResolver.flags() | KResolver::NoResolve);
      d->peerResolver.setFlags(d->peerResolver.flags() | KResolver::NoResolve);
    }
}

void KClientSocketBase::setFamily(int families)
{
  d->localResolver.setFamily(families);
  d->peerResolver.setFamily(families);
}

bool KClientSocketBase::lookup()
{
  if (state() == HostLookup && !blocking())
    return true;		// already doing lookup

  if (state() > HostLookup)
    return true;		// results are already available

  if (state() < HostLookup)
    {
      if (d->localResolver.serviceName().isNull() &&
	  !d->localResolver.nodeName().isNull())
	d->localResolver.setServiceName(QLatin1String(""));

      // don't restart the lookups if they had succeeded and
      // the input values weren't changed
      QObject::connect(&d->peerResolver,
		       SIGNAL(finished(KNetwork::KResolverResults)),
		       this, SLOT(lookupFinishedSlot()));
      QObject::connect(&d->localResolver,
		       SIGNAL(finished(KNetwork::KResolverResults)),
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
	  d->localResults = d->peerResults = KResolverResults();
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

bool KClientSocketBase::bind(const KResolverEntry& address)
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

bool KClientSocketBase::connect(const KResolverEntry& address, OpenMode mode)
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
	      KActiveSocketBase::open(mode | Unbuffered);
	      emit connected(address);
	    }
	}

      return true;
    }
  return false;
}

bool KClientSocketBase::disconnect()
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

bool KClientSocketBase::open(OpenMode mode)
{
    return connect(QString(), QString(), mode);
}

void KClientSocketBase::close()
{
  if (state() == Idle)
    return; 			// nothing to do

  if (state() == HostLookup)
    {
      d->peerResolver.cancel(false);
      d->localResolver.cancel(false);
    }

  d->localResults = d->peerResults = KResolverResults();

  socketDevice()->close();
  KActiveSocketBase::close();
  setState(Idle);
  emit stateChanged(Idle);
  emit closed();
}

bool KClientSocketBase::flush()
{
    return false;
}

// This function is unlike all the others because it is const
qint64 KClientSocketBase::bytesAvailable() const
{
  return socketDevice()->bytesAvailable();
}

// All the functions below look really alike
// Should I use a macro to define them?

qint64 KClientSocketBase::waitForMore(int msecs, bool *timeout)
{
  resetError();
  qint64 retval = socketDevice()->waitForMore(msecs, timeout);
  if (retval == -1)
    {
      copyError();
      emit gotError(error());
    }
  return retval;
}

qint64 KClientSocketBase::readData(char *data, qint64 maxlen, KSocketAddress* from)
{
  resetError();
  qint64 retval = socketDevice()->readData(data, maxlen, from);
  if (retval == -1)
    {
      copyError();
      emit gotError(error());
    }
  return retval;
}

qint64 KClientSocketBase::peekData(char *data, qint64 maxlen, KSocketAddress* from)
{
  resetError();
  qint64 retval = socketDevice()->peekData(data, maxlen, from);
  if (retval == -1)
    {
      copyError();
      emit gotError(error());
    }
  return retval;
}

qint64 KClientSocketBase::writeData(const char *data, qint64 len, const KSocketAddress* to)
{
  resetError();
  qint64 retval = socketDevice()->writeData(data, len, to);
  if (retval == -1)
    {
      copyError();
      emit gotError(error());
    }
  else
    emit bytesWritten(retval);
  return retval;
}

KSocketAddress KClientSocketBase::localAddress() const
{
  return socketDevice()->localAddress();
}

KSocketAddress KClientSocketBase::peerAddress() const
{
  return socketDevice()->peerAddress();
}

bool KClientSocketBase::emitsReadyRead() const
{
  return d->enableRead;
}

void KClientSocketBase::enableRead(bool enable)
{
  QMutexLocker locker(mutex());

  d->enableRead = enable;
  QSocketNotifier *n = socketDevice()->readNotifier();
  if (n)
    n->setEnabled(enable);
}

bool KClientSocketBase::emitsReadyWrite() const
{
  return d->enableWrite;
}

void KClientSocketBase::enableWrite(bool enable)
{
  QMutexLocker locker(mutex());

  d->enableWrite = enable;
  QSocketNotifier *n = socketDevice()->writeNotifier();
  if (n)
    n->setEnabled(enable);
}

void KClientSocketBase::slotReadActivity()
{
  if (d->enableRead)
    emit readyRead();
}

void KClientSocketBase::slotWriteActivity()
{
  if (d->enableWrite)
    emit readyWrite();
}

void KClientSocketBase::lookupFinishedSlot()
{
  if (d->peerResolver.isRunning() || d->localResolver.isRunning() || state() != HostLookup)
    return;

  QObject::disconnect(&d->peerResolver, 0L, this, SLOT(lookupFinishedSlot()));
  QObject::disconnect(&d->localResolver, 0L, this, SLOT(lookupFinishedSlot()));
  if (d->peerResolver.status() < 0 || d->localResolver.status() < 0)
    {
      setState(Idle);		// backtrack
      setError(LookupFailure);
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

void KClientSocketBase::stateChanging(SocketState newState)
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

void KClientSocketBase::copyError()
{
  setError(socketDevice()->error());
}

