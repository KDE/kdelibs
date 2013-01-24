/*
    Copyright 2012-2013 Patrick von Reth <vonreth@kde.org>

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
#include <QDebug>
#include <QMutex>

#include <windows.h>
#include <winioctl.h>



inline QString qGetLastError()
{
    LPVOID error = NULL;

    size_t len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                               FORMAT_MESSAGE_FROM_SYSTEM |
                               FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL,
                               GetLastError(),
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                               (LPWSTR) &error ,
                               0, NULL );

    QString out = QString::fromWCharArray((wchar_t*)error,len).trimmed();
    LocalFree(error);
    return out;
}

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





    template< class INFO, class QUERY>
    static INFO getDeviceInfo(const QString &devName, int code, QUERY *query = NULL)
    {
        INFO info;
        ZeroMemory(&info,sizeof(INFO));
        getDeviceInfoPrivate(devName,code,&info,sizeof(INFO),query);
        return info;
    }

    template<class QUERY>
    static void getDeviceInfo(const QString &devName, int code, char *out, size_t outSize, QUERY *query = NULL)
    {
        ZeroMemory(out,outSize);
        getDeviceInfoPrivate(devName,code,out,outSize,query);
    }


private:
    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;

    template< class INFO, class QUERY>
    static void getDeviceInfoPrivate(const QString &devName, int code,INFO *info,size_t size, QUERY *query = NULL)
    {
        wchar_t buff[MAX_PATH];
        QString dev = QString("\\\\.\\%1").arg(devName);
        buff[dev.toWCharArray(buff)] = 0;
        //    qDebug()<<"querying "<<dev;
        HANDLE h = ::CreateFile(buff, 0, FILE_SHARE_WRITE|FILE_SHARE_WRITE, NULL  , OPEN_EXISTING , 0, NULL);
        DWORD bytesReturned =  0;

        if(::DeviceIoControl(h, code, query, sizeof(QUERY), info, size, &bytesReturned, NULL) != TRUE)
        {
            //            qFatal("Failed to query %s reason: %s",qPrintable(dev),qPrintable(qGetLastError()));
            qWarning()<<"Failed to query"<<dev<<"reason:"<<qGetLastError();
        }
        ::CloseHandle(h);
    }

};

}
}
}
#endif // WINDEVICEMANAGER_H
