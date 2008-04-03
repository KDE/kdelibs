/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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
#ifndef Phonon_FAKE_BYTESTREAM_H
#define Phonon_FAKE_BYTESTREAM_H

#include "mediaproducer.h"
#include <phonon/bytestreaminterface.h>
class QTimer;

namespace Phonon
{
namespace Fake
{
    class ByteStream : public MediaProducer, public Phonon::ByteStreamInterface
    {
        Q_OBJECT
        Q_INTERFACES(Phonon::ByteStreamInterface)
        public:
            ByteStream(QObject *parent);
            ~ByteStream();

            qint64 currentTime() const;
            qint64 totalTime() const;
            Q_INVOKABLE qint32 aboutToFinishTime() const;
            Q_INVOKABLE qint64 streamSize() const;
            Q_INVOKABLE bool streamSeekable() const;
            bool isSeekable() const;

            Q_INVOKABLE void setStreamSeekable(bool);
            void writeData(const QByteArray &data);
            Q_INVOKABLE void setStreamSize(qint64);
            void endOfData();
            Q_INVOKABLE void setAboutToFinishTime(qint32);

            void play();
            void pause();
            void seek(qint64 time);

        public Q_SLOTS:
            virtual void stop();

        Q_SIGNALS:
            void finished();
            void aboutToFinish(qint32);
            void length(qint64);
            void needData();
            void enoughData();
            void seekStream(qint64);

        private Q_SLOTS:
            void consumeStream();

        private:
            qint64 m_aboutToFinishBytes;
            qint64 m_streamSize;
            qint64 m_bufferSize;
            qint64 m_streamPosition;
            bool m_streamSeekable;
            bool m_eof;
            bool m_aboutToFinishEmitted;
            QTimer *m_streamConsumeTimer;
    };
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80
#endif // Phonon_FAKE_BYTESTREAM_H
