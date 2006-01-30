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

#ifndef FAKEMANAGER_H
#define FAKEMANAGER_H

#include <kdehw/ifaces/devicemanager.h>
#include <QVariant>
#include <QStringList>

class FakeDevice;

class FakeManager : public KDEHW::Ifaces::DeviceManager
{
    Q_OBJECT

public:
    FakeManager();
    virtual ~FakeManager();

    virtual QStringList allDevices();
    virtual bool deviceExists( const QString &udi );

    virtual KDEHW::Ifaces::Device *createDevice( const QString &udi );

    virtual QStringList findDeviceStringMatch( const QString &key, const QString &value );
    virtual QStringList findDeviceByCapability( const KDEHW::Ifaces::Capability::Type &capability );


    FakeDevice *newDevice( const QString &udi );
    FakeDevice *findDevice( const QString &udi );
    void deleteDevice( const QString &udi );

private:
    QMap<QString, FakeDevice*> m_devices;

    friend class FakeDevice;
};

#endif
