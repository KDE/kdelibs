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

#ifndef MEDIASOURCE_P_H
#define MEDIASOURCE_P_H

#include "mediasource.h"
#include "objectdescription.h"
#include "abstractmediastream.h"
#include "streameventqueue_p.h"

#include <QtCore/QUrl>
#include <QtCore/QString>
#include <QtCore/QSharedData>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QFile;

namespace Phonon
{

class PHONON_EXPORT MediaSourcePrivate : public QSharedData
{
    public:
        MediaSourcePrivate(MediaSource::Type t)
            : type(t), discType(NoDisc), stream(0),
            streamEventQueue(0),
            ioDevice(0),
            autoDelete(false)
        {
        }

        virtual ~MediaSourcePrivate();

        void setStream(AbstractMediaStream *s);

        MediaSource::Type type;
        QUrl url;
        Phonon::DiscType discType;
        QString deviceName;

        // The AbstractMediaStream(2) may be deleted at any time by the application. If that happens
        // stream will be 0 automatically, but streamEventQueue will stay valid as we hold a
        // reference to it. This is necessary to avoid a races when setting the MediaSource while
        // another thread deletes the AbstractMediaStream2. StreamInterface(2) will then just get a
        // StreamEventQueue where nobody answers.
        QPointer<AbstractMediaStream> stream;
        StreamEventQueue *streamEventQueue;

//        AudioCaptureDevice audioCaptureDevice;
//        VideoCaptureDevice videoCaptureDevice;
        QIODevice *ioDevice;
        QList<MediaSource> linkedSources;
        bool autoDelete;
};

} // namespace Phonon

QT_END_NAMESPACE

#endif // MEDIASOURCE_P_H
// vim: sw=4 sts=4 et tw=100

