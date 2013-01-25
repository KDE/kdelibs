/*
    Copyright 2013 Patrick von Reth <vonreth@kde.org>
    
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

#ifndef WINDEVICE_H
#define WINDEVICE_H

#include <solid/ifaces/device.h>
#include "windevicemanager.h"

namespace Solid
{
namespace Backends
{
namespace Win
{

class WinDevice : public Solid::Ifaces::Device
{
    friend class WinDeviceManager;
    Q_OBJECT
public:
    WinDevice(const QString &udi);

   virtual QString udi() const;

   virtual QString parentUdi() const;
   virtual QString vendor() const;
   virtual QString product() const;

   virtual QString icon() const;

   virtual QStringList emblems() const;

   virtual QString description() const;

   virtual bool queryDeviceInterface(const Solid::DeviceInterface::Type &type) const;
   virtual QObject *createDeviceInterface(const Solid::DeviceInterface::Type &type);

    Solid::DeviceInterface::Type type() const;

    QString driveLetter() const;

private:
    QString m_udi;
    QString m_parentUdi;
    QString m_vendor;
    Solid::DeviceInterface::Type m_type;

    static QMap<QString,QString> m_driveLetters;



};

}
}
}

#endif // WINDEVICE_H
