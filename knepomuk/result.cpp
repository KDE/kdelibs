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

#include "result.h"

#include <QtCore/QSharedData>

class Nepomuk::Backbone::Result::Private : public QSharedData
{
public:
    Private()
        : status(0) {
    }

    QVariant value;
    int status;

    QString errorName;
    QString errorMsg;
};

Nepomuk::Backbone::Result::Result()
{
    d = new Private;
}


Nepomuk::Backbone::Result::Result( const Nepomuk::Backbone::Result& r )
{
    d = r.d;
}


Nepomuk::Backbone::Result::Result( int status, const QVariant& value )
{
    d = new Private;
    d->value = value;
    d->status = status;
}


Nepomuk::Backbone::Result::~Result()
{
}


void Nepomuk::Backbone::Result::setValue( const QVariant& v )
{
    d->value = v;
}


void Nepomuk::Backbone::Result::setStatus( int s )
{
    d->status = s;
}


const QVariant& Nepomuk::Backbone::Result::value() const
{
    return d->value;
}


int Nepomuk::Backbone::Result::status() const
{
    return d->status;
}


QString Nepomuk::Backbone::Result::errorName() const
{
    return d->errorName;
}


QString Nepomuk::Backbone::Result::errorMessage() const
{
    return d->errorMsg;
}


Nepomuk::Backbone::Result& Nepomuk::Backbone::Result::operator=( const Nepomuk::Backbone::Result& r )
{
    d = r.d;
    return *this;
}


Nepomuk::Backbone::Result Nepomuk::Backbone::Result::createSimpleResult( const QVariant& value )
{
    return Result( 0, value );
}


Nepomuk::Backbone::Result Nepomuk::Backbone::Result::createErrorResult( int status, const QString& name, const QString& message )
{
    Result r( status );
    r.d->errorName = name;
    r.d->errorMsg = message;
    return r;
}
