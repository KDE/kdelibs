/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

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

#include "streaminterface.h"
#include "streaminterface_p.h"
#include "abstractmediastream.h"
#include "abstractmediastream_p.h"
#include "mediasource.h"

namespace Phonon
{

StreamInterface::StreamInterface()
    : d(new StreamInterfacePrivate)
{
    d->q = this;
}

StreamInterface::~StreamInterface()
{
    if (d->connected) {
        d->mediaStream->d_func()->setStreamInterface(0);
    }
    delete d;
}

void StreamInterface::connectToSource(const MediaSource &mediaSource)
{
    Q_ASSERT(!d->connected);
    d->connected = true;
    d->mediaStream = mediaSource.stream();
    mediaSource.stream()->d_func()->setStreamInterface(this);
}

void StreamInterfacePrivate::disconnectMediaStream()
{
    Q_ASSERT(connected);
    connected = false;
    mediaStream = 0;
    q->endOfData();
    q->setStreamSeekable(false);
}

void StreamInterface::needData()
{
    Q_ASSERT(d->connected);
    if (d->mediaStream) {
        d->mediaStream->needData();
    }
}

void StreamInterface::enoughData()
{
    Q_ASSERT(d->connected);
    if (d->mediaStream) {
        d->mediaStream->enoughData();
    }
}

void StreamInterface::seekStream(qint64 offset)
{
    Q_ASSERT(d->connected);
    if (d->mediaStream) {
        d->mediaStream->seekStream(offset);
    }
}

} // namespace Phonon

// vim: sw=4 sts=4 et tw=100
