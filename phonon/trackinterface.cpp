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

#include "trackinterface.h"
#include "abstractmediaproducer.h"
#include "addoninterface.h"
#include <QList>
#include <QVariant>
#include "frontendinterface_p.h"

namespace Phonon
{
class TrackInterfacePrivate : public FrontendInterfacePrivate
{
    public:
        TrackInterfacePrivate(AbstractMediaProducer *mp) : FrontendInterfacePrivate(mp) {}

        virtual void backendObjectChanged(QObject *);
        TrackInterface *q;
};

TrackInterface::TrackInterface(AbstractMediaProducer *mp)
    : QObject(mp)
    , d(new TrackInterfacePrivate(mp))
{
    d->q = this;
}

void TrackInterfacePrivate::backendObjectChanged(QObject *backendObject)
{
    QObject::connect(backendObject, SIGNAL(trackChanged(int)), q, SIGNAL(trackChanged(int)));
    QObject::connect(backendObject, SIGNAL(availableTracksChanged(int)), q, SIGNAL(availableTracksChanged(int)));
}

TrackInterface::~TrackInterface()
{
    delete d;
}

bool TrackInterface::isValid() const
{
    if (!d || !d->media) {
        return false;
    }
    return d->media->hasInterface<TrackInterface>();
}

#define IFACE(retDefault) \
    AddonInterface *iface = d->iface(); \
    do { \
        if (!iface) { \
            return retDefault; \
        } \
    } while (false)

int TrackInterface::availableTracks() const
{
    IFACE(0);
    return iface->interfaceCall(AddonInterface::TrackInterface,
            AddonInterface::availableTracks).toInt();
}

int TrackInterface::currentTrack() const
{
    IFACE(0);
    return iface->interfaceCall(AddonInterface::TrackInterface,
            AddonInterface::track).toInt();
}

void TrackInterface::setCurrentTrack(int trackNumber)
{
    IFACE();
    iface->interfaceCall(AddonInterface::TrackInterface,
            AddonInterface::setTrack, QList<QVariant>() << QVariant(trackNumber));
}

bool TrackInterface::autoplayTracks() const
{
    IFACE(true);
    return iface->interfaceCall(AddonInterface::TrackInterface,
            AddonInterface::autoplayTracks).toBool();
}

void TrackInterface::setAutoplayTracks(bool b)
{
    IFACE();
    iface->interfaceCall(AddonInterface::TrackInterface,
            AddonInterface::setAutoplayTracks, QList<QVariant>() << QVariant(b));
}
} // namespace Phonon

#include "trackinterface.moc"
// vim: sw=4 sts=4 et tw=100
