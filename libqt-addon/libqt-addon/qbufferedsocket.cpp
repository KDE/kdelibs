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

#include <qmutex.h>
#include "qsocketimpl.h"
#include "qsocketaddress.h"
#include "qbufferedsocket.h"

#define BUFFER_MIN_ALLOC_THRESHOLD		32
#define CONDENSE_THRESHOLD			1460

ADDON_USE

class ADDON_NAMESPACE::QBufferedSocketPrivate
{
public:
  mutable QBufferedSocket::Buffer *input, *output;

  QBufferedSocketPrivate()
    : input(new QBufferedSocket::Buffer),
      output(new QBufferedSocket::Buffer)
  {
  }
};

QBufferedSocket::Buffer::Buffer()
  : start(0), end(0)
{
}

QBufferedSocket::Buffer::~Buffer()
{
}

void QBufferedSocket::Buffer::setContents(const QByteArray& contents)
{
  list.clear();
  list.append(contents);
  start = 0;
  end = contents.size();
}

QByteArray QBufferedSocket::Buffer::contents()
{
  if (isEmpty())
    return QByteArray();	// no data

  // check if we're condensed
  if (start == 0 && list.count() == 1)
    {
      QByteArray& first = list.first();
      if (first.size() != end)
	first.resize(end);
      return first;
    }

  // we're not condensed

  // FIXME
  // the following operation is not thread-safe:

  QByteArray condensed;
  QValueList<QByteArray>::ConstIterator it;
  for (it = list.begin(); it != list.end(); ++it)
    {
      uint cursize = condensed.size();
      uint elemsize = (*it).size() - start;
      condensed.resize(cursize + elemsize, QGArray::SpeedOptim);
      memcpy(condensed.data() + cursize, (*it).data(), elemsize);
      start = 0;
    }
  condensed.resize(condensed.size() - (list.last().size() - end));
  list.clear();
  list.append(condensed);

  return condensed;
}

Q_LONG QBufferedSocket::Buffer::spaceAvailable() const
{
  return -1;
}

bool QBufferedSocket::Buffer::isEmpty() const
{
  if (list.isEmpty() || start == end)
    return true;
  return false;
}

bool QBufferedSocket::Buffer::isFull() const
{
  if (spaceAvailable() == 0)
    return true;		// 0 bytes available

  return false;			// not full
}

Q_LONG QBufferedSocket::Buffer::feedBlock(const char *data, Q_ULONG len)
{
  if (list.isEmpty() || len > BUFFER_MIN_ALLOC_THRESHOLD ||
      end > BUFFER_MIN_ALLOC_THRESHOLD)
    {
      // we must create a new element
      if (!list.isEmpty())
	list.last().resize(end); // make sure doesn't have more data than we've fed it

      QByteArray array(len);
      memcpy(array.data(), data, len);
      end = len;
      list.append(array);
      return len;
    }

  // small data; resize instead
  QByteArray& last = list.last();
  last.resize((end + len) & ~(BUFFER_MIN_ALLOC_THRESHOLD-1));
  memcpy(last.data() + end, data, len);
  end += len;
  return len;
}

Q_LONG QBufferedSocket::Buffer::consumeBlock(char *data, Q_ULONG maxlen, bool discard)
{
  if (isEmpty())
    return 0;

  QValueList<QByteArray>::Iterator it;
  Q_ULONG copied = 0;
  for (it = list.begin(); it != list.end() || copied == maxlen; )
    {
      uint elemsize = (*it).size() - start;

      if (list.count() == 1)
	elemsize -= (*it).size() - end;

      if (elemsize > maxlen - copied)
	elemsize = maxlen - copied;

      if (data)
	memcpy(data + copied, (*it).data() + start, elemsize);

      if (discard)
	{
	  // adjust start and end pointers
	  start += elemsize;
	  if (list.count() == 1)
	    end -= elemsize;

	  if ((list.count() == 1 && start == end) ||
	      start == (*it).size())
	    it = list.remove(it);
	  // if list.count() was 1, it's now 0 and it == list.end()
	}
      else
	++it;
    }

  return copied;
}

Q_LONG QBufferedSocket::Buffer::receive(QIODevice *device)
{
  Q_LONG totalread = 0;
  while (1)
    {
      Q_LONG len = spaceAvailable();
      if (len == -1)
	len = 16384;		// seems like a nice value
	
      QByteArray data(len);
      len = device->readBlock(data.data(), data.size());
      if (len <= 0)
	return len;

      feedBlock(data.data(), len);
      totalread += len;
      if (data.size() != (uint)len)
	break;
    }
  return totalread;
}

Q_LONG QBufferedSocket::Buffer::send(QIODevice *device)
{
  if (isEmpty())
    return 0;

  if (list.count() > 1 && list.first().size() - start < CONDENSE_THRESHOLD)
    contents();			// causes a condensing

  Q_LONG totalwritten = 0;
  QValueList<QByteArray>::Iterator it;
  QIODevice::Offset ourstart = start;
  for (it = list.begin(); it != list.end(); ++it)
    {
      Q_ULONG len = (*it).size() - ourstart;
      if (it == list.fromLast())
	len -= (*it).size() - end;

      Q_ULONG written = device->writeBlock((*it).data() + ourstart, len);
      if (written <= 0)
	{
	  if (totalwritten)
	    totalwritten = written;
	  break;
	}

      ourstart = 0;
      totalwritten += written;

      if (written != len)
	break;			// can't write more
    }

  consumeBlock(0L, totalwritten);
  return totalwritten;
}

QBufferedSocket::SizedBuffer::SizedBuffer(Q_ULONG maxlen)
  : m_maxlen(maxlen)
{
}

QBufferedSocket::SizedBuffer::~SizedBuffer()
{
}

void QBufferedSocket::SizedBuffer::setContents(const QByteArray& contents)
{
  if (contents.size() <= m_maxlen)
    QBufferedSocket::Buffer::setContents(contents);
  else
    {
      QByteArray data = contents;
      data.resize(m_maxlen);
      QBufferedSocket::Buffer::setContents(data);
    }
}

Q_LONG QBufferedSocket::SizedBuffer::spaceAvailable() const
{
  if (isEmpty() && m_maxlen > 0)
    return m_maxlen;		// it's actually empty

  QValueList<QByteArray>::ConstIterator it;
  uint size = 0;
  for (it = list.begin(); it != list.end(); ++it)
    size += (*it).size();

  size -= start;
  size -= list.last().size() - end;
  return m_maxlen - size;
}

Q_LONG QBufferedSocket::SizedBuffer::feedBlock(const char *data, Q_ULONG len)
{
  Q_LONG avail = spaceAvailable();
  if (avail < len)
    len = avail;
  return QBufferedSocket::Buffer::feedBlock(data, len);
}

QBufferedSocket::QBufferedSocket(const QString& host, const QString& service,
				 QObject *parent, const char *name)
  : QStreamSocket(host, service, parent, name),
    d(new QBufferedSocketPrivate)
{
  setBlocking(false);
}

QBufferedSocket::~QBufferedSocket()
{
  delete d->input;
  delete d->output;
}

void QBufferedSocket::setSocketDevice(QSocketImpl* device)
{
  QStreamSocket::setSocketDevice(device);
  device->setBlocking(false);
}

bool QBufferedSocket::setSocketOptions(int opts)
{
  if (opts & Blocking)
    return false;

  return QStreamSocket::setSocketOptions(opts);
}

void QBufferedSocket::close()
{
  if (!d->output || d->output->isEmpty())
    closeNow();
  else
    {
      setState(Closing);
      socketDevice()->readNotifier()->setEnabled(false);
      emit stateChanged(Closing);
    }
}

Q_LONG QBufferedSocket::bytesAvailable() const
{
  if (!d->input)
    return QStreamSocket::bytesAvailable();

  QMutexLocker locker(mutex());
  return d->input->contents().size();
}

Q_LONG QBufferedSocket::waitForMore(int msecs, bool *timeout)
{
  Q_LONG retval = QStreamSocket::waitForMore(msecs, timeout);
  if (d->input)
    {
      resetError();
      slotReadActivity();
      return bytesAvailable();
    }
  return retval;
}

Q_LONG QBufferedSocket::readBlock(char *data, Q_ULONG maxlen)
{
  if (d->input)
    {
      if (d->input->isEmpty())
	{
	  setError(IO_ReadError, WouldBlock);
	  return -1;
	}
      resetError();
      return d->input->consumeBlock(data, maxlen);
    }
  return QStreamSocket::readBlock(data, maxlen);
}

Q_LONG QBufferedSocket::readBlock(char *data, Q_ULONG maxlen, QSocketAddress& from)
{
  from = peerAddress();
  return readBlock(data, maxlen);
}

Q_LONG QBufferedSocket::peekBlock(char *data, Q_ULONG maxlen)
{
  if (d->input)
    {
      if (d->input->isEmpty())
	{
	  setError(IO_ReadError, WouldBlock);
	  return -1;
	}
      resetError();
      return d->input->consumeBlock(data, maxlen, false);
    }
  return QStreamSocket::peekBlock(data, maxlen);
}

Q_LONG QBufferedSocket::peekBlock(char *data, Q_ULONG maxlen, QSocketAddress& from)
{
  from = peerAddress();
  return peekBlock(data, maxlen);
}

Q_LONG QBufferedSocket::writeBlock(const char *data, Q_ULONG len)
{
  if (d->output)
    {
      if (d->output->isFull())
	{
	  setError(IO_WriteError, WouldBlock);
	  return -1;
	}
      resetError();
      socketDevice()->writeNotifier()->setEnabled(true);
      return d->output->feedBlock(data, len);
    }

  return QStreamSocket::writeBlock(data, len);
}

Q_LONG QBufferedSocket::writeBlock(const char *data, Q_ULONG maxlen, const QSocketAddress&)
{
  return writeBlock(data, maxlen);
}

void QBufferedSocket::enableRead(bool enable)
{
  QStreamSocket::enableRead(enable);
  if (!enable && d->input)
    // reenable it
    socketDevice()->readNotifier()->setEnabled(true);
}

void QBufferedSocket::enableWrite(bool enable)
{
  QStreamSocket::enableRead(enable);
  if (!enable && d->output && !d->output->isEmpty())
    // reenable it
    socketDevice()->writeNotifier()->setEnabled(true);
}

void QBufferedSocket::stateChanging(SocketState newState)
{
  if (newState == Connecting || newState == Connected)
    {
      if (d->input)
	d->input->setContents(QByteArray());
      if (d->output)
	d->output->setContents(QByteArray());
    }
  QStreamSocket::stateChanging(newState);
}

QBufferedSocket::Buffer* QBufferedSocket::inputBuffer() const
{
  return d->input;
}

void QBufferedSocket::setInputBuffer(Buffer* newBuffer)
{
  QMutexLocker locker(mutex());
  if (newBuffer && d->input)
    newBuffer->setContents(d->input->contents());
  if (d->input)
    delete d->input;
  d->input = newBuffer;
}

QBufferedSocket::Buffer* QBufferedSocket::outputBuffer() const
{
  return d->output;
}

void QBufferedSocket::setOutputBuffer(Buffer* newBuffer)
{
  QMutexLocker locker(mutex());
  if (newBuffer && d->output)
    newBuffer->setContents(d->output->contents());
  if (d->output)
    delete d->output;
  d->output = newBuffer;
}

Q_ULONG QBufferedSocket::bytesToWrite() const
{
  if (!d->output)
    return 0;

  QMutexLocker locker(mutex());
  return d->output->contents().size();
}

void QBufferedSocket::closeNow()
{
  QStreamSocket::close();
}

bool QBufferedSocket::canReadLine() const
{
  if (!d->input)
    return false;

  QMutexLocker locker(mutex());
  QByteArray buffer = d->input->contents();
  int index = buffer.find('\n');
  if (index > 1 && buffer[index - 1] == '\r')
    return true;		// that's a newline

  return false;
}

QCString QBufferedSocket::readLine()
{
  QMutexLocker locker(mutex());
  if (!canReadLine())
    return QCString();

  QByteArray buffer = d->input->contents();
  int index = buffer.find('\n');
  QCString result(buffer.data(), index - 2);
  d->input->consumeBlock(0L, index);
  return result;
}

void QBufferedSocket::slotReadActivity()
{
  if (d->input && state() == Connected)
    {
      QMutexLocker locker(mutex());
      Q_LONG len = d->input->receive(socketDevice());
      if (len == -1)
	{
	  if (socketDevice()->error() != WouldBlock)
	    {
	      // nope, another error!
	      copyError();
	      closeNow();
	      return;
	    }
	}
      else if (len == 0)
	{
	  // remotely closed
	  resetError();
	  closeNow();
	  return;
	}
    }

  QStreamSocket::slotReadActivity();
}

void QBufferedSocket::slotWriteActivity()
{
  if (d->output && (state() == Connected || state() == Closing))
    {
      QMutexLocker locker(mutex());
      Q_LONG len = d->output->send(socketDevice());
      if (len == -1)
	{
	  if (socketDevice()->error() != WouldBlock)
	    {
	      // nope, another error!
	      copyError();
	      closeNow();
	      return;
	    }
	}
      else if (len == 0)
	{
	  // remotely closed
	  resetError();
	  closeNow();
	  return;
	}

      if (d->output->isEmpty())
	// deactivate the notifier until we have something to send
	socketDevice()->writeNotifier()->setEnabled(false);
    }

  if (state() != Closing)
    QStreamSocket::slotWriteActivity();
  else if (d->output && d->output->isEmpty())
    {
      QStreamSocket::close();	// finished sending data
    }
}

