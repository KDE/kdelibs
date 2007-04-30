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

#ifndef FAKEVOLUME_H
#define FAKEVOLUME_H

#include "fakeblock.h"
#include <solid/ifaces/volume.h>

class FakeVolume : public FakeBlock, virtual public Solid::Ifaces::Volume
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Volume)

public:
    explicit FakeVolume(FakeDevice *device);
    virtual ~FakeVolume();

public Q_SLOTS:
    virtual bool isIgnored() const;
    virtual bool isMounted() const;
    virtual QString mountPoint() const;
    virtual Solid::Volume::UsageType usage() const;
    virtual QString fsType() const;
    virtual QString label() const;
    virtual QString uuid() const;
    virtual qulonglong size() const;

public:
    virtual void mount(QObject *receiver, const char *member);
    virtual void unmount(QObject *receiver, const char *member);
    virtual void eject(QObject *receiver, const char *member);

public Q_SLOTS:
    QString createMountJob();
    QString createUnmountJob();
    QString createEjectJob();

Q_SIGNALS:
    void mountStateChanged(bool newState);
};

#endif
