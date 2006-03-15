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

#include <assert.h>
#include <string.h>

#include <QMutableListIterator>

#include "ksocketbase.h"
#include "ksocketbuffer_p.h"

using namespace KNetwork;
using namespace KNetwork::Internal;

KSocketBuffer::KSocketBuffer(qint64 size)
  : m_mutex(QMutex::Recursive), m_offset(0), m_size(size), m_length(0)
{
}

KSocketBuffer::KSocketBuffer(const KSocketBuffer& other)
  : KIOBufferBase(other), m_mutex(QMutex::Recursive)
{
  *this = other;
}

KSocketBuffer::~KSocketBuffer()
{
  // QValueList takes care of deallocating memory
}

KSocketBuffer& KSocketBuffer::operator=(const KSocketBuffer& other)
{
  QMutexLocker locker1(&m_mutex);
  QMutexLocker locker2(&other.m_mutex);

  KIOBufferBase::operator=(other);

  m_list = other.m_list;	// copy-on-write
  m_offset = other.m_offset;
  m_size = other.m_size;
  m_length = other.m_length;

  return *this;
}

bool KSocketBuffer::canReadLine() const
{
  QMutexLocker locker(&m_mutex);

  qint64 offset = m_offset;

  // walk the buffer
  for (int i = 0; i < m_list.size(); ++i)
    {
      if (m_list.at(i).indexOf('\n', offset) != -1)
	return true;
      if (m_list.at(i).indexOf('\r', offset) != -1)
	return true;
      offset = 0;
    }

  return false;			// not found
}

qint64 KSocketBuffer::readLine(char* data, qint64 maxSize)
{
  if (!canReadLine())
    return qint64(-1);		// empty

  QMutexLocker locker(&m_mutex);

  // find the offset of the newline in the buffer
  qint64 newline = 0;
  qint64 offset = m_offset;

  // walk the buffer
  for (int i = 0; i < m_list.size(); ++i)
    {
      int posnl = m_list.at(i).indexOf('\n', offset);
      if (posnl == -1)
	{
	  // not found in this one
	  newline += m_list.at(i).size();
	  offset = 0;
	  continue;
	}

      // we found it
      newline += posnl;
      break;
    }

  qint64 bytesToRead = newline + 1 - m_offset;
  if (bytesToRead > maxSize)
    bytesToRead = maxSize;		// don't read more than maxSize

  return consumeBuffer(data, bytesToRead);
}

qint64 KSocketBuffer::length() const
{
  return m_length;
}

qint64 KSocketBuffer::size() const
{
  return m_size;
}

bool KSocketBuffer::setSize(qint64 size)
{
  m_size = size;
  if (size == -1 || m_length < m_size)
    return true;

  // size is now smaller than length
  QMutexLocker locker(&m_mutex);

  // repeat the test
  if (m_length < m_size)
    return true;

  // discard from the beginning
  return (m_length - m_size) == consumeBuffer(0L, m_length - m_size, true);
}

qint64 KSocketBuffer::feedBuffer(const char *data, qint64 len)
{
  if (data == 0L || len == 0)
    return 0;			// nothing to write
  if (isFull())
    return -1;			// can't write

  QMutexLocker locker(&m_mutex);

  // verify if we can add len bytes
  if (m_size != -1 && (m_size - m_length) < len)
    len = m_size - m_length;

  QByteArray a(len);
  a.duplicate(data, len);
  m_list.append(a);

  m_length += len;
  return len;
}

qint64 KSocketBuffer::consumeBuffer(char *destbuffer, qint64 maxlen, bool discard)
{
  if (maxlen == 0 || isEmpty())
    return 0;

  QMutableListIterator<QByteArray> it(m_list);
  qint64 offset = m_offset;
  qint64 copied = 0;

  // walk the buffer
  while (it.hasNext() && maxlen)
    {
      QByteArray& item = it.next();
      // calculate how much we'll copy
      size_t to_copy = item.size() - offset;
      if (to_copy > maxlen)
	to_copy = maxlen;

      // do the copying
      if (destbuffer)
	memcpy(destbuffer + copied, item.data() + offset, to_copy);
      maxlen -= to_copy;
      copied += to_copy;

      if (item.size() - offset > to_copy)
	{
	  // we did not copy everything
	  offset += to_copy;
	  break;
	}
      else
	{
	  // we copied everything
	  // discard this element;
	  offset = 0;
	  if (discard)
	    it.remove();
	}
    }

  if (discard)
    {
      m_offset = offset;
      m_length -= copied;
      assert(m_length >= 0);
    }

  return copied;
}

void KSocketBuffer::clear()
{
  QMutexLocker locker(&m_mutex);
  m_list.clear();
  m_offset = 0;
  m_length = 0;
}

qint64 KSocketBuffer::sendTo(KActiveSocketBase* dev, qint64 len)
{
  if (len == 0 || isEmpty())
    return 0;

  QMutexLocker locker(&m_mutex);
  
  QMutableListIterator<QByteArray> it(m_list);
  qint64 offset = m_offset;
  qint64 written = 0;
  
  // walk the buffer
  while (it.hasNext() && (len || len == -1))
    {
      // we have to write each element up to len bytes
      // but since we can have several very small buffers, we can make things
      // better by concatenating a few of them into a big buffer
      // question is: how big should that buffer be? 2 kB should be enough

      Q_ULONG bufsize = 1460;
      if (len != -1 && len < bufsize)
	bufsize = len;
      QByteArray buf(bufsize);
      qint64 count = 0;

      while (it.hasNext() && count + (it.peekNext().size() - offset) <= bufsize)
	{
	  QByteArray& item = it.next();
	  memcpy(buf.data() + count, item.data() + offset, item.size() - offset);
	  count += item.size() - offset;
	  offset = 0;
	}

      // see if we can still fit more
      if (count < bufsize && it.hasNext())
	{
	  // getting here means this buffer (peekNext) is larger than
	  // (bufsize - count) (even for count == 0).
	  QByteArray& item = it.next();
	  memcpy(buf.data() + count, item.data() + offset, bufsize - count);
	  offset += bufsize - count;
	  count = bufsize;
	}

      // now try to write those bytes
      qint64 wrote = dev->write(buf, count);

      if (wrote == -1)
	// error?
	break;

      written += wrote;
      if (wrote != count)
	// can't fit more?
	break;
    }

  // discard data that has been written
  // this updates m_length too
  if (written)
    consumeBuffer(0L, written);

  return written;
}

qint64 KSocketBuffer::receiveFrom(KActiveSocketBase* dev, qint64 len)
{
  if (len == 0 || isFull())
    return 0;

  QMutexLocker locker(&m_mutex);

  if (len == -1)
    len = dev->bytesAvailable();
  if (len <= 0)
    // error or closing socket
    return len;

  // see if we can read that much
  if (m_size != -1 && len > (m_size - m_length))
    len = m_size - m_length;

  // here, len contains just as many bytes as we're supposed to read

  // now do the reading
  QByteArray a(len);
  len = dev->read(a.data(), len);

  if (len == -1)
    // error?
    return -1;

  // success
  // resize the buffer and add it
  a.truncate(len);
  m_list.append(a);
  m_length += len;
  return len;
}
