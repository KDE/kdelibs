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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef KDEHW_IFACES_STORAGE_H
#define KDEHW_IFACES_STORAGE_H

#include <kdehw/ifaces/block.h>

namespace KDEHW
{
namespace Ifaces
{
    class Storage : virtual public Block
    {
    public:
        virtual ~Storage() {}

        static Type type() { return Capability::Storage; }

        enum Bus { Ide, Usb, Ieee1394, Scsi, Sata, Platform };
        enum DriveType { Disk, Cdrom, Floppy, Tape, CompactFlash, MemoryStick, SmartMedia, SdMmc };

        virtual Bus bus() const = 0;
        virtual DriveType driveType() const = 0;

        virtual bool isRemovable() const = 0;
        virtual bool isEjectRequired() const = 0;
        virtual bool isHotpluggable() const = 0;
        virtual bool isMediaCheckEnabled() const = 0;

        virtual QString vendor() const = 0;
        virtual QString product() const = 0;
    };
}
}

#endif
