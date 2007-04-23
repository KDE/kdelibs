/*  This file is part of the KDE project
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#ifndef DVBHW_H
#define DVBHW_H

#include <solid/ifaces/dvbhw.h>
#include "deviceinterface.h"

class DvbHw : public DeviceInterface, virtual public Solid::Ifaces::DvbHw
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::DvbHw)

public:
    DvbHw(HalDevice *device);
    virtual ~DvbHw();

    virtual QString device() const;
    virtual int deviceAdapter() const;
    virtual Solid::DvbHw::DeviceType deviceType() const;
    virtual int deviceIndex() const;

private:
    bool parseTypeIndex(Solid::DvbHw::DeviceType *type, int *index) const;
};

#endif
