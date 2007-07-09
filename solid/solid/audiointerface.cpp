/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#include "audiointerface.h"
#include "audiointerface_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/audiointerface.h>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

Solid::AudioInterface::AudioInterface(QObject *backendObject)
    : DeviceInterface(*new AudioInterfacePrivate(), backendObject)
{
}

Solid::AudioInterface::~AudioInterface()
{

}


Solid::AudioInterface::AudioDriver Solid::AudioInterface::driver() const
{
    Q_D(const AudioInterface);
    return_SOLID_CALL(Ifaces::AudioInterface *, d->backendObject(), UnknownAudioDriver, driver());
}

QVariant Solid::AudioInterface::driverHandle() const
{
    Q_D(const AudioInterface);
    return_SOLID_CALL(Ifaces::AudioInterface *, d->backendObject(), QVariant(), driverHandle());
}

QString Solid::AudioInterface::name() const
{
    Q_D(const AudioInterface);
    return_SOLID_CALL(Ifaces::AudioInterface *, d->backendObject(), QString(), name());
}

Solid::AudioInterface::AudioInterfaceTypes Solid::AudioInterface::deviceType() const
{
    Q_D(const AudioInterface);
    return_SOLID_CALL(Ifaces::AudioInterface *, d->backendObject(), UnknownAudioInterfaceType, deviceType());
}

Solid::AudioInterface::SoundcardType Solid::AudioInterface::soundcardType() const
{
    Q_D(const AudioInterface);
    return_SOLID_CALL(Ifaces::AudioInterface *, d->backendObject(), InternalSoundcard, soundcardType());
}

#include "audiointerface.moc"
