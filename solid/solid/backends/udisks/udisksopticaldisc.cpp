/*  Copyright 2010  Michael Zanetti <mzanetti@kde.org>

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
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "udisksopticaldisc.h"

using namespace Solid::Backends::UDisks;

OpticalDisc::OpticalDisc(UDisksDevice *device)
    : UDisksStorageVolume(device)
{

}

OpticalDisc::~OpticalDisc()
{

}

qulonglong OpticalDisc::capacity() const
{
    return m_device->property("DeviceSize").toULongLong();
}

bool OpticalDisc::isRewritable() const
{
    //TODO: find this!
    return false;
}

bool OpticalDisc::isBlank() const
{
    return m_device->property("OpticalDiscIsBlank").toBool();
}

bool OpticalDisc::isAppendable() const
{
    return m_device->property("OpticalDiscIsAppendable").toBool();
}

Solid::OpticalDisc::DiscType OpticalDisc::discType() const
{
    QString type = m_device->property("DriveMedia").toString();

    if (type == "optical_cd")
    {
        return Solid::OpticalDisc::CdRom;
    }
    else if (type == "optical_cd_r")
    {
        return Solid::OpticalDisc::CdRecordable;
    }
    else if (type == "optical_cd_rw")
    {
        return Solid::OpticalDisc::CdRewritable;
    }
    else if (type == "optical_dvd")
    {
        return Solid::OpticalDisc::DvdRom;
    }
    else if (type == "optical_dvd_ram")
    {
        return Solid::OpticalDisc::DvdRam;
    }
    else if (type == "optical_dvd_r")
    {
        return Solid::OpticalDisc::DvdRecordable;
    }
    else if (type == "optical_dvd_rw")
    {
        return Solid::OpticalDisc::DvdRewritable;
    }
    else if (type == "optical_dvd_plus_r")
    {
        return Solid::OpticalDisc::DvdPlusRecordable;
    }
    else if (type == "optical_dvd_plus_rw")
    {
        return Solid::OpticalDisc::DvdPlusRewritable;
    }
    else if (type == "optical_dvd_plus_r_dl")
    {
        return Solid::OpticalDisc::DvdPlusRecordableDuallayer;
    }
    else if (type == "optical_dvd_plus_rw_dl")
    {
        return Solid::OpticalDisc::DvdPlusRewritableDuallayer;
    }
    else if (type == "optical_bd")
    {
        return Solid::OpticalDisc::BluRayRom;
    }
    else if (type == "optical_bd_r")
    {
        return Solid::OpticalDisc::BluRayRecordable;
    }
    else if (type == "optical_bd_re")
    {
        return Solid::OpticalDisc::BluRayRewritable;
    }
    else if (type == "optical_hddvd")
    {
        return Solid::OpticalDisc::HdDvdRom;
    }
    else if (type == "optical_hddvd_r")
    {
        return Solid::OpticalDisc::HdDvdRecordable;
    }
    else if (type == "optical_hddvd_rw")
    {
        return Solid::OpticalDisc::HdDvdRewritable;
    }
    else
    {
        return Solid::OpticalDisc::UnknownDiscType;
    }

}

Solid::OpticalDisc::ContentTypes OpticalDisc::availableContent() const
{
    Solid::OpticalDisc::ContentTypes content;

    if (m_device->property("OpticalDiscNumAudioTracks").toUInt() > 0) {
        content |= Solid::OpticalDisc::Audio;
    }
    if (m_device->property("OpticalDiscNumTracks").toUInt() > 0) {
        content |= Solid::OpticalDisc::Data;
    }
    //TODO: find those!
/*    if (m_device->property("")) {
        content |= Solid::OpticalDisc::VideoCd;
    }
    if (m_device->property("")) {
        content |= Solid::OpticalDisc::SuperVideoCd;
    }
    if (m_device->property("")) {
        content |= Solid::OpticalDisc::VideoDvd;
    }*/
    return content;

}

