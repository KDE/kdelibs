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

#ifndef QBUFFEREDSOCKET_H
#define QBUFFEREDSOCKET_H

#include <qobject.h>
#include <qcstring.h>
#include <qvaluelist.h>
#include "qstreamsocket.h"

#include "qtaddon.h"
ADDON_START

class QBufferedSocketPrivate;
/** @class QBufferedSocket qbufferedsocket.h qbufferedsocket.h
 *  @brief Buffered stream sockets.
 *
 * This class allows the user to create and operate buffered stream sockets
 * such as those used in most Internet connections. This class is
 * also the one that resembles the most to the old @ref QSocket implementation.
 *
 * @author Thiago Macieira <thiago.macieira@kdemail.net>
 */
class QBufferedSocket: public QStreamSocket
{
public:
  /** @class QBufferedSocket::Buffer qbufferedsocket.h qbufferedsocket.h
   *  @brief Socket buffering class.
   *
   * This class is the actual input and output buffer used for @ref QStreamSocket
   * objects. The default implementation uses an unlimited internal buffer which
   * grows and shrinks on demand. See @ref QBufferedSocket::SizedBuffer for
   * a buffer whose (maximum) size is determined.
   *
   * Especially note that objects of this type allow the user to inspect the buffer
   * contents through the @ref contents method, but the @ref QByteArray object that
   * is returned might or might not be modified after new input and output operations
   * in this object.
   */
  class Buffer
  {
  protected:
    QValueList<QByteArray> list;
    QIODevice::Offset start;	///< offset into the first element
    QIODevice::Offset end;	///< offset into the last element

  public:
    /**
     * Default constructor.
     */
    Buffer();

    /**
     * Destructor. The allocated resources are discarded.
     */
    virtual ~Buffer();

    /**
     * Sets the contents of this buffer as the specified data.
     *
     * This function is intended to be used as when replacing a buffer with
     * another, so, whenever possible, this function will use QByteArray's
     * explicitly sharing features. That also means that modification of the
     * @p contents parameter might modify the internal data in the buffer as well.
     *
     * Some implementations of this function might be lossy. The default
     * implementation never loses data.
     */
    virtual void setContents(const QByteArray& contents);

    /**
     * Return the condensed contents of this object into the QByteArray object.
     * This function has the side-effect of condensing the list of buffers
     * into a single QByteArray object, which will be returned.
     */
    virtual QByteArray contents();

    /**
     * Returns the number of bytes that this buffer can still hold
     * or -1 if it's unlimited.
     */
    virtual Q_LONG spaceAvailable() const;

    /**
     * Returns true if this buffer is empty.
     */
    bool isEmpty() const;

    /**
     * Returns trtue if this buffer is full.
     */
    bool isFull() const;

    /**
     * Feed the buffer (i.e., write into it). This function is analogous to
     * @ref QIODevice::writeBlock.
     */
    virtual Q_LONG feedBlock(const char *data, Q_ULONG len);

    /**
     * Consume the buffer (i.e., read from it). This function is analogous to
     * @ref QIODevice::readBlock.
     *
     * @param discard		if true, the data that is read is also discarded
     *				from the buffer
     */
    virtual Q_LONG consumeBlock(char *data, Q_ULONG maxlen, bool discard = true);

    /**
     * Receive from the device into this buffer.
     */
    virtual Q_LONG receive(QIODevice* device);

    /**
     * Send the contents of this buffer into the device.
     */
    virtual Q_LONG send(QIODevice* device);
  };

  /** @class QBufferedSocket::SizedBuffer qbufferedsocket.h qbufferedsocket.h
   *  @brief A socket buffer with maximum size.
   *
   * This class provides a buffer whose maximum size is specified and will
   * never grow bigger. However, this class also provides grow and shrink
   * on demand capabilities.
   *
   * @author Thiago Macieira <thiago.macieira@kdemail.net>
   */
  class SizedBuffer: public Buffer
  {
  public:
    /**
     * Default constructor.
     */
    SizedBuffer(Q_ULONG maxlen);

    /**
     * Destructor.
     */
    virtual ~SizedBuffer();

    /**
     * This function might lose data. If the given contents are larger than this
     * buffer's maximum size, the data will be shrunk to the maximum value. 
     * Especially note that since QByteArray shares data explicitly, this will
     * also affect the caller's buffer.
     */
    virtual void setContents(const QByteArray& contents);

    /**
     */
    virtual Q_LONG spaceAvailable() const;

    /**
     * Write into the buffer. If there is no more available space, this function
     * will fail to write.
     */
    virtual Q_LONG feedBlock(const char *data, Q_ULONG len);
 
  private:
    Q_ULONG m_maxlen;
 };

public:
  /**
   * Default constructor.
   *
   * @param node	destination host
   * @param service	destination service to connect to
   */
  QBufferedSocket(const QString& host = QString::null, const QString& service = QString::null,
		  QObject* parent = 0L, const char *name = 0L);

  /**
   * Destructor.
   */
  virtual ~QBufferedSocket();

  /**
   * Be sure to catch new devices.
   */
  virtual void setSocketDevice(QSocketImpl* device);

protected:
  /**
   * Buffered sockets can only operate in blocking mode.
   */
  virtual bool setSocketOptions(int opts);

public:
  /**
   * Closes the socket for new data, but allow data that had been buffered
   * for output with @ref writeBlock to be still be written.
   *
   * @sa closeNow
   */
  virtual void close();

  /**
   * Make use of the buffers.
   */
  virtual Q_LONG bytesAvailable() const;

  /**
   * Make use of buffers.
   */
  virtual Q_LONG waitForMore(int msecs, bool *timeout = 0L);

  /**
   * Reads data from the socket. Make use of buffers.
   */
  virtual Q_LONG readBlock(char *data, Q_ULONG maxlen);

  /**
   * @overload
   * Reads data from a socket.
   *
   * The @p from parameter is always set to @ref peerAddress()
   */
  virtual Q_LONG readBlock(char *data, Q_ULONG maxlen, QSocketAddress& from);

  /**
   * Peeks data from the socket.
   */
  virtual Q_LONG peekBlock(char *data, Q_ULONG maxlen);

  /**
   * @overload
   * Peeks data from the socket.
   *
   * The @p from parameter is always set to @ref peerAddress()
   */
  virtual Q_LONG peekBlock(char *data, Q_ULONG maxlen, QSocketAddress &from);

  /**
   * Writes data to the socket.
   */
  virtual Q_LONG writeBlock(const char *data, Q_ULONG len);

  /**
   * @overload
   * Writes data to the socket.
   *
   * The @p to parameter is discarded.
   */
  virtual Q_LONG writeBlock(const char *data, Q_ULONG len, const QSocketAddress& to);

  /**
   * Catch changes.
   */
  virtual void enableRead(bool enable);

  /**
   * Catch changes.
   */
  virtual void enableWrite(bool enable);

public:
  /**
   * Retrieves the input buffer object.
   */
  Buffer* inputBuffer() const;

  /**
   * Sets the input buffer object to this one.
   * Data is copied using the new object's setContents method.
   *
   * Setting this to null will cause buffering to stop.
   */
  virtual void setInputBuffer(Buffer* newBuffer);

  /**
   * Retrieves the output buffer object.
   */
  Buffer* outputBuffer() const;

  /**
   * Sets the output buffer object to this one.
   * Data is copied using the new object's setContents method.
   *
   * Setting this to null will cause buffering to stop.
   */
  virtual void setOutputBuffer(Buffer* newBuffer);

  /**
   * Returns the length of the output buffer.
   */
  virtual Q_ULONG bytesToWrite() const;

  /**
   * Closes the socket and discards any output data that had been buffered
   * with @ref writeBlock but that had not yet been written.
   *
   * @sa close
   */
  virtual void closeNow();

  /**
   * Returns true if a line can be read with @ref readLine
   */
  bool canReadLine() const;

  /**
   * Reads a line of data from the socket buffers.
   */
  QCString readLine();

protected slots:
  // protected slots

  /**
   * Handle buffering.
   */
  virtual void slotReadActivity();

  /**
   * Handle buffering.
   */
  virtual void slotWriteActivity();

protected:
  /**
   * Catch connection to clear the buffers
   */
  virtual void stateChanging(SocketState newState);

private:
  QBufferedSocket(const QBufferedSocket&);
  QBufferedSocket& operator=(const QBufferedSocket&);

  QBufferedSocketPrivate *d;
};

ADDON_END

using ADDON_NAMESPACE::QBufferedSocket;

#endif
