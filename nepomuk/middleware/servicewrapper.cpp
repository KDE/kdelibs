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

#include "servicewrapper.h"
#include "result.h"


class Nepomuk::Middleware::ServiceWrapper::Private
{
public:
    Service* service;
    Result lastResult;
};


Nepomuk::Middleware::ServiceWrapper::ServiceWrapper( Nepomuk::Middleware::Service* s )
    : d( new Private )
{
    d->service = s;
}


Nepomuk::Middleware::ServiceWrapper::~ServiceWrapper()
{
    delete d;
}


Nepomuk::Middleware::Service* Nepomuk::Middleware::ServiceWrapper::service() const
{
    return d->service;
}


int Nepomuk::Middleware::ServiceWrapper::lastStatus() const
{
    return d->lastResult.status();
}


QString Nepomuk::Middleware::ServiceWrapper::lastErrorName() const
{
    return d->lastResult.errorName();
}


QString Nepomuk::Middleware::ServiceWrapper::lastErrorMessage() const
{
    return d->lastResult.errorMessage();
}


bool Nepomuk::Middleware::ServiceWrapper::success() const
{
    return ( lastStatus() == 0 );
}


void Nepomuk::Middleware::ServiceWrapper::setLastResult( const Nepomuk::Middleware::Result& r )
{
    d->lastResult = r;
}
