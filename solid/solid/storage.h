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

#ifndef KDEHW_STORAGE_H
#define KDEHW_STORAGE_H

#include <kdehw/block.h>
#include <kdehw/ifaces/enums.h>

namespace KDEHW
{
    namespace Ifaces
    {
        class Storage;
    }

    class Storage : public Block, public Ifaces::Enums::Storage
    {
        Q_OBJECT
    public:
        Storage( Ifaces::Storage *iface, QObject *parent = 0 );
        virtual ~Storage();

        static Type type() { return Capability::Storage; }

        Bus bus() const;
        DriveType driveType() const;

        bool isRemovable() const;
        bool isEjectRequired() const;
        bool isHotpluggable() const;
        bool isMediaCheckEnabled() const;

        QString vendor() const;
        QString product() const;

    private:
        class Private;
        Private *d;
    };
}

#endif
