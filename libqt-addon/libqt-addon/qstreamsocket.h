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

#ifndef QSTREAMSOCKET_H
#define QSTREAMSOCKET_H

#include <qstring.h>

#include "qtaddon.h"
#include "qclientsocketbase.h"

ADDON_START
class QResolverEntry;
class QResolverResults;
class QServerSocket;

class QStreamSocketPrivate;
/** @class QStreamSocket qstreamsocket.h qstreamsocket.h
 *  @brief Simple stream socket
 *
 * This class provides functionality to creating unbuffered, stream
 * sockets. In the case of Internet (IP) sockets, this class creates and
 * uses TCP/IP sockets.
 *
 * @todo Add timeout handling and add synchronous connection
 *
 * @author Thiago Macieira <thiago.macieira@kdemail.net>
 * @version 0.9
 */
class QStreamSocket: public QClientSocketBase
{
  Q_OBJECT

public:
  /**
   * Default constructor.
   *
   * @param node	destination host
   * @param service	destination service to connect to
   */
  QStreamSocket(const QString& host = QString::null, const QString& service = QString::null,
		QObject* parent = 0L, const char *name = 0L);

  /**
   * Destructor. This closes the socket.
   */
  virtual ~QStreamSocket();

  /**
   * Retrieves the timeout value (in milliseconds).
   */
  int timeout() const;

  /**
   * Retrieves the remaining timeout time (in milliseconds). This value
   * equals @ref timeout() if there's no connection in progress.
   */
  int remainingTimeout() const;

  /**
   * Sets the timeout value. Setting this value while a connection attempt
   * is in progress will reset the timer.
   *
   * Please note that the timeout value is valid for the connection attempt
   * only. No other operations are timed against this value -- including the
   * name lookup associated.
   *
   * @param msecs		the timeout value in milliseconds
   */
  void setTimeout(int msecs);

  /**
   * Reimplemented from QClientSocketBase.
   */
  virtual bool bind(const QString& node = QString::null,
		    const QString& service = QString::null);

  /**
   * Unshadowing from QClientSocketBase.
   */
  virtual bool bind(const QResolverEntry& entry)
  { return QClientSocketBase::bind(entry); }

  /**
   * Reimplemented from QClientSocketBase.
   *
   * This function also implements timeout handling.
   */
  virtual bool connect(const QString& node = QString::null,
		       const QString& service = QString::null);

  /**
   * Unshadowing from QClientSocketBase.
   */
  virtual bool connect(const QResolverEntry& entry);

signals:
  /**
   * This signal is emitted when a connection timeout occurs.
   */
  void timedOut();

private slots:
  void hostFoundSlot();
  void connectionEvent();
  void timeoutSlot();

private:
  /**
   * @internal
   * If the user requested local bind before connection, bind the socket to one
   * suitable address and return true. Also sets d->local to the address used.
   *
   * Return false in case of error.
   */
  bool bindLocallyFor(const QResolverEntry& peer);

  /**
   * @internal
   * Finishes the connection process by setting internal values and
   * emitting the proper signals.
   *
   * Note: assumes d->local iterator points to the address that we bound
   * to.
   */
  void connectionSucceeded(const QResolverEntry& peer);

  QStreamSocket(const QStreamSocket&);
  QStreamSocket& operator=(const QStreamSocket&);

  QStreamSocketPrivate *d;

  friend class QServerSocket;
};

ADDON_END;

using ADDON_NAMESPACE::QStreamSocket;

#endif
