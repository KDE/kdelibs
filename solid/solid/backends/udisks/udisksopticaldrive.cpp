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

#include "udisksopticaldrive.h"

using namespace Solid::Backends::UDisks;

UDisksOpticalDrive::UDisksOpticalDrive(UDisksDevice *device)
    : UDisksStorageDrive(device)//, m_ejectInProgress(false)
{
  // TODO: ...
/*    connect(device, SIGNAL(conditionRaised(const QString &, const QString &)),
             this, SLOT(slotCondition(const QString &, const QString &)));
    m_device->connectActionSignal("ejectRequested",  this, SLOT(slotEjectRequested()));
    m_device->connectActionSignal("ejectDone",  this, SLOT(slotEjectDone(int, QDBusVariant, const QString&)));*/
}

UDisksOpticalDrive::~UDisksOpticalDrive()
{

}

bool UDisksOpticalDrive::eject()
{
// TODO
}

QList< int > UDisksOpticalDrive::writeSpeeds() const
{
    // TODO
    return QList<int>();
}

int UDisksOpticalDrive::writeSpeed() const
{
    // TODO
}

int UDisksOpticalDrive::readSpeed() const
{
    // TODO
}

Solid::OpticalDrive::MediumTypes UDisksOpticalDrive::supportedMedia() const
{
    QStringList compatibleDiscs = m_device->property("DriveMediaCompatibility").toStringList();
    
    Solid::OpticalDrive::MediumTypes types = 0;

    if (compatibleDiscs.contains("optical_cd"))
    {
        types |= Solid::OpticalDrive::Cdr; // TODO: No Solid::OpticalDrive::Cd ?
    }
    if (compatibleDiscs.contains("optical_cd_r"))
    {
        types |= Solid::OpticalDrive::Cdr;
    }
    if (compatibleDiscs.contains("optical_cd_rw"))
    {
        types |= Solid::OpticalDrive::Cdrw;
    }
    if (compatibleDiscs.contains("optical_dvd"))
    {
        types |= Solid::OpticalDrive::Dvd;
    }
    if (compatibleDiscs.contains("optical_dvd_ram"))
    {
        types |= Solid::OpticalDrive::Dvdram;
    }
    if (compatibleDiscs.contains("optical_dvd_r"))
    {
        types |= Solid::OpticalDrive::Dvdr;
    }
    if (compatibleDiscs.contains("optical_dvd_rw"))
    {
        types |= Solid::OpticalDrive::Dvdrw;
    }
    if (compatibleDiscs.contains("optical_dvd_plus_r"))
    {
        types |= Solid::OpticalDrive::Dvdplusr;
    }
    if (compatibleDiscs.contains("optical_dvd_plus_rw"))
    {
        types |= Solid::OpticalDrive::Dvdplusrw;
    }
    if (compatibleDiscs.contains("optical_dvd_plus_r_dl"))
    {
        types |= Solid::OpticalDrive::Dvdplusdl;
    }
    if (compatibleDiscs.contains("optical_dvd_plus_rw_dl"))
    {
        types |= Solid::OpticalDrive::Dvdplusdlrw;
    }
    if (compatibleDiscs.contains("optical_bd"))
    {
        types |= Solid::OpticalDrive::Bd;
    }
    if (compatibleDiscs.contains("optical_bd_r"))
    {
        types |= Solid::OpticalDrive::Bdr;
    }
    if (compatibleDiscs.contains("optical_bd_re"))
    {
        types |= Solid::OpticalDrive::Bdre;
    }
    if (compatibleDiscs.contains("optical_hddvd"))
    {
        types |= Solid::OpticalDrive::HdDvd;
    }
    if (compatibleDiscs.contains("optical_hddvd_r"))
    {
        types |= Solid::OpticalDrive::HdDvdr;
    }
    if (compatibleDiscs.contains("optical_hddvd_rw"))
    {
        types |= Solid::OpticalDrive::HdDvdrw;
    }

    //TODO: Check all types (e.g. optical_mrw)
    return types;
}

