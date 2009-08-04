/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

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

#include "halvideo.h"

using namespace Solid::Backends::Hal;

Video::Video(HalDevice *device)
    : DeviceInterface(device)
{

}

Video::~Video()
{

}

QStringList Video::supportedProtocols() const
{
    QStringList protocols;
    protocols << QString( "video4linux" );
    return protocols;
}

QStringList Video::supportedDrivers(QString protocol) const
{
    QStringList drivers;
    if ( protocol == "video4linux" ) {
        drivers << QString( "video4linux" ); //Retrocompatibility with KDE < 4.3
        if ( m_device->property("video4linux.version") == "2" ) {
            drivers << QString( "video4linux2" );
        } else {
            drivers << QString( "video4linux1" );
        }
    }
    return drivers;
}

QVariant Solid::Backends::Hal::Video::driverHandle(const QString &driver) const
{
    if ( driver=="video4linux" || driver=="video4linux1" || driver=="video4linux2" ) {
        return m_device->property("video4linux.device");
    }
    // TODO: Fill in the blank for other drivers

    return QVariant();
}

#include "backends/hal/halvideo.moc"
