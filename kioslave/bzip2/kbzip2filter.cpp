/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   $Id: kbzip2filter.cpp 465272 2005-09-29 09:47:40Z mueller $

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config.h>

#if defined( HAVE_BZIP2_SUPPORT )

// we don't need that
#define BZ_NO_STDIO
extern "C" {
	#include <bzlib.h>
}

#ifdef NEED_BZ2_PREFIX
        #define bzDecompressInit(x,y,z) BZ2_bzDecompressInit(x,y,z)
        #define bzDecompressEnd(x) BZ2_bzDecompressEnd(x)
        #define bzCompressEnd(x)  BZ2_bzCompressEnd(x)
        #define bzDecompress(x) BZ2_bzDecompress(x)
        #define bzCompress(x,y) BZ2_bzCompress(x, y)
        #define bzCompressInit(x,y,z,a) BZ2_bzCompressInit(x, y, z, a);
#endif

#include <kdebug.h>
#include <klibloader.h>

#include "kbzip2filter.h"

// For docu on this, see /usr/doc/bzip2-0.9.5d/bzip2-0.9.5d/manual_3.html

class KBzip2FilterFactory : public KLibFactory
{
public:
    KBzip2FilterFactory() : KLibFactory() {}
    virtual ~KBzip2FilterFactory(){}
    QObject *createObject( QObject *, const char *, const char*, const QStringList & )
    {
        return new KBzip2Filter;
    }
};

K_EXPORT_COMPONENT_FACTORY( kbzip2filter, KBzip2FilterFactory )

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
        (void)bzDecompressInit(&d->zStream, 0, 0);
        //kdDebug(7118) << "bzDecompressInit returned " << result << endl;
        // No idea what to do with result :)
    } else if ( mode == IO_WriteOnly ) {
        (void)bzCompressInit(&d->zStream, 5, 0, 0);
        //kdDebug(7118) << "bzDecompressInit returned " << result << endl;
    } else
        kdWarning(7118) << "Unsupported mode " << mode << ". Only IO_ReadOnly and IO_WriteOnly supported" << endl;
    m_mode = mode;
}

void KBzip2Filter::terminate()
{
    if ( m_mode == IO_ReadOnly )
    {
        int result = bzDecompressEnd(&d->zStream);
        kdDebug(7118) << "bzDecompressEnd returned " << result << endl;
    } else if ( m_mode == IO_WriteOnly )
    {
        int result = bzCompressEnd(&d->zStream);
        kdDebug(7118) << "bzCompressEnd returned " << result << endl;
    } else
        kdWarning(7118) << "Unsupported mode " << m_mode << ". Only IO_ReadOnly and IO_WriteOnly supported" << endl;
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

void KBzip2Filter::setInBuffer( const char *data, unsigned int size )
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
    int result = bzDecompress(&d->zStream);
    if ( result != BZ_OK )
    {
        kdDebug(7118) << "bzDecompress returned " << result << endl;
        kdDebug(7118) << "KBzip2Filter::uncompress " << ( result == BZ_OK ? OK : ( result == BZ_STREAM_END ? END : ERROR ) ) << endl;
    }

    switch (result) {
        case BZ_OK:
                return OK;
        case BZ_STREAM_END:
                return END;
        default:
                return ERROR;
    }
}

KBzip2Filter::Result KBzip2Filter::compress( bool finish )
{
    //kdDebug(7118) << "Calling bzCompress with avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable() << endl;
    int result = bzCompress(&d->zStream, finish ? BZ_FINISH : BZ_RUN );

    switch (result) {
        case BZ_OK:
        case BZ_FLUSH_OK:
        case BZ_RUN_OK:
        case BZ_FINISH_OK:
                return OK;
                break;
        case BZ_STREAM_END:
                kdDebug(7118) << "  bzCompress returned " << result << endl;
                return END;
		break;
        default:
                kdDebug(7118) << "  bzCompress returned " << result << endl;
                return ERROR;
                break;
    }
}

#endif
