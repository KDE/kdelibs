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

#include "winopticaldisc.h"
#include "winopticaldrive.h"


#ifdef HEAVE_DRIVER_KIT
#include <ntddcdrm.h>
#include <ntddmmc.h>
#endif

using namespace Solid::Backends::Win;

WinOpticalDisc::WinOpticalDisc(WinDevice *device) :
    WinStorageVolume(device),
    m_isRewritable(false)
{
    //TODO: blueray etc
    #ifdef HEAVE_DRIVER_KIT
    QMap<ulong,MediaProfiles> profiles = MediaProfiles::profiles(WinBlock::driveLetterFromUdi(m_device->udi()));

    if(profiles[ProfileCdRecordable].active)
    {
        m_discType = Solid::OpticalDisc::CdRecordable;
    }else if(profiles[ProfileCdRewritable].active)
    {
        m_discType = Solid::OpticalDisc::CdRewritable;
        m_isRewritable =  true;
    }
    else if(profiles[ProfileCdrom].active)
    {
        m_discType = Solid::OpticalDisc::CdRom;
    }
    else if(profiles[ProfileDvdRecordable].active)
    {
        m_discType = Solid::OpticalDisc::DvdRecordable;
    }
    else if(profiles[ProfileDvdRewritable].active)
    {
        m_discType = Solid::OpticalDisc::DvdRewritable;
        m_isRewritable =  true;
    }
    else if(profiles[ProfileDvdRom].active)
    {
        m_discType = Solid::OpticalDisc::DvdRom;
    }
    else
    {
        m_discType = Solid::OpticalDisc::UnknownDiscType;
    }
#endif


}

WinOpticalDisc::~WinOpticalDisc()
{
}

Solid::OpticalDisc::ContentTypes WinOpticalDisc::availableContent() const
{
    return Solid::OpticalDisc::NoContent;
}

Solid::OpticalDisc::DiscType WinOpticalDisc::discType() const
{
    return m_discType;
}

bool WinOpticalDisc::isAppendable() const
{
    return false;
}

bool WinOpticalDisc::isBlank() const
{

    wchar_t dLetter[MAX_PATH];
    int dLetterSize = WinBlock::driveLetterFromUdi(m_device->udi()).toWCharArray(dLetter);
    dLetter[dLetterSize] = (wchar_t)'\\';
    dLetter[dLetterSize+1] = 0;

    ULARGE_INTEGER sizeTotal;
    ULARGE_INTEGER sizeFree;
    if(GetDiskFreeSpaceEx(dLetter,&sizeFree,&sizeTotal,NULL) == TRUE)
    {
        return sizeFree.QuadPart>0 && sizeTotal.QuadPart == 0;
    }
    //FIXME: the call will fail on a blank cd, and inf there is no cd, but if we got a disc type we could guess that it is a blank cd
    return m_discType != Solid::OpticalDisc::UnknownDiscType ;
}

bool WinOpticalDisc::isRewritable() const
{
    return m_isRewritable;
}

qulonglong WinOpticalDisc::capacity() const
{
    return size();
}

#include "winopticaldisc.moc"
