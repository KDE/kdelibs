/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>
    Copyright (C) 2007 Jeff Mitchell <kde-dev@emailgoeshere.com>

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

#include "wmiportablemediaplayer.h"

using namespace Solid::Backends::Wmi;

PortableMediaPlayer::PortableMediaPlayer(WmiDevice *device)
    : DeviceInterface(device)
{

}

PortableMediaPlayer::~PortableMediaPlayer()
{

}

QStringList PortableMediaPlayer::supportedProtocols() const
{
    return m_device->property("portable_audio_player.access_method.protocols").toStringList();
}

QStringList PortableMediaPlayer::supportedDrivers(QString protocol) const
{
    QStringList drivers = m_device->property("portable_audio_player.access_method.drivers").toStringList();
    if(protocol.isNull())
        return drivers;
    QStringList returnedDrivers;
    QString temp;
    for(int i = 0; i < drivers.size(); i++)
    {
        temp = drivers.at(i);
        if(m_device->property("portable_audio_player." + temp + ".protocol") == protocol)
            returnedDrivers << temp;
    }
    return returnedDrivers;
}

QVariant Solid::Backends::Wmi::PortableMediaPlayer::driverHandle(const QString &driver) const
{
    if (driver=="mtp") {
        return m_device->property("usb.serial");
    }
    // TODO: Fill in the blank for other drivers

    return QVariant();
}

#include "backends/wmi/wmiportablemediaplayer.moc"
