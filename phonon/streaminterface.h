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

#ifndef PHONON_STREAMINTERFACE_H
#define PHONON_STREAMINTERFACE_H

#include "phonon_export.h"
#include <QtCore/QObject>

namespace Phonon
{
class StreamInterfacePrivate;
class MediaSource;
/**
 * \ingroup Backend
 * \author Matthias Kretz <kretz@kde.org>
 */
class PHONON_EXPORT StreamInterface
{
    friend class StreamInterfacePrivate;
    friend class AbstractMediaStreamPrivate;
    public:
        virtual ~StreamInterface();
        virtual void writeData(const QByteArray &data) = 0;
        virtual void endOfData() = 0;
        virtual void setStreamSize(qint64 newSize) = 0;
        virtual void setStreamSeekable(bool s) = 0;

        void connectToSource(const MediaSource &mediaSource);
        void needData();
        void enoughData();
        void seekStream(qint64);

    protected:
        StreamInterface();

        StreamInterfacePrivate *const d;
};
} // namespace Phonon

Q_DECLARE_INTERFACE(Phonon::StreamInterface, "StreamInterface1.phonon.kde.org")

#endif // PHONON_STREAMINTERFACE_H
