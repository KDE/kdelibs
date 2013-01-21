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

#include "winstoragevolume.h"

#include "windevicemanager.h"

#include <windows.h>

#include <iostream>

using namespace Solid::Backends::Win;

WinStorageVolume::WinStorageVolume(WinDevice *device)
    : WinBlock(device)
{
    updateCache();
}

WinStorageVolume::~WinStorageVolume()
{
}

bool WinStorageVolume::isIgnored() const
{
    return m_device->driveLetter().isNull();
}

Solid::StorageVolume::UsageType WinStorageVolume::usage() const
{
    return Solid::StorageVolume::FileSystem;//TODO:???
}


void WinStorageVolume::updateCache()
{
    wchar_t label[MAX_PATH];
    wchar_t fs[MAX_PATH];
    DWORD serial;
    DWORD flags;
    //TODO:get correct name
    wchar_t dLetter[MAX_PATH];
    int dLetterSize = m_device->driveLetter().toWCharArray(dLetter);
    dLetter[dLetterSize] = (wchar_t)'\\';
    dLetter[dLetterSize+1] = 0;

    GetVolumeInformation(dLetter,label,MAX_PATH,&serial,NULL,&flags,fs,MAX_PATH);
    m_label = QString::fromWCharArray(label);
    m_fs = QString::fromWCharArray(fs);
    m_uuid = QString::number(serial);


    ULARGE_INTEGER size;
    GetDiskFreeSpaceEx(dLetter,NULL,&size,NULL);
    m_size = size.QuadPart;
}


QString WinStorageVolume::fsType() const
{
    return m_fs;
}

QString WinStorageVolume::label() const
{
    return m_label;
}

QString WinStorageVolume::uuid() const
{
    return m_uuid;
}

qulonglong WinStorageVolume::size() const
{
    return m_size;
}

QString WinStorageVolume::encryptedContainerUdi() const
{
    return QString();
}

#include "winstoragevolume.moc"



