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

#include "fakedevice.h"

// Qt includes
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtXml/QDomNode>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtDBus/QDBusConnection>


using namespace Solid::Backends::Fake;

class FakeManager::Private
{
public:
    QMap<QString, FakeDevice *> loadedDevices;
    QMap<QString, QMap<QString,QVariant> > hiddenDevices;
    QString xmlFile;
    QSet<Solid::DeviceInterface::Type> supportedInterfaces;
};

FakeManager::FakeManager(QObject *parent, const QString &xmlFile)
 : Solid::Ifaces::DeviceManager(parent), d(new Private)
{
    QString machineXmlFile = xmlFile;
    d->xmlFile = machineXmlFile;

    QDBusConnection::sessionBus().registerObject("/org/kde/solid/fakehw", this, QDBusConnection::ExportNonScriptableSlots);

    parseMachineFile();

    d->supportedInterfaces << Solid::DeviceInterface::GenericInterface
                           << Solid::DeviceInterface::Processor
                           << Solid::DeviceInterface::Block
                           << Solid::DeviceInterface::StorageAccess
                           << Solid::DeviceInterface::StorageDrive
                           << Solid::DeviceInterface::OpticalDrive
                           << Solid::DeviceInterface::StorageVolume
                           << Solid::DeviceInterface::OpticalDisc
                           << Solid::DeviceInterface::Camera
                           << Solid::DeviceInterface::PortableMediaPlayer
                           << Solid::DeviceInterface::NetworkInterface
                           << Solid::DeviceInterface::AcAdapter
                           << Solid::DeviceInterface::Battery
                           << Solid::DeviceInterface::Button
                           << Solid::DeviceInterface::AudioInterface
                           << Solid::DeviceInterface::DvbInterface
                           << Solid::DeviceInterface::Video
                           << Solid::DeviceInterface::SerialInterface
                           << Solid::DeviceInterface::SmartCardReader;
}

FakeManager::~FakeManager()
{
    qDeleteAll(d->loadedDevices);
    delete d;
}

QString FakeManager::udiPrefix() const
{
    return "/org/kde/solid/fakehw";
}

QSet<Solid::DeviceInterface::Type> FakeManager::supportedInterfaces() const
{
    return d->supportedInterfaces;
}

QStringList FakeManager::allDevices()
{
    QStringList deviceUdiList;

    foreach (FakeDevice *device, d->loadedDevices)
    {
        deviceUdiList.append(device->udi());
    }

    return deviceUdiList;
}

QStringList FakeManager::devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type)
{
    if (!parentUdi.isEmpty())
    {
        QStringList found = findDeviceStringMatch(QLatin1String("parent"), parentUdi);

        if (type == Solid::DeviceInterface::Unknown)
        {
            return found;
        }

        QStringList result;

        QStringList::ConstIterator it = found.constBegin();
        QStringList::ConstIterator end = found.constEnd();

        for (; it!=end; ++it)
        {
            FakeDevice *device = d->loadedDevices[*it];

            if (device->queryDeviceInterface(type))
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
    if (d->loadedDevices.contains(udi))
    {
        return new FakeDevice(*d->loadedDevices[udi]);
    }

    return 0;
}

FakeDevice *FakeManager::findDevice(const QString &udi)
{
    if (d->loadedDevices.contains(udi))
    {
        return d->loadedDevices[udi];
    }

    return 0;
}

QStringList FakeManager::findDeviceStringMatch(const QString &key, const QString &value)
{
    QStringList result;
    FakeDevice *device;
    foreach (device, d->loadedDevices)
    {
        if (device->property(key).toString() == value)
        {
            result.append(device->udi());
        }
    }

    return result;
}

QStringList FakeManager::findDeviceByDeviceInterface(Solid::DeviceInterface::Type type)
{
    QStringList result;
    FakeDevice *device;
    foreach (device, d->loadedDevices)
    {
        if (device->queryDeviceInterface(type))
        {
            result.append(device->udi());
        }
    }

    return result;
}

void FakeManager::plug(const QString &udi)
{
    if (d->hiddenDevices.contains(udi))
    {
        QMap<QString, QVariant> properties = d->hiddenDevices.take(udi);
        d->loadedDevices[udi] = new FakeDevice(udi, properties);
        emit deviceAdded(udi);
    }
}

void FakeManager::unplug(const QString &udi)
{
    if (d->loadedDevices.contains(udi))
    {
        FakeDevice *dev = d->loadedDevices.take(udi);
        d->hiddenDevices[udi] = dev->allProperties();
        emit deviceRemoved(udi);
        delete dev;
    }
}

void FakeManager::parseMachineFile()
{
    QFile machineFile(d->xmlFile);
    if (!machineFile.open(QIODevice::ReadOnly))
    {
        qWarning() << Q_FUNC_INFO << "Error while opening " << d->xmlFile << endl;
        return;
    }

    QDomDocument fakeDocument;
    if (!fakeDocument.setContent(&machineFile))
    {
        qWarning() << Q_FUNC_INFO << "Error while creating the QDomDocument." << endl;
        machineFile.close();
        return;
    }
    machineFile.close();

    qDebug() << Q_FUNC_INFO << "Parsing fake computer XML: " << d->xmlFile << endl;
    QDomElement mainElement = fakeDocument.documentElement();
    QDomNode node = mainElement.firstChild();
    while (!node.isNull())
    {
        QDomElement tempElement = node.toElement();
        if (!tempElement.isNull() && tempElement.tagName() == QLatin1String("device"))
        {
            FakeDevice *tempDevice = parseDeviceElement(tempElement);
            if(tempDevice) {
                Q_ASSERT(!d->loadedDevices.contains(tempDevice->udi()));
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

    QDomNode propertyNode = deviceElement.firstChild();
    while (!propertyNode.isNull())
    {
        QDomElement propertyElement = propertyNode.toElement();
        if (!propertyElement.isNull() && propertyElement.tagName() == QLatin1String("property"))
        {
            QString propertyKey;
            QVariant propertyValue;

            propertyKey = propertyElement.attribute("key");
            propertyValue = QVariant(propertyElement.text());

            propertyMap.insert(propertyKey, propertyValue);
        }

        propertyNode = propertyNode.nextSibling();
    }

    if (!propertyMap.isEmpty())
    {
        device = new FakeDevice(udi, propertyMap);
    }

    return device;
}

#include "backends/fakehw/fakemanager.moc"
