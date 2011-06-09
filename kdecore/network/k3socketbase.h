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

/*
 * Even before our #ifdef, clean up the namespace
 */
#ifdef socket
#undef socket
#endif

#ifdef bind
#undef bind
#endif

#ifdef listen
#undef listen
#endif

#ifdef connect
#undef connect
#endif

#ifdef accept
#undef accept
#endif

#ifdef getpeername
#undef getpeername
#endif

#ifdef getsockname
#undef getsockname
#endif

#ifndef KSOCKETBASE_H
#define KSOCKETBASE_H

#include <QtCore/QIODevice>
#include <QtCore/QString>

#include <kdecore_export.h>
#include "k3socketaddress.h"

class QMutex;

namespace KNetwork {

class KResolverEntry;
class KSocketDevice;

class KSocketBasePrivate;
/** @class KSocketBase k3socketbase.h k3socketbase.h
 *  @brief Basic socket functionality.
 *
 * This class provides the basic socket functionlity for descended classes.
 * Socket classes are thread-safe and provide a recursive mutex should it be
 * needed.
 *
 * @note This class is abstract.
 *
 * @author Thiago Macieira <thiago@kde.org>
 * @deprecated Use KSocketFactory or KLocalSocket instead
 */
class KDECORE_EXPORT_DEPRECATED KSocketBase
{
public:
  /**
   * Possible socket options.
   *
   * These are the options that may be set on a socket:
   *  - Blocking: whether the socket shall operate in blocking
   *    or non-blocking mode. This flag defaults to on.
   *    See setBlocking().
   *  - AddressReusable: whether the address used by this socket will
   *    be available for reuse by other sockets. This flag defaults to off.
   *    See setAddressReuseable().
   *  - IPv6Only: whether an IPv6 socket will accept IPv4 connections
   *    through a mapped address. This flag defaults to off.
   *    See setIPv6Only().
   *  - KeepAlive: whether TCP should send keepalive probes when a connection
   *    has gone idle for far too long.
   *  - Broadcast: whether this socket is allowed to send broadcast packets
   *    and will receive packets sent to broadcast.
   *  - NoDelay: disable the Nagle algorithm for socket types that support
   *    it.
   */
  enum SocketOptions
    {
      Blocking = 0x01,
      AddressReuseable = 0x02,
      IPv6Only = 0x04,
      Keepalive = 0x08,
      Broadcast = 0x10,
      NoDelay = 0x20
    };

  /**
   * Possible socket error codes.
   *
   * This is a list of possible error conditions that socket classes may
   * be expected to find.
   *
   * - NoError: no error has been detected
   * - LookupFailure: if a name lookup has failed
   * - AddressInUse: address is already in use
   * - AlreadyBound: cannot bind again
   * - AlreadyCreated: cannot recreate the socket
   * - NotBound: operation required socket to be bound and it isn't
   * - NotCreated: operation required socket to exist and it doesn't
   * - WouldBlock: requested I/O operation would block
   * - ConnectionRefused: connection actively refused
   * - ConnectionTimedOut: connection timed out
   * - InProgress: operation (connection) is already in progress
   * - NetFailure: a network failure occurred (no route, host down, host unreachable or similar)
   * - NotSupported: requested operation is not supported
   * - Timeout: a timed operation timed out
   * - UnknownError: an unknown/unexpected error has happened
   * - RemotelyDisconnected: when a connection is disconnected by the other end (since 3.4)
   *
   * @sa error, errorString
   */
  enum SocketError
    {
      NoError = 0,
      LookupFailure,
      AddressInUse,
      AlreadyCreated,
      AlreadyBound,
      AlreadyConnected,
      NotConnected,
      NotBound,
      NotCreated,
      WouldBlock,
      ConnectionRefused,
      ConnectionTimedOut,
      InProgress,
      NetFailure,
      NotSupported,
      Timeout,
      UnknownError,
      RemotelyDisconnected
    };

public:
  /**
   * Default constructor.
   */
  KSocketBase();

  /**
   * Destructor.
   */
  virtual ~KSocketBase();

  /*
   * The following functions are shared by all descended classes and will have
   * to be reimplemented.
   */

protected:
  /**
   * Set the given socket options.
   *
   * The default implementation does nothing but store the mask internally.
   * Descended classes must override this function to achieve functionality and
   * must also call this implementation.
   *
   * @param opts	a mask of SocketOptions or-ed bits of options to set
   *			or unset
   * @returns true on success
   * @note this function sets the options corresponding to the bits enabled in @p opts
   *       but will also unset the optiosn corresponding to the bits not set.
   */
  virtual bool setSocketOptions(int opts);

  /**
   * Retrieves the socket options that have been set.
   *
   * The default implementation just retrieves the mask from an internal variable.
   * Descended classes may choose to override this function to read the values
   * from the operating system.
   *
   * @returns the mask of the options set
   */
  virtual int socketOptions() const;

public:
  /**
   * Sets this socket's blocking mode.
   *
   * In blocking operation, all I/O functions are susceptible to blocking --
   * i.e., will not return unless the I/O can be satisfied. In non-blocking
   * operation, if the I/O would block, the function will return an error
   * and set the corresponding error code.
   *
   * The default implementation toggles the Blocking flag with the current
   * socket options and calls setSocketOptions().
   *
   * @param enable		whether to set this socket to blocking mode
   * @returns whether setting this value was successful; it is NOT the
   *          final blocking mode.
   */
  virtual bool setBlocking(bool enable);

  /**
   * Retrieves this socket's blocking mode.
   *
   * @returns true if this socket is/will be operated in blocking mode,
   *          false if non-blocking.
   */
  bool blocking() const;

  /**
   * Sets this socket's address reuseable flag.
   *
   * When the address reuseable flag is active, the address used by
   * this socket is left reuseable for other sockets to bind. If
   * the flag is not active, no other sockets may reuse the same
   * address.
   *
   * The default implementation toggles the AddressReuseable flag with the current
   * socket options and calls setSocketOptions().
   *
   * @param enable		whether to set the flag on or off
   * @returns true if setting this flag was successful
   */
  virtual bool setAddressReuseable(bool enable);

  /**
   * Retrieves this socket's address reuseability flag.
   *
   * @returns true if this socket's address can be reused,
   *          false if it can't.
   */
  bool addressReuseable() const;

  /**
   * Sets this socket's IPv6 Only flag.
   *
   * When this flag is on, an IPv6 socket will only accept, connect, send to or
   * receive from IPv6 addresses. When it is off, it will also talk to
   * IPv4 addresses through v4-mapped addresses.
   *
   * This option has no effect on non-IPv6 sockets.
   *
   * The default implementation toggles the IPv6Only flag with the current
   * socket options and calls setSocketOptions().
   *
   * @param enable		whether to set the flag on or off
   * @returns true if setting this flag was successful
   */
  virtual bool setIPv6Only(bool enable);

  /**
   * Retrieves this socket's IPv6 Only flag.
   *
   * @returns true if this socket will ignore IPv4-compatible and IPv4-mapped
   *	      addresses, false if it will accept them.
   */
  bool isIPv6Only() const;

  /**
   * Sets this socket Broadcast flag.
   *
   * Datagram-oriented sockets cannot normally send packets to broadcast
   * addresses, nor will they receive packets that were sent to a broadcast
   * address. To do so, you need to enable the Broadcast flag.
   *
   * This option has no effect on stream-oriented sockets.
   *
   * @returns true if setting this flag was successful.
   */
  virtual bool setBroadcast(bool enable);

  /**
   * Retrieves this socket's Broadcast flag.
   *
   * @returns true if this socket can send and receive broadcast packets,
   *          false if it can't.
   */
  bool broadcast() const;

  /**
   * Sets this socket's NoDelay flag.
   *
   * Stream-oriented protocols, like TCP, have an internal algorithm
   * (called Nagle's algorithm) that collects data in a buffer so that
   * the transmission doesn't occur after every single write operation.
   * The side-effect is that the transmission of short messages is
   * delayed.
   *
   * Setting NoDelay to 'true' will disable this algorithm.
   *
   * @returns true if setting this flag was successful.
   */
  virtual bool setNoDelay(bool enable);

  /**
   * Retrieves this socket's NoDelay flag.
   *
   * @returns true if this socket's Nagle algorithm is disabled.
   */
  bool noDelay() const;

  /**
   * Retrieves the socket implementation used on this socket.
   *
   * This function creates the device if none has been set
   * using the default factory.
   */
  KSocketDevice* socketDevice() const;

  /**
   * Sets the socket implementation to be used on this socket.
   *
   * Note: it is an error to set this if the socket device has
   * already been set once.
   *
   * This function is provided virtual so that derived classes can catch
   * the setting of a device and properly set their own states and internal
   * variables. The parent class must be called.
   *
   * This function is called by socketDevice() above when the socket is
   * first created.
   */
  virtual void setSocketDevice(KSocketDevice* device);

  /**
   * Sets the internally requested capabilities for a socket device.
   *
   * Most socket classes can use any back-end implementation. However, a few
   * may require specific capabilities not provided in the default
   * implementation. By using this function, derived classes can request
   * that a backend with those capabilities be created when necessary.
   *
   * For the possible flags, see KSocketDevice::Capabilities. However, note
   * that only the Can* flags make sense in this context.
   *
   * @note Since socketDevice must always return a valid backend object, it
   *       is is possible that the created device does not conform to all
   *       requirements requested. Implementations sensitive to this fact
   *       should test the object returned by socketDevice() (through
   *       KSocketDevice::capabilities(), for instance) the availability.
   *
   * @param add		mask of KSocketDevice::Capabilities to add
   * @param remove	mask of bits to remove from the requirements
   * @return the current mask of requested capabilities
   */
  int setRequestedCapabilities(int add, int remove = 0);

protected:
  /**
   * Returns true if the socket device has been initialised in this
   * object, either by calling socketDevice() or setSocketDevice()
   */
  bool hasDevice() const;

  /**
   * Sets the socket's error code.
   *
   * @param error		the error code
   */
  void setError(SocketError error);

  /**
   * Resets the socket error code and the I/O Device's status.
   */
  void resetError();

public:
  /**
   * Retrieves the socket error code.
   * @sa errorString
   */
  SocketError error() const;

  /**
   * Returns the error string corresponding to this error condition.
   */
  QString errorString() const;

  /**
   * Returns the internal mutex for this class.
   *
   * Note on multithreaded use of sockets:
   * the socket classes are thread-safe by design, but you should be aware of
   * problems regarding socket creation, connection and destruction in
   * multi-threaded programs. The classes are guaranteed to work while
   * the socket exists, but it's not wise to call connect in multiple
   * threads.
   *
   * Also, this mutex must be unlocked before the object is destroyed, which
   * means you cannot use it to guard against other threads accessing the object
   * while destroying it. You must ensure there are no further references to this
   * object when deleting it.
   */
  QMutex* mutex() const;

public:
  /**
   * Returns the string describing the given error code, i18n'ed.
   *
   * @param code		the error code
   */
  static QString errorString(SocketError code);

  /**
   * Returns true if the given error code is a fatal one, false
   * otherwise. The parameter here is of type int so that
   * casting isn't necessary when using the parameter to signal
   * QClientSocketBase::gotError.
   *
   * @param code		the code to test
   */
  static bool isFatalError(int code);

private:
  /// @internal
  /// called by KSocketDevice
  void unsetSocketDevice();

  KSocketBase(const KSocketBase&);
  KSocketBase& operator =(const KSocketBase&);

  KSocketBasePrivate* const d;

  friend class KSocketDevice;
};

/**
 * @class KActiveSocketBase k3socketbase.h k3socketbase.h
 * @brief Abstract class for active sockets
 *
 * This class provides the standard interfaces for active sockets, i.e.,
 * sockets that are used to connect to external addresses.
 *
 * @author Thiago Macieira <thiago@kde.org>
 * @deprecated Use KSocketFactory or KLocalSocket instead
 */
class KDECORE_EXPORT_DEPRECATED KActiveSocketBase: public QIODevice, virtual public KSocketBase
{
  Q_OBJECT
public:
  /**
   * Constructor.
   */
  KActiveSocketBase(QObject* parent);

  /**
   * Destructor.
   */
  virtual ~KActiveSocketBase();

  /**
   * Unshadow errorString from QIODevice
   */
  QString errorString() const;

  /**
   * @reimp
   */
  virtual void setSocketDevice(KSocketDevice* device);

  /**
   * Reimplemented from QIODevice.
   */
  virtual bool open(OpenMode mode);

  /**
   * Binds this socket to the given address.
   *
   * The socket will be constructed with the address family,
   * socket type and protocol as those given in the
   * @p address object.
   *
   * @param address		the address to bind to
   * @returns true if the binding was successful, false otherwise
   */
  virtual bool bind(const KResolverEntry& address) = 0;

  /**
   * Connect to a remote host.
   *
   * This will make this socket try to connect to the remote host.
   * If the socket is not yet created, it will be created using the
   * address family, socket type and protocol specified in the
   * @p address object.
   *
   * If this function returns with error InProgress, calling it
   * again with the same address after a time will cause it to test
   * if the connection has succeeded in the mean time.
   *
   * @param address		the address to connect to
   * @param mode                mode for connection, from QIODevice
   *
   * @returns true if the connection was successful or has been successfully
   *          queued; false if an error occurred.
   */
  virtual bool connect(const KResolverEntry& address,
		       OpenMode mode = ReadWrite) = 0;

  /**
   * Disconnects this socket from a connection, if possible.
   *
   * If this socket was connected to an endpoint, the connection
   * is severed, but the socket is not closed. If the socket wasn't
   * connected, this function does nothing.
   *
   * If the socket hadn't yet been created, this function does nothing
   * either.
   *
   * Not all socket types can disconnect. Most notably, only
   * connectionless datagram protocols such as UDP support this operation.
   *
   * @return true if the socket is now disconnected or false on error.
   */
  virtual bool disconnect() = 0;

  /**
   * Sockets are sequential
   */
  virtual bool isSequential() const;

  /**
   * This call is not supported on sockets. Reimplemented from QIODevice.
   * This will always return 0.
   */
  virtual qint64 size() const;

  /**
   * This call is not supported on sockets. Reimplemented from QIODevice.
   * This will always return 0.
   */
  virtual qint64 pos() const;

  /**
   * This call is not supported on sockets. Reimplemented from QIODevice.
   * This will always return false.
   */
  virtual bool seek(qint64);

  /**
   * This call is not supported on sockets. Reimplemented from QIODevice.
   * This will always return true.
   */
  virtual bool atEnd() const;

  /**
   * Reads data from the socket.
   *
   * Reimplemented from QIODevice. See QIODevice::read for
   * more information.
   */
  qint64 read(char *data, qint64 maxlen);

  /**
   * Reads data from the socket.
   *
   * Reimplemented from QIODevice. See QIODevice::read for
   * more information.
   */
  QByteArray read(qint64 len);

  /** @overload
   * Receives data and the source address.
   *
   * This call will read data in the socket and will also
   * place the sender's address in @p from object.
   *
   * @param data		where to write the read data to
   * @param maxlen		the maximum number of bytes to read
   * @param from		the address of the sender will be stored here
   * @returns the actual number of bytes read
   */
  qint64 read(char *data, qint64 maxlen, KSocketAddress& from);

  /**
   * Peeks the data in the socket and the source address.
   *
   * This call will allow you to peek the data to be received without actually
   * receiving it -- that is, it will be available for further peekings and
   * for the next read call.
   *
   * @param data		where to write the peeked data to
   * @param maxlen		the maximum number of bytes to peek
   * @returns the actual number of bytes copied into @p data
   */
  qint64 peek(char *data, qint64 maxlen);

  /**
   * @overload
   * Peeks the data in the socket and the source address.
   *
   * This call will allow you to peek the data to be received without actually
   * receiving it -- that is, it will be available for further peekings and
   * for the next read call.
   *
   * @param data		where to write the peeked data to
   * @param maxlen		the maximum number of bytes to peek
   * @param from		the address of the sender will be stored here
   * @returns the actual number of bytes copied into @p data
   */
  qint64 peek(char *data, qint64 maxlen, KSocketAddress& from);

  /**
   * Writes the given data to the socket.
   *
   * Reimplemented from QIODevice. See QIODevice::write for
   * more information.
   */
  qint64 write(const char *data, qint64 len);

  /**
   * Writes the given data to the socket.
   *
   * Reimplemented from QIODevice. See QIODevice::write for
   * more information.
   */
  qint64 write(const QByteArray& data);

  /** @overload
   * Writes the given data to the destination address.
   *
   * Note that not all socket connections allow sending data to different
   * addresses than the one the socket is connected to.
   *
   * @param data		the data to write
   * @param len			the length of the data
   * @param to			the address to send to
   * @returns the number of bytes actually sent
   */
  qint64 write(const char *data, qint64 len, const KSocketAddress& to);

  /**
   * Waits up to @p msecs for more data to be available on this socket.
   *
   * If msecs is -1, this call will block indefinetely until more data
   * is indeed available; if it's 0, this function returns immediately.
   *
   * If @p timeout is not NULL, this function will set it to indicate
   * if a timeout occurred.
   *
   * @returns the number of bytes available
   */
  virtual qint64 waitForMore(int msecs, bool *timeout = 0L) = 0;

  /**
   * This call is not supported on sockets. Reimplemented from QIODevice.
   */
  void ungetChar(char);

  /**
   * Returns this socket's local address.
   */
  virtual KSocketAddress localAddress() const = 0;

  /**
   * Return this socket's peer address, if we are connected.
   * If the address cannot be retrieved, the returned object will contain
   * an invalid address.
   */
  virtual KSocketAddress peerAddress() const = 0;

  // FIXME KDE 4.0:
  // enable this function
#if 0
  /**
   * Returns this socket's externally-visible address, if known.
   */
  virtual KSocketAddress externalAddress() const = 0;
#endif

protected:
  /**
   * Reads data from the socket.
   *
   * Reimplemented from QIODevice. See QIODevice::readData for
   * more information.
   */
  virtual qint64 readData(char *data, qint64 len);

  /** @overload
   * Receives data and the source address.
   *
   * This call will read data in the socket and will also
   * place the sender's address in @p from object.
   *
   * @param data		where to write the read data to
   * @param maxlen		the maximum number of bytes to read
   * @param from		the address of the sender will be stored here
   * @returns the actual number of bytes read
   */
  virtual qint64 readData(char *data, qint64 maxlen, KSocketAddress* from) = 0;

  /**
   * Peeks the data in the socket and the source address.
   *
   * This call will allow you to peek the data to be received without actually
   * receiving it -- that is, it will be available for further peekings and
   * for the next read call.
   *
   * @param data		where to write the peeked data to
   * @param maxlen		the maximum number of bytes to peek
   * @param from		the address of the sender will be stored here
   * @returns the actual number of bytes copied into @p data
   */
  virtual qint64 peekData(char *data, qint64 maxlen, KSocketAddress* from) = 0;

  /**
   * Writes the given data to the socket.
   *
   * Reimplemented from QIODevice. See QIODevice::writeData for
   * more information.
   */
  virtual qint64 writeData(const char *data, qint64 len);

  /** @overload
   * Writes the given data to the destination address.
   *
   * Note that not all socket connections allow sending data to different
   * addresses than the one the socket is connected to.
   *
   * @param data		the data to write
   * @param len			the length of the data
   * @param to			the address to send to
   * @returns the number of bytes actually sent
   */
  virtual qint64 writeData(const char *data, qint64 len, const KSocketAddress* to) = 0;

  /**
   * Sets the socket's error code.
   *
   * @param error		the error code
   */
  void setError(SocketError error);

  /**
   * Resets the socket error code and the I/O Device's status.
   */
  void resetError();
};

/**
 * @class KPassiveSocketBase k3socketbase.h k3socketbase.h
 * @brief Abstract base class for passive sockets.
 *
 * This socket provides the initial functionality for passive sockets,
 * i.e., sockets that accept incoming connections.
 *
 * @author Thiago Macieira <thiago@kde.org>
 * @deprecated Use KSocketFactory or KLocalSocket instead
 */
class KDECORE_EXPORT_DEPRECATED KPassiveSocketBase: virtual public KSocketBase
{
public:
  /**
   * Constructor
   */
  KPassiveSocketBase();

  /**
   * Destructor
   */
  virtual ~KPassiveSocketBase();

  /**
   * Binds this socket to the given address.
   *
   * The socket will be constructed with the address family,
   * socket type and protocol as those given in the
   * @p address object.
   *
   * @param address		the address to bind to
   * @returns true if the binding was successful, false otherwise
   */
  virtual bool bind(const KResolverEntry& address) = 0;

  /**
   * Puts this socket into listening mode.
   *
   * Placing a socket in listening mode means that it will
   * be allowed to receive incoming connections from
   * remote hosts.
   *
   * Note that some socket types or protocols cannot be
   * put in listening mode.
   *
   * @param backlog		the number of accepted connections to
   *				hold before starting to refuse
   * @returns true if the socket is now in listening mode
   */
  virtual bool listen(int backlog) = 0;

  /**
   * Closes this socket. All resources used are freed. Note that closing
   * a passive socket does not close the connections accepted with it.
   */
  virtual void close() = 0;

  /**
   * Accepts a new incoming connection.
   *
   * If this socket was in listening mode, you can call this
   * function to accept an incoming connection.
   *
   * If this function cannot accept a new connection (either
   * because it is not listening for one or because the operation
   * would block), it will return NULL.
   *
   * Also note that descended classes will override this function
   * to return specialized socket classes.
   */
  virtual KActiveSocketBase* accept() = 0;

  /**
   * Returns this socket's local address.
   */
  virtual KSocketAddress localAddress() const = 0;

  /**
   * Returns this socket's externally-visible address if known.
   */
  virtual KSocketAddress externalAddress() const = 0;

private:
  KPassiveSocketBase(const KPassiveSocketBase&);
  KPassiveSocketBase& operator = (const KPassiveSocketBase&);
};

}				// namespace KNetwork

#endif
