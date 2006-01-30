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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef FAKEDEVICE_H
#define FAKEDEVICE_H

#include <kdehw/ifaces/device.h>
#include <kdehw/ifaces/capability.h>

#include "fakeprocessor.h"

#include <QStringList>

class FakeManager;

class FakeDevice : public KDEHW::Ifaces::Device
{
    Q_OBJECT

public:
    FakeDevice(const QString &udi, FakeManager *manager);
    virtual ~FakeDevice();

    virtual QString udi() const;
    virtual QString parentUdi() const;
    void setParent( const QString &udi );

    virtual QString vendor() const;
    virtual QString product() const;

    virtual bool setProperty( const QString &key, const QVariant &value );
    virtual QVariant property( const QString &key ) const;

    virtual QMap<QString, QVariant> allProperties() const;

    virtual bool removeProperty( const QString &key );
    virtual bool propertyExists( const QString &key ) const;

    virtual bool addCapability( const KDEHW::Ifaces::Capability::Type &capability );
    virtual bool queryCapability( const KDEHW::Ifaces::Capability::Type &capability ) const;
    virtual KDEHW::Ifaces::Capability *asCapability( const KDEHW::Ifaces::Capability::Type &capability );

    virtual bool lock(const QString &reason);
    virtual bool unlock();
    virtual bool isLocked() const;
    virtual QString lockReason() const;

    void raiseCondition( const QString &condition, const QString &reason );

    bool isBroken();
    void setBroken( bool broken );

private:
    FakeManager *m_manager;
    QString m_udi;
    QString m_parent;
    KDEHW::Ifaces::Capability::Types m_capabilities;
    QMap<QString, QVariant> m_data;
    bool m_brokenDevice;
    bool m_locked;
    QString m_lockReason;
    FakeProcessor *m_processor;
};

#endif
