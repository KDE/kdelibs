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

#ifndef KDEHW_IFACES_CDROM_H
#define KDEHW_IFACES_CDROM_H

#include <QList>
#include <kdehw/ifaces/storage.h>
#include <kdehw/ifaces/enums.h>

namespace KDEHW
{
namespace Ifaces
{
    class Cdrom : virtual public Storage, public Enums::Cdrom
    {
    public:
        virtual ~Cdrom();

        static Type type() { return Capability::Cdrom; }

        virtual MediumTypes supportedMedia() const = 0;
        virtual int readSpeed() const = 0;
        virtual int writeSpeed() const = 0;
        virtual QList<int> writeSpeeds() const = 0;

    protected:
    //signals:
        virtual void ejectPressed() = 0;
    };
}
}

#endif
