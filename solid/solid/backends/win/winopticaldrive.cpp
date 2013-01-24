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

#include "winopticaldrive.h"

#include <ntddcdrm.h>
#include <ntddmmc.h>

using namespace Solid::Backends::Win;

WinOpticalDrive::WinOpticalDrive(WinDevice *device) :
    WinStorageDrive(device)
{
    //for opticaldisc
    //    char buffer[1024];
    //    WinDeviceManager::getDeviceInfo(m_device->driveLetter(),IOCTL_STORAGE_GET_MEDIA_TYPES_EX,buffer,1024);

    //    GET_MEDIA_TYPES *info = (GET_MEDIA_TYPES*)buffer;
    //    qDebug()<<"cd"<<info->DeviceType;
    //    ulong characteristics = info->MediaInfo[0].DeviceSpecific.DiskInfo.MediaCharacteristics;
    //    switch(info->DeviceType)
    //    {
    //    case FILE_DEVICE_CD_ROM:
    //        if(characteristics == MEDIA_WRITE_ONCE)
    //        {
    //            m_supportedTypes = Solid::OpticalDrive::Cdr;
    //        }
    //        else if(characteristics == MEDIA_READ_WRITE)
    //        {
    //            m_supportedTypes = Solid::OpticalDrive::Cdrw;
    //        }
    //        break;
    //    case FILE_DEVICE_DVD:
    //        if(characteristics == MEDIA_WRITE_ONCE)
    //        {
    //            m_supportedTypes = Solid::OpticalDrive::Dvdr;
    //        }
    //        else if(characteristics == MEDIA_READ_WRITE)
    //        {
    //            m_supportedTypes = Solid::OpticalDrive::Dvdrw;
    //        }
    //        break;
    //    default:
    //        m_supportedTypes = 0;
    //    }

    //thx to http://www.adras.com/Determine-optical-drive-type-and-capabilities.t6826-144-1.html
    typedef struct _sPROFILES
    {
        DWORD dwProfileType;
        QString pProfileText;
    } sPROFILES;

#define STR(s) {(DWORD)s, QString(#s)}

    sPROFILES Profiles[] =
    {
        STR(ProfileInvalid),
        STR(ProfileNonRemovableDisk),
        STR(ProfileRemovableDisk),
        STR(ProfileMOErasable),
        STR(ProfileMOWriteOnce),
        STR(ProfileAS_MO),
        STR(ProfileCdrom),
        STR(ProfileCdRecordable),
        STR(ProfileCdRewritable),
        STR(ProfileDvdRom),
        STR(ProfileDvdRecordable),
        STR(ProfileDvdRam),
        STR(ProfileDvdRewritable),
        STR(ProfileDvdRWSequential),
        STR(ProfileDvdPlusRW),
        STR(ProfileDDCdrom),
        STR(ProfileDDCdRecordable),
        STR(ProfileDDCdRewritable),
    };

    size_t buffSize = 10024;
    char buffer[buffSize];
    GET_CONFIGURATION_IOCTL_INPUT input;
    input.Feature = FeatureProfileList;
    input.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_ALL;

    WinDeviceManager::getDeviceInfo<GET_CONFIGURATION_IOCTL_INPUT>(m_device->driveLetter(),IOCTL_CDROM_GET_CONFIGURATION,buffer,buffSize,&input);

    GET_CONFIGURATION_HEADER *info = (GET_CONFIGURATION_HEADER*)buffer;

    qDebug()<<info->DataLength;
    FEATURE_DATA_PROFILE_LIST* profile = (FEATURE_DATA_PROFILE_LIST*)info->Data;
    FEATURE_DATA_PROFILE_LIST_EX* feature = profile->Profiles;
    qDebug()<<"Got "<<profile->Header.AdditionalLength/4<<"supported typesw";
    for(int i = 0;i<profile->Header.AdditionalLength/4;++feature,++i)
    {
        for (DWORD dw = 0; dw < ARRAYSIZE(Profiles); dw++)
        {
            qDebug()<<"FOOO";
            if ( (FEATURE_PROFILE_TYPE)(Profiles[dw].dwProfileType) ==
                 (feature->ProfileNumber[0] << 8 |
                  feature->ProfileNumber[1] << 0))
            {
                qDebug()<<"Profile"<<Profiles[dw].pProfileText<< Profiles[dw].dwProfileType
                       <<(feature->Current?"Yes":"No");
            }
        }


    }
}

WinOpticalDrive::~WinOpticalDrive()
{
}

Solid::OpticalDrive::MediumTypes WinOpticalDrive::supportedMedia() const
{
    return m_supportedTypes;
}


bool WinOpticalDrive::eject()
{
    return true;
}


QList<int> WinOpticalDrive::writeSpeeds() const
{
    return QList<int>();
}


int WinOpticalDrive::writeSpeed() const
{
    return 0;
}


int WinOpticalDrive::readSpeed() const
{
    return 0;
}

#include "winopticaldrive.moc"
