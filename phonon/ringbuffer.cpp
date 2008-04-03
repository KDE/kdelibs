/*  This file is part of the KDE project
    Copyright (C) 2008 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "ringbuffer.h"
#include <QtCore/QSharedMemory>

struct RingBufferBasePrivate
{
    QAtomicInt readPosition;
    QAtomicInt writePosition;
    QAtomicInt size;
};

RingBufferBase::RingBufferBase()
    : d(new RingBufferBasePrivate)
{
}

RingBufferBase::~RingBufferBase()
{
    delete d;
}

int RingBufferBase::_writePosition()
{
    return d->writePosition.fetchAndAddAcquire(1);
}

void RingBufferBase::_incSize()
{
    d->size.ref();
}

int RingBufferBase::_readPosition()
{
    return d->readPosition.fetchAndAddRelaxed(1);
}

bool RingBufferBase::_canRead()
{
    const int s = d->size.fetchAndAddRelaxed(-1);
    if (s > 0) {
        return true;
    }
    d->size.fetchAndAddRelaxed(1);
    return false;
}

int RingBufferBase::_size() const
{
    return d->size;
}

struct SharedMemoryRingBufferBasePrivate
{
    QSharedMemory sharedMemory;
    RingBufferBase d;
};

SharedMemoryRingBufferBase::SharedMemoryRingBufferBase(int sizeOfArray, const QString &key)
    : d2(new SharedMemoryRingBufferBasePrivate)
{
    d2->sharedMemory.setKey(key);
    if (!d2->sharedMemory.attach()) {
        if (!d2->sharedMemory.create(sizeOfArray + sizeof(RingBufferBasePrivate))) {
            qFatal("failed to attach to or create shared memory");
        }
        delete d;
        d = reinterpret_cast<RingBufferBasePrivate *>(reinterpret_cast<char *>(d2->sharedMemory.data()) + sizeOfArray);
        d->readPosition = 0;
        d->writePosition = 0;
        d->size = 0;
    }
}

SharedMemoryRingBufferBase::~SharedMemoryRingBufferBase()
{
    d = 0;
    delete d2;
}

void *SharedMemoryRingBufferBase::_dataMemory() const
{
    return d2->sharedMemory.data();
}
