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

#ifndef QSERVERSOCKET_H
#define QSERVERSOCKET_H

#include <qobject.h>
#include "qsocketbase.h"

ADDON_START

class QSocketImpl;
class QStreamSocket;

class QServerSocketPrivate;
/**
 * @class QServerSocket qserversocket.h qserversocket.h
 * @brief A server socket for accepting connections.
 *
 * This class provides functionality for creating a socket to
 * listen for incoming connections and subsequently accept them.
 *
 * To use this class, you must first set the parameters for the listening
 * socket's address, then place it in listening mode.
 *
 * A typical example would look like:
 * \code
 *   QServerSocket *ss = new QServerSocket(service);
 *   QObject::connect(ss, SIGNAL(readyAccept()), this, SLOT(slotReadyAccept()))
 *   ss->listen();
 * \endcode
 *
 * In that case, this class will place the socket into listening mode on the
 * service pointed to by @p service and will emit the @ref readyAccept signal
 * when a connection is ready for accepting. The called slot is responsible for
 * calling @ref accept.
 *
 * @author Thiago Macieira <thiago.macieira@kdemail.net>
 */
class QServerSocket: public QObject, public QPassiveSocketBase
{
  Q_OBJECT
public:
  /**
   * Default constructor.
   * When not specified, the service to bind will be left for the operating system
   * to randomly select.
   */
  QServerSocket(QObject* parent = 0L, const char *name = 0L);

  /**
   * Construct this object specifying the service to listen on.
   *
   * @param service		the service name to listen on
   */
  QServerSocket(const QString& service, QObject* parent = 0L, const char *name = 0L);

  /**
   * Construct this object specifying the node and service names to listen on.
   *
   * @param node		the node to bind to
   * @param service		the service port to listen on
   */
  QServerSocket(const QString& node, const QString& service,
		QObject* parent = 0L, const char *name = 0L);

  /**
   * Destructor.
   */
  ~QServerSocket();

protected:
  /**
   * Sets the socket options. Reimplemented from QSocketBase.
   */
  virtual bool setSocketOptions(int opts);

public:
  /**
   * Returns the internal QResolver object used for
   * looking up the host name and service.
   *
   * This can be used to set extra options to the
   * lookup process other than the default values, as well
   * as obtaining the error codes in case of lookup failure.
   */
  QResolver& resolver() const;

  /**
   * Returns the internal list of resolved results for the binding address.
   */
  const QResolverResults& resolverResults() const;

  /**
   * Enables or disables name resolution. If this flag is set to true,
   * the @ref bind operation will trigger name lookup
   * operations (i.e., converting a hostname into its binary form).
   * If the flag is set to false, those operations will instead
   * try to convert a string representation of an address without
   * attempting name resolution.
   *
   * This is useful, for instance, when IP addresses are in
   * their string representation (such as "1.2.3.4") or come
   * from other sources like @ref QSocketAddress.
   *
   * @param enable	whether to enable
   */
  void setResolutionEnabled(bool enable);

  /**
   * Sets the allowed families for the resolutions.
   *
   * @param families		the families that we want/accept
   * @see QResolver::SocketFamilies for possible values
   */
  void setFamily(int families);

  /**
   * Sets the address to bind to.
   *
   * @param service		the service name to listen on
   */
  void setAddress(const QString& service);

  /**
   * @overload
   * Sets the address to bind to.
   *
   * @param node		the node to bind to
   * @param service		the service port to listen on
   */
  void setAddress(const QString& node, const QString& service);

  /**
   * Starts the lookup for peer and local hostnames as
   * well as their services.
   *
   * If the blocking mode for this object is on, this function will
   * wait for the lookup results to be available (by calling the 
   * @ref QResolver::wait method on the resolver objects).
   *
   * When the lookup is done, the signal @ref hostFound will be
   * emitted (only once, even if we're doing a double lookup).
   * If the lookup failed (for any of the two lookups) the 
   * @ref gotError signal will be emitted with the appropriate
   * error condition (see @ref QSocketBase::SocketError).
   *
   * This function returns true on success and false on error. Note that
   * this is not the lookup result!
   */
  virtual bool lookup();

  /**
   * Binds this socket to the given nodename and service,
   * or use the default ones if none are given.
   *
   * Upon successful binding, the @ref bound signal will be
   * emitted. If an error is found, the @ref gotError
   * signal will be emitted.
   *
   * This function returns true on success.
   *
   * @param node	the nodename
   * @param service	the service
   */
  virtual bool bind(const QString& node, const QString& service);

  /**
   * Binds the socket to the given service name.
   * @overload
   *
   * @param service	the service
   */
  virtual bool bind(const QString& service);

  /**
   * Binds the socket to the addresses previously set with @ref setAddress.
   * @overload
   *
   * @param service	the service
   */
  virtual bool bind();

  /**
   * Connect this socket to this specific address. Reimplemented from QSocketBase.
   *
   * Unlike @ref bind(const QString&, const QString&) above, this function
   * really does bind the socket. No lookup is performed. The @ref bound signal
   * will be emitted.
   */
  virtual bool bind(const QResolverEntry& address);

  /**
   * Puts this socket into listening mode. Reimplemented from @ref QPassiveSocketBase.
   *
   * Placing a socket into listening mode means it will be able to receive incoming
   * connections through the @ref accept method.
   *
   * If you do not call this method but call @ref accept directly, the socket will
   * be placed into listening mode automatically.
   *
   * @param backlog		the number of connection the system is to
   *                            queue without @ref accept being called
   * @returns true if the socket is now in listening mode.
   */
  virtual bool listen(int backlog = 5);	// 5 is arbitrary

  /**
   * Closes this socket.
   */
  virtual void close();

  /**
   * Accepts one incoming connection and return the associated, open
   * socket.
   *
   * If this function cannot accept a new connection, it will return NULL.
   * The specific object class returned by this function may vary according
   * to the implementation: derived classes may return specialised objects
   * descended from QStreamSocket.
   *
   * @note This function should return a QStreamSocket object, but compiler
   *       deficiencies prevent such an adjustment. Therefore, we return
   *       the base class for active sockets, but it is guaranteed
   *       that the object will be a QStreamSocket or derived from it.
   *
   * @sa QBufferedSocket
   */
  virtual QActiveSocketBase* accept();

  /**
   * Returns this socket's local address.
   */
  virtual QSocketAddress localAddress() const;

  /**
   * Returns this socket's externally-visible address if know.
   */
  virtual QSocketAddress externalAddress() const;

private slots:
  void lookupFinishedSlot();

signals:
  /**
   * This signal is emitted when this object finds an error.
   * The @p code parameter contains the error code that can
   * also be found by calling @ref error.
   */
  void gotError(int code);

  /**
   * This signal is emitted when the lookup is successfully completed.
   */
  void hostFound();

  /**
   * This signal is emitted when the socket successfully binds
   * to an address.
   *
   * @param local	the local address we bound to
   */
  void bound(const QResolverEntry& local);

  /**
   * This signal is emitted when the socket completes the
   * closing/shut down process.
   */
  void closed();

  /**
   * This signal is emitted whenever the socket is ready for
   * accepting -- i.e., there is at least one connection waiting to
   * be accepted.
   */
  void readyAccept();

protected:
  /**
   * Convenience function to set this object's error code to match
   * that of the socket device.
   */
  void copyError();

private:
  bool doBind();

private:
  QServerSocket(const QClientSocketBase&);
  QServerSocket& operator=(const QClientSocketBase&);

  QServerSocketPrivate *d;
};

ADDON_END

using ADDON_NAMESPACE::QServerSocket;

#endif
