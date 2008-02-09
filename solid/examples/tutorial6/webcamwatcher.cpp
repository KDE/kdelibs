/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "webcamwatcher.h"

#include <solid/device.h>
#include <solid/devicenotifier.h>
#include <solid/deviceinterface.h>
#include <solid/video.h>

#include <klocale.h>
#include <kdebug.h>

WebcamWatcher::WebcamWatcher( QObject * parent ) : QObject( parent )
{
    // get a list of all the webcams in the system
    int found = 0;
    foreach (const Solid::Device &device, Solid::Device::listFromType(Solid::DeviceInterface::Video, QString()))
    {
        m_videoDevices << device.udi();
        getDetails( device );
        found++;
    }
    if ( found == 0 )
    {
        kDebug() << "No video devices found";
    }
    // on deviceAdded, check to see if the device was added
    connect( Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(const QString&)), SLOT(deviceAdded(const QString &)) );
    // likewise if removed
    connect( Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(const QString&)), SLOT(deviceRemoved(const QString &)) );
}

WebcamWatcher::~WebcamWatcher()
{

}

void WebcamWatcher::deviceAdded( const QString & udi )
{
    Solid::Device dev( udi );
    if ( dev.is<Solid::Video>() )
    {
        m_videoDevices << udi;
        getDetails( dev );
    }
}

void WebcamWatcher::getDetails( const Solid::Device & dev )
{
    kDebug() << "New video device at " << dev.udi();
    const Solid::Device * vendorDevice = &dev;
    while ( vendorDevice->isValid() && vendorDevice->vendor().isEmpty() )
    {
        vendorDevice = new Solid::Device( vendorDevice->parentUdi() );
    }
    if ( vendorDevice->isValid() )
    {
        kDebug() << "vendor: " << vendorDevice->vendor() << ", product: " << vendorDevice->product();
    }
    QStringList protocols = dev.as<Solid::Video>()->supportedProtocols();
    if ( protocols.contains( "video4linux" ) )
    {
        QStringList drivers = dev.as<Solid::Video>()->supportedDrivers( "video4linux" );
        if ( drivers.contains( "video4linux" ) )
        {
            kDebug() << "V4L device path is" << dev.as<Solid::Video>()->driverHandle( "video4linux" ).toString();
        }
    }
}

void WebcamWatcher::deviceRemoved( const QString & udi )
{
    Solid::Device dev = Solid::Device( udi );
    int i;
    if ( ( i = m_videoDevices.indexOf( udi ) ) != - 1 ) {
        kDebug() << udi;
        m_videoDevices.removeAt( i );
    }
}
