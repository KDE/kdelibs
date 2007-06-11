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

#include "service.h"
#include "servicedbusinterfacebackend.h"
#include "message.h"
#include "result.h"
#include "servicedesc.h"


class Nepomuk::Middleware::Service::Private
{
public:
    DBus::ServiceBackend* backend;
    ServiceDesc desc;
};



Nepomuk::Middleware::Service::Service( const ServiceDesc& desc, QObject* parent )
    : QObject( parent ),
      d( new Private() )
{
    d->backend = new DBus::ServiceBackend( desc.url, desc.type, this );
    d->desc = desc;
}


Nepomuk::Middleware::Service::~Service()
{
    delete d->backend;
    delete d;
}


const Nepomuk::Middleware::ServiceDesc& Nepomuk::Middleware::Service::desc() const
{
    return d->desc;
}


const QString& Nepomuk::Middleware::Service::type() const
{
    return d->desc.type;
}


bool Nepomuk::Middleware::Service::isAvailable() const
{
    return d->backend->isAvailable();
}


const QString& Nepomuk::Middleware::Service::url() const
{
    return d->desc.url;
}


const QString& Nepomuk::Middleware::Service::name() const
{
    return d->desc.name;
}


Nepomuk::Middleware::Result Nepomuk::Middleware::Service::methodCall( const Nepomuk::Middleware::Message& message ) const
{
    return d->backend->methodCall( message );
}

#include "service.moc"
