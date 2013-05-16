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

#ifndef WINDEVICEMANAGER_H
#define WINDEVICEMANAGER_H

#include <solid/ifaces/devicemanager.h>


#include <QSet>
#include <QDebug>
#include <QMutex>

#include <windows.h>
#include <winioctl.h>

inline QString qGetLastError(ulong errorNummber = GetLastError())
{
    LPVOID error = NULL;
    size_t len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                               FORMAT_MESSAGE_FROM_SYSTEM |
                               FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL,
                               errorNummber,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                               (LPWSTR) &error ,
                               0, NULL );
    QString out = QString::fromWCharArray((wchar_t*)error,len).trimmed().append(" %1").arg(errorNummber);
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
    Q_OBJECT
public:
    WinDeviceManager(QObject *parent=0);
    ~WinDeviceManager();

    virtual QString udiPrefix() const;

    virtual QSet<Solid::DeviceInterface::Type> supportedInterfaces() const;

    virtual QStringList allDevices();

    virtual QStringList devicesFromQuery(const QString &parentUdi,
                                         Solid::DeviceInterface::Type type = Solid::DeviceInterface::Unknown);

    virtual QObject *createDevice(const QString &udi);

    static const WinDeviceManager *instance();




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

signals:
    void powerChanged();

private slots:
    void updateDeviceList();

private:
    static WinDeviceManager *m_instance;
    HWND m_windowID;
    QSet<QString> m_devices;
    QStringList m_devicesList;
    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void promoteAddedDevice(const QSet<QString> &udi);
    void promoteRemovedDevice(const QSet<QString> &udi);

    void promotePowerChange();

    template< class INFO, class QUERY>
    static void getDeviceInfoPrivate(const QString &devName, int code,INFO *info,size_t size, QUERY *query = NULL)
    {
        static QMutex mutex;
        QMutexLocker lock(&mutex);
        Q_ASSERT(!devName.isNull());
        wchar_t deviceNameBuffer[MAX_PATH];
        QString dev = QLatin1String("\\\\.\\") + devName;
        deviceNameBuffer[dev.toWCharArray(deviceNameBuffer)] = 0;

        HANDLE handle = ::CreateFile(deviceNameBuffer, 0, FILE_SHARE_WRITE|FILE_SHARE_WRITE, NULL  , OPEN_EXISTING , 0, NULL);
        if(handle == INVALID_HANDLE_VALUE)
        {
            qWarning() <<" Invalid Handle" << devName << "reason:" << qGetLastError() << " is probaply a subst path or more seriously there is  bug!";
            return;
        }

        DWORD bytesReturned =  0;

        if(::DeviceIoControl(handle, code, query, sizeof(QUERY), info, size, &bytesReturned, NULL) != TRUE)
        {
            //TODO:cleanup
            ulong err = GetLastError();
            if(err == ERROR_ACCESS_DENIED)//we are probably a cd drive and have to get read acces
            {
                ::CloseHandle(handle);
                handle = ::CreateFile(deviceNameBuffer, GENERIC_READ, FILE_SHARE_WRITE|FILE_SHARE_WRITE, NULL  , OPEN_EXISTING , 0, NULL);
                if(handle == INVALID_HANDLE_VALUE)
                {

                    err = GetLastError();
                    if(err == ERROR_ACCESS_DENIED)
                    {
                        //we would need admin rights
                        //                        DebugBreak();
                        qWarning() << "we would need admin rights" << dev << "reason:" << qGetLastError(err);
                    }
                    else
                    {
                        qWarning() << "Invalid Handle" << dev << "reason:" << qGetLastError(err) << " this should not happen.";
                    }
                    return;
                }
                if(::DeviceIoControl(handle, code, query, sizeof(QUERY), info, size, &bytesReturned, NULL) == TRUE)
                {
                    ::CloseHandle(handle);
                    return;
                }
                err = GetLastError();
                if(err == ERROR_NOT_READY)
                {
                    //the drive is a cd drive with no disk
                    ::CloseHandle(handle);
                    return;
                }
            }
#if 0
            ::CloseHandle(handle);
            qFatal("Failed to query %s reason: %s", qPrintable(dev), qPrintable(qGetLastError(err)));
#else
            qWarning() << "Failed to query" << dev << "reason:" << qGetLastError(err);
#endif
        }
        ::CloseHandle(handle);
    }

};

}
}
}
#endif // WINDEVICEMANAGER_H
