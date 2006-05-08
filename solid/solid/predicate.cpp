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

#include "predicate.h"

#include <kdehw/ifaces/device.h>
#include <QStringList>

namespace KDEHW
{
    class Predicate::Private
    {
    public:
        enum OperatorType { AtomType, AndType, OrType };

        Private() : isValid( false ), type( AtomType ),
                    operand1( 0 ), operand2( 0 ) {}

        bool isValid;
        OperatorType type;

        Capability::Type capability;
        QString property;
        QVariant value;

        Predicate *operand1;
        Predicate *operand2;
    };
}


KDEHW::Predicate::Predicate()
    : d( new Private() )
{
}

KDEHW::Predicate::Predicate( const Predicate &other )
    : d( new Private() )
{
    *this = other;
}

KDEHW::Predicate::Predicate( const Capability::Type &capability,
                             const QString &property, const QVariant &value )
    : d( new Private() )
{
    d->isValid = true;
    d->capability = capability;
    d->property = property;
    d->value = value;
}

KDEHW::Predicate::Predicate( const QString &capability,
                             const QString &property, const QVariant &value )
    : d( new Private() )
{
    QMap<QString, Capability::Type> map;
    map["Processor"] = Capability::Processor;
    map["Block"] = Capability::Block;
    map["Storage"] = Capability::Storage;
    map["Cdrom"] = Capability::Cdrom;
    map["Volume"] = Capability::Volume;
    map["OpticalDisc"] = Capability::OpticalDisc;
    map["Camera"] = Capability::Camera;
    map["PortableMediaPlayer"] = Capability::PortableMediaPlayer;
    map["NetworkIface"] = Capability::NetworkIface;
    map["Display"] = Capability::Display;

    if ( map.contains( capability ) )
    {
        d->isValid = true;
        d->capability = map[capability];
        d->property = property;
        d->value = value;
    }
}

KDEHW::Predicate::~Predicate()
{
    if ( d->type!=Private::AtomType )
    {
        delete d->operand1;
        delete d->operand2;
    }

    delete d;
}

KDEHW::Predicate &KDEHW::Predicate::operator=( const Predicate &other )
{
    d->isValid = other.d->isValid;
    d->type = other.d->type;

    if ( d->type!=Private::AtomType )
    {
        d->operand1 = new Predicate( *( other.d->operand1 ) );
        d->operand2 = new Predicate( *( other.d->operand2 ) );
    }
    else
    {
        d->capability = other.d->capability;
        d->property = other.d->property;
        d->value = other.d->value;
    }

    return *this;
}

KDEHW::Predicate KDEHW::Predicate::operator&( const Predicate &other )
{
    Predicate result;

    result.d->isValid = true;
    result.d->type = Private::AndType;
    result.d->operand1 = new Predicate( *this );
    result.d->operand2 = new Predicate( other );

    return result;
}

KDEHW::Predicate KDEHW::Predicate::operator|( const Predicate &other )
{
    Predicate result;

    result.d->isValid = true;
    result.d->type = Private::OrType;
    result.d->operand1 = new Predicate( *this );
    result.d->operand2 = new Predicate( other );

    return result;
}

bool KDEHW::Predicate::isValid() const
{
    return d->isValid;
}

bool KDEHW::Predicate::matches( Ifaces::Device *device ) const
{
    if ( !d->isValid ) return false;

    switch( d->type )
    {
    case Private::OrType:
        return d->operand1->matches( device )
            || d->operand2->matches( device );
    case Private::AndType:
        return d->operand1->matches( device )
            && d->operand2->matches( device );
    case Private::AtomType:
    {
        Ifaces::Capability *iface = device->asCapability( d->capability );

        if ( iface!=0 && ( iface->qobject()!=0 ) )
        {
            QVariant value = iface->qobject()->property( d->property.toLatin1() );
            return ( value == d->value );
        }
        break;
    }
    }

    return false;
}

QString KDEHW::Predicate::toString() const
{
    if ( !d->isValid ) return "False";

    if ( d->type!=Private::AtomType )
    {
        QString op = " AND ";
        if ( d->type==Private::OrType ) op = " OR ";

        return "[ "+d->operand1->toString()+op+d->operand2->toString()+" ]";
    }
    else
    {
        QString capability = "Unknown";

        switch( d->capability )
        {
        case Capability::Processor:
            capability = "Processor";
            break;
        case Capability::Block:
            capability = "Block";
            break;
        case Capability::Storage:
            capability = "Storage";
            break;
        case Capability::Cdrom:
            capability = "Cdrom";
            break;
        case Capability::Volume:
            capability = "Volume";
            break;
        case Capability::OpticalDisc:
            capability = "OpticalDisc";
            break;
        case Capability::Camera:
            capability = "Camera";
            break;
        case Capability::PortableMediaPlayer:
            capability = "PortableMediaPlayer";
            break;
        case Capability::NetworkIface:
            capability = "NetworkIface";
            break;
        case Capability::Display:
            capability = "Display";
            break;
        case Capability::Unknown:
            break;
        }

        QString value;

        switch ( d->value.type() )
        {
        case QVariant::StringList:
        {
            value = "{";

            QStringList list = d->value.toStringList();

            QStringList::ConstIterator it = list.begin();
            QStringList::ConstIterator end = list.end();

            for ( ; it!=end; ++it )
            {
                value+= "'"+*it+"'";

                if ( it+1!=end )
                {
                    value+= ", ";
                }
            }

            value+= "}";
            break;
        }
        case QVariant::Bool:
            value = ( d->value.toBool()?"true":"false" );
            break;
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::LongLong:
        case QVariant::ULongLong:
            value = d->value.toString();
            break;
        default:
            value = "'"+d->value.toString()+"'";
            break;
        }

        return capability+"."+d->property+" == "+value;
    }
}




