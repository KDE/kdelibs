/*  This file is part of the KDE project
    Copyright (C) 2007-2008 Matthias Kretz <kretz@kde.org>

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

#ifndef PHONON_ABSTRACTMEDIASTREAM2_H
#define PHONON_ABSTRACTMEDIASTREAM2_H

#include "abstractmediastream.h"

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QByteArray;

namespace Phonon
{
class MediaObject;
class AbstractMediaStream2Private;

/** \class AbstractMediaStream2 abstractmediastream2.h Phonon/AbstractMediaStream2
 * \brief Base class for custom media data streams.
 *
 * Implement this class to provide a custom data stream to the backend. The class supports both, the
 * push and the pull model.
 *
 * Push:
 * \code
 * PushStream::PushStream(QObject *parent)
 *   : AbstractMediaStream2(parent), m_timer(new QTimer(this))
 * {
 *   setStreamSize(getMediaStreamSize());
 *
 *   connect(m_timer, SIGNAL(timeout()), SLOT(moreData()));
 *   m_timer->setInterval(0);
 * }
 *
 * void PushStream::moreData()
 * {
 *   const QByteArray data = getMediaData();
 *   if (data.isEmpty()) {
 *     endOfData();
 *   } else {
 *     writeData(data);
 *   }
 * }
 *
 * void PushStream::needData()
 * {
 *   m_timer->start();
 *   moreData();
 * }
 *
 * void PushStream::enoughData()
 * {
 *   m_timer->stop();
 * }
 * \endcode
 *
 * Pull:
 * \code
 * PullStream::PullStream(QObject *parent)
 *   : AbstractMediaStream2(parent)
 * {
 *   setStreamSize(getMediaStreamSize());
 * }
 *
 * void PullStream::needData()
 * {
 *   const QByteArray data = getMediaData();
 *   if (data.isEmpty()) {
 *     endOfData();
 *   } else {
 *     writeData(data);
 *   }
 * }
 * \endcode
 *
 * \ingroup Playback
 * \author Matthias Kretz <kretz@kde.org>
 */
class PHONON_EXPORT AbstractMediaStream2 : public AbstractMediaStream
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AbstractMediaStream2)
    friend class MediaObject;
    friend class MediaObjectPrivate;
    friend class MediaSourcePrivate;
    protected:
        /////////////////////////////////////////////////////////////
        // functions an implementation will call:
        /////////////////////////////////////////////////////////////

        /**
         * Constructs an AbstractMediaStream2 object with a \p parent.
         */
        explicit AbstractMediaStream2(QObject *parent = 0);

        void resetDone();
        void seekStreamDone();

        /////////////////////////////////////////////////////////////
        // functions to implement:
        /////////////////////////////////////////////////////////////

        /**
         * Reimplement this function to be notified when the backend needs data.
         *
         * When this function is called you should try to call writeData or endOfData before
         * returning.
         *
         * \param size The number of bytes that are needed. If possible, pass \p size bytes of media
         *             data in the next writeData call.
         */
        virtual void needData(quint32 size) = 0;

        /////////////////////////////////////////////////////////////
        // internal
        /////////////////////////////////////////////////////////////

        bool event(QEvent *e);
        AbstractMediaStream2(AbstractMediaStream2Private &dd, QObject *parent);

    private:
        // hide needData() explicitly
        virtual void needData() {}

        Q_PRIVATE_SLOT(d_func(), void _k_handleStreamEvent())
};

} // namespace Phonon

QT_END_NAMESPACE
QT_END_HEADER

#endif // PHONON_ABSTRACTMEDIASTREAM2_H
