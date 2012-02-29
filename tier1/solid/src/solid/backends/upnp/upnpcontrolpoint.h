/*
    Copyright 2010 Paulo Romulo Alves Barros <paulo.romulo@kdemail.net>

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

#ifndef SOLID_BACKENDS_UPNP_UPNPCONTROLPOINT_H
#define SOLID_BACKENDS_UPNP_UPNPCONTROLPOINT_H

#include <QtCore/QObject>
#include <QtCore/QMutex>

#include <HUpnpCore/HClientDevice>
#include <HUpnpCore/HControlPoint>
#include <QtCore/QStringList>

namespace Solid
{
namespace Backends
{
namespace UPnP
{

    class UPnPControlPoint : public QObject
    {
        public:
            static UPnPControlPoint* acquireInstance();

            static void releaseInstance();

            Herqq::Upnp::HControlPoint* controlPoint();

            QStringList allDevices();

            virtual ~UPnPControlPoint();

        private:
            explicit UPnPControlPoint();

            static UPnPControlPoint* instance();

            static UPnPControlPoint* inst;

            static QMutex mutex;

            Herqq::Upnp::HControlPoint* m_controlPoint;
    };

}
}
}

#endif // SOLID_BACKENDS_UPNP_UPNPCONTROLPOINT_H
