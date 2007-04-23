/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef DEVICEINTERFACE_H
#define DEVICEINTERFACE_H

#include <QObject>
#include <QStringList>

#include <solid/ifaces/deviceinterface.h>

#include "haldevice.h"

class DeviceInterface : public QObject, virtual public Solid::Ifaces::DeviceInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::DeviceInterface)
public:
    DeviceInterface(HalDevice *device);
    virtual ~DeviceInterface();

protected:
    HalDevice *m_device;

public:
    inline static QStringList toStringList(Solid::DeviceInterface::Type type)
    {
        QStringList list;

        switch(type)
        {
        case Solid::DeviceInterface::GenericInterface:
            // Doesn't exist with HAL
            break;
        case Solid::DeviceInterface::Processor:
            list << "processor";
            break;
        case Solid::DeviceInterface::Block:
            list << "block";
            break;
        case Solid::DeviceInterface::Storage:
            list << "storage";
            break;
        case Solid::DeviceInterface::Cdrom:
            list << "storage.cdrom";
            break;
        case Solid::DeviceInterface::Volume:
            list << "volume";
            break;
        case Solid::DeviceInterface::OpticalDisc:
            list << "volume.disc";
            break;
        case Solid::DeviceInterface::Camera:
            list << "camera";
            break;
        case Solid::DeviceInterface::PortableMediaPlayer:
            list << "portable_audio_player";
            break;
        case Solid::DeviceInterface::NetworkHw:
            list << "net";
            break;
        case Solid::DeviceInterface::AcAdapter:
            list << "ac_adapter";
            break;
        case Solid::DeviceInterface::Battery:
            list << "battery";
            break;
        case Solid::DeviceInterface::Button:
            list << "button";
            break;
        case Solid::DeviceInterface::Display:
            list << "display_device";
            break;
        case Solid::DeviceInterface::AudioHw:
            list << "alsa" << "oss";
            break;
        case Solid::DeviceInterface::DvbHw:
            list << "dvb";
            break;
        case Solid::DeviceInterface::Unknown:
            break;
        }

        return list;
    }

    inline static Solid::DeviceInterface::Type fromString(const QString &capability)
    {
        if (capability == "processor")
            return Solid::DeviceInterface::Processor;
        else if (capability == "block")
            return Solid::DeviceInterface::Block;
        else if (capability == "storage")
            return Solid::DeviceInterface::Storage;
        else if (capability == "storage.cdrom")
            return Solid::DeviceInterface::Cdrom;
        else if (capability == "volume")
            return Solid::DeviceInterface::Volume;
        else if (capability == "volume.disc")
            return Solid::DeviceInterface::OpticalDisc;
        else if (capability == "camera")
            return Solid::DeviceInterface::Camera;
        else if (capability == "portable_audio_player")
            return Solid::DeviceInterface::PortableMediaPlayer;
        else if (capability == "net")
            return Solid::DeviceInterface::NetworkHw;
        else if (capability == "ac_adapter")
            return Solid::DeviceInterface::AcAdapter;
        else if (capability == "battery")
            return Solid::DeviceInterface::Battery;
        else if (capability == "button")
            return Solid::DeviceInterface::Button;
        else if (capability == "display_device")
            return Solid::DeviceInterface::Display;
        else if (capability == "alsa" || capability == "oss")
            return Solid::DeviceInterface::AudioHw;
        else if (capability == "dvb")
            return Solid::DeviceInterface::DvbHw;
        else
            return Solid::DeviceInterface::Unknown;
    }
};

#endif
