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

#include "kbzip2filter.h"
#define BZ_NO_STDIO // we don't need that
extern "C" {
#include <bzlib.h>
}

#include <kdebug.h>
#include <klibloader.h>

#include <config.h>

// For docu on this, see /usr/doc/bzip2-0.9.5d/bzip2-0.9.5d/manual_3.html

class KBzip2FilterFactory : public KLibFactory
{
public:
    KBzip2FilterFactory() : KLibFactory() {}
    ~KBzip2FilterFactory(){}
    QObject *createObject( QObject *, const char *, const char*, const QStringList & )
    {
        return new KBzip2Filter;
    }
};

extern "C" {
    void *init_kbzip2filter()
    {
        return new KBzip2FilterFactory;
    }
}

// Not really useful anymore
class KBzip2Filter::KBzip2FilterPrivate
{
public:
    bz_stream zStream;
};

KBzip2Filter::KBzip2Filter()
{
    d = new KBzip2FilterPrivate;
    d->zStream.bzalloc = 0;
    d->zStream.bzfree = 0;
    d->zStream.opaque = 0;
    m_mode = 0;
}


KBzip2Filter::~KBzip2Filter()
{
    delete d;
}

void KBzip2Filter::init( int mode )
{
    d->zStream.next_in = 0;
    d->zStream.avail_in = 0;
    if ( mode == IO_ReadOnly )
    {
#ifdef NEED_BZ2_PREFIX
        int result = BZ2_bzDecompressInit(&d->zStream, 0, 0);
#else
        int result = bzDecompressInit(&d->zStream, 0, 0);
#endif
        //kdDebug(7118) << "bzDecompressInit returned " << result << endl;
        // No idea what to do with result :)
    } else if ( mode == IO_WriteOnly )
    {
#ifdef NEED_BZ2_PREFIX
        int result = BZ2_bzCompressInit(&d->zStream, 5, 0, 0);
#else
        int result = bzCompressInit(&d->zStream, 5, 0, 0);
#endif
        //kdDebug(7118) << "bzDecompressInit returned " << result << endl;
    } else
        kdWarning() << "Unsupported mode " << mode << ". Only IO_ReadOnly and IO_WriteOnly supported" << endl;
    m_mode = mode;
}

void KBzip2Filter::terminate()
{
    if ( m_mode == IO_ReadOnly )
    {
#ifdef NEED_BZ2_PREFIX
        int result = BZ2_bzDecompressEnd(&d->zStream);
#else
        int result = bzDecompressEnd(&d->zStream);
#endif
        kdDebug(7118) << "bzDecompressEnd returned " << result << endl;
    } else if ( m_mode == IO_WriteOnly )
    {
#ifdef NEED_BZ2_PREFIX
        int result = BZ2_bzCompressEnd(&d->zStream);
#else
        int result = bzCompressEnd(&d->zStream);
#endif
        kdDebug(7118) << "bzCompressEnd returned " << result << endl;
    } else
        kdWarning() << "Unsupported mode " << m_mode << ". Only IO_ReadOnly and IO_WriteOnly supported" << endl;
}


void KBzip2Filter::reset()
{
    kdDebug(7118) << "KBzip2Filter::reset" << endl;
    // bzip2 doesn't seem to have a reset call...
    terminate();
    init( m_mode );
}

void KBzip2Filter::setOutBuffer( char * data, uint maxlen )
{
    d->zStream.avail_out = maxlen;
    d->zStream.next_out = data;
}
void KBzip2Filter::setInBuffer( const char * data, uint size )
{
    d->zStream.avail_in = size;
    d->zStream.next_in = const_cast<char *>(data);
}
int KBzip2Filter::inBufferAvailable() const
{
    return d->zStream.avail_in;
}
int KBzip2Filter::outBufferAvailable() const
{
    return d->zStream.avail_out;
}

KBzip2Filter::Result KBzip2Filter::uncompress()
{
    //kdDebug(7118) << "Calling bzDecompress with avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable() << endl;
#ifdef NEED_BZ2_PREFIX
    int result = BZ2_bzDecompress(&d->zStream);
#else
    int result = bzDecompress(&d->zStream);
#endif
    if ( result != BZ_OK )
    {
        kdDebug(7118) << "bzDecompress returned " << result << endl;
        kdDebug(7118) << "KBzip2Filter::uncompress " << ( result == BZ_OK ? OK : ( result == BZ_STREAM_END ? END : ERROR ) ) << endl;
    }
    return ( result == BZ_OK ? OK : ( result == BZ_STREAM_END ? END : ERROR ) );
}

KBzip2Filter::Result KBzip2Filter::compress( bool finish )
{
    //kdDebug(7118) << "Calling bzCompress with avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable() << endl;
#ifdef NEED_BZ2_PREFIX
    int result = BZ2_bzCompress(&d->zStream, finish ? BZ_FINISH : BZ_RUN );
#else
    int result = bzCompress(&d->zStream, finish ? BZ_FINISH : BZ_RUN );
#endif
    if ( result != BZ_OK )
        kdDebug(7118) << "  bzCompress returned " << result << endl;
    return ( (result == BZ_OK || result == BZ_FLUSH_OK || result == BZ_RUN_OK || result == BZ_FINISH_OK) ? OK : ( result == BZ_STREAM_END ? END : ERROR ) );
}
