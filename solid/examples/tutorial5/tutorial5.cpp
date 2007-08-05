/*  This file is part of the KDE project
    Copyright (C) 2007 Christopher Blauvelt <cblauvelt@gmail.com>

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

//QT specific includes
#include <QList>

//solid specific includes
#include <solid/deviceinterface.h>
#include <solid/networking.h>
#include <solid/device.h>
#include <solid/networkinterface.h>

//kde specific includes
#include <kcomponentdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

#include <iostream>

using namespace std;

int main(int args, char **argv)
{
    KComponentData data("tutorial5");
    
    //get a Network Device
    QList<Solid::Device> netlist = Solid::Device::listFromType(Solid::DeviceInterface::NetworkInterface, QString());
    
    //check to see if no network devices were found
    if(netlist.empty())
    {
        kDebug() << "No network devices found!";
        return 0;
    }
    
    Solid::Device device = netlist[0];
    Solid::NetworkInterface *netdev = device.as<Solid::NetworkInterface>();
    //keep the program from crashing in the event that there's a bug in solid
    if(!netdev)
    {
        kDebug() << "Device could not be converted.  There is a bug.";
        return 0;
    }
    
    kDebug() << "The iface of " << device.udi() << " is " << netdev->ifaceName();
    
    return 0;
}

#include "tutorial5.moc"
