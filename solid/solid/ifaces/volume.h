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

#ifndef KDEHW_IFACES_VOLUME_H
#define KDEHW_IFACES_VOLUME_H

#include <kdehw/ifaces/block.h>

namespace KDEHW
{
namespace Ifaces
{
    class Volume : virtual public Block
    {
    public:
        virtual ~Volume() {}

        static Type type() { return Capability::Volume; }

        enum UsageType { FileSystem, PartitionTable, Raid, Other, Unused };

        virtual bool isIgnored() const = 0;
        virtual bool isMounted() const = 0;
        virtual QString mountPoint() const = 0;
        virtual UsageType usage() const = 0;
        virtual QString fsType() const = 0;
        virtual QString label() const = 0;

    protected:
    //signals:
        virtual void mount( QString device, QString mountPoint ) = 0;
        virtual void unmount( QString device, QString mountPoint, bool forced ) = 0;
    };
}
}

#endif
