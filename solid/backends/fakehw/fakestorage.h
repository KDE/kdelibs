/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>

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

#ifndef FAKESTORAGE_H
#define FAKESTORAGE_H

#include "fakeblock.h"
#include <solid/ifaces/storage.h>

class FakeStorage : public FakeBlock, virtual public Solid::Ifaces::Storage
{
    Q_OBJECT
    Q_INTERFACES( Solid::Ifaces::Storage )
    Q_PROPERTY( Bus bus READ bus )
    Q_PROPERTY( DriveType driveType READ driveType )
    Q_PROPERTY( bool removable READ isRemovable )
    Q_PROPERTY( bool ejectRequired READ isEjectRequired )
    Q_PROPERTY( bool hotpluggable READ isHotpluggable )
    Q_PROPERTY( bool mediaCheckEnabled READ isMediaCheckEnabled )
    Q_PROPERTY( QString vendor READ vendor )
    Q_PROPERTY( QString product READ product )
    Q_ENUMS( Bus DriveType )

public:
    FakeStorage( FakeDevice *device );
    ~FakeStorage();

public Q_SLOTS:
    virtual Bus bus() const;
    virtual DriveType driveType() const;

    virtual bool isRemovable() const;
    virtual bool isEjectRequired() const;
    virtual bool isHotpluggable() const;
    virtual bool isMediaCheckEnabled() const;

    virtual QString vendor() const;
    virtual QString product() const;
};

#endif
