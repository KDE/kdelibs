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

#include "message.h"

class Nepomuk::Backbone::Message::Private : public QSharedData
{
public:
    QList<QVariant> arguments;

    QString service;
    QString method;

    bool valid;
};


Nepomuk::Backbone::Message::Message()
{
    d = new Private;
    d->valid = false;
}


Nepomuk::Backbone::Message::Message( const Message& m )
{
    d = m.d;
}


Nepomuk::Backbone::Message::Message( const QString& service, const QString& method )
{
    d = new Private;
    d->service = service;
    d->method = method;
    d->valid = true;
}


Nepomuk::Backbone::Message::~Message()
{
}


bool Nepomuk::Backbone::Message::isValid() const
{
    return d->valid;
}


QList<QVariant> Nepomuk::Backbone::Message::arguments() const
{
    return d->arguments;
}


void Nepomuk::Backbone::Message::addArgument( const QVariant& arg )
{
    d->arguments.append( arg );
}


void Nepomuk::Backbone::Message::setArguments( const QList<QVariant>& args )
{
    d->arguments = args;
}


void Nepomuk::Backbone::Message::setService( const QString& s )
{
    d->service = s;
}


void Nepomuk::Backbone::Message::setMethod( const QString& s )
{
    d->method = s;
}


const QString& Nepomuk::Backbone::Message::service() const
{
    return d->service;
}


const QString& Nepomuk::Backbone::Message::method() const
{
    return d->method;
}


const QVariant& Nepomuk::Backbone::Message::operator[]( int i ) const
{
    return d->arguments[i];
}


QVariant& Nepomuk::Backbone::Message::operator[]( int i )
{
    return d->arguments[i];
}


int Nepomuk::Backbone::Message::numArgs() const
{
    return d->arguments.count();
}


Nepomuk::Backbone::Message& Nepomuk::Backbone::Message::operator=( const Nepomuk::Backbone::Message& m )
{
    d = m.d;
    return *this;
}
