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
    inline static QString toString( Type capability )
    {
        switch( capability )
        {
            case Capability::Processor:
                return "processor";
            case Capability::Block:
                return "block";
            case Capability::Storage:
                return "storage";
            case Capability::Cdrom:
                return "cdrom";
            case Capability::Volume:
                return "volume";
            case Capability::OpticalDisc:
                return "opticaldisc";
            case Capability::Camera:
                return "camera";
            case Capability::PortableMediaPlayer:
                return "portablemediaplayer";
            case Capability::NetworkHw:
                return "networkhw";
            case Capability::AcAdapter:
                return "acadapter";
            case Capability::Battery:
                return "battery";
            case Capability::Button:
                return "button";
            case Capability::Display:
                return "display";
            case Capability::AudioHw:
                return "audiohw";
            default:
                return QString();
        }
    }

    inline static Type fromString( const QString &capability )
    {
        if ( capability == "processor" )
            return Capability::Processor;
        else if ( capability == "block" )
            return Capability::Block;
        else if ( capability == "storage" )
            return Capability::Storage;
        else if ( capability == "cdrom" )
            return Capability::Cdrom;
        else if ( capability == "volume" )
            return Capability::Volume;
        else if ( capability == "opticaldisc" )
            return Capability::OpticalDisc;
        else if ( capability == "camera" )
            return Capability::Camera;
        else if ( capability == "portablemediaplayer" )
            return Capability::PortableMediaPlayer;
        else if ( capability == "networkhw" )
            return Capability::NetworkHw;
        else if ( capability == "acadapter" )
            return Capability::AcAdapter;
        else if ( capability == "battery" )
            return Capability::Battery;
        else if ( capability == "button" )
            return Capability::Button;
        else if ( capability == "display" )
            return Capability::Display;
        else if ( capability == "audiohw" )
            return Capability::AudioHw;
        else
            return Capability::Unknown;
    }

private:
    FakeDevice *m_device;
};

#endif
