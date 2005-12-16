/*  This file is part of the KDE project
    Copyright (C) 2005 Kevin Ottens <ervin@kde.org>

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

#ifndef HALDEVICE_H
#define HALDEVICE_H

#include "ifaces/device.h"

class HalDevicePrivate;

class HalDevice : public KDEHW::Ifaces::Device
{
    Q_OBJECT

public:
    HalDevice(const QString &udi);
    virtual ~HalDevice();

    virtual QString udi() const;

    virtual bool setProperty( const QString &key, const QVariant &value );
    virtual QVariant property( const QString &key ) const;

    virtual QMap<QString, QVariant> allProperties() const;

    virtual bool removeProperty( const QString &key );
    virtual bool propertyExists( const QString &key ) const;

    virtual bool addCapability( const QString &capability );
    virtual bool queryCapability( const QString &capability ) const;

    virtual bool lock(const QString &reason);
    virtual bool unlock();

private slots:
    void slotPropertyModified( const QString &key, bool added, bool removed );
    void slotCondition( const QString &condition, const QString &reason );

private:
    HalDevicePrivate *d;
};

#endif
