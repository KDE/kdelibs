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
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/predicate.h>

//kde specific includes
#include <kcomponentdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

#include <iostream>

using namespace std;

int main(int args, char **argv)
{
    KComponentData componentData("tutorial7");

    //make a predicate
    Solid::Predicate defaultPredicate = Solid::Predicate(Solid::DeviceInterface::StorageVolume);
    kDebug() << "First Predicate: " << defaultPredicate.toString();

    Solid::Predicate testPredicate = defaultPredicate;

    defaultPredicate = defaultPredicate & Solid::Predicate(Solid::DeviceInterface::OpticalDisc);
    kDebug() << "Second Predicate: " << defaultPredicate.toString();

    testPredicate &= Solid::Predicate(Solid::DeviceInterface::OpticalDisc);
    kDebug() << "Test Predicate: " << testPredicate.toString();

    if(testPredicate.toString() != defaultPredicate.toString()) {
        kDebug() << "Danger Will Robinson.";
    } else {
        kDebug() << "Nothing to see here.  Move along!";
    }

    //test for devices
    QList<Solid::Device> devicelist = Solid::Device::listFromQuery(defaultPredicate);
    if (!devicelist.empty()) {
        kDebug() << "Devices that match your query.";
        foreach (Solid::Device device, devicelist) {
            kDebug() << device.udi();
        }
    } else {
        kDebug() << "No devices matched your query.";
    }

    return 0;
}

#include "tutorial7.moc"
