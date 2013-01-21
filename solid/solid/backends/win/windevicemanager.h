/*
    Copyright 2012 Patrick von Reth <vonreth@kde.org>
    
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

#ifndef WINDEVICEMANAGER_H
#define WINDEVICEMANAGER_H

#include <solid/ifaces/devicemanager.h>


#include <QSet>


#include <windows.h>
#include <winioctl.h>


namespace Solid
{
namespace Backends
{
namespace Win
{

class WinDeviceManager : public Solid::Ifaces::DeviceManager
{
public:
    WinDeviceManager(QObject *parent=0);

    virtual QString udiPrefix() const;

    virtual QSet<Solid::DeviceInterface::Type> supportedInterfaces() const;

    virtual QStringList allDevices();

    virtual QStringList devicesFromQuery(const QString &parentUdi,
                                          Solid::DeviceInterface::Type type = Solid::DeviceInterface::Unknown);

    virtual QObject *createDevice(const QString &udi);


    template <class  INFO,class QUERY>
    static INFO getDeviceInfo(QString devName,int code, QUERY *query)
    {
        wchar_t buff[MAX_PATH];
        QString dev = QString("\\\\.\\%1").arg(devName);
        buff[dev.toWCharArray(buff)] = 0;
        HANDLE h = ::CreateFile(buff, 0, 0, NULL, OPEN_EXISTING, 0, NULL);

        INFO info;
        ZeroMemory(&info,sizeof(info));

        DWORD bytesReturned =  0;

        ::DeviceIoControl(h, code, query, sizeof(*query), &info, sizeof(info), &bytesReturned, NULL);
        ::CloseHandle(h);
        return info;
    }

//    template <typename  INFO>
//    static INFO getDeviceInfo(QString devName,int code);


private:
    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;

};

}
}
}
#endif // WINDEVICEMANAGER_H
