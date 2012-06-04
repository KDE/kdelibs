/* This file is part of the KDE libraries
   Copyright (C) 2011 Mario Bensi <mbensi@ipsquad.net>

   Based on kbzip2filter:
   Copyright (C) 2000, 2009 David Faure <faure@kde.org>

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

#include "knonefilter.h"

#include <QtCore/QFile>

class KNoneFilter::Private
{
public:
    Private()
        : mode(0), outBufferSize(0), inBufferSize(0)
    {
    }

    int mode;
    int outBufferSize;
    int inBufferSize;
    QByteArray inBuffer;
    char* outBuffer;
};

KNoneFilter::KNoneFilter()
    : d(new Private)
{
}


KNoneFilter::~KNoneFilter()
{
    delete d;
}

bool KNoneFilter::init(int mode)
{
    if (mode == QIODevice::ReadOnly) {
        device()->open(QIODevice::ReadOnly);
    } else if (mode == QIODevice::WriteOnly) {
        device()->open(QIODevice::WriteOnly);
    } else {
        return false;
    }

    d->mode = mode;
    return true;
}

int KNoneFilter::mode() const
{
    return d->mode;
}

bool KNoneFilter::terminate()
{
    return true;
}


void KNoneFilter::reset()
{
}

bool KNoneFilter::readHeader()
{
    return true;
}

bool KNoneFilter::writeHeader( const QByteArray & /*fileName*/ )
{
    return true;
}

void KNoneFilter::setOutBuffer( char * data, uint maxlen )
{
    d->outBufferSize = maxlen;
    d->outBuffer = data;
}

void KNoneFilter::setInBuffer( const char * data, uint size )
{
    d->inBuffer.setRawData(data, size);
    d->inBufferSize = size;
}

int KNoneFilter::inBufferAvailable() const
{
    return 0;
}

int KNoneFilter::outBufferAvailable() const
{
    return d->outBufferSize - d->inBufferSize;
}

KNoneFilter::Result KNoneFilter::uncompress()
{
#ifndef NDEBUG
    if (d->mode == 0) {
        return KFilterBase::Error;
    } else if (d->mode == QIODevice::WriteOnly) {
        return KFilterBase::Error;
    }
    Q_ASSERT ( d->mode == QIODevice::ReadOnly );
#endif

    memcpy( d->outBuffer, d->inBuffer.data(), d->inBufferSize );
    d->outBuffer = d->inBuffer.data();
    return KFilterBase::End;
}

KNoneFilter::Result KNoneFilter::compress( bool finish )
{
    Q_ASSERT ( d->mode == QIODevice::WriteOnly );

    if (finish) {
        device()->close();
        return KFilterBase::End;
    } else {
        device()->write(d->inBuffer.data(), d->inBufferSize);
        return KFilterBase::Ok;
    }
}
