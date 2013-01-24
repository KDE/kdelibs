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

QMap<ulong,Solid::OpticalDrive::MediumTypes> WinOpticalDrive::MediaProfiles::profileMap = QMap<ulong,Solid::OpticalDrive::MediumTypes>();


WinOpticalDrive::MediaProfiles Profiles[] =
{
    WinOpticalDrive::MediaProfiles(ProfileCdRecordable,Solid::OpticalDrive::Cdr),
    WinOpticalDrive::MediaProfiles(ProfileCdRewritable,Solid::OpticalDrive::Cdrw),
    WinOpticalDrive::MediaProfiles(ProfileDvdRom,Solid::OpticalDrive::Dvd),
    WinOpticalDrive::MediaProfiles(ProfileDvdRecordable,Solid::OpticalDrive::Dvdr),
    WinOpticalDrive::MediaProfiles(ProfileDvdRewritable,Solid::OpticalDrive::Dvdrw),
    WinOpticalDrive::MediaProfiles(ProfileDvdRam,Solid::OpticalDrive::Dvdram),
    WinOpticalDrive::MediaProfiles(ProfileDvdPlusR,Solid::OpticalDrive::Dvdplusr),
    WinOpticalDrive::MediaProfiles(ProfileDvdPlusRW,Solid::OpticalDrive::Dvdplusrw),
    WinOpticalDrive::MediaProfiles(ProfileDvdPlusRDualLayer,Solid::OpticalDrive::Dvdplusdl),
    WinOpticalDrive::MediaProfiles(ProfileDvdPlusRWDualLayer,Solid::OpticalDrive::Dvdplusdlrw),
    WinOpticalDrive::MediaProfiles(ProfileBDRom,Solid::OpticalDrive::Bd),
    WinOpticalDrive::MediaProfiles(ProfileBDRRandomWritable,Solid::OpticalDrive::Bdr),
    WinOpticalDrive::MediaProfiles(ProfileBDRSequentialWritable,Solid::OpticalDrive::Bdr),
    WinOpticalDrive::MediaProfiles(ProfileBDRewritable,Solid::OpticalDrive::Bdre),
    WinOpticalDrive::MediaProfiles(ProfileHDDVDRom,Solid::OpticalDrive::HdDvd),
    WinOpticalDrive::MediaProfiles(ProfileHDDVDRecordable,Solid::OpticalDrive::HdDvdr),
    WinOpticalDrive::MediaProfiles(ProfileHDDVDRewritable,Solid::OpticalDrive::HdDvdrw),

};


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


    size_t buffSize = 1024;
    char buffer[buffSize];
    GET_CONFIGURATION_IOCTL_INPUT input;
    ZeroMemory(&input,sizeof(GET_CONFIGURATION_IOCTL_INPUT));
    input.Feature = FeatureProfileList;
    input.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_ALL;

    WinDeviceManager::getDeviceInfo<GET_CONFIGURATION_IOCTL_INPUT>(m_device->driveLetter(),IOCTL_CDROM_GET_CONFIGURATION,buffer,buffSize,&input);

    GET_CONFIGURATION_HEADER *info = (GET_CONFIGURATION_HEADER*)buffer;
    FEATURE_DATA_PROFILE_LIST* profile = (FEATURE_DATA_PROFILE_LIST*)info->Data;
    FEATURE_DATA_PROFILE_LIST_EX* feature = profile->Profiles;
    for(int i = 0;i<profile->Header.AdditionalLength/4;++feature,++i)
    {
        ulong val =  (feature->ProfileNumber[0] << 8 | feature->ProfileNumber[1] << 0);
        if(MediaProfiles::profileMap.contains(val))
        {
            m_supportedTypes |= MediaProfiles::profileMap[val];
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
