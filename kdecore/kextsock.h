/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000-2002 Thiago Macieira <thiagom@mail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */
#ifndef KEXTSOCK_H
#define KEXTSOCK_H

#include <sys/time.h>

#include <qstring.h>
#include <qptrlist.h>
#include <qiodevice.h>

#include "kbufferedio.h"
#include "ksockaddr.h"

/* External reference to netdb.h */
struct addrinfo;
struct kde_addrinfo;
class KAddressInfo;		/* our abstraction of it */
class QSocketNotifier;

/*
 * This is extending QIODevice's error codes
 *
 * According to qiodevice.h, the last error is IO_UnspecifiedError
 * These errors will never occur in functions declared in QIODevice
 * (except open, but you shouldn't call open)
 */
#define IO_ListenError		(IO_UnspecifiedError+1)
#define IO_AcceptError		(IO_UnspecifiedError+2)
#define IO_LookupError		(IO_UnspecifiedError+3)

class KExtendedSocketPrivate;
/**
 * The extended socket class.
 *
 * This class should be used instead of @ref KSocket whenever the user needs
 * fine-grained control over the socket being created. Unlike KSocket, which
 * does everything at once, without much intervention, KExtendedSocket allows
 * intervention at every step of the process and the setting of parameters.
 *
 * This class allows for the creation of both server and client sockets. The
 * only difference is that the passiveSocket flag must be passed either to
 * the constructor or to @ref setSocketFlags(). If passiveSocket is used, the class will
 * enable functions @ref listen() and @ref accept() and related signals, and will
 * also disable @ref readBlock() and @ref writeBlock().
 *
 * To create a Unix socket, one would pass flag unixSocket to the constructor
 * or @ref setSocketFlags(). The hostname and service/port can be set to whatever is
 * necessary. If no hostname is given, but a service/port is, the socket created
 * will be implementation dependant (usually in /tmp). In any other case, the
 * fields will be concatenated.
 *
 * To create an Internet socket, inetSocket flag can be used. If, on the other
 * hand a specific IP protocol is desired, ipv4Socket and/or ipv6Socket can be
 * used.
 *
 * Note that the socket type selection flags are cumulative. One could select
 * Unix and Internet sockets by using unixSocket | inetSocket. Or, for instance,
 * to make sure only IPv4 and IPv6 sockets are selected, even if future implementations
 * support newer IP protocols, ipv4Socket | ipv6Socket is your guy.
 *
 * @author Thiago Macieira <thiagom@mail.com>
 * @version $Id$
 * @short an extended socket
 */
class KExtendedSocket: public KBufferedIO // public QObject, public QIODevice
{
  Q_OBJECT

public:
  /**
   * flags that can be passed down to the member functions
   */
  enum Flags
  {
    /* socket address families */
    /*
     * NOTE: if you change this, you have to change function valid_socket() as well
     * These values are hard coded!
     */
    anySocket = 0x00,
    knownSocket = 0x01,
    unixSocket = knownSocket | 0x02,
    inetSocket = knownSocket | 0x04,
    ipv4Socket = inetSocket | 0x100,
    ipv6Socket = inetSocket | 0x200,

    passiveSocket = 0x1000,	/* passive socket (i.e., one that accepts connections) */
    canonName = 0x2000,		/* request that the canon name be found */
    noResolve = 0x4000,		/* do not attempt to resolve, treat as numeric host */

    streamSocket = 0x8000,	/* request a streaming socket (e.g., TCP) */
    datagramSocket = 0x10000,	/* request a datagram socket (e.g., UDP) */
    rawSocket = 0x20000,	/* request a raw socket. This probably requires privileges */

    inputBufferedSocket = 0x200000, /* buffer input in this socket */
    outputBufferedSocket = 0x400000, /* buffer output in this socket */
    bufferedSocket = 0x600000	/* make this a fully buffered socket */
  };

  /**
   * status of the class
   * The status are sequential. If a change to one status is requested,
   * all the prior status will be passed and their actions, performed
   */
  enum SockStatus
  {
    // the numbers are scattered so that we leave room for future expansion
    error = -1,			// invalid status!

    nothing = 0,		// no status, the class has just been created

    lookupInProgress = 50,	// lookup is in progress. Signals will be sent
    lookupDone = 70,		// lookup has been done. Flags cannot be changed
				// from this point on

    created = 100,		// ::socket() has been called, a socket exists
    bound = 140,		// socket has been bound

    connecting = 200,		// socket is connecting (not passiveSocket)
    connected = 220,		// socket has connected (not passiveSocket)

    listening = 200,		// socket is listening (passiveSocket)
    accepting = 220,		// socket is accepting (passiveSocket)

    closing = 350,		// socket is closing (delayed close)

    done = 400			// socket has been closed
  };

public:
  /**
   * Creates an empty KExtendedSocket.
   */
  KExtendedSocket();

  /**
   * Creates a socket with the given hostname and port.
   * @param host	the hostname
   * @param port	the port number
   * @param flags	flags
   */
  KExtendedSocket(const QString& host, int port, int flags = 0);

  /**
   * Creates a socket with the given hostname and service.
   * @param host	the hostname
   * @param serv	the service
   * @param flags	flags
   */
  KExtendedSocket(const QString& host, const QString& service, int flags = 0);

  /**
   * Destroys the socket, disconnecting if still connected and
   * freeing any related resources still being kept.
   */
  virtual ~KExtendedSocket();

  /**
   * Resets the socket, disconnecting if still connected and
   * freeing any related resources still being kept.
   * @since 3.1
   */
  void reset();

  /*
   * --- status, flags and internal variables --- *
   */

  /**
   * Returns the class status.
   * @return the class status
   * @see ::SockStatus
   */
  int socketStatus() const;

  /**
   * Returns the related system error code
   * Except for IO_LookupError errors, these are codes found in
   * errno
   * @return the system error code
   */
  int systemError() const;

  /**
   * Sets the given flags. 
   * @param flags	the flags to be set
   * @return the new flags status, or -1 if flags can no longer be set
   */
  int setSocketFlags(int flags);

  /**
   * Returns the current flags
   * @return the current flags
   * @see ::Flags
   */
  int socketFlags() const;

  /**
   * Sets the hostname to the given value.
   * @param host	the hostname
   * @return true on success, false on error
   */
  bool setHost(const QString& host);

  /**
   * Returns the hostname.
   * @return the hostname or QString::null if no host has been set
   */
  QString host() const;

  /**
   * Sets the port/service.
   * @param port	the port
   */
  bool setPort(int port);

  /**
   * Sets the port/service.
   * @param port	the port
   * @return true if successful, false on error (e.g. connection already established)
   */
  bool setPort(const QString& service);

  /**
   * Returns the port/service. If it is a port, the string contains a number.
   * @return the port or QString::null if it has not been set.
   */
  QString port() const;

  /**
   * Sets the address where we will connect to.
   * @param host	the hostname
   * @param port	port number
   * @return true if successful, false on error (e.g. connection already established)
   */
  bool setAddress(const QString& host, int port);

  /**
   * Sets the address where we will connect to.
   * @param host	the hostname
   * @param serv	the service
   * @return true if successful, false on error (e.g. connection already established)
   */
  bool setAddress(const QString& host, const QString& serv);

  /**
   * Sets the hostname to which we will bind locally before connecting.
   * @param host	the hostname
   * @return false if this is a passiveSocket, otherwise true.
   */
  bool setBindHost(const QString& host);

  /**
   * Unsets the bind hostname. That is, don't request a binding host.
   * @return true if successful, false on error (e.g. connection already established)
   */
  bool unsetBindHost();

  /**
   * Returns the hostname to which the socket will be/is bound.
   * @return the host or QString::null if it has not been set.
   */
  QString bindHost() const;

  /**
   * Sets the port/service to which we will bind before connecting
   * @param port	the port number
   * @return true if successful, false on error (e.g. connection already established)
   */
  bool setBindPort(int port);

  /**
   * Sets the port/service to which we will bind before connecting.
   * @param service	the port number or service name
   * @return true if successful, false on error (e.g. connection already established)
   */
  bool setBindPort(const QString& service);

  /**
   * Unsets the bind port/service.
   * @return true if successful, false on error (e.g. connection already established)
   */
  bool unsetBindPort();

  /**
   * Returns the service to which the socket will be/is bound.
   * @return the host or QString::null if it has not been set.
   */
  QString bindPort() const;

  /**
   * Sets both host and port to which we will bind the socket. Will return
   * false if this is a passiveSocket.
   * @param host	the hostname
   * @param port	the port number
   * @return true if successful, false on error (e.g. connection already established)
   */
  bool setBindAddress(const QString& host, int port);

  /**
   * Sets both host and service to which we will bind the socket. Will return
   * false if this is a passiveSocket.
   * @param host	the hostname
   * @param serv	the service
   * @return true if successful, false on error (e.g. connection already established)
   */
  bool setBindAddress(const QString& host, const QString& service);

  /**
   * Unsets the bind address for the socket. That means that we won't
   * attempt to bind to an address before connecting.
   * @return true if successful, false on error (e.g. connection already established)
   */
  bool unsetBindAddress();

  /**
   * Sets the timeout value for the connection (if this is not passiveSocket) or
   * acception (if it is). In the event the given function
   * (connect or accept) returns due to time out, it's possible to call it again.
   *
   * Setting the timeout to 0 disables the timeout feature.
   *
   * @param secs	the timeout length, in seconds
   * @param usecs	the timeout complement, in microseconds
   * @return false if setting timeout makes no sense in the context.
   */
  bool setTimeout(int secs, int usecs = 0);

  /**
   * Returns the timeout value for the connection.
   * @return the timeout value. 0 if there is no timeout.
   */
  timeval timeout() const;

  /**
   * Sets/unsets blocking mode for the socket. When non-blocking mode is enabled,
   * I/O operations might return error and set errno to EWOULDBLOCK. Also,
   * it's not recommended to use this when using the class signals.
   *
   * @param enable	if true, set blocking mode. False, non-blocking mode.
   * @return false on error.
   */
  bool setBlockingMode(bool enable);

  /**
   * Returns the current blocking mode for this socket.
   * @return true if in blocking mode
   */
  bool blockingMode();

  /**
   * Sets/unsets address reusing flag for this socket.
   *
   * This function returns true if the value was set correctly. That is NOT
   * the result of the set.
   * @param enable	if true, set address reusable
   * @return true on success, false on failure. If the socket was not yet created,
   * the value is only remembered. In this case the return value is always true.
   */
  bool setAddressReusable(bool enable);

  /**
   * Returns whether this socket's address can be reused
   * @return true if the address can be reused
   */
  bool addressReusable();

  /**
   * Sets the buffer sizes for this socket.
   *
   * This implementation allows any size for both parameters. The value given
   * will be interpreted as the maximum size allowed for the buffers, after
   * which the I/O functions will stop buffering. The value of -1 will be
   * interpreted as "unlimited" size. The value of -2 means "no change".
   *
   * Note: changing the buffer size to 0 for any buffer will cause the given
   * buffer's to be discarded. Likewise, setting the size to a value less than
   * the current size will cause the buffer to be shrunk to the wanted value,
   * as if the data had been read.
   *
   * Note 2: The closed() signal will only be emitted for a connection closed
   * by the remote end if we are doing input buffering. That happens because if we aren't
   * reading from the connection, we don't know when it closed.
   * @param rsize	read buffer size
   * @param wsize	write buffer size
   * @return true on success, false if this is not possible in this state (e.g. connection
   *         not established yet)
   */
  virtual bool setBufferSize(int rsize, int wsize = -2);

  /**
   * Returns the local socket address
   * @return the local socket address, can be 0 if the connection has not been established
   *         yet
   */
  const KSocketAddress *localAddress();

  /**
   * Returns the peer socket address. Use KExtendedSocket::resolve() to
   * resolve this to a human-readable hostname/service or port.
   * @return the peer address, can be 0 if the connection has not been established yet
   *         or the socket is passive
   */
  const KSocketAddress *peerAddress();

  /**
   * Returns the file descriptor
   * @return the file descriptor. -1 if there is no fd yet.
   */
  inline int fd() const
  { return sockfd; }

  /*
   * -- socket creation -- *
   */

  /**
   * Performs lookup on the addresses we were given before.
   *
   * This will perform lookups on the bind addresses if they were given.
   * @return 0 or an error. Do not rely on the values returned by lookup
   * as of now. They are not specified.
   */
  virtual int lookup();

  /**
   * Starts an asynchronous lookup for the addresses given.
   *
   * When the lookup is done, the lookupReady signal will be emitted.
   *
   * Note that, depending on the parameters for the lookup, this function might
   * know the results without the need for blocking or queueing an
   * asynchronous lookup. That means that the lookupReady signal might be
   * emitted by this function, so your code should be prepared for that.
   *
   * One such case is when noResolve flag is set.
   * If this function is able to determine the results without queueing
   * and the lookup failed, this function will return -1.
   * 
   * @return 0 on success or -1 on error. Note that
   * returning 0 means that either we are in the process of doing
   * lookup or that it has finished already.
   */
  virtual int startAsyncLookup();

  /**
   * Cancels any on-going asynchronous lookups
   */
  virtual void cancelAsyncLookup();

  /**
   * Place the socket in listen mode. The parameters are the same as for
   * the system listen() call. 
   * @param N		the queue length for pending connections
   * @return 0 on success, -1 on system error (errno
   *         available) and -2 if this is not a passiveSocket.
   */
  virtual int listen(int N = 5); // 5 is arbitrary

  /**
   * Accepts an incoming connection from the socket. If this socket is in
   * blocking mode, this function will block until a connection is received.
   * Otherwise, it might return with error. The sock parameter will be
   * initialised with the newly created socket.
   *
   * Upon successful acception (i.e., this function returns 0), the newly
   * created socket will be already connected. The socket will be unbuffered
   * and readyRead() and readyWrite() signals will be disabled.
   *
   * @param sock	a pointer to an KExtendedSocket variable
   * @return 0 on success, -1 on system error (errno set) and -2 if this is
   * not a passiveSocket and -3 if this took too long (time out)
   */
  virtual int accept(KExtendedSocket *&sock);

  /**
   * Attempts to connect to the remote host. 
   * After successful connection (return value 0), the socket will be ready
   * for I/O operations. Note, however, that not all signals may be enabled
   * for emission by this socket:
   * @li @ref readyRead and @ref readyWrite signals will be enabled only if
   *    @ref enableRead or @ref enableWrite were called. You can still enable
   *    them by calling those functions, of course.
   * @li #closed() will only be sent if we are indeed reading from the input
   *    stream. That is, if this socket is buffering the input. See @ref setBufferSize
   *
   * Note that, in general, functions inherited/overriden from KBufferedIO will only
   * work on buffered sockets, like bytesAvailable and bytesToWrite.
   * @return    The return values are:
   * @li 0: success
   * @li -1: system error, errno was set accordingly
   * @li -2: this socket cannot connect(); this is a passiveSocket. It can also
   *   mean that the function was unable to make a connection with the given
   *   bind address or that an asynchronous connection attempt is already
   *   in progress.
   * @li -3: connection timed out
   *
   */
  virtual int connect();

  /**
   * Starts an asynchronous connect. This works exactly the same as #connect,
   * except that the connection result won't be returned. 
   *
   * Note that those signals might be emitted before this function returns, so your
   * code should be prepared for that condition.
   * @return 0 on successful queueing of the connect or -1 on error. 
   *         If this function returns 0, then the @ref connectionSuccess() or the 
   *         @ref connectionFailed() signals will be emitted.
   */
  virtual int startAsyncConnect();

  /**
   * Cancels any on-going asynchronous connection attempt.
   */
  virtual void cancelAsyncConnect();

  /**
   * Implementation of @ref QIODevice::open() pure virtual function.
   * This depends on the target host address already being there.
   * If this is a passiveSocket, this is identical to call listen(); else, if
   * this is not a passiveSocket and no connection attempt is in progress, this
   * is like connect(). If one is in progress, this function will fail.
   * @param mode the open mode. Must be IO_Raw | IO_ReadWrite
   * @return true if successful, false when an error occurred or the most was
   *         not correct
   */
  virtual bool open(int mode = IO_Raw | IO_ReadWrite);

  /**
   * Closes the socket. If we have data still in the write buffer yet to be
   * sent, the socket won't be closed right now. It'll be closed after we managed
   * to send everything out.
   * If you want to close the socket now, you may want to call @ref flush() first,
   * and then @ref closeNow().
   */
  virtual void close();

  /**
   * Closes the socket now, discarding the contents of the write buffer, if any.
   * The read buffer's contents are kept until they are emptied by read operations
   * or the class is destroyed.
   */
  virtual void closeNow();

  /**
   * Releases the socket and anything we have holding on it. The class cannot
   * be used anymore. In other words, this is just like closeNow(), but it does
   * not actually close the socket.
   *
   * This is useful if you just want to connect and don't need the rest of the
   * class.
   *
   * Note that the buffers' contents will be discarded.
   *
   * Use of this method is discouraged, because the socket created might be such that
   * normal library routines can't handle (read, write, close, etc.)
   */
  virtual void release();

  /*
   * -- I/O --
   */

  /**
   * Flushes the socket buffer. You need not call this method during normal
   * operation as we will try and send everything as soon as possible.
   * However, if you want to make sure that data in the buffer is being sent
   * at this moment, you can call this function. It will try to send as much
   * data as possible, but it will stop as soon as the kernel cannot receive
   * any more data, and would possibly block.
   *
   * By repeatedly calling this function, the behaviour will be like that of
   * a blocking socket. Indeed, if this function is called with the kernel not
   * ready to receive data, it will block, unless this is a non-blocking socket.
   *
   * This function does not touch the read buffer. You can empty it by calling
   * @ref readBlock() with a null destination buffer.
   */
  virtual void flush();

  /**
   * Returns length of this socket. This call is not supported on sockets.
   * @return the length of this socket, or 0 if unsupported
   */
  virtual inline Q_ULONG size() const
  { return 0; }

  /**
   * Returns relative position from start. This call is not supported on sockets.
   * @return the relative position from the start, or 0 if unsupported
   */
  virtual inline Q_ULONG at() const
  { return 0; }

  /**
   * Returns true if we are at position. This is not supported on sockets.
   * @param i the position to check
   * @return true if we art at the given position, or always true if unsupported.
   */
  virtual inline bool at(int i)
  { Q_UNUSED(i);return true; }

  /**
   * Returns true if we are at the end. This is not supported on sockets, but
   * we always are at the end in a socket...
   * @return true if we are at the end. Always false if unsupported.
   */
  virtual inline bool atEnd() const
  { return false; }

  /**
   * Reads a block of data from the socket.
   *
   * If the socket is not buffered, this function will simply call the underlying
   * read method. This function will block if the socket is not on non-blocking mode
   * (see @ref setBlockingMode) and there is not enough data to be read in the
   * Operating System yet. If we are in non-blocking operation, the call will
   * fail in this case.
   *
   * However, if we are buffering, this function will instead read from the
   * buffer while there is available data. This function will never block
   * in buffering mode, which means that if you try to read while the buffers
   * are empty, this function will always return -1 and set the system error to
   * EWOULDBLOCK (aka EAGAIN), so as to mimic non-blocking operation.
   *
   * @param data	where we will write the read data to
   * @param maxlen	maximum length of data to be read
   * @return the number of bytes effectively read, or a negative number in case
   * or an error. If the @p data param is not null, then this is also the number
   * of bytes copied into that buffer. If the return value is different than
   * @p maxlen, then this function encountered a situation in which no more
   * bytes were available. Subsequent calls might cause this function to one
   * of these behaviours:
   * @li return an error, with EWOULDBLOCK system error, if we buffering
   *   or we are in non-blocking mode
   * @li otherwise, it'll block
   * This function returns 0, if the function detected end-of-file condition
   * (socket was closed)
   */
  virtual Q_LONG readBlock(char *data, Q_ULONG maxlen);

  /**
   * Writes a block of data to the socket.
   *
   * If the socket is not buffered, this function will simply call the underlying
   * write method. This means that the function might block if that method blocks
   * as well. That situation is possible if we are not in non-blocking mode and
   * the operating system buffers are full for this socket. If we are in
   * non-blocking mode and the operating system buffers are full, this function
   * will return -1 and the system error will be set to EWOULDBLOCK.
   *
   * If we are buffering, this function will simply transfer the data into the
   * write buffer. This function will then always succeed, as long as there is
   * enough room in the buffer. If the buffer size was limited and that limit
   * is reached, this function will copy no more bytes than that limit. Trying
   * to write with a full buffer will return -1 and set system error to
   * EWOULDBLOCK.
   *
   * @param data	the data to write
   * @param len		the length of data to write
   * @return the number of bytes written from @p data buffer.
   * The return value might be less than @p len if the output buffers cannot
   * accommodate that many bytes and -1 in the case of an errro.
   */
  virtual Q_LONG writeBlock(const char *data, Q_ULONG len);

  /**
   * Peeks at a block of data from the socket.
   *
   * This is exactly like read, except that the data won't be flushed from the
   * read buffer.
   *
   * If this socket is not buffered, this function will always return with
   * 0 bytes copied.
   *
   * @param data	where to store the data
   * @param maxlen	how many bytes to copy, at most
   * @return the number of bytes copied. 0 does not mean end-of-file 
   *         condition.
   */
  virtual int peekBlock(char *data, uint maxlen);

  /**
   * Reimplementation of unreadBlock() method. This is so because unreading in
   * sockets doesn't make sense, so this function will always return -1 (error)
   * and set the system error to ENOSYS.
   * @return always -1 (error)
   */
  virtual int unreadBlock(const char *data, uint len);

  /**
   * Returns the number of available bytes yet to be read via @ref readBlock
   * and family of functions. 
   *
   * Note: as of now, this only works on input-buffered sockets. This will
   * change in the future
   * @return The number of available bytes, or -1 on error or -2 if this call is invalid
   * in the current state.
   */
  virtual int bytesAvailable() const;

  /**
   * Waits @p msec milliseconds for more data to be available (use 0 to
   * wait forever). The return value is the amount of data available for
   * read in the read buffer.
   *
   * @param msec	milliseconds to wait
   * @return -1 in case of system error and -2 in case of invalid socket 
   *         state
   */
  virtual int waitForMore(int msec);

  /**
   * Gets a single character (unsigned char) from the stream.
   * @return the value of the character. Negative if there was an error.
   */
  virtual int getch();

  /**
   * Writes a single character (unsigned char) to the stream. All other bits
   * will be ignored.
   * @param ch	character to write, converted to char
   */
  virtual int putch(int ch);

  /**
   * Unreads one character from the stream. This is not possible on sockets.
   * @return always returns -1 on sockets.
   */
  virtual int ungetch(int)
  { return -1; }

  /**
   * Toggles the emission of the readyRead signal.
   *
   * Note that this signal is emitted every time more data is available to be
   * read, so you might get flooded with it being emitted every time, when in
   * non-buffered mode. However, in buffered mode, this signal will be
   * emitted only when there is data coming in from the wire.
   * By default, this flag is set to false, i.e., signal not being emitted.
   * @param enable	if true, the signal will be emitted
   */
  virtual void enableRead(bool enable);

  /**
   * Toggles the emission of the readyWrite signal.
   *
   * Note that this signal is emitted only when the OS is ready to receive more
   * data, which means that the write buffer is empty. And when that is reached,
   * this signal will possibly be emitted on every loop, so you might
   * want to disable it. By default, this flag is set to false.
   * @param enable	if true, the signal will be emitted
   */
  virtual void enableWrite(bool enable);

signals:
  /**
   * This signal is emitted whenever an asynchronous lookup process is done.
   * The parameter @p count tells 
   * @param count the number of results
   */
  void lookupFinished(int count);

  /**
   * This signal is emitted whenever we connected asynchronously to a host.
   */
  void connectionSuccess();

  /**
   * This signal is emitted whenever our asynchronous connection attempt
   * failed to all hosts listed.
   * @param error	the errno code of the last connection attempt
   */
  void connectionFailed(int error);

  /**
   * This signal is emitted whenever this socket is ready to accept another
   * socket.
   * @see accept()
   */
  void readyAccept();

protected:
  int sockfd;			// file descriptor of the socket

protected slots:

  void socketActivityRead();
  void socketActivityWrite();
  void dnsResultsReady();
  void startAsyncConnectSlot();
  void connectionEvent();

protected:

  QSocketNotifier *readNotifier();
  QSocketNotifier *writeNotifier();

private:

  // protection against accidental use
  KExtendedSocket(KExtendedSocket&);
  KExtendedSocket& operator=(KExtendedSocket&);

  /**
   * This is actually a wrapper around getaddrinfo().
   * @internal
   */
  static int doLookup(const QString& host, const QString& serv, addrinfo& hint,
		      kde_addrinfo** result);

protected:
  /**
   * Sets the error code
   */
  void setError(int errorkind, int error);

  inline void cleanError()
  { setError(IO_Ok, 0); }

  /**
   * Sets the socket status. For derived classes only.
   */
  void setSocketStatus(int status);

public:
  /**
   * Performs resolution on the given socket address.
   *
   * That is, tries to resolve the raw form of the socket address into a textual
   * representation.
   *
   * @param sockaddr	the socket address
   * @param host	where the hostname will be written
   * @param port	where the service-port will be written
   * @param flags	the same flags as getnameinfo()
   * @returns 0 on success, nonzero otherwise.
   */
  static int resolve(sockaddr* sock, ksocklen_t len, QString& host, QString& port, int flags = 0);

  /**
   * Performs resolution on the given socket address.
   *
   * That is, tries to resolve the raw form of the socket address into a textual
   * representation.
   *
   * @param sockaddr	the socket address
   * @param host	where the hostname will be written
   * @param port	where the service-port will be written
   * @param flags	the same flags as getnameinfo()
   * @returns 0 on success, nonzero otherwise.
   */
  static int resolve(KSocketAddress* sock, QString& host, QString& port, int flags = 0);

  /**
   * Performs lookup on the given hostname/port combination and returns a list
   * of matching addresses.
   * The error code can be transformed into string by @ref KExtendedSocket::strError()
   * with code of IO_LookupError.
   *
   * IMPORTANT: the result values of the @ref QPtrList must be deleted after use. So,
   * if you don't copy the @ref KAddressInfo objects, the best way to assure that
   * is to call setAutoDelete(true) on the list right after this function
   * returns. If you do copy the results out, you must assure that the objects
   * get deleted when they are not needed any more.
   *
   * @param host	the hostname to look up
   * @param port	the port/service to look up
   * @param flags	flags to be used when looking up, @ref Flags
   * @param error	pointer to a variable holding the error code
   * @return a list of @ref KAddressInfos
   */
  static QPtrList<KAddressInfo> lookup(const QString& host, const QString& port, int flags = 0, int *error = 0);

  /**
   * Returns the local socket address
   * Remember to delete the returned object when it is no longer needed.
   * @param fd		the file descriptor
   * @return the local socket address or 0 if an error occurred. Delete after use.
   */
  static KSocketAddress *localAddress(int fd);

  /**
   * Returns the peer socket address. Use KExtendedSocket::resolve() to
   * resolve this to a human-readable hostname/service or port.
   * Remember to delete the returned object when it is no longer needed.
   * @param fd		the file descriptor
   * @return the peer socket address or 0 if an error occurred. Delete after use.
   */
  static KSocketAddress *peerAddress(int fd);

  /**
   * Returns the representing text of this error code
   * @param code	the error code, as seen in status()
   * @param syserr	the system error, as from systemError()
   * @return the text for the given error code
   */
  static QString strError(int code, int syserr);

  /**
   * Sets/unsets address reusing flag for this socket.
   *
   * This function returns true if the value was set correctly. That is NOT
   * the result of the set.
   * @param fd	the file descriptor
   * @param enable	if true, set address reusable
   * @return true on success, false on failure.
   */
  static bool setAddressReusable(int fd, bool enable);

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KExtendedSocketPrivate *d;

  friend class KSocket;
  friend class KServerSocket;
};

/**
 * Contains information about an internet address. It wraps addrinfo,
 * see getaddrinfo(3) for more information.
 */
class KAddressInfo
{
private:
  addrinfo *ai;
  KSocketAddress *addr;

  inline KAddressInfo() : ai(0), addr(0)
  { }

  KAddressInfo(addrinfo *ai);
  KAddressInfo(KAddressInfo&) { }
  KAddressInfo& operator=(KAddressInfo&) { return *this; }

public:
  ~KAddressInfo();

  /**
   * Returns the KAddressInfo's @ref KSocketAddress.
   * Only valid as long as the KAddressInfo exists.
   */
  inline operator const KSocketAddress*() const
  { return addr; }

  /**
   * Returns the KAddressInfo's addrinfo.
   */
  inline operator const addrinfo&() const
  { return *ai; }

  /**
   * Returns a pointer to KAddressInfo's addrinfo. 
   * Only valid as long as the KAddressInfo exists.
   */
  inline operator const addrinfo*() const
  { return ai; }

  /**
   * Returns the KAddressInfo's @ref KSocketAddress.
   * Only valid as long as the KAddressInfo exists.
   * @return the KAddressInfo's KSocketAddress.
   */
  inline const KSocketAddress* address() const
  { return addr; }

  /**
   * Returns the flags of the address info (see getaddrinfo(3)).
   * @return the flags of the addres info.
   */
  int flags() const;

  /**
   * Returns the family of the address info (see getaddrinfo(3)).
   * @return the family of the addres info.
   */
  int family() const;

  /**
   * Returns the socket type of the address info (see getaddrinfo(3)).
   * @return the socket type of the addres info.
   */
  int socktype() const;

  /**
   * Returns the protocol of the address info (see getaddrinfo(3)).
   * @return the protocol of the addres info.
   */
  int protocol() const;


  /**
   * Returns the official name of the host (see getaddrinfo(3)).
   * Only valid as long as the KAddressInfo exists.
   * @return the official name of the host
   */
  const char* canonname() const;

  /**
   * Returns the length of the KSocketAddress.
   * @return the KSocketAddress's length
   */
  inline int length() const
  { if (addr) return addr->size(); return 0; }

  friend class KExtendedSocket;
};

#endif // KEXTSOCK_H
