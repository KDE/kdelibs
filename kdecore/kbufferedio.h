/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Thiago Macieira <thiagom@mail.com>
 *
 *  $Id$
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

#ifndef KBUFFEREDIO_H
#define KBUFFEREDIO_H

#include <qcstring.h>
#include <qptrlist.h>
#include "kasyncio.h"

class KBufferedIOPrivate;
/**
 * This abstract class implements basic functionality for buffered
 * input/output.
 *
 * Through the available methods, you can find out how many bytes are
 * available for reading, how many are still unsent and you can peek at
 * the buffered data.
 *
 * This class was intentionally written to resemble QSocket, because
 * @ref KExtendedSocket is a subclass of this one. This is so that applications
 * written using @ref QSocket's buffering characteristics will be more easily
 * ported to the more powerful @ref KExtendedSocket class.
 *
 * KBufferedIO already provides a powerful internal buffering algorithm. However,
 * this does not include the I/O itself, which must be implemented in
 * derived classes. Thus, to implement a class that does some I/O, you must
 * override, in addition to the pure virtual @ref QIODevice methods, these two:
 * @li @ref closeNow()
 * @li @ref waitForMore()
 *
 * If your derived class reimplements the buffering algorithm, you must then
 * decide which buffering functions to override. For instance, you may want to
 * change the protected functions like @ref feedReadBuffer() and @ref consumeReadBuffer().
 *
 * @author Thiago Macieira <thiagom@mail.com>
 * @version $Id$
 * @short Buffered I/O
 */
class KBufferedIO: public KAsyncIO
{
  Q_OBJECT

protected:
  // no default public constructor
  KBufferedIO();

public:
  /**
   * The modes for @ref closed() signal
   */
  enum closeModes
  {
    availRead = 0x01,
    dirtyWrite = 0x02,
    involuntary = 0x10,
    delayed = 0x20,
    closedNow = 0x40
  };

  /**
   * Destroys this class. The flushing of the buffers is implementation dependant.
   * The default implementation discards the contents
   */
  virtual ~KBufferedIO();

  /**
   * Closes the stream now, discarding the contents of the
   * write buffer. That is, we won't try to flush that
   * buffer before closing. If you want that buffer to be
   * flushed, you can call @ref QIODevice::flush(), which is blocking, and
   * then closeNow, or you can call @ref QIODevice::close() for a delayed
   * close.
   */
  virtual void closeNow() = 0;

  /**
   * Sets the internal buffer size to value.
   *
   * Not all implementations support this.
   *
   * The parameters may be 0 to make the class unbuffered or -1
   * to let the class choose the size (which may be unlimited) or
   * -2 to leave the buffer size untouched.
   *
   * Note that setting the write buffer size to any value smaller than
   * the current size of the buffer will force it to flush first,
   * which can make this call blocking.
   *
   * The default implementation does not support setting the buffer
   * sizes. You can only call this function with values -1 for "don't care"
   * or -2 for "unchanged"
   * @param rsize	the size of the read buffer
   * @param wsize	the size of the write buffer
   * @return true if setting both was ok. If false is returned, the
   * buffers were left unchanged.
   */
  virtual bool setBufferSize(int rsize, int wsize = -2);

  /**
   * Returns the number of bytes available for reading in the read buffer
   * @return the number of bytes available for reading
   */
  virtual int bytesAvailable() const;

  /**
   * Waits for more data to be available and returns the amount of available data then.
   * 
   * @param msec	number of milliseconds to wait, -1 to wait forever
   * @return -1 if we cannot wait (e.g., that doesn't make sense in this stream)
   */
  virtual int waitForMore(int msec) = 0;

  /**
   * Returns the number of bytes yet to write, still in the write buffer
   * @return the number of unwritten bytes in the write buffer
   */
  virtual int bytesToWrite() const;

  /**
   * Checks whether there is enough data in the buffer to read a line
   *
   * The default implementation reads directly from @ref inBuf, so if your
   * implementation changes the meaning of that member, then you must override
   * this function.
   * @return true when there is enough data in the buffer to read a line
   */
  virtual bool canReadLine() const;

  // readBlock, peekBlock and writeBlock are not defined in this class (thus, left
  // pure virtual) because this does not mean only reading and writing
  // to the buffers. It may be necessary to do I/O to complete the
  // transaction (e.g., user wants to read more than is in the buffer).
  // Reading and writing to the buffer are available for access through
  // protected member functions

  /**
   * Reads into the user buffer at most maxlen bytes, but does not
   * consume that data from the read buffer. This is useful to check
   * whether we already have the needed data to process something.
   *
   * This function may want to try and read more data from the system
   * provided it won't block.
   *
   * @param data	the user buffer pointer, at least maxlen bytes long
   * @param maxlen	the maximum length to be peeked
   * @return the number of bytes actually copied.
   */
  virtual int peekBlock(char *data, uint maxlen) = 0;

  /**
   * Unreads some data. That is, write the data to the beginning of the
   * read buffer, so that next calls to readBlock or peekBlock will see
   * this data instead.
   *
   * Note not all devices implement this since this could mean a semantic
   * problem. For instance, sockets are sequential devices, so they won't
   * accept unreading.
   * @param data	the data to be unread
   * @param size	the size of the data
   * @return the number of bytes actually unread
   */
  virtual int unreadBlock(const char *data, uint len);

signals:
  /**
   * This signal gets sent whenever bytes are written from the buffer.
   * @param nbytes the number of bytes sent.
   */
  void bytesWritten(int nbytes);

  // There is no read signal here. We use the readyRead signal inherited
  // from KAsyncIO for that purpose

  /**
   * This signal gets sent when the stream is closed. The @p state parameter
   * will give the current state, in OR-ed bits:
   * @li availRead:	read buffer contains data to be read
   * @li dirtyWrite:	write buffer wasn't empty when the stream closed
   * @li involuntary:	the stream wasn't closed due to user request
   *			(i.e., call to close). Probably remote end closed it
   * @li delayed:	the stream was closed voluntarily by the user, but it
   *			happened only after the write buffer was emptied
   * @li closedNow:	the stream was closed voluntarily by the user, by
   *			explicitly calling @ref closeNow, which means the
   *			write buffer's contents may have been discarded
   * @param state the state (see function description)
   */
  void closed(int state);

protected:
  /**
   * For an explanation on how this buffer work, please refer to the comments
   * at the top of kbufferedio.cpp
   */
  QPtrList<QByteArray> inBuf;

  /**
   * For an explanation on how this buffer work, please refer to the comments
   * at the top of kbufferedio.cpp
   */
  QPtrList<QByteArray> outBuf;

  unsigned inBufIndex, outBufIndex;

  /**
   * Consumes data from the input buffer.
   * That is, this will copy the data stored in the input (read) buffer
   * into the given @p destbuffer, as much as @p nbytes.
   * @param nbytes	the maximum amount of bytes to copy into the buffer
   * @param destbuffer	the destination buffer into which to copy the data
   * @param discard	whether to discard the copied data after the operation
   * @return the real amount of data copied. If it is less than
   * nbytes, then all the buffer was copied.
   */
  virtual unsigned consumeReadBuffer(unsigned nbytes, char *destbuffer, bool discard = true);

  /**
   * Consumes data from the output buffer.
   * Since this is called whenever we managed to send data out the wire, we
   * can only discard this amount from the buffer. There is no copying and no
   * "peeking" for the output buffer.
   *
   * Note this function should be called AFTER the data was sent. After it
   * is called, the data is no longer available in the buffer. And don't pass
   * wrong nbytes values.
   * @param nbytes	the amount of bytes to discard
   */
  virtual void consumeWriteBuffer(unsigned nbytes);

  /**
   * Feeds data into the input buffer.
   * This happens when we detected available data in the device and read it.

   * The data will be appended to the buffer or inserted at the beginning,
   * depending on whether @p atBeginning is set or not.
   * @param nbytes	the number of bytes in the buffer
   * @param buffer	the data that was read
   * @param atBeginning	whether to append or insert at the beginning
   * @return the number of bytes that have been appended
   */
  virtual unsigned feedReadBuffer(unsigned nbytes, const char *buffer, bool atBeginning = false);

  /**
   * Feeds data into the output buffer.
   * This happens when the user told us to write some data.
   * The data will be appended to the buffer.
   * @param nbytes	the number of bytes in the buffer
   * @param buffer	the data that is to be written
   * @return the number of bytes that have been appended
   */
  virtual unsigned feedWriteBuffer(unsigned nbytes, const char *buffer);

  /**
   * Returns the number of bytes in the read buffer
   * @return the size of the read buffer in bytes
   */
  virtual unsigned readBufferSize() const;

  /**
   * Returns the number of bytes in the write buffer
   * @return the size of the write buffer in bytes
   */
  virtual unsigned writeBufferSize() const;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KBufferedIOPrivate *d;
};

#endif // KBUFFEREDIO_H
