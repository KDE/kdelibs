/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kgzipfilter.h"
#include <zlib.h>
#include <kdebug.h>
#include <klibloader.h>

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */

class KGzipFilterFactory : public KLibFactory
{
public:
    KGzipFilterFactory() : KLibFactory() {}
    ~KGzipFilterFactory(){}
    QObject *createObject( QObject *, const char *, const char*, const QStringList & )
    {
        return new KGzipFilter;
    }
};

extern "C" {
    void *init_kgzipfilter()
    {
        return new KGzipFilterFactory;
    }
}

class KGzipFilter::KGzipFilterPrivate
{
public:
    // The reason for this stuff here is to avoid including zlib.h in kgzipdev.h
    z_stream zStream;
    bool bCompressed;
};

KGzipFilter::KGzipFilter()
{
    d = new KGzipFilterPrivate;
    d->zStream.zalloc = (alloc_func)0;
    d->zStream.zfree = (free_func)0;
    d->zStream.opaque = (voidpf)0;
}


KGzipFilter::~KGzipFilter()
{
    delete d;
}

void KGzipFilter::init()
{
    // warning this may be read-only specific
    d->zStream.next_in = Z_NULL;
    d->zStream.avail_in = 0;
    /*outputBuffer.resize( 8*1024 );// Start with a modest buffer
      zStream.avail_out = outputBuffer.size();
      zStream.next_out = (Bytef *) outputBuffer.data();*/

    int result = inflateInit2(&d->zStream, -MAX_WBITS);
    kdDebug() << "inflateInit returned " << result << endl;
    // Not idea what to do with result :)
    d->bCompressed = true;
}

void KGzipFilter::terminate()
{
    // readonly specific !
    int result = inflateEnd(&d->zStream);
    kdDebug() << "inflateEnd returned " << result << endl;
}


void KGzipFilter::reset()
{
    int result = inflateReset(&d->zStream);
    kdDebug() << "inflateReset returned " << result << endl;
}

bool KGzipFilter::readHeader()
{
    kdDebug() << "KGzipFilter::readHeader" << endl;
    // Assume not compressed until we successfully decode the header
    d->bCompressed = false;
    // Assume the first block of data contains the whole header.
    // The right way is to build this as a big state machine which
    // is a pain in the ass.
    // With 8K-blocks, we don't risk much anyway.
    Bytef *p = d->zStream.next_in;
    int i = d->zStream.avail_in;
    if ((i -= 10)  < 0) return false; // Need at least 10 bytes
    if (*p++ != 0x1f) return false; // GZip magic
    if (*p++ != 0x8b) return false;
    int method = *p++;
    int flags = *p++;
    if ((method != Z_DEFLATED) || (flags & RESERVED) != 0) return false;
    p += 6;
    if ((flags & EXTRA_FIELD) != 0) // skip extra field
    {
        if ((i -= 2) < 0) return false; // Need at least 2 bytes
        int len = *p++;
        len += (*p++) << 8;
        if ((i -= len) < 0) return false; // Need at least len bytes
        p += len;
    }
    if ((flags & ORIG_NAME) != 0) // skip original file name
    {
        while( (i > 0) && (*p))
        {
            i--; p++;
        }
        if (--i <= 0) return false;
        p++;
    }
    if ((flags & COMMENT) != 0) // skip comment
    {
        while( (i > 0) && (*p))
        {
            i--; p++;
        }
        if (--i <= 0) return false;
        p++;
    }
    if ((flags & HEAD_CRC) != 0) // skipthe header crc
    {
        if ((i-=2) < 0) return false;
        p += 2;
    }

    d->zStream.avail_in = i;
    d->zStream.next_in = p;
    d->bCompressed = true;
    kdDebug() << "header OK" << endl;
    return true;
}

void KGzipFilter::setOutBuffer( char * data, uint maxlen )
{
    d->zStream.avail_out = maxlen;
    d->zStream.next_out = (Bytef *) data;
}
void KGzipFilter::setInBuffer( char * data, uint size )
{
    d->zStream.avail_in = size;
    d->zStream.next_in = (Bytef*) data;
}
int KGzipFilter::inBufferAvailable() const
{
    return d->zStream.avail_in;
}
int KGzipFilter::outBufferAvailable() const
{
    return d->zStream.avail_out;
}

KGzipFilter::Result KGzipFilter::uncompress_noop()
{
    // I'm not sure we really need support for that (uncompressed streams),
    // but why not, it can't hurt to have it. One case I can think of is someone
    // naming a tar file "blah.tar.gz" :-)
    if ( d->zStream.avail_in > 0 )
    {
        int n = (d->zStream.avail_in < d->zStream.avail_out) ? d->zStream.avail_in : d->zStream.avail_out;
        memcpy( d->zStream.next_out, d->zStream.next_in, n );
        d->zStream.avail_out -= n;
        d->zStream.next_in += n;
        d->zStream.avail_in -= n;
        return OK;
    } else
        return END;
}

KGzipFilter::Result KGzipFilter::uncompress()
{
    if ( d->bCompressed )
    {
        //kdDebug() << "Calling inflate with avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable() << endl;
        int result = inflate(&d->zStream, Z_SYNC_FLUSH);
        //kdDebug() << "inflate returned " << result << endl;
        return ( result == Z_OK ? OK : ( result == Z_STREAM_END ? END : ERROR ) );
    } else
        return uncompress_noop();
}
