/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef KDEHW_IFACES_ENUMS_H
#define KDEHW_IFACES_ENUMS_H

#include <QFlags>

namespace KDEHW
{
namespace Ifaces
{
namespace Enums
{
    struct Device
    {
        /**
         * PropertyModified : A property value has changed in the device
         * PropertyAdded : A new property has been added to the device
         * PropertyRemoved : A property has been removed from the device
         */
        enum PropertyChange { PropertyModified, PropertyAdded, PropertyRemoved };
    };

    struct Capability
    {
        enum Type { Unknown = 0, Processor = 1, Block = 2,
                    Storage = 4, Cdrom = 8,
                    Volume = 16, OpticalDisc = 32,
                    Display = 64 };
        Q_DECLARE_FLAGS( Types, Type );
    };
    Q_DECLARE_OPERATORS_FOR_FLAGS( Capability::Types )

    struct Storage
    {
        enum Bus { Ide, Usb, Ieee1394, Scsi, Sata, Platform };
        enum DriveType { HardDisk, CdromDrive, Floppy, Tape, CompactFlash, MemoryStick, SmartMedia, SdMmc };
    };

    struct Cdrom
    {
        enum MediumType { Cdr, Cdrw, Dvd, Dvdr, Dvdrw, Dvdram, Dvdplusr, Dvdplusrw, Dvdplusdl };
        Q_DECLARE_FLAGS( MediumTypes, MediumType );
    };
    Q_DECLARE_OPERATORS_FOR_FLAGS( Cdrom::MediumTypes )

    struct Volume
    {
        enum UsageType { FileSystem, PartitionTable, Raid, Other, Unused };
    };

    struct OpticalDisc
    {
        enum ContentType { Audio, Data, VideoCd, SuperVideoCd, VideoDvd };
        enum DiscType { CdRom, CdRecordable, CdRewritable, DvdRom, DvdRam,
                        DvdRecordable, DvdRewritable,
                        DvdPlusRecordable, DvdPlusRewritable, UnknownDiscType };
        Q_DECLARE_FLAGS( ContentTypes, ContentType )
    };
    Q_DECLARE_OPERATORS_FOR_FLAGS( OpticalDisc::ContentTypes )

    struct Display
    {
        /**
         * Lcd: Display is a LCD (Liquid Crystal Display) panel.
         * Crt: Display is a CRT (Cathode Ray Tube) tube.
         * TvOut: Display is a TV Out port.
         * UnknowDisplayType: Display type is unknow.
         */
        enum DisplayType{ Lcd, Crt, TvOut, UnknowDisplayType };
    };
}
}
}

#endif
