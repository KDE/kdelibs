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

#include <qsocketnotifier.h>
#include <qcstring.h>

#include "qresolver.h"
#include "qsocketaddress.h"
#include "qsocketimpl.h"
#include "qhttpproxysocketimpl.h"

ADDON_USE

QResolverEntry QHttpProxySocketImpl::defaultProxy;

class ADDON_NAMESPACE::QHttpProxySocketImplPrivate
{
public:
  QResolverEntry proxy;
  QCString request;
  QCString reply;
  QSocketAddress peer;

  QHttpProxySocketImplPrivate()
    : proxy(QHttpProxySocketImpl::defaultProxy)
  { }
};

QHttpProxySocketImpl::QHttpProxySocketImpl(const QSocketBase* parent)
  : QSocketImpl(parent), d(new QHttpProxySocketImplPrivate)
{
}

QHttpProxySocketImpl::QHttpProxySocketImpl(const QResolverEntry& proxy)
  : d(new QHttpProxySocketImplPrivate)
{
  d->proxy = proxy;
}

QHttpProxySocketImpl::~QHttpProxySocketImpl()
{
  // nothing special to be done during closing
  // QSocketImpl::~QSocketImpl closes the socket

  delete d;
}

int QHttpProxySocketImpl::capabilities() const
{
  return CanConnectString | CanNotBind | CanNotListen | CanNotUseDatagrams;
}

const QResolverEntry&
QHttpProxySocketImpl::proxyServer() const
{
  return d->proxy;
}

void QHttpProxySocketImpl::setProxyServer(const QResolverEntry& proxy)
{
  d->proxy = proxy;
}

void QHttpProxySocketImpl::close()
{
  d->reply = d->request = QCString();
  d->peer = QSocketAddress();
  QSocketImpl::close();
}

QSocketAddress QHttpProxySocketImpl::peerAddress() const
{
  if (isOpen())
    return d->peer;
  return QSocketAddress();
}

QSocketAddress QHttpProxySocketImpl::externalAddress() const
{
  return QSocketAddress();
}

bool QHttpProxySocketImpl::connect(const QResolverEntry& address)
{
  if (d->proxy.family() == AF_UNSPEC)
    // no proxy server set !
    return QSocketImpl::connect(address);

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

bool QHttpProxySocketImpl::connect(const QString& node, const QString& service)
{
  // same safety checks as above
  if (m_sockfd == -1 && (d->proxy.family() == AF_UNSPEC ||
			 node.isEmpty() || service.isEmpty()))
    {
      // no proxy server set !
      setError(IO_ConnectError, NotSupported);
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
      if (!QSocketImpl::connect(d->proxy))
	return false;		// also unable to contact proxy server
      setState(0);		// unset open flag

      // prepare the request
      QString request = QString::fromLatin1("CONNECT %1:%2 HTTP/1.1\r\n"
					    "Cache-Control: no-cache\r\n"
					    "Host: \r\n"
					    "\r\n");
      QString node2 = node;
      if (node.contains(':'))
	node2 = '[' + node + ']';

      d->request = request.arg(node2).arg(service).latin1();
    }

  return parseServerReply();
}

bool QHttpProxySocketImpl::parseServerReply()
{
  // make sure we're connected
  if (!QSocketImpl::connect(d->proxy))
    if (error() == InProgress)
      return true;
    else if (error() != NoError)
      return false;

  if (!d->request.isEmpty())
    {
      // send request
      Q_LONG written = writeBlock(d->request, d->request.length());
      if (written < 0)
	{
	  qDebug("QHttpProxySocketImpl: would block writing request!");
	  if (error() == WouldBlock)
	    setError(IO_ConnectError, InProgress);
	  return error() == WouldBlock; // error
	}
      qDebug("QHttpProxySocketImpl: request written");

      d->request.remove(0, written);

      if (!d->request.isEmpty())
	{
	  setError(IO_ConnectError, InProgress);
	  return true;		// still in progress
	}
    }

  // request header is sent
  // must parse reply, but must also be careful not to read too much
  // from the buffer

  int index;
  if (!blocking())
    {
      Q_LONG avail = bytesAvailable();
      qDebug("QHttpProxySocketImpl: %ld bytes available", avail);
      setState(0);
      if (avail == 0)
	{
	  setError(IO_ConnectError, InProgress);
	  return true;
	}
      else if (avail < 0)
	return false;		// error!

      QByteArray buf(avail);
      if (peekBlock(buf.data(), avail) < 0)
	return false;		// error!

      QCString fullHeaders = d->reply + buf.data();
      // search for the end of the headers
      index = fullHeaders.find("\r\n\r\n");
      if (index == -1)
	{
	  // no, headers not yet finished...
	  // consume data from socket
	  readBlock(buf.data(), avail);
	  d->reply += buf.data();
	  setError(IO_ConnectError, InProgress);
	  return true;
	}

      // headers are finished
      index -= d->reply.length();
      d->reply += fullHeaders.mid(d->reply.length(), index + 4);

      // consume from socket
      readBlock(buf.data(), index + 4);
    }
  else
    {
      int state = 0;
      if (d->reply.right(3) == "\r\n\r")
	state = 3;
      else if (d->reply.right(2) == "\r\n")
	state = 2;
      else if (d->reply.right(1) == "\r")
	state = 1;
      while (state != 4)
	{
	  char c = getch();
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
  qDebug("QHttpProxySocketImpl: get reply: %s\n",
	 d->reply.left(d->reply.find('\r')).data());
  if (d->reply.left(7) != "HTTP/1." ||
      (index = d->reply.find(' ')) == -1 ||
      d->reply[index + 1] != '2')
    {
      setError(IO_ConnectError, NetFailure);
      return false;
    }

  // we've got it
  resetError();
  setState(IO_Open);
  return true;
}
