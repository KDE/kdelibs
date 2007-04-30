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

#ifndef PHONON_ABSTRACTMEDIASTREAM_H
#define PHONON_ABSTRACTMEDIASTREAM_H

#include "phonon_export.h"
#include "phononnamespace.h"
#include <QObject>
class QByteArray;

namespace Phonon
{
class MediaObject;
class AbstractMediaStreamPrivate;

class PHONON_EXPORT AbstractMediaStream : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AbstractMediaStream);
    friend class MediaObject;
    friend class MediaObjectPrivate;
    friend class StreamInterface;
    public:
        virtual ~AbstractMediaStream();

    protected:
        explicit AbstractMediaStream(QObject *parent = 0);
        qint64 streamSize() const;
        void setStreamSize(qint64);

        bool streamSeekable() const;
        void setStreamSeekable(bool);

        /**
         * \warning Don't call this function before the first needData() is emitted.
         */
        void writeData(const QByteArray &);

        /**
         * \warning Don't call this function before the first needData() is emitted.
         */
        void endOfData();

        void error(Phonon::ErrorType errorType, const QString &errorString);

        virtual void needData() = 0;
        virtual void enoughData() = 0;

        /**
         * Reimplement this function if your stream is seekable.
         *
         * \warning Do not call the parent implementation.
         */
        virtual void seekStream(qint64);

        AbstractMediaStream(AbstractMediaStreamPrivate &dd, QObject *parent);
        AbstractMediaStreamPrivate *d_ptr;
};

} // namespace Phonon
#endif // PHONON_ABSTRACTMEDIASTREAM_H
