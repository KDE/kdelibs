/*  This file is part of the KDE project
    Copyright (C) 2006 Michaël Larouche <michael.larouche@kdemail.net>

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
#include "fakemanager.h"

// Qt includes
#include <QtXml>
#include <QFile>
#include <QLatin1String>
#include <QTimer>
#include <QDBusConnection>

// KDE includes
#include <kstandarddirs.h>
#include <kdebug.h>

#include "fakedevice.h"


class FakeManager::Private
{
public:
    QMap<QString, FakeDevice*> loadedDevices;
    QMap<QString, QMap<QString,QVariant> > hiddenDevices;
    QString xmlFile;
};

FakeManager::FakeManager(QObject *parent, const QStringList&)
 : Solid::Ifaces::DeviceManager(parent), d(new Private)
{
    d->xmlFile = KStandardDirs::locate("data", "solidfakehwbackend/fakecomputer.xml");

    QDBusConnection::sessionBus().registerObject( "/org/kde/solid/fakehw", this, QDBusConnection::ExportNonScriptableSlots );

    parseMachineFile();
}

FakeManager::FakeManager(QObject *parent, const QStringList&, const QString &xmlFile)
 : Solid::Ifaces::DeviceManager(parent), d(new Private)
{
    QString machineXmlFile = xmlFile;
    if( machineXmlFile.isEmpty() )
    {
        machineXmlFile = KStandardDirs::locate("data", "solidfakehwbackend/fakecomputer.xml");
    }
    d->xmlFile = machineXmlFile;

    QDBusConnection::sessionBus().registerObject( "/org/kde/solid/fakehw", this, QDBusConnection::ExportNonScriptableSlots );

    parseMachineFile();
}

FakeManager::~FakeManager()
{
    delete d;
}


QStringList FakeManager::allDevices()
{
    QStringList deviceUdiList;

    foreach(FakeDevice *device, d->loadedDevices.values())
    {
        deviceUdiList.append(device->udi());
    }

    return deviceUdiList;
}

bool FakeManager::deviceExists(const QString &udi)
{
    return d->loadedDevices.contains(udi);
}

QStringList FakeManager::devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type)
{
    if( !parentUdi.isEmpty() )
    {
        QStringList found = findDeviceStringMatch(QLatin1String("parent"), parentUdi);

        if( type == Solid::DeviceInterface::Unknown )
        {
            return found;
        }

        QStringList result;

        QStringList::Iterator it = found.begin();
        QStringList::ConstIterator end = found.end();

        for ( ; it!=end; ++it )
        {
            FakeDevice *device = d->loadedDevices[*it];

            if ( device->queryDeviceInterface(type) )
            {
                result << *it;
            }
        }

        return result;
    }
    else if(type != Solid::DeviceInterface::Unknown)
    {
        return findDeviceByDeviceInterface(type);
    }
    else
    {
        return allDevices();
    }
}

QObject *FakeManager::createDevice(const QString &udi)
{
    if( d->loadedDevices.contains(udi) )
    {
        return d->loadedDevices[udi];
    }

    return 0;
}

FakeDevice *FakeManager::findDevice(const QString &udi)
{
    if( d->loadedDevices.contains(udi) )
    {
        return d->loadedDevices[udi];
    }

    return 0;
}

QStringList FakeManager::findDeviceStringMatch(const QString &key, const QString &value)
{
    QStringList result;
    FakeDevice *device;
    foreach(device, d->loadedDevices.values())
    {
        if( device->property(key).toString() == value )
        {
            result.append( device->udi() );
        }
    }

    return result;
}

QStringList FakeManager::findDeviceByDeviceInterface( const Solid::DeviceInterface::Type &type )
{
    QStringList result;
    FakeDevice *device;
    foreach(device, d->loadedDevices.values())
    {
        if( device->queryDeviceInterface(type) )
        {
            result.append( device->udi() );
        }
    }

    return result;
}

void FakeManager::raiseDeviceInterfaceAdded( const QString &udi, Solid::DeviceInterface::Type type )
{
    emit newDeviceInterface( udi, type );
}

void FakeManager::plug( const QString &udi )
{
    if ( d->hiddenDevices.contains( udi ) )
    {
        QMap<QString, QVariant> properties = d->hiddenDevices.take( udi );
        d->loadedDevices[udi] = new FakeDevice( udi, properties );
        emit deviceAdded( udi );
    }
}

void FakeManager::unplug( const QString &udi )
{
    if ( d->loadedDevices.contains( udi ) )
    {
        FakeDevice *dev = d->loadedDevices.take( udi );
        d->hiddenDevices[udi] = dev->allProperties();
        emit deviceRemoved( udi );
    }
}

void FakeManager::parseMachineFile()
{
    QFile machineFile(d->xmlFile);
    if( !machineFile.open(QIODevice::ReadOnly) )
    {
        kDebug() << k_funcinfo << "Error while opening " << d->xmlFile << endl;
        return;
    }

    QDomDocument fakeDocument;
    if( !fakeDocument.setContent(&machineFile) )
    {
        kDebug() << k_funcinfo << "Error while creating the QDomDocument." << endl;
        machineFile.close();
        return;
    }
    machineFile.close();

    kDebug() << k_funcinfo << "Parsing fake computer XML: " << d->xmlFile << endl;
    QDomElement mainElement = fakeDocument.documentElement();
    QDomNode node = mainElement.firstChild();
    while( !node.isNull() )
    {
        QDomElement tempElement = node.toElement();
        if( !tempElement.isNull() && tempElement.tagName() == QLatin1String("device") )
        {
            FakeDevice *tempDevice = parseDeviceElement(tempElement);
            if(tempDevice)
            {
                d->loadedDevices.insert(tempDevice->udi(), tempDevice);
                emit deviceAdded(tempDevice->udi());
            }
        }

        node = node.nextSibling();
    }
}

FakeDevice *FakeManager::parseDeviceElement(const QDomElement &deviceElement)
{
    FakeDevice *device = 0;
    QMap<QString,QVariant> propertyMap;
    QString udi = deviceElement.attribute("udi");
    kDebug() << k_funcinfo << "Listing device: " << udi << endl;

    QDomNode propertyNode = deviceElement.firstChild();
    while( !propertyNode.isNull() )
    {
        QDomElement propertyElement = propertyNode.toElement();
        if( !propertyElement.isNull() && propertyElement.tagName() == QLatin1String("property") )
        {
            QString propertyKey;
            QVariant propertyValue;

            propertyKey = propertyElement.attribute("key");
            propertyValue = QVariant(propertyElement.text());

            propertyMap.insert( propertyKey, propertyValue );
        }

        propertyNode = propertyNode.nextSibling();
    }

    if( !propertyMap.isEmpty() )
    {
        kDebug() << k_funcinfo << "Creating FakeDevice for " << udi << endl;
        device = new FakeDevice(udi, propertyMap);
    }

    return device;
}

#include "fakemanager.moc"
