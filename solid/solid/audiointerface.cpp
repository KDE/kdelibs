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

QStringList Solid::AudioInterface::driverHandles() const
{
    Q_D(const AudioInterface);

    if (!d->driverHandles.isEmpty())
    {
        // cached
        return d->driverHandles;
    }

    Ifaces::AudioInterface *iface = qobject_cast<Ifaces::AudioInterface *>(d->backendObject());
    if (iface)
    {
        QString handle = iface->driverHandler();
        if (iface->driver() == Alsa)
        {
            // we expect the handle to be of the form hw:X or hw:X,Y
            const int colon = handle.indexOf(':');
            if (-1 == colon)
            {
                return QStringList(handle);
            }
            handle = handle.right(handle.size() - colon - 1);

            // get cardnum, devicenum and subdevicenum
            int comma = handle.indexOf(',');
            QString cardnum;
            QString devicenum;
            QString subdevicenum;
            if (comma > -1)
            {
                cardnum = handle.left(comma);
                handle = handle.right(handle.size() - 1 - comma);
                comma = handle.indexOf(',');
                if (comma > -1)
                {
                    devicenum = handle.left(comma);
                    subdevicenum = handle.right(handle.size() - 1 - comma);
                }
                else
                {
                    devicenum = handle;
                }
            }
            else
            {
                cardnum = handle;
            }
            handle = QLatin1String("CARD=") + cardnum;
            if (!devicenum.isEmpty())
            {
                handle += QLatin1String(",DEV=") + devicenum;
                if (!subdevicenum.isEmpty())
                {
                    handle += QLatin1String(",SUBDEV=") + subdevicenum;
                }
            }

            if (iface->deviceType()  & Solid::AudioInterface::AudioOutput)
            {
                // first try dmix for concurrent access then plain hw and if the hw formats don't
                // work plughw
                d->driverHandles << QLatin1String("dmix:") + handle;
            }
            if (iface->deviceType()  & Solid::AudioInterface::AudioInput)
            {
                // first try dsnoop for concurrent access, then plain hw
                d->driverHandles << QLatin1String("dsnoop:") + handle;
            }
            d->driverHandles << QLatin1String("hw:") + handle
                << QLatin1String("plughw:") + handle;

            return d->driverHandles;
        }
        else
        {
            return QStringList(handle);
        }
    }
    return QStringList();
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
