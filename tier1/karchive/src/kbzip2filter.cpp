/* This file is part of the KDE libraries
   Copyright (C) 2000-2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbzip2filter.h"

#include <config-compression.h>

#if HAVE_BZIP2_SUPPORT

// we don't need that
#define BZ_NO_STDIO
extern "C" {
	#include <bzlib.h>
}

#if NEED_BZ2_PREFIX
        #define bzDecompressInit(x,y,z) BZ2_bzDecompressInit(x,y,z)
        #define bzDecompressEnd(x) BZ2_bzDecompressEnd(x)
        #define bzCompressEnd(x)  BZ2_bzCompressEnd(x)
        #define bzDecompress(x) BZ2_bzDecompress(x)
        #define bzCompress(x,y) BZ2_bzCompress(x, y)
        #define bzCompressInit(x,y,z,a) BZ2_bzCompressInit(x, y, z, a);
#endif

#include <QDebug>

#include <qiodevice.h>



// For docu on this, see /usr/doc/bzip2-0.9.5d/bzip2-0.9.5d/manual_3.html

class KBzip2Filter::Private
{
public:
    Private()
    : isInitialized(false)
    {
        memset(&zStream, 0, sizeof(zStream));
        mode = 0;
    }

    bz_stream zStream;
    int mode;
    bool isInitialized;
};

KBzip2Filter::KBzip2Filter()
    :d(new Private)
{
}


KBzip2Filter::~KBzip2Filter()
{
    delete d;
}

bool KBzip2Filter::init( int mode )
{
    if (d->isInitialized) {
        terminate();
    }

    d->zStream.next_in = 0;
    d->zStream.avail_in = 0;
    if ( mode == QIODevice::ReadOnly )
    {
        const int result = bzDecompressInit(&d->zStream, 0, 0);
        if (result != BZ_OK) {
            //qDebug() << "bzDecompressInit returned " << result;
            return false;
        }
    } else if ( mode == QIODevice::WriteOnly ) {
        const int result = bzCompressInit(&d->zStream, 5, 0, 0);
        if (result != BZ_OK) {
            //qDebug() << "bzDecompressInit returned " << result;
            return false;
        }
    } else {
        //qWarning() << "Unsupported mode " << mode << ". Only QIODevice::ReadOnly and QIODevice::WriteOnly supported";
        return false;
    }
    d->mode = mode;
    d->isInitialized = true;
    return true;
}

int KBzip2Filter::mode() const
{
    return d->mode;
}

bool KBzip2Filter::terminate()
{
    if (d->mode == QIODevice::ReadOnly) {
        const int result = bzDecompressEnd(&d->zStream);
        if (result != BZ_OK) {
            //qDebug() << "bzDecompressEnd returned " << result;
            return false;
        }
    } else if (d->mode == QIODevice::WriteOnly) {
        const int result = bzCompressEnd(&d->zStream);
        if (result != BZ_OK) {
            //qDebug() << "bzCompressEnd returned " << result;
            return false;
        }
    } else {
        //qWarning() << "Unsupported mode " << d->mode << ". Only QIODevice::ReadOnly and QIODevice::WriteOnly supported";
        return false;
    }
    d->isInitialized = false;
    return true;
}


void KBzip2Filter::reset()
{
    // bzip2 doesn't seem to have a reset call...
    terminate();
    init( d->mode );
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
    //qDebug() << "Calling bzDecompress with avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable();
    int result = bzDecompress(&d->zStream);
    if ( result < BZ_OK )
    {
        qWarning() << "bzDecompress returned" << result;
    }

    switch (result) {
        case BZ_OK:
                return KFilterBase::Ok;
        case BZ_STREAM_END:
                return KFilterBase::End;
        default:
                return KFilterBase::Error;
    }
}

KBzip2Filter::Result KBzip2Filter::compress( bool finish )
{
    //qDebug() << "Calling bzCompress with avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable();
    int result = bzCompress(&d->zStream, finish ? BZ_FINISH : BZ_RUN );

    switch (result) {
        case BZ_OK:
        case BZ_FLUSH_OK:
        case BZ_RUN_OK:
        case BZ_FINISH_OK:
                return KFilterBase::Ok;
                break;
        case BZ_STREAM_END:
                //qDebug() << "  bzCompress returned " << result;
                return KFilterBase::End;
		break;
        default:
                //qDebug() << "  bzCompress returned " << result;
                return KFilterBase::Error;
                break;
    }
}

#endif  /* HAVE_BZIP2_SUPPORT */
