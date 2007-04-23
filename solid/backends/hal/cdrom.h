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

#ifndef CDROM_H
#define CDROM_H

#include <solid/ifaces/cdrom.h>
#include "storage.h"

class Cdrom : public Storage, virtual public Solid::Ifaces::Cdrom
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Cdrom)

public:
    Cdrom(HalDevice *device);
    virtual ~Cdrom();

    virtual Solid::Cdrom::MediumTypes supportedMedia() const;
    virtual int readSpeed() const;
    virtual int writeSpeed() const;
    virtual QList<int> writeSpeeds() const;

Q_SIGNALS:
    void ejectPressed();

private Q_SLOTS:
    void slotCondition(const QString &name, const QString &reason);
};

#endif
