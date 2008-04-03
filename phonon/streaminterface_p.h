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

#ifndef STREAMINTERFACE_P_H
#define STREAMINTERFACE_P_H

#include "streaminterface.h"
#include "streameventqueue_p.h"
#include "mediasource.h"
#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

namespace Phonon
{

class StreamInterfacePrivateHelper;

class StreamInterfacePrivate : public LockFreeQueueBase::DataReadyHandler
{
    friend class StreamInterface;
    friend class StreamInterface2;
    friend class StreamInterfacePrivateHelper;

    public:
        void disconnectMediaStream();

    protected:
        inline StreamInterfacePrivate()
            : qobject(0),
            streamEventQueue(0),
            eventDispatcher(QAbstractEventDispatcher::instance()),
            connected(false),
            awakeSignalConnected(false)
        {
            Q_ASSERT(eventDispatcher);
        }

        inline ~StreamInterfacePrivate()
        {
            delete qobject;
        }

        virtual void dataReady();
        void _k_handleStreamEvent();
        void handleStreamEvent(QObject *sender);

        MediaSource mediaSource;
        QObject *qobject;
        StreamInterface *q;
        StreamEventQueue *streamEventQueue;
        QAbstractEventDispatcher *eventDispatcher;
        quint8 connected : 1;
        quint8 awakeSignalConnected : 1;
};

class StreamInterfacePrivateHelper : public QObject
{
    Q_OBJECT
    public:
        StreamInterfacePrivateHelper(StreamInterfacePrivate *qq) : q(qq) {}

    private:
        StreamInterfacePrivate *const q;
};

} // namespace Phonon

QT_END_NAMESPACE

#endif // STREAMINTERFACE_P_H
// vim: sw=4 sts=4 et tw=100
