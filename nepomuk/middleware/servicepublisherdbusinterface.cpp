/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "servicepublisherdbusinterface.h"
#include "servicepublisher.h"

static QMap<QString, Nepomuk::Middleware::DBus::ServicePublisherInterfaceFactory*> s_serviceTypeDBusInterfaceMap;

Nepomuk::Middleware::DBus::ServicePublisherInterface::ServicePublisherInterface( Nepomuk::Middleware::ServicePublisher* parent )
    : QDBusAbstractAdaptor( parent ),
      m_servicePublisher( parent )
{
}


Nepomuk::Middleware::DBus::ServicePublisherInterface::~ServicePublisherInterface()
{
}


Nepomuk::Middleware::ServicePublisher* Nepomuk::Middleware::DBus::ServicePublisherInterface::servicePublisher() const
{
    return m_servicePublisher;
}


QString Nepomuk::Middleware::DBus::ServicePublisherInterface::identificationDescriptor() const
{
    // FIXME: implement me
    return "<nepomuk:service></nepomuk:service>";
}


bool Nepomuk::Middleware::DBus::registerServiceType( const QString& type, Nepomuk::Middleware::DBus::ServicePublisherInterfaceFactory* factory )
{
    if( s_serviceTypeDBusInterfaceMap.contains( type ) )
        return false;

    s_serviceTypeDBusInterfaceMap[type] = factory;

    return true;
}


Nepomuk::Middleware::DBus::ServicePublisherInterfaceFactory*
Nepomuk::Middleware::DBus::servicePublisherInterfaceFactory( const QString& type )
{
    return s_serviceTypeDBusInterfaceMap[type];
}

#include "servicepublisherdbusinterface.moc"
