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

#include "config.h"

#include <string.h>

#include <qptrlist.h>
#include <qcstring.h>
#include "kbufferedio.h"

/*
 * The KBufferedIO class has two purposes: first, it defines an API on how
 * that classes providing buffered I/O should provide. Next, it implements on
 * top of that API a generic buffering, that should suffice for most cases.
 *
 * The buffering implemented consists of two separate buffer areas, one for
 * the input (or read) buffer, and one for the output (or write) buffer. Each
 * of those buffers is implemented through a QList of QByteArrays instead of
 * simply QByteArrays. The idea is that, instead of having one large, contiguous
 * buffer area, we have several small ones. Even though this could be seen as
 * a waste of memory, it makes our life easier, because we can just append a new
 * QByteArray to the list and not have to worry with copying the rest of the
 * buffer, should we need to expand.
 *
 * This way, we have the capability of unlimited buffering, which can grow to
 * the extent of available memory.
 *
 * For each buffer, we provide three kinds of functions, available as protected
 * members: consume, feed and size. The size functions calculate the current
 * size of the buffer, by adding each individual QByteArray size. The feed
 * functions are used by the I/O functions that receive data from somewhere,
 * i.e., from the system, in the case of the input buffer, and from the user,
 * in the case of the output buffer. These two functions are used to give
 * the buffers more data. And the consume functions are used by the functions
 * that send out data (to the system, for the write buffer, and to the user,
 * for the read buffer).
 *
 * Note that for your own implementation, you can have your readBlock function
 * merely call consumeReadBuffer, similarly to peekBlock. As for
 * the writeBlock function, you'd call feedWriteBuffer.
 *
 * Now, the function receiving data from the system will need to simply call
 * feedReadBuffer, much in the same way of unreadBlock. The tricky part is
 * for the output function. We do not provide a member function that copies
 * data from the output buffer into another buffer for sending. We believe that
 * would be a waste of resources and CPU time, since you'd have to allocate
 * that buffer, copy data into it and then call the OS, which will likely just
 * copy data out of it.
 *
 * Instead, we found it better to leave it to you to access outBuf member
 * variable directly and use the buffers there. Should you want to copy that
 * into a larger buffer before sending, that's up to you.
 *
 * Both buffers work in the same way: they're an "array" of buffers, each
 * concatenated to the other. All data in all buffers is valid data, except
 * for the first QByteArray, whose valid data starts at inBufIndex/outBufIndex
 * bytes from the start. That is, the data starts in the first QByteArray buffer
 * that many bytes from the start and goes on contiguously until the last
 * QByteArray. This has been decided like that because we didn't want to
 * create a new QByteArray of the remaining bytes in the first buffer, after
 * a consume operation, because that could take some time. It is faster
 * this way, although not really easy.
 *
 * If you want to take a look at an implementation of a buffered I/O class,
 * refer to KExtendedSocket's source code.
 */

// constructor
KBufferedIO::KBufferedIO() :
  inBufIndex(0), outBufIndex(0)
{
}

// destructor
KBufferedIO::~KBufferedIO()
{
}

// sets the buffer sizes
// this implementation doesn't support setting the buffer sizes
// if any parameter is different than -1 or -2, fail
bool KBufferedIO::setBufferSize(int rsize, int wsize /* = -2 */)
{
  if (wsize != -2 && wsize != -1)
    return false;
  if (rsize != -2 && rsize != -1)
    return false;

  return true;
}

int KBufferedIO::bytesAvailable() const
{
  return readBufferSize();
}

int KBufferedIO::bytesToWrite() const
{
  return writeBufferSize();
}

// This function will scan the read buffer for a '\n'
bool KBufferedIO::canReadLine() const
{
  if (bytesAvailable() == 0)
    return false;		// no new line in here

  QByteArray* buf;

  // scan each QByteArray for the occurence of '\n'
  QPtrList<QByteArray> &buflist = ((KBufferedIO*)this)->inBuf;
  buf = buflist.first();
  char *p = buf->data() + inBufIndex;
  while (buf != NULL)
    {
      int n = buf->size();
      while (n--)
	if (*p++ == '\n')
	  return true;
      buf = buflist.next();
      if (buf != NULL)
	p = buf->data();
    }

  return false;			// no new line found
}

// unreads the current data
// that is, writes into the read buffer, at the beginning
int KBufferedIO::unreadBlock(const char *data, uint len)
{
  return feedReadBuffer(len, data, true);
};

//
// protected member functions
//

unsigned KBufferedIO::consumeReadBuffer(unsigned nbytes, char *destbuffer, bool discard)
{
  {
    register unsigned u = readBufferSize();
    if (nbytes > u)
      nbytes = u;		// we can't consume more than there is
  }

  QByteArray *buf;
  unsigned copied = 0;

  buf = inBuf.first();
  while (nbytes && buf)
    {
      // should we copy it all?
      unsigned to_copy = buf->size() - inBufIndex;
      if (to_copy > nbytes)
	to_copy = nbytes;

      if (destbuffer)
	memcpy(destbuffer + copied, buf->data() + inBufIndex, to_copy);
      nbytes -= to_copy;
      copied += to_copy;

      if (discard)
	{
	  // discard the contents
	  if (buf->size() - inBufIndex > to_copy)
	    {
	      inBufIndex += to_copy;
	      break;		// we aren't copying everything, that means that's
				// all the user wants
	    }
	  else
	    {
	      inBuf.remove();
	      inBufIndex = 0;
	      buf = inBuf.first();
	    }
	}
    }

  return copied;
}

void KBufferedIO::consumeWriteBuffer(unsigned nbytes)
{
  QByteArray *buf = outBuf.first();
  if (buf == NULL)
    return;			// nothing to consume

  if (nbytes < buf->size() - outBufIndex)
    // we want to consume less than there is in the first buffer
    outBufIndex += nbytes;
  else
    {
      nbytes -= buf->size() - outBufIndex;
      outBufIndex = 0;
      outBuf.remove();

      while ((buf = outBuf.next()) != NULL)
	if (buf->size() <= nbytes)
	  {
	    nbytes -= buf->size();
	    outBuf.remove();
	  }
	else
	  {
	    outBufIndex = nbytes;
	    break;
	  }
    }
}

unsigned KBufferedIO::feedReadBuffer(unsigned nbytes, const char *buffer, bool atBeginning)
{
  if (nbytes == 0)
    return 0;

  QByteArray *a = new QByteArray(nbytes);
  a->duplicate(buffer, nbytes);

  if (atBeginning)
    inBuf.prepend(a);
  else
    inBuf.append(a);

  return nbytes;
}

unsigned KBufferedIO::feedWriteBuffer(unsigned nbytes, const char *buffer)
{
  if (nbytes == 0)
    return 0;

  QByteArray *a = new QByteArray(nbytes);
  a->duplicate(buffer, nbytes);
  outBuf.append(a);
  return nbytes;
}

unsigned KBufferedIO::readBufferSize() const
{
  unsigned count = 0;
  QByteArray *buf = ((KBufferedIO*)this)->inBuf.first();
  while (buf != NULL)
    {
      count += buf->size();
      buf = ((KBufferedIO*)this)->inBuf.next();
    }

  return count - inBufIndex;
}

unsigned KBufferedIO::writeBufferSize() const
{
  unsigned count = 0;
  QByteArray *buf = ((KBufferedIO*)this)->outBuf.first();
  while (buf != NULL)
    {
      count += buf->size();
      buf = (const_cast<KBufferedIO*>(this))->outBuf.next();
    }

  return count - outBufIndex;
}

void KBufferedIO::virtual_hook( int id, void* data )
{ KAsyncIO::virtual_hook( id, data ); }

#include "kbufferedio.moc"
