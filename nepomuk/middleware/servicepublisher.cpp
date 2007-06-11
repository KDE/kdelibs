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

#include "servicepublisher.h"
#include "servicedesc.h"
#include "error.h"


class Nepomuk::Middleware::ServicePublisher::Private
{
public:
    Private() {
        success = true;
    }

    ServiceDesc desc;

    bool success;
    QString errorName;
    QString errorMsg;
};


Nepomuk::Middleware::ServicePublisher::ServicePublisher( const QString& name, const QString& url, const QString& type )
    : d( new Private() )
{
    d->desc.name = name;
    d->desc.url = url;
    d->desc.type = type;
}


Nepomuk::Middleware::ServicePublisher::~ServicePublisher()
{
    delete d;
}


QString Nepomuk::Middleware::ServicePublisher::name() const
{
    return d->desc.name;
}


QString Nepomuk::Middleware::ServicePublisher::url() const
{
    return d->desc.url;
}


QString Nepomuk::Middleware::ServicePublisher::type() const
{
    return d->desc.type;
}


Nepomuk::Middleware::ServiceDesc Nepomuk::Middleware::ServicePublisher::desc() const
{
    return d->desc;
}


void Nepomuk::Middleware::ServicePublisher::setError( const QString& errorName, const QString& msg )
{
    // we cache the success for speed reasons
    d->success = ( errorName == Error::NoError );
    d->errorName = errorName;
    d->errorMsg = msg;
}


QString Nepomuk::Middleware::ServicePublisher::errorName() const
{
    return d->errorName;
}


QString Nepomuk::Middleware::ServicePublisher::errorMessage() const
{
    return d->errorMsg;
}


bool Nepomuk::Middleware::ServicePublisher::success() const
{
    return d->success;
}

#include "servicepublisher.moc"
