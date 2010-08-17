/*  Copyright 2010  Michael Zanetti <mzanetti@kde.org>
              2010  Lukas Tinkl <ltinkl@redhat.com>

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
#include "udisks.h"
#include "udisksstorageaccess.h"
#include "udisksdevice.h"

using namespace Solid::Backends::UDisks;

UDisksOpticalDrive::UDisksOpticalDrive(UDisksDevice *device)
    : UDisksStorageDrive(device), m_ejectInProgress(false)
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
    if (m_ejectInProgress)
        return false;
    m_ejectInProgress = true;
    emit ejectRequested(m_device->udi());

    QDBusConnection c = QDBusConnection::systemBus();

    // check if the device is mounted and call umount if needed
    if (m_device->queryDeviceInterface(Solid::DeviceInterface::StorageAccess))
    {
        const UDisks::UDisksStorageAccess accessIface(const_cast<UDisksDevice *>(m_device));
        if (accessIface.isAccessible())
        {
            QDBusMessage msg = QDBusMessage::createMethodCall(UD_DBUS_SERVICE, m_device->udi(), UD_DBUS_INTERFACE_DISKS_DEVICE, "FilesystemUnmount");

            msg << QStringList();   // options, unused now

            c.call(msg, QDBus::NoBlock);
        }
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(UD_DBUS_SERVICE, m_device->udi(), UD_DBUS_INTERFACE_DISKS_DEVICE, "DriveEject");
    msg << QStringList();
    return c.callWithCallback(msg, this, SLOT(slotDBusReply(const QDBusMessage &)), SLOT(slotDBusError(const QDBusError &)));
}

void UDisksOpticalDrive::slotDBusReply(const QDBusMessage &/*reply*/)
{
    m_ejectInProgress = false;
    emit ejectDone(Solid::NoError, QVariant(), m_device->udi());
}

void UDisksOpticalDrive::slotDBusError(const QDBusError &error)
{
    m_ejectInProgress = false;
    emit ejectDone(Solid::UnauthorizedOperation, error.name()+": "+error.message(), m_device->udi());
}

QList< int > UDisksOpticalDrive::writeSpeeds() const
{
    // FIXME unsupported in DK
    return QList<int>();
}

int UDisksOpticalDrive::writeSpeed() const
{
    // FIXME unsupported in DK
    return 0;
}

int UDisksOpticalDrive::readSpeed() const
{
    // FIXME unsupported in DK
    return 0;
}

Solid::OpticalDrive::MediumTypes UDisksOpticalDrive::supportedMedia() const
{
    const QStringList mediaTypes = m_device->property("DriveMediaCompatibility").toStringList();
    Solid::OpticalDrive::MediumTypes supported;

    QMap<Solid::OpticalDrive::MediumType, QString> map;
    map[Solid::OpticalDrive::Cdr] = "optical_cd_r";
    map[Solid::OpticalDrive::Cdrw] = "optical_cd_rw";
    map[Solid::OpticalDrive::Dvd] = "optical_dvd";
    map[Solid::OpticalDrive::Dvdr] = "optical_dvd_r";
    map[Solid::OpticalDrive::Dvdrw] ="optical_dvd_rw";
    map[Solid::OpticalDrive::Dvdram] ="optical_dvd_ram";
    map[Solid::OpticalDrive::Dvdplusr] ="optical_dvd_plus_r";
    map[Solid::OpticalDrive::Dvdplusrw] ="optical_dvd_plus_rw";
    map[Solid::OpticalDrive::Dvdplusdl] ="optical_dvd_plus_r_dl";
    map[Solid::OpticalDrive::Dvdplusdlrw] ="optical_dvd_plus_rw_dl";
    map[Solid::OpticalDrive::Bd] ="optical_bd";
    map[Solid::OpticalDrive::Bdr] ="optical_bd_r";
    map[Solid::OpticalDrive::Bdre] ="optical_bd_re";
    map[Solid::OpticalDrive::HdDvd] ="optical_hddvd";
    map[Solid::OpticalDrive::HdDvdr] ="optical_hddvd_r";
    map[Solid::OpticalDrive::HdDvdrw] ="optical_hddvd_rw";
    // TODO add these to Solid
    //map[Solid::OpticalDrive::Mo] ="optical_mo";
    //map[Solid::OpticalDrive::Mr] ="optical_mrw";
    //map[Solid::OpticalDrive::Mrw] ="optical_mrw_w";

    foreach ( const Solid::OpticalDrive::MediumType & type, map.keys() )
    {
        if ( mediaTypes.contains( map[type] ) )
        {
            supported |= type;
        }
    }

    return supported;
}

