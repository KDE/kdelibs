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

#include "angleinterface.h"
#include "abstractmediaproducer.h"
#include "addoninterface.h"
#include <QList>
#include <QVariant>
#include "frontendinterface_p.h"

namespace Phonon
{

class AngleInterfacePrivate : public FrontendInterfacePrivate
{
    public:
        AngleInterfacePrivate(AbstractMediaProducer *mp) : FrontendInterfacePrivate(mp) {}

        virtual void backendObjectChanged(QObject *);
        AngleInterface *q;
};

AngleInterface::AngleInterface(AbstractMediaProducer *mp)
    : QObject(mp),
    d(new AngleInterfacePrivate(mp))
{
    d->q = this;
    d->_backendObjectChanged();
}

void AngleInterfacePrivate::backendObjectChanged(QObject *backendObject)
{
    QObject::connect(backendObject, SIGNAL(angleChanged(int)), q, SIGNAL(angleChanged(int)));
    QObject::connect(backendObject, SIGNAL(availableAnglesChanged(int)), q, SIGNAL(availableAnglesChanged(int)));
}

AngleInterface::~AngleInterface()
{
    delete d;
}

bool AngleInterface::isValid() const
{
    if (!d || !d->media) {
        return false;
    }
    return d->media->hasInterface<AngleInterface>();
}

#define IFACE(retDefault) \
    AddonInterface *iface = d->iface(); \
    do { \
        if (!iface) { \
            return retDefault; \
        } \
    } while (false)

int AngleInterface::availableAngles() const
{
    IFACE(0);
    return iface->interfaceCall(AddonInterface::AngleInterface,
            AddonInterface::availableAngles).toInt();
}

int AngleInterface::currentAngle() const
{
    IFACE(0);
    return iface->interfaceCall(AddonInterface::AngleInterface,
            AddonInterface::angle).toInt();
}

void AngleInterface::setCurrentAngle(int trackNumber)
{
    IFACE();
    iface->interfaceCall(AddonInterface::AngleInterface,
            AddonInterface::setAngle, QList<QVariant>() << QVariant(trackNumber));
}

} // namespace Phonon
#include "angleinterface.moc"
// vim: sw=4 sts=4 et tw=100
