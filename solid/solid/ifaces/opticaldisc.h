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

#ifndef KDEHW_IFACES_OPTICALDISC_H
#define KDEHW_IFACES_OPTICALDISC_H

#include <QFlags>
#include <kdehw/ifaces/volume.h>

namespace KDEHW
{
namespace Ifaces
{
    class OpticalDisc : virtual public Volume
    {
    public:
        virtual ~OpticalDisc() {}

        static Type type() { return Capability::OpticalDisc; }

        enum ContentType { Audio, Data, VideoCd, SuperVideoCd, VideoDvd };
        Q_DECLARE_FLAGS( ContentTypes, ContentType )

        enum DiscType { CdRom, CdRecordable, CdRewritable, DvdRom, DvdRam,
                        DvdRecordable, DvdRewritable,
                        DvdPlusRecordable, DvdPlusRewritable, Unknown };

        virtual ContentTypes availableContent() const = 0;
        virtual DiscType type() const = 0;
        virtual bool isAppendable() const = 0;
        virtual bool isBlank() const = 0;
        virtual bool isRewritable() const = 0;
        virtual long capacity() const = 0;
    };

    Q_DECLARE_OPERATORS_FOR_FLAGS( OpticalDisc::ContentTypes )
}
}

#endif
