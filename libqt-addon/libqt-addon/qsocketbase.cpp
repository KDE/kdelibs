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
#include <qmutex.h>

#include "qsocketbase.h"
#include "qsocketimpl.h"

ADDON_USE

class ADDON_NAMESPACE::QSocketBasePrivate
{
public:
  int socketOptions;
  int socketError;
  int capabilities;

  mutable QSocketImpl* device;

  QMutex mutex;

  QSocketBasePrivate()
    : mutex(true)		// create recursive
  { }
};

QSocketBase::QSocketBase()
  : d(new QSocketBasePrivate)
{
  d->socketOptions = Blocking;
  d->socketError = 0;
  d->device = 0L;
  d->capabilities = 0;
}

QSocketBase::~QSocketBase()
{
  delete d->device;
  delete d;
}

bool QSocketBase::setSocketOptions(int opts)
{
  d->socketOptions = opts;
  return true;
}

int QSocketBase::socketOptions() const
{
  return d->socketOptions;
}

bool QSocketBase::setBlocking(bool enable)
{
  return setSocketOptions((socketOptions() & ~Blocking) | (enable ? Blocking : 0));
}

bool QSocketBase::blocking() const
{
  return socketOptions() & Blocking;
}

bool QSocketBase::setAddressReuseable(bool enable)
{
  return setSocketOptions((socketOptions() & ~AddressReuseable) | (enable ? AddressReuseable : 0));
}

bool QSocketBase::addressReuseable() const
{
  return socketOptions() & AddressReuseable;
}

bool QSocketBase::setIPv6Only(bool enable)
{
  return setSocketOptions((socketOptions() & ~IPv6Only) | (enable ? IPv6Only : 0));
}

bool QSocketBase::isIPv6Only() const
{
  return socketOptions() & IPv6Only;
}

QSocketImpl* QSocketBase::socketDevice() const
{
  if (d->device)
    return d->device;

  // it doesn't exist, so create it
  QMutexLocker locker(mutex());
  if (d->device)
    return d->device;

  QSocketBase* that = const_cast<QSocketBase*>(this);
  QSocketImpl* dev = 0;
  if (d->capabilities)
    dev = QSocketImpl::createDefault(that, d->capabilities);
  if (!dev)
    dev = QSocketImpl::createDefault(that);
  that->setSocketDevice(dev);
  return d->device;
}

void QSocketBase::setSocketDevice(QSocketImpl* device)
{
  QMutexLocker locker(mutex());
  if (d->device == 0L)
    d->device = device;
}

int QSocketBase::setRequestedCapabilities(int add, int remove)
{
  d->capabilities |= add;
  d->capabilities &= ~remove;
  return d->capabilities;
}

bool QSocketBase::hasDevice() const
{
  return d->device != 0L;
}

void QSocketBase::setError(SocketError error)
{
  d->socketError = error;
}

QSocketBase::SocketError QSocketBase::error() const
{
  return static_cast<QSocketBase::SocketError>(d->socketError);
}

void QSocketBase::unsetSocketDevice()
{
  d->device = 0L;
}

QMutex* QSocketBase::mutex() const
{
  return &d->mutex;
}

QActiveSocketBase::QActiveSocketBase()
{
}

QActiveSocketBase::~QActiveSocketBase()
{
}

int QActiveSocketBase::getch()
{
  unsigned char c;
  if (readBlock((char*)&c, 1) != 1)
    return -1;

  return c;
}

int QActiveSocketBase::putch(int ch)
{
  unsigned char c = (unsigned char)ch;
  if (writeBlock((char*)&c, 1) != 1)
    return -1;

  return c;
}

void QActiveSocketBase::setError(int status, SocketError error)
{
  QSocketBase::setError(error);
  setStatus(status);
}

void QActiveSocketBase::resetError()
{
  QSocketBase::setError(NoError);
  resetStatus();
}

QPassiveSocketBase::QPassiveSocketBase()
{
}

QPassiveSocketBase::~QPassiveSocketBase()
{
}
