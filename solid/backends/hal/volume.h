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

#ifndef VOLUME_H
#define VOLUME_H

#include <solid/ifaces/volume.h>
#include "block.h"

class Volume : public Block, virtual public Solid::Ifaces::Volume
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Volume)

public:
    Volume(HalDevice *device);
    virtual ~Volume();

    virtual bool isIgnored() const;
    virtual bool isMounted() const;
    virtual QString mountPoint() const;
    virtual Solid::Volume::UsageType usage() const;
    virtual QString fsType() const;
    virtual QString label() const;
    virtual QString uuid() const;
    virtual qulonglong size() const;

    virtual KJob *mount();
    virtual KJob *unmount();
    virtual KJob *eject();

Q_SIGNALS:
    void mountStateChanged(bool newState);

private Q_SLOTS:
    void slotPropertyChanged(const QMap<QString,int> &changes);
};

#endif
