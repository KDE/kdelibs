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
#include <time.h>
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


// #define DEBUG_GZIP

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

// Not really necessary anymore, now that this is a dynamically-loaded lib.
class KGzipFilter::KGzipFilterPrivate
{
public:
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

void KGzipFilter::init( int mode )
{
    d->zStream.next_in = Z_NULL;
    d->zStream.avail_in = 0;
    if ( mode == IO_ReadOnly )
    {
        int result = inflateInit2(&d->zStream, -MAX_WBITS); // windowBits is passed < 0 to suppress zlib header
        if ( result != Z_OK )
            kdDebug(7005) << "inflateInit returned " << result << endl;
        // No idea what to do with result :)
    } else if ( mode == IO_WriteOnly )
    {
        int result = deflateInit2(&d->zStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY); // same here
        if ( result != Z_OK )
            kdDebug(7005) << "deflateInit returned " << result << endl;
    } else {
        kdWarning(7005) << "KGzipFilter: Unsupported mode " << mode << ". Only IO_ReadOnly and IO_WriteOnly supported" << endl;
    }
    m_mode = mode;
    d->bCompressed = true;
    m_headerWritten = false;
}

void KGzipFilter::terminate()
{
    if ( m_mode == IO_ReadOnly )
    {
        int result = inflateEnd(&d->zStream);
        if ( result != Z_OK )
            kdDebug(7005) << "inflateEnd returned " << result << endl;
    } else if ( m_mode == IO_WriteOnly )
    {
        int result = deflateEnd(&d->zStream);
        if ( result != Z_OK )
            kdDebug(7005) << "deflateEnd returned " << result << endl;
    }
}


void KGzipFilter::reset()
{
    if ( m_mode == IO_ReadOnly )
    {
        int result = inflateReset(&d->zStream);
        if ( result != Z_OK )
            kdDebug(7005) << "inflateReset returned " << result << endl;
    } else if ( m_mode == IO_WriteOnly ) {
        int result = deflateReset(&d->zStream);
        if ( result != Z_OK )
            kdDebug(7005) << "deflateReset returned " << result << endl;
        m_headerWritten = false;
    }
}

bool KGzipFilter::readHeader()
{
#ifdef DEBUG_GZIP
    kdDebug(7005) << "KGzipFilter::readHeader avail=" << d->zStream.avail_in << endl;
#endif
    // Assume not compressed until we successfully decode the header
    d->bCompressed = false;
    // Assume the first block of data contains the whole header.
    // The right way is to build this as a big state machine which
    // is a pain in the ass.
    // With 8K-blocks, we don't risk much anyway.
    Bytef *p = d->zStream.next_in;
    int i = d->zStream.avail_in;
    if ((i -= 10)  < 0) return false; // Need at least 10 bytes
#ifdef DEBUG_GZIP
    kdDebug(7005) << "KGzipFilter::readHeader first byte is " << QString::number(*p,16) << endl;
#endif
    if (*p++ != 0x1f) return false; // GZip magic
#ifdef DEBUG_GZIP
    kdDebug(7005) << "KGzipFilter::readHeader second byte is " << QString::number(*p,16) << endl;
#endif
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
#ifdef DEBUG_GZIP
        kdDebug(7005) << "ORIG_NAME=" << p << endl;
#endif
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
    if ((flags & HEAD_CRC) != 0) // skip the header crc
    {
        if ((i-=2) < 0) return false;
        p += 2;
    }

    d->zStream.avail_in = i;
    d->zStream.next_in = p;
    d->bCompressed = true;
#ifdef DEBUG_GZIP
    kdDebug(7005) << "header OK" << endl;
#endif
    return true;
}

/* Output a 16 bit value, lsb first */
#define put_short(w) \
    *p++ = (uchar) ((w) & 0xff); \
    *p++ = (uchar) ((ushort)(w) >> 8);

/* Output a 32 bit value to the bit stream, lsb first */
#define put_long(n) \
    put_short((n) & 0xffff); \
    put_short(((ulong)(n)) >> 16);

bool KGzipFilter::writeHeader( const QCString & fileName )
{
    Bytef *p = d->zStream.next_out;
    int i = d->zStream.avail_out;
    *p++ = 0x1f;
    *p++ = 0x8b;
    *p++ = Z_DEFLATED;
    *p++ = ORIG_NAME;
    put_long( time( 0L ) ); // Modification time (in unix format)
    *p++ = 0; // Extra flags (2=max compress, 4=fastest compress)
    *p++ = 3; // Unix

    for ( uint j = 0 ; j < fileName.length() ; ++j )
        *p++ = fileName[j];
    *p++ = 0;
    int headerSize = p - d->zStream.next_out;
    i -= headerSize;
    Q_ASSERT(i>0);
    m_crc = crc32(0L, Z_NULL, 0);
    d->zStream.next_out = p;
    d->zStream.avail_out = i;
    m_headerWritten = true;
    return true;
}

void KGzipFilter::writeFooter()
{
    Q_ASSERT( m_headerWritten );
    if (!m_headerWritten) kdDebug() << kdBacktrace();
    Bytef *p = d->zStream.next_out;
    int i = d->zStream.avail_out;
    //kdDebug(7005) << "KGzipFilter::writeFooter writing CRC= " << QString::number( m_crc, 16 ) << endl;
    put_long( m_crc );
    //kdDebug(7005) << "KGzipFilter::writing writing totalin= " << d->zStream.total_in << endl;
    put_long( d->zStream.total_in );
    i -= p - d->zStream.next_out;
    d->zStream.next_out = p;
    d->zStream.avail_out = i;
}

void KGzipFilter::setOutBuffer( char * data, uint maxlen )
{
    d->zStream.avail_out = maxlen;
    d->zStream.next_out = (Bytef *) data;
}
void KGzipFilter::setInBuffer( const char * data, uint size )
{
#ifdef DEBUG_GZIP
    kdDebug(7005) << "KGzipFilter::setInBuffer avail_in=" << size << endl;
#endif
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
    Q_ASSERT ( m_mode == IO_ReadOnly );
    if ( d->bCompressed )
    {
#ifdef DEBUG_GZIP
        kdDebug(7005) << "Calling inflate with avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable() << endl;
        kdDebug(7005) << "    next_in=" << d->zStream.next_in << endl;
#endif
        int result = inflate(&d->zStream, Z_SYNC_FLUSH);
#ifdef DEBUG_GZIP
        kdDebug(7005) << " -> inflate returned " << result << endl;
        kdDebug(7005) << "Now avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable() << endl;
        kdDebug(7005) << "    next_in=" << d->zStream.next_in << endl;
#else
        if ( result != Z_OK && result != Z_STREAM_END )
            kdDebug(7005) << "Warning: inflate() returned " << result << endl;
#endif
        return ( result == Z_OK ? OK : ( result == Z_STREAM_END ? END : ERROR ) );
    } else
        return uncompress_noop();
}

KGzipFilter::Result KGzipFilter::compress( bool finish )
{
    Q_ASSERT ( d->bCompressed );
    Q_ASSERT ( m_mode == IO_WriteOnly );

    Bytef* p = d->zStream.next_in;
    ulong len = d->zStream.avail_in;
#ifdef DEBUG_GZIP
    kdDebug(7005) << "  calling deflate with avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable() << endl;
#endif
    int result = deflate(&d->zStream, finish ? Z_FINISH : Z_NO_FLUSH);
    if ( result != Z_OK && result != Z_STREAM_END )
        kdDebug(7005) << "  deflate returned " << result << endl;
    if ( m_headerWritten )
    {
        //kdDebug(7005) << "Computing CRC for the next " << len - d->zStream.avail_in << " bytes" << endl;
        m_crc = crc32(m_crc, p, len - d->zStream.avail_in);
    }
    if ( result == Z_STREAM_END && m_headerWritten )
    {
        //kdDebug(7005) << "KGzipFilter::compress finished, write footer" << endl;
        writeFooter();
    }
    return ( result == Z_OK ? OK : ( result == Z_STREAM_END ? END : ERROR ) );
}
