/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include "servicepublisher.h"
#include "servicedesc.h"
#include "error.h"


class Nepomuk::Backbone::ServicePublisher::Private
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


Nepomuk::Backbone::ServicePublisher::ServicePublisher( const QString& name, const QString& url, const QString& type )
    : d( new Private() )
{
    d->desc.name = name;
    d->desc.url = url;
    d->desc.type = type;
}


Nepomuk::Backbone::ServicePublisher::~ServicePublisher()
{
    delete d;
}


QString Nepomuk::Backbone::ServicePublisher::name() const
{
    return d->desc.name;
}


QString Nepomuk::Backbone::ServicePublisher::url() const
{
    return d->desc.url;
}


QString Nepomuk::Backbone::ServicePublisher::type() const
{
    return d->desc.type;
}


Nepomuk::Backbone::ServiceDesc Nepomuk::Backbone::ServicePublisher::desc() const
{
    return d->desc;
}


void Nepomuk::Backbone::ServicePublisher::setError( const QString& errorName, const QString& msg )
{
    // we cache the success for speed reasons
    d->success = ( errorName == Error::NoError );
    d->errorName = errorName;
    d->errorMsg = msg;
}


QString Nepomuk::Backbone::ServicePublisher::errorName() const
{
    return d->errorName;
}


QString Nepomuk::Backbone::ServicePublisher::errorMessage() const
{
    return d->errorMsg;
}


bool Nepomuk::Backbone::ServicePublisher::success() const
{
    return d->success;
}

#include "servicepublisher.moc"
