/*  This file is part of the KDE project
    Copyright (C) 2006 Michaël Larouche <michael.larouche@kdemail.net>

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
#ifndef FAKECAPABILITY_H
#define FAKECAPABILITY_H

#include <QObject>
#include <solid/ifaces/capability.h>

#include "fakedevice.h"

class FakeCapability : public QObject, virtual public Solid::Ifaces::Capability
{
    Q_OBJECT
public:
    explicit FakeCapability( FakeDevice *device );
    ~FakeCapability();

protected:
    FakeDevice *fakeDevice() const
    {
        return m_device;
    }
    FakeDevice *fakeDevice()
    {
        return m_device;
    }

public:
    inline static QString toString( Solid::Capability::Type capability )
    {
        switch( capability )
        {
        case Solid::Capability::Processor:
            return "processor";
        case Solid::Capability::Block:
            return "block";
        case Solid::Capability::Storage:
            return "storage";
        case Solid::Capability::Cdrom:
            return "cdrom";
        case Solid::Capability::Volume:
            return "volume";
        case Solid::Capability::OpticalDisc:
            return "opticaldisc";
        case Solid::Capability::Camera:
            return "camera";
        case Solid::Capability::PortableMediaPlayer:
            return "portablemediaplayer";
        case Solid::Capability::NetworkHw:
            return "networkhw";
        case Solid::Capability::AcAdapter:
            return "acadapter";
        case Solid::Capability::Battery:
            return "battery";
        case Solid::Capability::Button:
            return "button";
        case Solid::Capability::Display:
            return "display";
        case Solid::Capability::AudioHw:
            return "audiohw";
        default:
            return QString();
        }
    }

    inline static Solid::Capability::Type fromString( const QString &capability )
    {
        if ( capability == "processor" )
            return Solid::Capability::Processor;
        else if ( capability == "block" )
            return Solid::Capability::Block;
        else if ( capability == "storage" )
            return Solid::Capability::Storage;
        else if ( capability == "cdrom" )
            return Solid::Capability::Cdrom;
        else if ( capability == "volume" )
            return Solid::Capability::Volume;
        else if ( capability == "opticaldisc" )
            return Solid::Capability::OpticalDisc;
        else if ( capability == "camera" )
            return Solid::Capability::Camera;
        else if ( capability == "portablemediaplayer" )
            return Solid::Capability::PortableMediaPlayer;
        else if ( capability == "networkhw" )
            return Solid::Capability::NetworkHw;
        else if ( capability == "acadapter" )
            return Solid::Capability::AcAdapter;
        else if ( capability == "battery" )
            return Solid::Capability::Battery;
        else if ( capability == "button" )
            return Solid::Capability::Button;
        else if ( capability == "display" )
            return Solid::Capability::Display;
        else if ( capability == "audiohw" )
            return Solid::Capability::AudioHw;
        else
            return Solid::Capability::Unknown;
    }

private:
    FakeDevice *m_device;
};

#endif
