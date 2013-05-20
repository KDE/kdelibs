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

#include <solid/deviceinterface.h>
#include "windevicemanager.h"
#include "windevice.h"
#include "winprocessor.h"
#include "winblock.h"
#include "winbattery.h"
#include "winacadapter.h"

#include <dbt.h>

#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QApplication>


using namespace Solid::Backends::Win;

WinDeviceManager *WinDeviceManager::m_instance = NULL;

WinDeviceManager::WinDeviceManager(QObject *parent)
    :DeviceManager(parent)
{
    if(m_instance)
        qFatal("There are multiple WinDeviceManager instances");
    m_instance = this;
    m_supportedInterfaces << Solid::DeviceInterface::GenericInterface
                             //                          << Solid::DeviceInterface::Block
                          << Solid::DeviceInterface::StorageAccess
                          << Solid::DeviceInterface::StorageDrive
                          << Solid::DeviceInterface::OpticalDrive
                          << Solid::DeviceInterface::StorageVolume
                          << Solid::DeviceInterface::OpticalDisc
                          << Solid::DeviceInterface::Processor
                          << Solid::DeviceInterface::Battery;


    updateDeviceList();

    wchar_t title[] = L"KDEWinDeviceManager";

    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.lpfnWndProc	= WinDeviceManager::WndProc;
    wcex.hInstance		= (HINSTANCE)::GetModuleHandle(NULL);
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
    wcex.lpszClassName	= title;
    if(RegisterClassEx(&wcex) ==0)
    {
        qWarning() << "Failed to initialize KDEWinDeviceManager we will be unable to detect device changes";
        return;
    }

    m_windowID = CreateWindow(title, title, WS_ICONIC, 0, 0,
                              CW_USEDEFAULT, 0, NULL, NULL, wcex.hInstance, NULL);
    if(m_windowID == NULL)
    {
        qWarning() << "Failed to initialize KDEWinDeviceManager we will be unable to detect device changes";
        return;
    }
    ShowWindow(m_windowID, SW_HIDE);
}

WinDeviceManager::~WinDeviceManager()
{
    PostMessage(m_windowID, WM_CLOSE, 0, 0);
    m_instance = NULL;
}

QString WinDeviceManager::udiPrefix() const
{
    return QString();
}


QSet<Solid::DeviceInterface::Type> Solid::Backends::Win::WinDeviceManager::supportedInterfaces() const
{
    return m_supportedInterfaces;
}

QStringList WinDeviceManager::allDevices()
{
    return m_devicesList;
}


QStringList WinDeviceManager::devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type)
{

    QStringList list;
    if (!parentUdi.isEmpty())
    {
        foreach(const QString &udi,allDevices()){
            WinDevice device(udi);
            if(device.type() == type && device.parentUdi() == parentUdi ){
                list << udi;
            }
        }
    } else if (type!=Solid::DeviceInterface::Unknown) {
        foreach(const QString &udi,allDevices()){
            WinDevice device(udi);
            if(device.queryDeviceInterface(type)){
                list << udi;
            }
        }
    } else {
        list << allDevices();
    }
    return list;

}


QObject *Solid::Backends::Win::WinDeviceManager::createDevice(const QString &udi)
{
    if (allDevices().contains(udi)) {
        return new WinDevice(udi);
    } else {
        return 0;
    }
}

const WinDeviceManager *WinDeviceManager::instance()
{
    return m_instance;
}

void WinDeviceManager::updateDeviceList()
{
    QSet<QString> devices = WinProcessor::getUdis();
    devices += WinBlock::getUdis();
    devices += WinBattery::getUdis();
    devices += WinAcadapter::getUdis();


    m_devices = devices;
    m_devicesList = m_devices.toList();
    qSort(m_devicesList);
}




LRESULT WinDeviceManager::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //some parts of the code are based on http://www.codeproject.com/Articles/119168/Hardware-Change-Detection
    switch(message)
    {
    case WM_DEVICECHANGE:
    {

        if ((wParam == DBT_DEVICEARRIVAL) || (wParam == DBT_DEVICEREMOVECOMPLETE))
        {
            DEV_BROADCAST_HDR* header = reinterpret_cast<DEV_BROADCAST_HDR*>(lParam);
            if (header->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {

                DEV_BROADCAST_VOLUME* devNot = reinterpret_cast<DEV_BROADCAST_VOLUME*>(lParam);
                switch(wParam)
                {

                case DBT_DEVICEREMOVECOMPLETE:
                {
                    QSet<QString> udis = WinBlock::getFromBitMask(devNot->dbcv_unitmask);
                    m_instance->promoteRemovedDevice(udis);
                }
                    break;
                case DBT_DEVICEARRIVAL:
                {
                    QSet<QString> udis = WinBlock::updateUdiFromBitMask(devNot->dbcv_unitmask);
                    m_instance->promoteAddedDevice(udis);
                }
                    break;
                }
                break;
            }
        }
    }
        break;
    case WM_POWERBROADCAST:
    {

    }
    break;
    case WM_DESTROY:
    {
        PostQuitMessage(0);
    }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void WinDeviceManager::promoteAddedDevice(const QSet<QString> &udi)
{
    QSet<QString> tmp = udi - m_devices;//dont report devices that are already known(cd drive)
    m_devices += tmp;
    m_devicesList = m_devices.toList();
    qSort(m_devicesList);
    foreach(const QString &s,tmp)
    {

        emit deviceAdded(s);
    }
}

void WinDeviceManager::promoteRemovedDevice(const QSet<QString> &udi)
{
    m_devices -= udi;
    m_devicesList = m_devices.toList();
    qSort(m_devicesList);
    foreach(const QString &s,udi)
    {
        emit deviceRemoved(s);
    }
}

void WinDeviceManager::promotePowerChange()
{
    emit powerChanged();
}



#include <windevicemanager.moc>

