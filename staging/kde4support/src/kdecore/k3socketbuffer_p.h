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

#ifndef KSOCKETBUFFER_P_H
#define KSOCKETBUFFER_P_H

#include <QMutex>
#include <QByteArray>
#include <QList>

namespace KNetwork {

class KActiveSocketBase;

  namespace Internal {

/**
 * @internal
 * @class KSocketBuffer k3socketbuffer_p.h k3socketbuffer_p.h
 * @brief generic socket buffering code
 *
 * This class implements generic buffering used by KBufferedSocket.
 *
 * @author Thiago Macieira <thiago@kde.org>
 * @deprecated Use KSocketFactory or KLocalSocket instead
 */
class KSocketBuffer
{
public:
  /**
   * Default constructor.
   *
   * @param size	the maximum size of the buffer
   */
  KSocketBuffer(qint64 size = -1);

  /**
   * Copy constructor.
   */
  KSocketBuffer(const KSocketBuffer& other);

  /**
   * Virtual destructor. Frees the buffer and discards its contents.
   */
  ~KSocketBuffer();

  /**
   * Assignment operator.
   */
  KSocketBuffer& operator=(const KSocketBuffer& other);

  /**
   * Returns true if a line can be read from the buffer.
   */
  bool canReadLine() const;

  /**
   * Reads a line from the buffer and discard it from the buffer.
   */
  qint64 readLine(char *data, qint64 maxSize);

  /**
   * Returns the number of bytes in the buffer. Note that this is not
   * the size of the buffer.
   *
   * @sa size
   */
  qint64 length() const;

  /**
   * Returns true if the buffer is empty of data.
   */
  inline bool isEmpty() const
  { return length() == 0; }

  /**
   * Retrieves the buffer size. The value of -1 indicates that
   * the buffer has no defined upper limit.
   *
   * @sa length for the length of the data stored
   */
  qint64 size() const;

  /**
   * Sets the size of the buffer, if allowed.
   *
   * @param size	the maximum size, use -1 for unlimited.
   * @returns true on success, false if an error occurred.
   * @note if the new size is less than length(), the buffer will be truncated
   */
  bool setSize(qint64 size);

  /**
   * Returns true if the buffer is full (i.e., cannot receive more data)
   */
  inline bool isFull() const
  { return size() != -1 && size() == length(); }

  /**
   * Adds data to the end of the buffer.
   *
   * @param data	the data to be added
   * @param len		the data length, in bytes
   * @returns the number of bytes added to the end of the buffer.
   */
  qint64 feedBuffer(const char *data, qint64 len);

  /**
   * Clears the buffer.
   */
  void clear();

  /**
   * Consumes data from the beginning of the buffer.
   *
   * @param data	where to copy the data to
   * @param maxlen	the maximum length to copy, in bytes
   * @param discard	if true, the bytes copied will be discarded
   * @returns the number of bytes copied from the buffer
   */
  qint64 consumeBuffer(char *data, qint64 maxlen, bool discard = true);

  /**
   * Sends at most @p len bytes of data to the I/O Device.
   *
   * @param device	the device to which to send data
   * @param len		the amount of data to send; -1 to send everything
   * @returns the number of bytes sent and discarded from the buffer, -1
   *          indicates an error.
   */
  qint64 sendTo(KActiveSocketBase* device, qint64 len = -1);

  /**
   * Tries to receive @p len bytes of data from the I/O device.
   *
   * @param device	the device to receive from
   * @param len		the number of bytes to receive; -1 to read as much
   *                    as possible
   * @returns the number of bytes received and copied into the buffer,
   *	      -1 indicates an error.
   */
  qint64 receiveFrom(KActiveSocketBase* device, qint64 len = -1);

protected:
  mutable QMutex m_mutex;
  QList<QByteArray> m_list;
  qint64 m_offset;	///< offset of the start of data in the first element

  qint64 m_size;		///< the maximum length of the buffer
  mutable qint64 m_length;
};

} }			// namespace KNetwork::Internal

#endif
