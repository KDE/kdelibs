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
#include <bzlib.h>
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

class KBzip2Filter::KBzip2FilterPrivate
{
public:
    // The reason for this stuff here is to avoid including zlib.h in kbzip2dev.h
    bz_stream zStream;
};

KBzip2Filter::KBzip2Filter()
{
    d = new KBzip2FilterPrivate;
    d->zStream.bzalloc = 0;
    d->zStream.bzfree = 0;
    d->zStream.opaque = 0;
}


KBzip2Filter::~KBzip2Filter()
{
    delete d;
}

void KBzip2Filter::init()
{
    // warning this may be read-only specific
    d->zStream.next_in = 0;
    d->zStream.avail_in = 0;
    /*outputBuffer.resize( 8*1024 );// Start with a modest buffer
      zStream.avail_out = outputBuffer.size();
      zStream.next_out = outputBuffer.data();*/

#ifdef NEED_BZ2_PREFIX
    int result = BZ2_bzDecompressInit(&d->zStream, 0, 0);
#else
    int result = bzDecompressInit(&d->zStream, 0, 0);
#endif
    kdDebug() << "bzDecompressInit returned " << result << endl;
    // Not idea what to do with result :)
}

void KBzip2Filter::terminate()
{
    // readonly specific !
#ifdef NEED_BZ2_PREFIX
    int result = BZ2_bzDecompressEnd(&d->zStream);
#else
    int result = bzDecompressEnd(&d->zStream);
#endif
    kdDebug() << "bzDecompressEnd returned " << result << endl;
}


void KBzip2Filter::reset()
{
    // bzip2 doesn't seem to have a reset call...
    terminate();
    init();
}

bool KBzip2Filter::readHeader()
{
    return true;
}

void KBzip2Filter::setOutBuffer( char * data, uint maxlen )
{
    d->zStream.avail_out = maxlen;
    d->zStream.next_out = data;
}
void KBzip2Filter::setInBuffer( char * data, uint size )
{
    d->zStream.avail_in = size;
    d->zStream.next_in = data;
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
    //kdDebug() << "Calling bzDecompress with avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable() << endl;
#ifdef NEED_BZ2_PREFIX
    int result = BZ2_bzDecompress(&d->zStream);
#else
    int result = bzDecompress(&d->zStream);
#endif
    if ( result != BZ_OK )
    {
        kdDebug() << "bzDecompress returned " << result << endl;
        kdDebug() << "KBzip2Filter::uncompress" << ( result == BZ_OK ? OK : ( result == BZ_STREAM_END ? END : ERROR ) ) << endl;
    }
    return ( result == BZ_OK ? OK : ( result == BZ_STREAM_END ? END : ERROR ) );
}
