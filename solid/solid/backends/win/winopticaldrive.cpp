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




using namespace Solid::Backends::Win;

WinOpticalDrive::WinOpticalDrive(WinDevice *device) :
    WinStorageDrive(device)
{
    QMap<ulong,MediaProfiles> profiles = MediaProfiles::profiles(m_device->driveLetter());
    foreach(const MediaProfiles p,profiles.values())
    {
        m_supportedTypes |= p.type;
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


MediaProfiles::MediaProfiles() :
    profile(0),
    type(0),
    active(false)
{
}

MediaProfiles::MediaProfiles(ulong profile, Solid::OpticalDrive::MediumTypes type, QString name):
    profile(profile),
    type(type),
    name(name),
    active(false)
{
}

MediaProfiles::MediaProfiles(FEATURE_DATA_PROFILE_LIST_EX *feature) :
    profile(0),
    type(0),
    active(false)
{
    ulong val =  (feature->ProfileNumber[0] << 8 | feature->ProfileNumber[1] << 0);
    MediaProfiles p =  MediaProfiles::getProfile(val);
    if(!p.isNull())
    {
        profile = p.profile;
        type = p.type;
        name = p.name;
        active =  feature->Current;
    }
}

bool MediaProfiles::isNull()
{
    return name.isNull();
}

QMap<ulong,MediaProfiles> MediaProfiles::profiles(const QString &drive)
{

    //thx to http://www.adras.com/Determine-optical-drive-type-and-capabilities.t6826-144-1.html

    QMap<ulong,MediaProfiles> out;
    size_t buffSize = 1024;
    char buffer[buffSize];
    GET_CONFIGURATION_IOCTL_INPUT input;
    ZeroMemory(&input,sizeof(GET_CONFIGURATION_IOCTL_INPUT));
    input.Feature = FeatureProfileList;
    input.RequestType = SCSI_GET_CONFIGURATION_REQUEST_TYPE_ALL;

    WinDeviceManager::getDeviceInfo<GET_CONFIGURATION_IOCTL_INPUT>(drive,IOCTL_CDROM_GET_CONFIGURATION,buffer,buffSize,&input);

    GET_CONFIGURATION_HEADER *info = (GET_CONFIGURATION_HEADER*)buffer;
    FEATURE_DATA_PROFILE_LIST* profile = (FEATURE_DATA_PROFILE_LIST*)info->Data;
    FEATURE_DATA_PROFILE_LIST_EX* feature = profile->Profiles;
    for(int i = 0;i<profile->Header.AdditionalLength/4;++feature,++i)
    {
        MediaProfiles p = MediaProfiles(feature);
        if(!p.isNull())
        {
            out.insert(p.profile,p);
        }
    }

    return out;
}

const MediaProfiles MediaProfiles::getProfile(ulong val)
{
#define AddProfile(profile,type) profiles.insert(profile,MediaProfiles(profile,type,#profile))
    static QMap<ulong,MediaProfiles> profiles;
    if(profiles.isEmpty())
    {
        AddProfile(ProfileCdrom,0);
        AddProfile(ProfileCdRecordable,Solid::OpticalDrive::Cdr);
        AddProfile(ProfileCdRewritable,Solid::OpticalDrive::Cdrw);
        AddProfile(ProfileDvdRom,Solid::OpticalDrive::Dvd);
        AddProfile(ProfileDvdRecordable,Solid::OpticalDrive::Dvdr);
        AddProfile(ProfileDvdRewritable,Solid::OpticalDrive::Dvdrw);
        AddProfile(ProfileDvdRam,Solid::OpticalDrive::Dvdram);
        AddProfile(ProfileDvdPlusR,Solid::OpticalDrive::Dvdplusr);
        AddProfile(ProfileDvdPlusRW,Solid::OpticalDrive::Dvdplusrw);
        AddProfile(ProfileDvdPlusRDualLayer,Solid::OpticalDrive::Dvdplusdl);
        AddProfile(ProfileDvdPlusRWDualLayer,Solid::OpticalDrive::Dvdplusdlrw);
        AddProfile(ProfileBDRom,Solid::OpticalDrive::Bd);
        AddProfile(ProfileBDRRandomWritable,Solid::OpticalDrive::Bdr);
        AddProfile(ProfileBDRSequentialWritable,Solid::OpticalDrive::Bdr);
        AddProfile(ProfileBDRewritable,Solid::OpticalDrive::Bdre);
        AddProfile(ProfileHDDVDRom,Solid::OpticalDrive::HdDvd);
        AddProfile(ProfileHDDVDRecordable,Solid::OpticalDrive::HdDvdr);
        AddProfile(ProfileHDDVDRewritable,Solid::OpticalDrive::HdDvdrw);
    }
    return profiles[val];
}

#include "winopticaldrive.moc"
