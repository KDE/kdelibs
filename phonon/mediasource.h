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
#include <QtCore/QSharedData>

class QString;
class QUrl;
class QIODevice;

namespace Phonon
{

class MediaSourcePrivate;
class AbstractMediaStream;

/**
 * Note that all constructors of this class are implicit, so that you can simply write
 * \code
 * MediaObject m;
 * QString filename("/home/foo/bar.ogg");
 * QUrl url("http://www.example.com/stream.mp3");
 * QBuffer *someBuffer;
 * m.setCurrentSource(filename);
 * m.setCurrentSource(url);
 * m.setCurrentSource(someBuffer);
 * m.setCurrentSource(Phonon::Cd);
 * \endcode
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
class PHONON_EXPORT MediaSource
{
    public:
        /**
         * Identifies the type of media described by the MediaSource object.
         * 
         * \see MediaSource::type()
         */
        enum Type {
            /**
             * The MediaSource object does not describe any valid source.
             */
            Invalid = -1,
            /**
             * The MediaSource object describes a local file.
             */
            LocalFile,
            /**
             * The MediaSource object describes a URL, which can be both a local file and a file on
             * the network.
             */
            Url,
            /**
             * The MediaSource object describes a disc.
             */
            Disc,
            /**
             * The MediaSource object describes a data stream.
             *
             * This is also the type used for QIODevices.
             *
             * \see AbstractMediaStream
             */
            Stream
        };
        /**
         * Creates an invalid MediaSource object.
         *
         * \see Invalid
         */
        MediaSource();

        /**
         * Creates a MediaSource object for a local file or a Qt resource.
         *
         * \param filename file name of a local media file or a Qt resource that was compiled in.
         */
        MediaSource(const QString &filename);

        /**
         * Creates a MediaSource object for a URL.
         *
         * \param url URL to a media file or stream.
         */
        MediaSource(const QUrl &url);

        /**
         * Creates a MediaSource object for discs.
         *
         * \param discType See \ref DiscType
         * \param deviceName A platform dependent device name. This can be useful if the computer
         * has more than one CD drive. It is recommended to use Solid to retrieve the device name in
         * a portable way.
         */
        MediaSource(Phonon::DiscType discType, const QString &deviceName = QString());

        /**
         * Creates a MediaSource object for a data stream.
         *
         * Your application can provide the media data by subclassing AbstractMediaStream and
         * passing a pointer to that object. %Phonon will never delete the \p stream.
         *
         * \param stream The AbstractMediaStream subclass to provide the media data.
         */
        MediaSource(AbstractMediaStream *stream);

        /**
         * Creates a MediaSource object for a QIODevice.
         *
         * This constructor can be very handy in the combination of QByteArray and QBuffer.
         *
         * \param ioDevice An arbitrary readable QIODevice subclass. If the device is not opened
         * MediaSource will open it as QIODevice::ReadOnly. Sequential I/O devices are possible,
         * too. For those MediaObject::isSeekable() will have to return false obviously.
         */
        MediaSource(QIODevice *ioDevice);

        /**
         * Creates a MediaSource object for capture devices.
         */
        //MediaSource(const AudioCaptureDevice &, const VideoCaptureDevice &);

        /**
         * Destroys the MediaSource object.
         */
        ~MediaSource();

        /**
         * Constructs a copy of \p rhs.
         *
         * This constructor is fast thanks to explicit sharing.
         */
        MediaSource(const MediaSource &rhs);

        /**
         * Assigns \p rhs to this MediaSource and returns a reference to this MediaSource.
         *
         * This operation is fast thanks to explicit sharing.
         */
        MediaSource &operator=(const MediaSource &rhs);

        /**
         * Returns true if this MediaSource is equal to \p rhs; otherwise returns false.
         */
        bool operator==(const MediaSource &rhs) const;

        /**
         * Returns the type of the MediaSource (depends on the constructor that was used).
         *
         * \see Type
         */
        Type type() const;

        /**
         * Returns the file name of the MediaSource if type() == LocalFile; otherwise returns
         * QString().
         */
        QString filename() const;

        /**
         * Returns the url of the MediaSource if type() == URL or type() == LocalFile; otherwise
         * returns QUrl().
         */
        QUrl url() const;

        /**
         * Returns the disc type of the MediaSource if type() == Disc; otherwise returns \ref
         * NoDisc.
         */
        Phonon::DiscType discType() const;

        /**
         * Returns the device name of the MediaSource if type() == Disc; otherwise returns
         * QString().
         */
        QString deviceName() const;

        /**
         * Returns the media stream of the MediaSource if type() == Stream; otherwise returns 0.
         * QIODevices are handled as streams, too.
         */
        AbstractMediaStream *stream() const;

        //AudioCaptureDevice audioCaptureDevice() const;
        //VideoCaptureDevice videoCaptureDevice() const;

    private:
        QExplicitlySharedDataPointer<MediaSourcePrivate> d;
};

} // namespace Phonon
#endif // PHONON_MEDIASOURCE_H
