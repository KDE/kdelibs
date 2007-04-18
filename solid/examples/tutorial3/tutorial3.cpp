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
#include <solid/devicemanager.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/processor.h>

//kde specific includes
#include <kcomponentdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

#include <iostream>

using namespace std;

int main(int args, char **argv)
{
    KComponentData componentData("tutorial3");
    
    Solid::DeviceManager &manager = Solid::DeviceManager::self();
    
    //get a Processor
    Solid::DeviceList list = manager.findDevicesFromQuery(Solid::DeviceInterface::Processor, QString());

    //take the first processor
    Solid::Device device = list[0];
    if(device.is<Solid::Processor>() ) kDebug() << "We've got a processor!" << endl;
    else kDebug() << "Device is not a processor." << endl;

    Solid::Processor *processor = device.as<Solid::Processor>();
    kDebug() << "This processors maximum speed is: " << processor->maxSpeed() << endl;
    
    return 0;
}

#include "tutorial3.moc"
