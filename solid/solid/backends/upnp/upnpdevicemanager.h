/*
   This file is part of the KDE project

   Copyright 2010 Paulo Romulo Alves Barros <paulo.romulo@kdemail.net>
   
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

#ifndef SOLID_BACKENDS_UPNP_UPNP_DEVICE_MANAGER_H
#define SOLID_BACKENDS_UPNP_UPNP_DEVICE_MANAGER_H

#include <solid/ifaces/devicemanager.h>

#include <QtCore/QSet>
#include <QtCore/QStringList>

#include <HDeviceProxy>
#include <HControlPoint>

namespace Solid
{
namespace Backends
{
namespace UPnP
{

    class UPnPDeviceManager : public Solid::Ifaces::DeviceManager
    {
        Q_OBJECT

        public:
            explicit UPnPDeviceManager(QObject* parent = 0);

            virtual ~UPnPDeviceManager();

            virtual QString udiPrefix() const;

            virtual QSet<Solid::DeviceInterface::Type> supportedInterfaces() const;

            virtual QStringList allDevices();

            virtual QStringList devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type = Solid::DeviceInterface::Unknown);

            virtual QObject *createDevice(const QString &udi);

        public Q_SLOTS:
            void rootDeviceOnline(Herqq::Upnp::HDeviceProxy*);

            void rootDeviceOffline(Herqq::Upnp::HDeviceProxy*);

        private:
            QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;

            Herqq::Upnp::HControlPoint* m_controlPoint;
    };

}
}
}

#endif // SOLID_BACKENDS_UPNP_UPNP_DEVICE_MANAGER_H