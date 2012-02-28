/*
    Copyright 2006 Michaël Larouche <michael.larouche@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOLID_BACKENDS_FAKEHW_FAKEMANAGER_H
#define SOLID_BACKENDS_FAKEHW_FAKEMANAGER_H

#include <solid/ifaces/devicemanager.h>

class QDomElement;

using namespace Solid::Ifaces;

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeDevice;

/**
 * @brief a Fake manager that read a device list from a XML file.
 * This fake manager is used for unit tests and developers.
 *
 * @author Michaël Larouche <michael.larouche@kdemail.net>
 */
class FakeManager : public Solid::Ifaces::DeviceManager
{
    Q_OBJECT
public:
    FakeManager(QObject *parent, const QString &xmlFile);
    virtual ~FakeManager();

    virtual QString udiPrefix() const ;
    virtual QSet<Solid::DeviceInterface::Type> supportedInterfaces() const;

    /**
     * Return the list of UDI of all available devices.
     */
    virtual QStringList allDevices();

    virtual QStringList devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type);

    virtual QObject *createDevice(const QString &udi);
    virtual FakeDevice *findDevice(const QString &udi);

public Q_SLOTS:
    void plug(const QString &udi);
    void unplug(const QString &udi);

private Q_SLOTS:
    /**
     * @internal
     * Parse the XML file that represent the fake machine.
     */
    void parseMachineFile();
    /**
     * @internal
     * Parse a device node and the return the device.
     */
    FakeDevice *parseDeviceElement(const QDomElement &element);

private:
    QStringList findDeviceStringMatch(const QString &key, const QString &value);
    QStringList findDeviceByDeviceInterface(Solid::DeviceInterface::Type type);

    class Private;
    Private *d;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEMANAGER_H
