/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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

#include "bytestream.h"
#include "bytestream_p.h"
#include "factory.h"
#include "bytestreaminterface.h"

#define PHONON_CLASSNAME ByteStream
#define PHONON_INTERFACENAME ByteStreamInterface
namespace Phonon
{
PHONON_HEIR_IMPL(AbstractMediaProducer)

PHONON_INTERFACE_GETTER(qint64, totalTime, -1)
PHONON_GETTER(qint32, aboutToFinishTime, d->aboutToFinishTime)
PHONON_GETTER(qint64, streamSize, d->streamSize)
PHONON_GETTER(bool, streamSeekable, d->streamSeekable)

qint64 ByteStream::remainingTime() const
{
    K_D(const ByteStream);
    if (!d->m_backendObject)
        return -1;
    qint64 ret;
    if (BACKEND_GET(qint64, ret, "remainingTime"))
        return ret;
    return totalTime() - currentTime();
}

PHONON_SETTER(setStreamSeekable, streamSeekable, bool)
PHONON_SETTER(setStreamSize, streamSize, qint64)
PHONON_SETTER(setAboutToFinishTime, aboutToFinishTime, qint32)

/*inline void ** ByteStreamPrivate::writeDataParameters(const QByteArray &data)
{
    void *param[] = { 0, static_cast<const void *>(&data), 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    return param;
} */

void ByteStream::writeData(const QByteArray &data)
{
    K_D(ByteStream);
    if (k_ptr->backendObject())
    {
        ByteStreamInterface *bs = qobject_cast<ByteStreamInterface *>(d->m_backendObject);
        bs->writeData(data);
    }
}

void ByteStream::endOfData()
{
    K_D(ByteStream);
    if (k_ptr->backendObject())
        INTERFACE_CALL(endOfData());
}

bool ByteStreamPrivate::aboutToDeleteBackendObject()
{
    if (m_backendObject)
    {
        pBACKEND_GET(qint32, aboutToFinishTime, "aboutToFinishTime");
        pBACKEND_GET(qint64, streamSize, "streamSize");
        pBACKEND_GET(bool, streamSeekable, "streamSeekable");
    }
    return AbstractMediaProducerPrivate::aboutToDeleteBackendObject();
}

void ByteStreamPrivate::setupBackendObject()
{
    Q_Q(ByteStream);
    Q_ASSERT(m_backendObject);
    AbstractMediaProducerPrivate::setupBackendObject();

    QObject::connect(m_backendObject, SIGNAL(finished()), q, SIGNAL(finished()));
    QObject::connect(m_backendObject, SIGNAL(aboutToFinish(qint32)), q, SIGNAL(aboutToFinish(qint32)));
    QObject::connect(m_backendObject, SIGNAL(length(qint64)), q, SIGNAL(length(qint64)));
    QObject::connect(m_backendObject, SIGNAL(needData()), q, SIGNAL(needData()));
    QObject::connect(m_backendObject, SIGNAL(enoughData()), q, SIGNAL(enoughData()));
    QObject::connect(m_backendObject, SIGNAL(seekStream(qint64)), q, SIGNAL(seekStream(qint64)));

    pBACKEND_CALL1("setAboutToFinishTime", qint32, aboutToFinishTime);
}

} //namespace Phonon

#include "bytestream.moc"

#undef PHONON_CLASSNAME
#undef PHONON_INTERFACENAME
// vim: sw=4 ts=4 tw=80
