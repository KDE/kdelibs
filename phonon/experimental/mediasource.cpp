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

#include "mediasource.h"
#include "mediasource_p.h"

#define S_D(Class) Class##Private *d = reinterpret_cast<Class##Private *>(Phonon::MediaSource::d.data())

namespace Phonon
{
namespace Experimental
{

MediaSource::MediaSource(const MediaSource &rhs)
    : Phonon::MediaSource(rhs)
{
}

MediaSource &MediaSource::operator=(const MediaSource &rhs)
{
    d = rhs.d;
    return *this;
}

bool MediaSource::operator==(const MediaSource &rhs) const
{
    return d == rhs.d;
}

VideoCaptureDevice MediaSource::videoCaptureDevice() const
{
    S_D(const MediaSource);
    return d->videoCaptureDevice;
}

MediaSource::MediaSource(const VideoCaptureDevice &videoDevice)
    : Phonon::MediaSource(*new MediaSourcePrivate(VideoCaptureDeviceSource))
{
    S_D(MediaSource);
    d->videoCaptureDevice = videoDevice;
}

MediaSource::MediaSource(const QList<Phonon::MediaSource> &mediaList)
    : Phonon::MediaSource(*new MediaSourcePrivate(Link))
{
    d->linkedSources = mediaList;
    foreach (const Phonon::MediaSource &ms, mediaList) {
        Q_ASSERT(static_cast<MediaSource::Type>(ms.type()) != Link);
    }
}

QList<Phonon::MediaSource> MediaSource::substreams() const
{
    return d->linkedSources;
}

} // namespace Experimental
} // namespace Phonon
