/*  -*- C++ -*-
 *  Copyright (C) 2003 Thiago Macieira <thiago@kde.org>
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

#include <QSocketNotifier>
#include <QByteArray>

#include "kresolver.h"
#include "ksocketaddress.h"
#include "ksocketdevice.h"
#include "khttpproxysocketdevice.h"

using namespace KNetwork;

KResolverEntry KHttpProxySocketDevice::defaultProxy;

class KNetwork::KHttpProxySocketDevicePrivate
{
public:
  KResolverEntry proxy;
  QByteArray request;
  QByteArray reply;
  KSocketAddress peer;

  KHttpProxySocketDevicePrivate()
    : proxy(KHttpProxySocketDevice::defaultProxy)
  { }
};

KHttpProxySocketDevice::KHttpProxySocketDevice(const KSocketBase* parent)
  : KSocketDevice(parent), d(new KHttpProxySocketDevicePrivate)
{
}

KHttpProxySocketDevice::KHttpProxySocketDevice(const KResolverEntry& proxy)
  : d(new KHttpProxySocketDevicePrivate)
{
  d->proxy = proxy;
}

KHttpProxySocketDevice::~KHttpProxySocketDevice()
{
  // nothing special to be done during closing
  // KSocketDevice::~KSocketDevice closes the socket

  delete d;
}

int KHttpProxySocketDevice::capabilities() const
{
  return CanConnectString | CanNotBind | CanNotListen | CanNotUseDatagrams;
}

const KResolverEntry&
KHttpProxySocketDevice::proxyServer() const
{
  return d->proxy;
}

void KHttpProxySocketDevice::setProxyServer(const KResolverEntry& proxy)
{
  d->proxy = proxy;
}

void KHttpProxySocketDevice::close()
{
  d->reply = d->request = QByteArray();
  d->peer = KSocketAddress();
  KSocketDevice::close();
}

KSocketAddress KHttpProxySocketDevice::peerAddress() const
{
  if (isOpen())
    return d->peer;
  return KSocketAddress();
}

KSocketAddress KHttpProxySocketDevice::externalAddress() const
{
  return KSocketAddress();
}

bool KHttpProxySocketDevice::connect(const KResolverEntry& address)
{
  if (d->proxy.family() == AF_UNSPEC)
    // no proxy server set !
    return KSocketDevice::connect(address);

  if (isOpen())
    {
      // socket is already open
      resetError();
      return true;
    }

  if (m_sockfd == -1)
    // socket isn't created yet
    return connect(address.address().nodeName(), 
		   address.address().serviceName());

  d->peer = address.address();
  return parseServerReply();
}

bool KHttpProxySocketDevice::connect(const QString& node, const QString& service)
{
  // same safety checks as above
  if (m_sockfd == -1 && (d->proxy.family() == AF_UNSPEC ||
			 node.isEmpty() || service.isEmpty()))
    {
      // no proxy server set !
      setError(NotSupported);
      return false;
    }

  if (isOpen())
    {
      // socket is already open
      return true;
    }

  if (m_sockfd == -1)
    {
      // must create the socket
      if (!KSocketDevice::connect(d->proxy))
	return false;		// also unable to contact proxy server
      KActiveSocketBase::close();

      // prepare the request
      QString request = QLatin1String("CONNECT %1:%2 HTTP/1.1\r\n"
				      "Cache-Control: no-cache\r\n"
				      "Host: \r\n"
				      "\r\n");
      QString node2 = node;
      if (node.contains(':'))
	node2 = '[' + node + ']';

      d->request = request.arg(node2).arg(service).toLatin1();
    }

  return parseServerReply();
}

bool KHttpProxySocketDevice::parseServerReply()
{
  // make sure we're connected
  if (!KSocketDevice::connect(d->proxy))
    if (error() == InProgress)
      return true;
    else if (error() != NoError)
      return false;

  if (!d->request.isEmpty())
    {
      // send request
      qint64 written = writeData(d->request, d->request.length());
      if (written < 0)
	{
	  qDebug("KHttpProxySocketDevice: would block writing request!");
	  if (error() == WouldBlock)
	    setError(InProgress);
	  return error() == WouldBlock; // error
	}
      qDebug("KHttpProxySocketDevice: request written");

      d->request.remove(0, written);

      if (!d->request.isEmpty())
	{
	  setError(InProgress);
	  return true;		// still in progress
	}
    }

  // request header is sent
  // must parse reply, but must also be careful not to read too much
  // from the buffer

  int index;
  if (!blocking())
    {
      qint64 avail = bytesAvailable();
      qDebug("KHttpProxySocketDevice: %lld bytes available", avail);
      KActiveSocketBase::close();
      if (avail == 0)
	{
	  setError(InProgress);
	  return true;
	}
      else if (avail < 0)
	return false;		// error!

      QByteArray buf;
      buf.resize(avail);
      if (peekData(buf.data(), avail) < 0)
	return false;		// error!

      QByteArray fullHeaders = d->reply + buf;
      // search for the end of the headers
      index = fullHeaders.indexOf("\r\n\r\n");
      if (index == -1)
	{
	  // no, headers not yet finished...
	  // consume data from socket
	  readData(buf.data(), avail);
	  d->reply += buf.data();
	  setError(InProgress);
	  return true;
	}

      // headers are finished
      index -= d->reply.length();
      d->reply += fullHeaders.mid(d->reply.length(), index + 4);

      // consume from socket
      readData(buf.data(), index + 4);
    }
  else
    {
      int state = 0;
      if (d->reply.endsWith("\r\n\r"))
	state = 3;
      else if (d->reply.endsWith("\r\n"))
	state = 2;
      else if (d->reply.endsWith("\r"))
	state = 1;
      while (state != 4)
	{
	  char c;
	  getChar(&c);
	  d->reply += c;

	  if ((state == 3 && c == '\n') ||
	      (state == 1 && c == '\n') ||
	      c == '\r')
	    ++state;
	  else
	    state = 0;
	}
    }	    

  // now really parse the reply
  qDebug("KHttpProxySocketDevice: get reply: %s\n",
	 d->reply.left(d->reply.indexOf('\r')).data());
  if (d->reply.left(7) != "HTTP/1." ||
      (index = d->reply.indexOf(' ')) == -1 ||
      d->reply[index + 1] != '2')
    {
      setError(NetFailure);
      return false;
    }

  // we've got it
  resetError();
  KActiveSocketBase::open(ReadOnly);
  return true;
}
