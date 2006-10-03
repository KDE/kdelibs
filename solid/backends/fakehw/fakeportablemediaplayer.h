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

#ifndef FAKEPORTABLEMEDIAPLAYER_H
#define FAKEPORTABLEMEDIAPLAYER_H

#include "fakecapability.h"
#include <solid/ifaces/portablemediaplayer.h>

class FakePortableMediaPlayer : public FakeCapability, virtual public Solid::Ifaces::PortableMediaPlayer
{
    Q_OBJECT
    Q_INTERFACES( Solid::Ifaces::PortableMediaPlayer )
    Q_PROPERTY( AccessType accessMethod READ accessMethod )
    Q_PROPERTY( QStringList outputFormats READ outputFormats )
    Q_PROPERTY( QStringList inputFormats READ inputFormats )
    Q_PROPERTY( QStringList playlistFormats READ playlistFormats )
    Q_ENUMS( AccessType )

public:
    explicit FakePortableMediaPlayer( FakeDevice *device );
    ~FakePortableMediaPlayer();

public Q_SLOTS:
    virtual AccessType accessMethod() const;
    virtual QStringList outputFormats() const;
    virtual QStringList inputFormats() const;
    virtual QStringList playlistFormats() const;
};

#endif
