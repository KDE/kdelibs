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

#include "mediasource.h"
#include "mediasource_p.h"
#include "iodevicestream.h"
#include <QtCore/QFile>

namespace Phonon
{

MediaSource::MediaSource()
    : d(new MediaSourcePrivate(Invalid))
{
}

MediaSource::MediaSource(const QString &filename)
    : d(new MediaSourcePrivate(LocalFile))
{
    if (filename.size() > 0 && filename.startsWith(QLatin1Char(':'))) {
        // it's a Qt resource -> use QFile
        d->type = Stream;
        d->resourceFile = new QFile(filename);
        d->stream = new IODeviceStream(d->resourceFile, d->resourceFile);
    } else {
        d->url = QUrl::fromLocalFile(filename);
    }
}

MediaSource::MediaSource(const QUrl &url)
    : d(new MediaSourcePrivate(Url))
{
    d->url = url;
}

MediaSource::MediaSource(Phonon::DiscType dt, const QString &deviceName)
    : d(new MediaSourcePrivate(Disc))
{
    if (dt == NoDisc) {
        d->type = Invalid;
        return;
    }
    d->discType = dt;
    d->deviceName = deviceName;
}

MediaSource::MediaSource(AbstractMediaStream *stream)
    : d(new MediaSourcePrivate(Stream))
{
    d->stream = stream;
}

MediaSource::MediaSource(QIODevice *ioDevice)
    : d(new MediaSourcePrivate(Stream))
{
    d->stream = new IODeviceStream(ioDevice, ioDevice);
}

MediaSource::MediaSource(const QList<MediaSource> &mediaList)
    : d(new MediaSourcePrivate(Link))
{
    d->linkedSources = mediaList;
    foreach (MediaSource ms, mediaList) {
        Q_ASSERT(ms.type() != Link);
    }
}

QList<MediaSource> MediaSource::substreams() const
{
    return d->linkedSources;
}

MediaSource::~MediaSource()
{
    delete d->resourceFile;
    d->resourceFile = 0;
}

MediaSource::MediaSource(const MediaSource &rhs)
    : d(rhs.d)
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

MediaSource::Type MediaSource::type() const
{
    if (d->type == Stream && d->stream == 0) {
        return Invalid;
    }
    return d->type;
}

QString MediaSource::filename() const
{
    return d->url.toLocalFile();
}

QUrl MediaSource::url() const
{
    return d->url;
}

Phonon::DiscType MediaSource::discType() const
{
    return d->discType;
}

QString MediaSource::deviceName() const
{
    return d->deviceName;
}

AbstractMediaStream *MediaSource::stream() const
{
    return d->stream;
}

//X AudioCaptureDevice MediaSource::audioCaptureDevice() const
//X {
//X     return d->audioCaptureDevice;
//X }
//X 
//X VideoCaptureDevice MediaSource::videoCaptureDevice() const
//X {
//X     return d->videoCaptureDevice;
//X }

} // namespace Phonon

// vim: sw=4 sts=4 et tw=100
