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

#include "message.h"

class Nepomuk::Middleware::Message::Private : public QSharedData
{
public:
    QList<QVariant> arguments;

    QString service;
    QString method;

    bool valid;
};


Nepomuk::Middleware::Message::Message()
{
    d = new Private;
    d->valid = false;
}


Nepomuk::Middleware::Message::Message( const Message& m )
{
    d = m.d;
}


Nepomuk::Middleware::Message::Message( const QString& service, const QString& method )
{
    d = new Private;
    d->service = service;
    d->method = method;
    d->valid = true;
}


Nepomuk::Middleware::Message::~Message()
{
}


bool Nepomuk::Middleware::Message::isValid() const
{
    return d->valid;
}


QList<QVariant> Nepomuk::Middleware::Message::arguments() const
{
    return d->arguments;
}


void Nepomuk::Middleware::Message::addArgument( const QVariant& arg )
{
    d->arguments.append( arg );
}


void Nepomuk::Middleware::Message::setArguments( const QList<QVariant>& args )
{
    d->arguments = args;
}


void Nepomuk::Middleware::Message::setService( const QString& s )
{
    d->service = s;
}


void Nepomuk::Middleware::Message::setMethod( const QString& s )
{
    d->method = s;
}


const QString& Nepomuk::Middleware::Message::service() const
{
    return d->service;
}


const QString& Nepomuk::Middleware::Message::method() const
{
    return d->method;
}


const QVariant& Nepomuk::Middleware::Message::operator[]( int i ) const
{
    return d->arguments[i];
}


QVariant& Nepomuk::Middleware::Message::operator[]( int i )
{
    return d->arguments[i];
}


int Nepomuk::Middleware::Message::numArgs() const
{
    return d->arguments.count();
}


Nepomuk::Middleware::Message& Nepomuk::Middleware::Message::operator=( const Nepomuk::Middleware::Message& m )
{
    d = m.d;
    return *this;
}
