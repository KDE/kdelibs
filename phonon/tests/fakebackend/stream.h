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

#ifndef PHONON_FAKE_STREAM_H
#define PHONON_FAKE_STREAM_H

#include <phonon/streaminterface.h>
#include <QtCore/QObject>

namespace Phonon
{
namespace Fake
{
class MediaObject;

class Stream : public StreamInterface2
{
    Q_OBJECT
    public:
        Stream(const MediaSource &mediaSource, MediaObject *parent);
        void writeData(const QByteArray &data);
        void endOfData();
        void setStreamSize(qint64 newSize);
        void setStreamSeekable(bool s);
        void consumeStream(int msec);

    private:
        bool m_eof;
        bool m_streamSeekable;
        qint64 m_streamSize;
        qint64 m_bufferSize;
        qint64 m_streamPosition;
        MediaObject *m_mediaObject;
};

} // namespace Fake
} // namespace Phonon
#endif // PHONON_FAKE_STREAM_H
