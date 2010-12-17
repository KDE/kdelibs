/*
    Copyright 2010 Alex Fiestas <alex@eyeos.org>
    Copyright 2010 UFO Coders <info@ufocoders.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "udevaudiointerface.h"

#include "udevaudiointerface_p.h"
#include "udevdevice.h"

#include <QFile>

using namespace Solid::Backends::UDev;

AudioInterface::AudioInterface(UDevDevice *device)
    : DeviceInterface(device),
    d(new UdevAudioInterfacePrivate(device))
{
}

AudioInterface::~AudioInterface()
{

}

Solid::AudioInterface::AudioDriver AudioInterface::driver() const
{
    return d->m_driver;
}

QVariant AudioInterface::driverHandle() const
{
    if (d->m_driver == Solid::AudioInterface::Alsa)
    {
        QList<QVariant> list;
        if (d->m_cardnum != -1) {
            list << QVariant(d->m_cardnum);
            if (d->m_devicenum != -1) {
                list << QVariant(d->m_devicenum);
            }
        } else if(!d->m_deviceFile.isEmpty()) {
            list << QVariant(d->m_deviceFile);
        }
        return list;
    }
    else if (d->m_driver == Solid::AudioInterface::OpenSoundSystem)
    {
        if (!d->m_deviceFile.isEmpty()) {
            return QVariant(d->m_deviceFile);
        }
    }

    return QVariant();
}

QString AudioInterface::name() const
{
    return d->m_name;
}

Solid::AudioInterface::AudioInterfaceTypes AudioInterface::deviceType() const
{
        return d->m_type;
}

Solid::AudioInterface::SoundcardType AudioInterface::soundcardType() const
{
    return d->soundcardType();
}

#include "backends/udev/udevaudiointerface.moc"
