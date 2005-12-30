/*  This file is part of the KDE project
    Copyright (C) 2005 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include <kinstance.h>
#include <kdebug.h>

#include "devicemanager.h"
#include "device.h"
#include "fakemanager.h"

int main()
{
    KInstance instance( "kdehw_test" );

    KDEHW::DeviceManager &manager
        = KDEHW::DeviceManager::selfForceBackend( new FakeManager() );
//        = KDEHW::DeviceManager::self();

    KDEHW::DeviceList devices = manager.allDevices();

    kdDebug() << "=======" << endl;
    //kdDebug() << ":" << devices << ":" << endl;
    kdDebug() << ":" << manager.deviceExists("/org/freedesktop/Hal/devices/computer") << ":" << endl;
    kdDebug() << "=======" << endl;

    foreach ( KDEHW::Device dev , devices )
    {
        QMap<QString, QVariant> properties = dev.allProperties();

        kdDebug() << "udi = '" << dev.udi() << "'" << endl;

        QMap<QString, QVariant>::ConstIterator it = properties.begin();
        QMap<QString, QVariant>::ConstIterator end = properties.end();

        for ( ; it!=end; ++it )
        {
            kdDebug() << "  " << it.key() << " = '" << it.value() << endl;
        }
    }

    KDEHW::Device dev = manager.findDevice( "/org/freedesktop/Hal/devices/computer" );
    kdDebug() << "Computer vendor : " << dev.property( "system.kernel.machine" ) << endl;
    kdDebug() << "Try locking : " << dev.lock("foo") << endl;
    kdDebug() << "Try abusing lock : " << dev.lock("foofoo") << endl;
    kdDebug() << "Try unlocking : " << dev.unlock() << endl;

    return 0;
}
