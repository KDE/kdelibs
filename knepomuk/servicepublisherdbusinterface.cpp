/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include "servicepublisherdbusinterface.h"
#include "servicepublisher.h"

static QMap<QString, Nepomuk::Backbone::DBus::ServicePublisherInterfaceFactory*> s_serviceTypeDBusInterfaceMap;

Nepomuk::Backbone::DBus::ServicePublisherInterface::ServicePublisherInterface( Nepomuk::Backbone::ServicePublisher* parent )
    : QDBusAbstractAdaptor( parent ),
      m_servicePublisher( parent )
{
}


Nepomuk::Backbone::DBus::ServicePublisherInterface::~ServicePublisherInterface()
{
}


Nepomuk::Backbone::ServicePublisher* Nepomuk::Backbone::DBus::ServicePublisherInterface::servicePublisher() const
{
    return m_servicePublisher;
}


QString Nepomuk::Backbone::DBus::ServicePublisherInterface::identificationDescriptor() const
{
    // FIXME: implement me
    return "<nepomuk:service></nepomuk:service>";
}


bool Nepomuk::Backbone::DBus::registerServiceType( const QString& type, Nepomuk::Backbone::DBus::ServicePublisherInterfaceFactory* factory )
{
    if( s_serviceTypeDBusInterfaceMap.contains( type ) )
        return false;

    s_serviceTypeDBusInterfaceMap[type] = factory;

    return true;
}


Nepomuk::Backbone::DBus::ServicePublisherInterfaceFactory*
Nepomuk::Backbone::DBus::servicePublisherInterfaceFactory( const QString& type )
{
    return s_serviceTypeDBusInterfaceMap[type];
}

#include "servicepublisherdbusinterface.moc"
