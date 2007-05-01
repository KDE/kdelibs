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

#ifndef PHONON_MEDIASOURCE_H
#define PHONON_MEDIASOURCE_H

#include "phonon_export.h"
#include "phononnamespace.h"
#include "objectdescription.h"
#include <QExplicitlySharedDataPointer>

class QString;
class QUrl;
class QIODevice;

namespace Phonon
{

class MediaSourcePrivate;
class AbstractMediaStream;

class PHONON_EXPORT MediaSource
{
    public:
        enum Type {
            Invalid,
            LocalFile,
            Url,
            Disc,
            Stream
        };
        /**
         * Creates an invalid MediaSource object.
         *
         * \see type
         */
        MediaSource();
        MediaSource(const QString &filename);
        MediaSource(const QUrl &url);
        MediaSource(Phonon::DiscType, const QString &deviceName = QString());
        MediaSource(AbstractMediaStream *);
        MediaSource(QIODevice *);
        //MediaSource(const AudioCaptureDevice &, const VideoCaptureDevice &);
        ~MediaSource();
        MediaSource(const MediaSource &);
        MediaSource &operator=(const MediaSource &);
        bool operator==(const MediaSource &) const;

        Type type() const;
        QString filename() const;
        QUrl url() const;
        Phonon::DiscType discType() const;
        QString deviceName() const;
        AbstractMediaStream *stream() const;
        //AudioCaptureDevice audioCaptureDevice() const;
        //VideoCaptureDevice videoCaptureDevice() const;

    private:
        QExplicitlySharedDataPointer<MediaSourcePrivate> d;
};

} // namespace Phonon
#endif // PHONON_MEDIASOURCE_H
