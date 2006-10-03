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

#ifndef FAKECDROM_H
#define FAKECDROM_H

#include <solid/ifaces/cdrom.h>
#include "fakestorage.h"

class FakeCdrom : public FakeStorage, virtual public Solid::Ifaces::Cdrom
{
    Q_OBJECT
    Q_INTERFACES( Solid::Ifaces::Cdrom )
    Q_PROPERTY( MediumTypes supportedMedia READ supportedMedia )
    Q_PROPERTY( int readSpeed READ readSpeed )
    Q_PROPERTY( int writeSpeed READ writeSpeed )
    Q_PROPERTY( QList<int> writeSpeeds READ writeSpeeds )
    Q_ENUMS( MediumType )

public:
    explicit FakeCdrom( FakeDevice *device );
    virtual ~FakeCdrom();

public Q_SLOTS:
    virtual MediumTypes supportedMedia() const;
    virtual int readSpeed() const;
    virtual int writeSpeed() const;
    virtual QList<int> writeSpeeds() const;

Q_SIGNALS:
    void ejectPressed();
};

#endif
